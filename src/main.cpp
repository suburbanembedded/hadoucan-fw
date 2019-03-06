#include "main.h"
#include "cmsis_os.h"
#include "usb_device.h"
#include "usbd_cdc_if.h"

#include "uart1_printf.hpp"

#include "USB_RX_task.hpp"
#include "USB_TX_task.hpp"
#include "USB_rx_buffer_task.hpp"
#include "USB_tx_buffer_task.hpp"

#include "lawicel/Lawicel_parser_stm32.hpp"
#include "STM32_fdcan_rx.hpp"
#include "STM32_fdcan_tx.hpp"

#include "freertos_cpp_util/Task_static.hpp"
#include "freertos_cpp_util/BSema_static.hpp"
#include "freertos_cpp_util/Mutex_static.hpp"
#include "freertos_cpp_util/Condition_variable.hpp"
#include "freertos_cpp_util/object_pool/Object_pool.hpp"

#include "common_util/Byte_util.hpp"

#include "W25Q16JV.hpp"
#include "W25Q16JV_conf_region.hpp"

#include "../external/tinyxml2/tinyxml2.h"

#include <array>
#include <vector>
#include <algorithm>
#include <cstdio>
#include <cinttypes>

USB_RX_task usb_rx_task __attribute__ (( section(".ram_dtcm_noload") ));
USB_TX_task usb_tx_task __attribute__ (( section(".ram_dtcm_noload") ));

USB_rx_buffer_task usb_rx_buffer_task __attribute__ (( section(".ram_dtcm_noload") ));
USB_tx_buffer_task usb_tx_buffer_task __attribute__ (( section(".ram_dtcm_noload") ));

class USB_lawicel_task : public Task_static<1024>
{
public:

	USB_lawicel_task()
	{
		m_usb_tx_buffer = nullptr;
	}

	static bool usb_input_drop(uint8_t c)
	{
		switch(c)
		{
			// case '\r':
			// {
			// 	return true;
			// }
			case '\n':
			{
				return true;
			}
			default:
			{
				return false;
			}
		}

		return false;
	}

	void set_usb_tx(USB_tx_buffer_task* const usb_tx_buffer)
	{
		m_usb_tx_buffer = usb_tx_buffer;
	}

	bool write_string_usb(const char* str)
	{
		m_usb_tx_buffer->write(str);
		return true;
	}

	void work() override
	{
		m_can.set_can_instance(FDCAN1);
		m_can.set_can_handle(&hfdcan1);

		m_parser.set_can(&m_can);
		m_parser.set_write_string_func(
			std::bind(&USB_lawicel_task::write_string_usb, this, std::placeholders::_1)
			);

		std::function<bool(void)> has_line_pred = std::bind(&USB_rx_buffer_task::has_line, &usb_rx_buffer_task);

		//a null terminated string
		//maybe using std::string is a better idea, or a stringstream....
		std::vector<uint8_t> usb_line;
		usb_line.reserve(128);

		for(;;)
		{
			{
				uart1_log<64>(LOG_LEVEL::TRACE, "USB_lawicel_task", "wait(lock, has_line_pred)");
				std::unique_lock<Mutex_static> lock(usb_rx_buffer_task.get_mutex());
				usb_rx_buffer_task.get_cv().wait(lock, std::cref(has_line_pred));
				uart1_log<64>(LOG_LEVEL::TRACE, "USB_lawicel_task", "woke");

				if(!usb_rx_buffer_task.get_line(&usb_line))
				{
					continue;
				}
			}
			//we unlock lock so buffering can continue

			//drop what usb_input_drop says we should drop
			auto end_it = std::remove_if(usb_line.begin(), usb_line.end(), &usb_input_drop);
			usb_line.erase(end_it, usb_line.end());

			//drop lines that are now empty
			if(strnlen((char*)usb_line.data(), usb_line.size()) == 0)
			{
				uart1_log<64>(LOG_LEVEL::WARN, "USB_lawicel_task", "Empty line");
				continue;
			}

			//drop lines that are only '\r'
			if(usb_line.front() == '\r')
			{
				uart1_log<64>(LOG_LEVEL::WARN, "USB_lawicel_task", "Line only contains \\r");
				continue;
			}

			uart1_log<64>(LOG_LEVEL::TRACE, "USB_lawicel_task", "got line: [%s]", usb_line.data());

			//process line
			if(!m_parser.parse_string((char*)usb_line.data()))
			{
				uart1_log<64>(LOG_LEVEL::ERROR, "USB_lawicel_task", "parse error");
			}
			else
			{
				uart1_log<64>(LOG_LEVEL::TRACE, "USB_lawicel_task", "ok");
			}
		}
	}

	STM32_fdcan_tx m_can;

	Lawicel_parser* get_lawicel()
	{
		return &m_parser;
	}

protected:

	Lawicel_parser_stm32 m_parser;

	USB_tx_buffer_task* m_usb_tx_buffer;
};
USB_lawicel_task usb_lawicel_task __attribute__ (( section(".ram_dtcm_noload") ));

bool can_rx_to_lawicel(const std::string& str)
{
	return usb_lawicel_task.get_lawicel()->queue_rx_packet(str);
}

class LED_task : public Task_static<512>
{
public:

	void work() override
	{
		for(;;)
		{
			HAL_GPIO_WritePin(GPIOD, RED1_Pin, GPIO_PIN_SET);
			vTaskDelay(250);
			HAL_GPIO_WritePin(GPIOD, RED1_Pin, GPIO_PIN_RESET);
			vTaskDelay(250);
			HAL_GPIO_WritePin(GPIOD, RED1_Pin, GPIO_PIN_SET);

			HAL_GPIO_WritePin(GPIOD, GREEN1_Pin, GPIO_PIN_SET);
			vTaskDelay(250);
			HAL_GPIO_WritePin(GPIOD, GREEN1_Pin, GPIO_PIN_RESET);
			vTaskDelay(250);
			HAL_GPIO_WritePin(GPIOD, GREEN1_Pin, GPIO_PIN_SET);

			HAL_GPIO_WritePin(GPIOD, RED2_Pin, GPIO_PIN_SET);
			vTaskDelay(250);
			HAL_GPIO_WritePin(GPIOD, RED2_Pin, GPIO_PIN_RESET);
			vTaskDelay(250);
			HAL_GPIO_WritePin(GPIOD, RED2_Pin, GPIO_PIN_SET);

			HAL_GPIO_WritePin(GPIOD, GREEN2_Pin, GPIO_PIN_SET);
			vTaskDelay(250);
			HAL_GPIO_WritePin(GPIOD, GREEN2_Pin, GPIO_PIN_RESET);
			vTaskDelay(250);
			HAL_GPIO_WritePin(GPIOD, GREEN2_Pin, GPIO_PIN_SET);
		}
	}

};
LED_task led_task __attribute__ (( section(".ram_d2_s2_noload") ));

class Timesync_task : public Task_static<512>
{
public:

	void work() override
	{
		if(!ic_config())
		{
			uart1_log<64>(LOG_LEVEL::ERROR, "Timesync_task", "ic_config failed");
		}
		
		for(;;)
		{
			vTaskSuspend(nullptr);
		}
	}

	bool oc_config()
	{
		// 100 MHz input clock
		// 2000 prescaler -> 20us per tick
		// 50000 counts -> 1s overflow

		htim3.Instance = TIM3;
		htim3.Init.Prescaler = 2000;
		htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
		htim3.Init.Period = 50000;
		htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
		htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
		if(HAL_TIM_Base_Init(&htim3) != HAL_OK)
		{
			return false;
		}

		TIM_ClockConfigTypeDef sClockSourceConfig = {0};
		sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
		if(HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
		{
			return false;
		}

		TIM_MasterConfigTypeDef sMasterConfig = {0};
		sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
		sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
		if(HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
		{
			return false;
		}

		HAL_TIM_Base_Start(&htim3);

		if(HAL_TIM_PWM_Init(&htim3) != HAL_OK)
		{
			return false;
		}

		TIM_OC_InitTypeDef sConfigOC = {0};
		sConfigOC.OCMode = TIM_OCMODE_PWM1;
		sConfigOC.Pulse = 5;
		sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
		sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
		if(HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
		{
			return false;
		}

		HAL_GPIO_WritePin(MASTER_TIMESYNC_nOE_GPIO_Port, MASTER_TIMESYNC_nOE_Pin, GPIO_PIN_RESET);

		HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);

		return true;
	}

	bool ic_config()
	{
		// 100 MHz input clock
		// 50000 prescaler -> 500us per tick
		// 50000 counts -> 25s overflow

		htim3.Instance = TIM3;
		htim3.Init.Prescaler = 50000;
		htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
		htim3.Init.Period = 50000;
		htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
		htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
		if(HAL_TIM_Base_Init(&htim3) != HAL_OK)
		{
			return false;
		}

		TIM_ClockConfigTypeDef sClockSourceConfig = {0};
		sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
		if(HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
		{
			return false;
		}

		if(HAL_TIM_IC_Init(&htim3) != HAL_OK)
		{
			return false;
		}

		TIM_SlaveConfigTypeDef sSlaveConfig = {0};
		sSlaveConfig.SlaveMode = TIM_SLAVEMODE_RESET;
		sSlaveConfig.InputTrigger = TIM_TS_TI1FP1;
		sSlaveConfig.TriggerPolarity = TIM_INPUTCHANNELPOLARITY_RISING;
		sSlaveConfig.TriggerFilter = 0;
		if(HAL_TIM_SlaveConfigSynchronization(&htim3, &sSlaveConfig) != HAL_OK)
		{
			return false;
		}

		TIM_MasterConfigTypeDef sMasterConfig = {0};
		sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
		sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
		if(HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
		{
			return false;
		}

		TIM_IC_InitTypeDef sConfigIC = {0};
		sConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_RISING;
		sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;
		sConfigIC.ICPrescaler = TIM_ICPSC_DIV1;
		sConfigIC.ICFilter = 0;
		if(HAL_TIM_IC_ConfigChannel(&htim3, &sConfigIC, TIM_CHANNEL_1) != HAL_OK)
		{
			return false;
		}

		HAL_GPIO_WritePin(MASTER_TIMESYNC_nOE_GPIO_Port, MASTER_TIMESYNC_nOE_Pin, GPIO_PIN_SET);

		HAL_TIM_Base_Start(&htim3);

		HAL_TIM_IC_Start(&htim3, TIM_CHANNEL_1);

		return true;
	}
};
Timesync_task timesync_task __attribute__ (( section(".ram_d2_s2_noload") ));
/*
class TinyXML_inc_printer : public tinyxml2::XMLVisitor
{
public:
	TinyXML_inc_printer()
	{
		indent_level = 0;
	}

	bool VisitEnter(const tinyxml2::XMLDocument& doc) override
	{
		indent_level = 0;
		return true;
	}
	bool VisitExit(const tinyxml2::XMLDocument& doc) override
	{
		return true;
	}
	bool VisitEnter(const tinyxml2::XMLElement& ele, const tinyxml2::XMLAttribute* attr) override
	{
		print_indent();

		uart1_printf<64>("<%s", ele.Name());
		if(attr)
		{
			tinyxml2::XMLAttribute const* node = attr;
			
			do
			{
				uart1_printf<64>(" %s=\"%s\"", node->Name(), node->Value());
				node = node->Next();
			} while(node);
		}

		if(ele.NoChildren())
		{
			uart1_printf<64>("/>\n");
		}
		else
		{
			uart1_printf<64>(">");

			if(ele.FirstChild()->ToText() == nullptr)
			{
				indent_level++;

				uart1_printf<64>("\n");
			}
		}

		return true;
	}
	bool VisitExit(const tinyxml2::XMLElement& ele) override
	{
		if(ele.NoChildren())
		{
			return true;
		}

		if(ele.Parent() != nullptr)
		{
			if(ele.FirstChild()->ToText() == nullptr)
			{
				indent_level--;
				print_indent();
				uart1_printf<64>("</%s>\n", ele.Name());
			}
			else
			{
				uart1_printf<64>("</%s>\n", ele.Name());
			}
		}
		else
		{
			print_indent();
			uart1_printf<64>("</%s>\n", ele.Name());	
		}

		return true;
	}
	bool Visit(const tinyxml2::XMLDeclaration& decl) override
	{
		print_indent();
		uart1_printf<64>("<?%s?>\n", decl.Value());
		return true;
	}
	bool Visit(const tinyxml2::XMLText& text) override
	{
		uart1_printf<64>("%s", text.Value());
		return true;
	}
	bool Visit(const tinyxml2::XMLComment& com) override
	{
		print_indent();
		uart1_printf<64>("<!--%s-->\n", com.Value());
		return true;
	}
	bool Visit(const tinyxml2::XMLUnknown& unk) override
	{
		return true;
	}
protected:

	void print_indent()
	{
		for(size_t i = 0 ; i < indent_level; i++)
		{
			uart1_printf<64>("\t");
		}
	}

	size_t indent_level;
};
*/
class QSPI_task : public Task_static<2048>
{
public:

	void work() override
	{
		uart1_log<64>(LOG_LEVEL::INFO, "qspi", "Ready");

		m_qspi.set_handle(&hqspi);

		if(!m_qspi.init())
		{
			uart1_log<64>(LOG_LEVEL::ERROR, "qspi", "m_qspi.init failed");

			for(;;)
			{
				vTaskSuspend(nullptr);
			}
		}

		m_fs.initialize();
		m_fs.set_flash(&m_qspi);

		uint8_t mfg_id = 0;
		uint16_t flash_pn = 0;
		if(m_qspi.get_jdec_id(&mfg_id, &flash_pn))
		{
			uart1_log<128>(LOG_LEVEL::INFO, "qspi", "mfg id %02" PRIX32, uint32_t(mfg_id));
			uart1_log<128>(LOG_LEVEL::INFO, "qspi", "flash pn %04" PRIX32, uint32_t(flash_pn));
		}
		else
		{
			uart1_log<64>(LOG_LEVEL::ERROR, "qspi", "get_jdec_id failed");
		}

		uint64_t unique_id = 0;
		if(m_qspi.get_unique_id(&unique_id))
		{
			// uart1_log<128>(LOG_LEVEL::INFO, "qspi", "flash sn %016" PRIX64, unique_id);
			//aparently PRIX64 is broken
			uart1_log<128>(LOG_LEVEL::INFO, "qspi", "flash sn %08" PRIX32 "%08" PRIX32, Byte_util::get_upper_half(unique_id), Byte_util::get_lower_half(unique_id));
		}
		else
		{
			uart1_log<64>(LOG_LEVEL::ERROR, "qspi", "get_unique_id failed");
		}

		uart1_log<64>(LOG_LEVEL::INFO, "qspi", "Mounting flash fs");
		int mount_ret = m_fs.mount();
		if(mount_ret != SPIFFS_OK)
		{
			uart1_log<128>(LOG_LEVEL::ERROR, "qspi", "Flash mount failed: %d", mount_ret);
			uart1_log<128>(LOG_LEVEL::ERROR, "qspi", "You will need to reload the config");

			uart1_log<128>(LOG_LEVEL::INFO, "qspi", "Format flash");
			int format_ret = m_fs.format();
			if(format_ret != SPIFFS_OK)
			{
				uart1_log<128>(LOG_LEVEL::ERROR, "qspi", "Flash format failed: %d", format_ret);
				uart1_log<128>(LOG_LEVEL::ERROR, "qspi", "Try a power cycle, your board may be broken");
				for(;;)
				{
					vTaskSuspend(nullptr);
				}
			}

			uart1_log<64>(LOG_LEVEL::INFO, "qspi", "Mounting flash fs");
			mount_ret = m_fs.mount();
			if(mount_ret != SPIFFS_OK)
			{
				uart1_log<128>(LOG_LEVEL::ERROR, "qspi", "Flash mount failed right after we formatted it: %d", mount_ret);
				uart1_log<128>(LOG_LEVEL::ERROR, "qspi", "Try a power cycle, your board may be broken");
				for(;;)
				{
					vTaskSuspend(nullptr);
				}
			}
		}
		uart1_log<64>(LOG_LEVEL::INFO, "qspi", "Flash mount ok");

		// write_default_config();
		// write_default_bitrate_table();

		for(;;)
		{
			vTaskSuspend(nullptr);
		}
	}

	bool load_config()
	{
		return false;
	}
	bool write_default_config()
	{
		tinyxml2::XMLDocument config_doc;

		{
			tinyxml2::XMLDeclaration* decl = config_doc.NewDeclaration("xml version=\"1.0\" standalone=\"yes\" encoding=\"UTF-8\"");
			config_doc.InsertFirstChild(decl);
		}

		tinyxml2::XMLElement* config_doc_root = config_doc.NewElement("config");
		config_doc.InsertEndChild(config_doc_root);

		//General Config Settings
		tinyxml2::XMLElement* node = config_doc.NewElement("autopoll");
		node->SetText(false);
		config_doc_root->InsertEndChild(node);

		node = config_doc.NewElement("listen_only");
		node->SetText(false);
		config_doc_root->InsertEndChild(node);

		node = config_doc.NewElement("timesync");
		node->SetText("slave");
		config_doc_root->InsertEndChild(node);

		{
			tinyxml2::XMLElement* timestamp = config_doc.NewElement("timestamp");
			config_doc_root->InsertEndChild(timestamp);

			node = config_doc.NewElement("enable");
			node->SetText(true);
			timestamp->InsertEndChild(node);

			node = config_doc.NewElement("prescaler");
			node->SetText(2000);
			timestamp->InsertEndChild(node);

			node = config_doc.NewElement("period");
			node->SetText(50000);
			timestamp->InsertEndChild(node);
		}

		{
			tinyxml2::XMLComment* comment = config_doc.NewComment("clock may only be 24000000 or 60000000");
			config_doc_root->InsertEndChild(comment);

			node = config_doc.NewElement("clock");
			node->SetText(60000000U);
			config_doc_root->InsertEndChild(node);
		}

		{
			tinyxml2::XMLElement* bitrate = config_doc.NewElement("bitrate");
			config_doc_root->InsertEndChild(bitrate);

			node = config_doc.NewElement("nominal");
			node->SetText(500000);
			bitrate->InsertEndChild(node);

			node = config_doc.NewElement("data");
			node->SetText(4000000);
			bitrate->InsertEndChild(node);
		}
		
		{
			tinyxml2::XMLElement* protocol = config_doc.NewElement("protocol");
			config_doc_root->InsertEndChild(protocol);

			node = config_doc.NewElement("extended_id");
			node->SetText(true);
			protocol->InsertEndChild(node);

			node = config_doc.NewElement("fd");
			node->SetText(true);
			protocol->InsertEndChild(node);

			node = config_doc.NewElement("brs");
			node->SetText(false);
			protocol->InsertEndChild(node);
		}

		{
			tinyxml2::XMLElement* filter = config_doc.NewElement("filter");
			config_doc_root->InsertEndChild(filter);

			node = config_doc.NewElement("accept_code");
			node->SetText("00000000");
			filter->InsertEndChild(node);

			node = config_doc.NewElement("accept_mask");
			node->SetText("FFFFFFFF");
			filter->InsertEndChild(node);
		}

		{
			tinyxml2::XMLElement* debug = config_doc.NewElement("debug");
			config_doc_root->InsertEndChild(debug);

			tinyxml2::XMLComment* comment = config_doc.NewComment("log_level may be TRACE, DEBUG, INFO, WARN, ERROR, FATAL");
			debug->InsertEndChild(comment);

			node = config_doc.NewElement("log_level");
			node->SetText("DEBUG");
			debug->InsertEndChild(node);

			node = config_doc.NewElement("baud");
			node->SetText(115200U);
			debug->InsertEndChild(node);
		}

		//mem, compact, fulldepth
		// tinyxml2::XMLPrinter xml_printer(nullptr, true, 0);
		tinyxml2::XMLPrinter xml_printer(nullptr, false, 0);
		config_doc.Print(&xml_printer);

		const char* doc_str = xml_printer.CStr();
		int doc_str_len = xml_printer.CStrSize() - 1;

		for(size_t i = 0; i < (doc_str_len); i++)
		{
			uart1_printf<16>("%c", doc_str[i]);
		}

		uart1_log<128>(LOG_LEVEL::INFO, "qspi", "Writing config.xml");
		spiffs_file fd = SPIFFS_open(m_fs.get_fs(), "config.xml", SPIFFS_CREAT | SPIFFS_TRUNC | SPIFFS_RDWR, 0);
		if(fd < 0)
		{
			uart1_log<128>(LOG_LEVEL::ERROR, "qspi", "Opening config.xml failed: %" PRId32, SPIFFS_errno(m_fs.get_fs()));
			return false;
		}

		if(SPIFFS_write(m_fs.get_fs(), fd, const_cast<char*>(doc_str), doc_str_len) < 0)
		{
			uart1_log<128>(LOG_LEVEL::ERROR, "qspi", "Writing config.xml failed: %" PRId32, SPIFFS_errno(m_fs.get_fs()));
			return false;
		}

		if(SPIFFS_close(m_fs.get_fs(), fd) < 0)
		{
			uart1_log<128>(LOG_LEVEL::ERROR, "qspi", "Closing config.xml failed: %" PRId32, SPIFFS_errno(m_fs.get_fs()));
			return false;
		}

		return true;
	}
	bool write_default_bitrate_table()
	{
		tinyxml2::XMLDocument table_doc;

		{
			tinyxml2::XMLDeclaration* decl = table_doc.NewDeclaration("xml version=\"1.0\" standalone=\"yes\" encoding=\"UTF-8\"");
			table_doc.InsertFirstChild(decl);
		}

		tinyxml2::XMLElement* table_doc_root = table_doc.NewElement("bitrate_tables");
		table_doc.InsertEndChild(table_doc_root);

		//Bit rate table
		{
			tinyxml2::XMLElement* table = table_doc.NewElement("table");
			table->SetAttribute("clock", 24000000U);
			table_doc_root->InsertEndChild(table);

			tinyxml2::XMLElement* entry = table_doc.NewElement("entry");
			entry->SetAttribute("type",  "nominal");
			entry->SetAttribute("rate",  5000U);
			entry->SetAttribute("pre",   192U);
			entry->SetAttribute("tseg1", 16U);
			entry->SetAttribute("tseg2", 8U);
			entry->SetAttribute("sjw",   2U);
			table->InsertEndChild(entry);

			entry = table_doc.NewElement("entry");
			entry->SetAttribute("type",  "nominal");
			entry->SetAttribute("rate",  10000U);
			entry->SetAttribute("pre",   120U);
			entry->SetAttribute("tseg1", 16U);
			entry->SetAttribute("tseg2", 3U);
			entry->SetAttribute("sjw",   2U);
			table->InsertEndChild(entry);

			entry = table_doc.NewElement("entry");
			entry->SetAttribute("type",  "nominal");
			entry->SetAttribute("rate",  20000U);
			entry->SetAttribute("pre",   60U);
			entry->SetAttribute("tseg1", 16U);
			entry->SetAttribute("tseg2", 3U);
			entry->SetAttribute("sjw",   2U);
			table->InsertEndChild(entry);

			entry = table_doc.NewElement("entry");
			entry->SetAttribute("type",  "nominal");
			entry->SetAttribute("rate",  50000U);
			entry->SetAttribute("pre",   24U);
			entry->SetAttribute("tseg1", 16U);
			entry->SetAttribute("tseg2", 3U);
			entry->SetAttribute("sjw",   2U);
			table->InsertEndChild(entry);

			entry = table_doc.NewElement("entry");
			entry->SetAttribute("type",  "nominal");
			entry->SetAttribute("rate",  100000U);
			entry->SetAttribute("pre",   12U);
			entry->SetAttribute("tseg1", 16U);
			entry->SetAttribute("tseg2", 3U);
			entry->SetAttribute("sjw",   2U);
			table->InsertEndChild(entry);

			entry = table_doc.NewElement("entry");
			entry->SetAttribute("type",  "nominal");
			entry->SetAttribute("rate",  125000U);
			entry->SetAttribute("pre",   12U);
			entry->SetAttribute("tseg1", 13U);
			entry->SetAttribute("tseg2", 3U);
			entry->SetAttribute("sjw",   1U);
			table->InsertEndChild(entry);

			entry = table_doc.NewElement("entry");
			entry->SetAttribute("type",  "nominal");
			entry->SetAttribute("rate",  250000U);
			entry->SetAttribute("pre",   6U);
			entry->SetAttribute("tseg1", 12U);
			entry->SetAttribute("tseg2", 2U);
			entry->SetAttribute("sjw",   1U);
			table->InsertEndChild(entry);

			entry = table_doc.NewElement("entry");
			entry->SetAttribute("type",  "nominal");
			entry->SetAttribute("rate",  500000U);
			entry->SetAttribute("pre",   3U);
			entry->SetAttribute("tseg1", 13U);
			entry->SetAttribute("tseg2", 2U);
			entry->SetAttribute("sjw",   1U);
			table->InsertEndChild(entry);

			entry = table_doc.NewElement("entry");
			entry->SetAttribute("type",  "nominal");
			entry->SetAttribute("rate",  800000U);
			entry->SetAttribute("pre",   3U);
			entry->SetAttribute("tseg1", 7U);
			entry->SetAttribute("tseg2", 2U);
			entry->SetAttribute("sjw",   1U);
			table->InsertEndChild(entry);

			entry = table_doc.NewElement("entry");
			entry->SetAttribute("type",  "nominal");
			entry->SetAttribute("rate",  1000000U);
			entry->SetAttribute("pre",   3U);
			entry->SetAttribute("tseg1", 5U);
			entry->SetAttribute("tseg2", 2U);
			entry->SetAttribute("sjw",   1U);
			table->InsertEndChild(entry);

			entry = table_doc.NewElement("entry");
			entry->SetAttribute("type",  "data");
			entry->SetAttribute("rate",  2000000U);
			entry->SetAttribute("pre",   2U);
			entry->SetAttribute("tseg1", 4U);
			entry->SetAttribute("tseg2", 1U);
			entry->SetAttribute("sjw",   1U);
			table->InsertEndChild(entry);

			entry = table_doc.NewElement("entry");
			entry->SetAttribute("type",  "data");
			entry->SetAttribute("rate",  6000000U);
			entry->SetAttribute("pre",   1U);
			entry->SetAttribute("tseg1", 1U);
			entry->SetAttribute("tseg2", 2U);
			entry->SetAttribute("sjw",   1U);
			table->InsertEndChild(entry);
		}

		{
			tinyxml2::XMLElement* table = table_doc.NewElement("table");
			table->SetAttribute("clock", 60000000U);
			table_doc_root->InsertEndChild(table);

			tinyxml2::XMLElement* entry = table_doc.NewElement("entry");
			entry->SetAttribute("type",  "nominal");
			entry->SetAttribute("rate",  250000U);
			entry->SetAttribute("pre",   24U);
			entry->SetAttribute("tseg1", 7U);
			entry->SetAttribute("tseg2", 2U);
			entry->SetAttribute("sjw",   1U);
			table->InsertEndChild(entry);

			entry = table_doc.NewElement("entry");
			entry->SetAttribute("type",  "nominal");
			entry->SetAttribute("rate",  500000U);
			entry->SetAttribute("pre",   12U);
			entry->SetAttribute("tseg1", 7U);
			entry->SetAttribute("tseg2", 2U);
			entry->SetAttribute("sjw",   1U);
			table->InsertEndChild(entry);

			entry = table_doc.NewElement("entry");
			entry->SetAttribute("type",  "nominal");
			entry->SetAttribute("rate",  1000000U);
			entry->SetAttribute("pre",   5U);
			entry->SetAttribute("tseg1", 8U);
			entry->SetAttribute("tseg2", 3U);
			entry->SetAttribute("sjw",   1U);
			table->InsertEndChild(entry);

			entry = table_doc.NewElement("entry");
			entry->SetAttribute("type",  "data");
			entry->SetAttribute("rate",  2000000U);
			entry->SetAttribute("pre",   3U);
			entry->SetAttribute("tseg1", 7U);
			entry->SetAttribute("tseg2", 2U);
			entry->SetAttribute("sjw",   1U);
			table->InsertEndChild(entry);

			entry = table_doc.NewElement("entry");
			entry->SetAttribute("type",  "data");
			entry->SetAttribute("rate",  4000000U);
			entry->SetAttribute("pre",   3U);
			entry->SetAttribute("tseg1", 3U);
			entry->SetAttribute("tseg2", 1U);
			entry->SetAttribute("sjw",   1U);
			table->InsertEndChild(entry);

			entry = table_doc.NewElement("entry");
			entry->SetAttribute("type",  "data");
			entry->SetAttribute("rate",  6000000U);
			entry->SetAttribute("pre",   2U);
			entry->SetAttribute("tseg1", 3U);
			entry->SetAttribute("tseg2", 1U);
			entry->SetAttribute("sjw",   1U);
			table->InsertEndChild(entry);

			entry = table_doc.NewElement("entry");
			entry->SetAttribute("type",  "data");
			entry->SetAttribute("rate",  10000000U);
			entry->SetAttribute("pre",   1U);
			entry->SetAttribute("tseg1", 3U);
			entry->SetAttribute("tseg2", 2U);
			entry->SetAttribute("sjw",   1U);
			table->InsertEndChild(entry);

			entry = table_doc.NewElement("entry");
			entry->SetAttribute("type",  "data");
			entry->SetAttribute("rate",  12000000U);
			entry->SetAttribute("pre",   1U);
			entry->SetAttribute("tseg1", 2U);
			entry->SetAttribute("tseg2", 2U);
			entry->SetAttribute("sjw",   1U);
			table->InsertEndChild(entry);
		}

		//mem, compact, fulldepth
		// tinyxml2::XMLPrinter xml_printer(nullptr, true, 0);
		tinyxml2::XMLPrinter xml_printer(nullptr, false, 0);
		table_doc.Print(&xml_printer);

		const char* doc_str = xml_printer.CStr();
		int doc_str_len = xml_printer.CStrSize() - 1;

		for(size_t i = 0; i < (doc_str_len); i++)
		{
			uart1_printf<16>("%c", doc_str[i]);
		}

		return true;
	}
protected:

	W25Q16JV m_qspi;
	W25Q16JV_conf_region m_fs;

};
QSPI_task qspi_task __attribute__ (( section(".ram_d2_s2_noload") ));

class Main_task : public Task_static<512>
{
public:
	void work() override
	{
		//init
		usb_rx_buffer_task.set_usb_rx(&usb_rx_task);
		usb_tx_buffer_task.set_usb_tx(&usb_tx_task);
		usb_lawicel_task.set_usb_tx(&usb_tx_buffer_task);

		//TODO: refactor can handle init
		hfdcan1.Instance = FDCAN1;
		stm32_fdcan_rx_task.set_packet_callback(&can_rx_to_lawicel);
		stm32_fdcan_rx_task.set_can_instance(FDCAN1);
		stm32_fdcan_rx_task.set_can_handle(&hfdcan1);

		//can RX
		stm32_fdcan_rx_task.launch("stm32_fdcan_rx", 1);

		//protocol state machine
		usb_lawicel_task.launch("usb_lawicel", 2);

		//process usb packets
		usb_rx_buffer_task.launch("usb_rx_buf", 4);
		usb_tx_buffer_task.launch("usb_tx_buf", 5);

		//actually send usb packets on the wire
		usb_rx_task.launch("usb_rx", 3);
		usb_tx_task.launch("usb_tx", 4);

		led_task.launch("led", 1);
		qspi_task.launch("qspi", 1);
		timesync_task.launch("timesync", 1);

		uart1_log<64>(LOG_LEVEL::INFO, "main", "Ready");

		for(;;)
		{
			vTaskSuspend(nullptr);
		}
	}
};
Main_task main_task __attribute__ (( section(".ram_d2_s2_noload") ));

extern "C"
{
	USBD_CDC_HandleTypeDef usb_cdc_class_data;
	void* USBD_cdc_class_malloc(size_t size)
	{
		if(size != sizeof(usb_cdc_class_data))
		{
			return nullptr;
		}

		return &usb_cdc_class_data;
	}

	void USBD_cdc_class_free(void* ptr)
	{

	}

	int8_t CDC_Init_HS(void);
	int8_t CDC_DeInit_HS(void);
	int8_t CDC_Control_HS(uint8_t cmd, uint8_t* pbuf, uint16_t length);
	int8_t CDC_Receive_HS(uint8_t* pbuf, uint32_t *Len);
	void CDC_TX_Cmpl_HS(void);

	USBD_CDC_ItfTypeDef USBD_Interface_fops_HS =
	{
		CDC_Init_HS,
		CDC_DeInit_HS,
		CDC_Control_HS,
		CDC_Receive_HS,
		CDC_TX_Cmpl_HS
	};

	int8_t CDC_Init_HS(void)
	{
		usb_rx_task.handle_init_callback();
		usb_tx_task.handle_init_callback();
		return USBD_OK;
	}

	int8_t CDC_DeInit_HS(void)
	{
	/* USER CODE BEGIN 9 */
		return USBD_OK;
	/* USER CODE END 9 */
	}

	int8_t CDC_Receive_HS(uint8_t* Buf, uint32_t *Len)
	{
		return usb_rx_task.handle_rx_callback(Buf, *Len);
	}

	void CDC_TX_Cmpl_HS(void)
	{
		usb_tx_task.notify_tx_complete_callback();
	}

	int8_t CDC_Control_HS(uint8_t cmd, uint8_t* pbuf, uint16_t length)
	{
		return USBD_OK;
	}

	static char USB_SERIAL_NUMBER[25] = {0};
	char* get_usb_serial_number()
	{
		return USB_SERIAL_NUMBER;
	}
	void set_usb_serial_number(char id_str[25])
	{
		snprintf(USB_SERIAL_NUMBER, 25, "%s", id_str);
	}

	void handle_config_assert(const char* file, const int line, const char* msg)
	{
		uart1_log<64>(LOG_LEVEL::FATAL, "freertos", "configASSERT in %s at %d, %s", file, line, msg);
	}
}

void get_unique_id(std::array<uint32_t, 3>* id)
{
	volatile uint32_t* addr = reinterpret_cast<uint32_t*>(0x1FF1E800);

	std::copy_n(addr, 3, id->data());
}

void get_unique_id_str(std::array<char, 25>* id_str)
{
	//0x012345670123456701234567
	std::array<uint32_t, 3> id;
	get_unique_id(&id);

	snprintf(id_str->data(), id_str->size(), "%08" PRIX32 "%08" PRIX32 "%08" PRIX32, id[0], id[1], id[2]);
}

void set_gpio_low_power(GPIO_TypeDef* const gpio)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	GPIO_InitStruct.Pin = GPIO_PIN_All;
	GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
	// GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Pull = GPIO_PULLDOWN;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(gpio, &GPIO_InitStruct);
}

void set_all_gpio_low_power()
{
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOD_CLK_ENABLE();
	__HAL_RCC_GPIOE_CLK_ENABLE();
	__HAL_RCC_GPIOF_CLK_ENABLE();
	__HAL_RCC_GPIOG_CLK_ENABLE();
	__HAL_RCC_GPIOH_CLK_ENABLE();
	__HAL_RCC_GPIOI_CLK_ENABLE();
	__HAL_RCC_GPIOJ_CLK_ENABLE();
	__HAL_RCC_GPIOK_CLK_ENABLE();

	set_gpio_low_power(GPIOA);
	set_gpio_low_power(GPIOB);
	set_gpio_low_power(GPIOC);
	set_gpio_low_power(GPIOD);
	set_gpio_low_power(GPIOE);
	set_gpio_low_power(GPIOF);
	set_gpio_low_power(GPIOG);
	set_gpio_low_power(GPIOH);
	set_gpio_low_power(GPIOI);
	set_gpio_low_power(GPIOJ);
	set_gpio_low_power(GPIOK);

	__HAL_RCC_GPIOA_CLK_DISABLE();
	__HAL_RCC_GPIOB_CLK_DISABLE();
	__HAL_RCC_GPIOC_CLK_DISABLE();
	__HAL_RCC_GPIOD_CLK_DISABLE();
	__HAL_RCC_GPIOE_CLK_DISABLE();
	__HAL_RCC_GPIOF_CLK_DISABLE();
	__HAL_RCC_GPIOG_CLK_DISABLE();
	__HAL_RCC_GPIOH_CLK_DISABLE();
	__HAL_RCC_GPIOI_CLK_DISABLE();
	__HAL_RCC_GPIOJ_CLK_DISABLE();
	__HAL_RCC_GPIOK_CLK_DISABLE();
}

int main(void)
{
	{
		//errata 2.2.9
		volatile uint32_t* AXI_TARG7_FN_MOD = 
		reinterpret_cast<uint32_t*>(
			0x51000000 + 
			0x1108 + 
			0x1000*7U
		);

		const uint32_t AXI_TARGx_FN_MOD_READ_ISS_OVERRIDE  = 0x00000001;
		const uint32_t AXI_TARGx_FN_MOD_WRITE_ISS_OVERRIDE = 0x00000002;

		SET_BIT(*AXI_TARG7_FN_MOD, AXI_TARGx_FN_MOD_READ_ISS_OVERRIDE);
	}

	//confg mpu
	if(1)
	{
		/*
		ITCMRAM, 0x00000000, 64K

		FLASH, 0x08000000, 128K

		DTCMRAM, 0x20000000, 128K

		AXI_D1_SRAM, 0x24000000, 512K,  CPU Inst/Data

		AHB_D2_SRAM1, 0x30000000, 128K, CPU Inst
		AHB_D2_SRAM2, 0x30020000, 128K, CPU Data
		AHB_D2_SRAM3, 0x30040000, 32K,  Peripheral Buffers

		AHB_D3_SRAM4, 0x38000000, 64K

		BBRAM, 0x38800000, 4K

		QUADSPI, 0x90000000, 16M

		Peripherals, 0x40000000, 512M
		*/

		MPU_Region_InitTypeDef mpu_reg;
		
		HAL_MPU_Disable();

		/*
		// Global
		// Normal, no access
		mpu_reg.Enable = MPU_REGION_ENABLE;
		mpu_reg.Number = MPU_REGION_NUMBER0;
		mpu_reg.BaseAddress = 0x00000000;
		mpu_reg.Size = MPU_REGION_SIZE_4GB;
		mpu_reg.SubRegionDisable = 0x00;
		mpu_reg.AccessPermission = MPU_REGION_NO_ACCESS;
		mpu_reg.TypeExtField = MPU_TEX_LEVEL1;
		mpu_reg.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;
		mpu_reg.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
		mpu_reg.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;
		mpu_reg.IsShareable = MPU_ACCESS_NOT_SHAREABLE;
		HAL_MPU_ConfigRegion(&mpu_reg);
		*/

		// ITCMRAM
		// Normal, Non-cacheable
		mpu_reg.Enable = MPU_REGION_ENABLE;
		mpu_reg.Number = MPU_REGION_NUMBER1;
		mpu_reg.BaseAddress = 0x00000000;
		mpu_reg.Size = MPU_REGION_SIZE_64KB;
		mpu_reg.SubRegionDisable = 0x00;
		mpu_reg.AccessPermission = MPU_REGION_FULL_ACCESS;
		mpu_reg.TypeExtField = MPU_TEX_LEVEL1;
		mpu_reg.DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE;
		mpu_reg.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
		mpu_reg.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;
		mpu_reg.IsShareable = MPU_ACCESS_NOT_SHAREABLE;
		HAL_MPU_ConfigRegion(&mpu_reg);

		// FLASH
		mpu_reg.Enable = MPU_REGION_ENABLE;
		mpu_reg.Number = MPU_REGION_NUMBER2;
		mpu_reg.BaseAddress = 0x08000000;
		mpu_reg.Size = MPU_REGION_SIZE_128KB;
		mpu_reg.SubRegionDisable = 0x00;
		mpu_reg.AccessPermission = MPU_REGION_PRIV_RO_URO;
		mpu_reg.TypeExtField = MPU_TEX_LEVEL0;
		mpu_reg.DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE;
		mpu_reg.IsCacheable = MPU_ACCESS_CACHEABLE;
		mpu_reg.IsBufferable = MPU_ACCESS_BUFFERABLE;
		mpu_reg.IsShareable = MPU_ACCESS_NOT_SHAREABLE;
		HAL_MPU_ConfigRegion(&mpu_reg);

		// DTCMRAM
		// Normal, Non-cacheable
		mpu_reg.Enable = MPU_REGION_ENABLE;
		mpu_reg.Number = MPU_REGION_NUMBER3;
		mpu_reg.BaseAddress = 0x20000000;
		mpu_reg.Size = MPU_REGION_SIZE_128KB;
		mpu_reg.SubRegionDisable = 0x00;
		mpu_reg.AccessPermission = MPU_REGION_FULL_ACCESS;
		mpu_reg.TypeExtField = MPU_TEX_LEVEL1;
		mpu_reg.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;
		mpu_reg.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
		mpu_reg.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;
		mpu_reg.IsShareable = MPU_ACCESS_NOT_SHAREABLE;
		HAL_MPU_ConfigRegion(&mpu_reg);

		// AXI_D1_SRAM
		// Write-back, no write allocate
		mpu_reg.Enable = MPU_REGION_ENABLE;
		mpu_reg.Number = MPU_REGION_NUMBER4;
		mpu_reg.BaseAddress = 0x24000000;
		mpu_reg.Size = MPU_REGION_SIZE_512KB;
		mpu_reg.SubRegionDisable = 0x00;
		// mpu_reg.AccessPermission = MPU_REGION_FULL_ACCESS;
		mpu_reg.AccessPermission = MPU_REGION_PRIV_RO_URO;
		mpu_reg.TypeExtField = MPU_TEX_LEVEL0;
		mpu_reg.DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE;
		mpu_reg.IsCacheable = MPU_ACCESS_CACHEABLE;
		mpu_reg.IsBufferable = MPU_ACCESS_BUFFERABLE;
		mpu_reg.IsShareable = MPU_ACCESS_NOT_SHAREABLE;
		HAL_MPU_ConfigRegion(&mpu_reg);

		// AHB_D2_SRAM1
		// Write-back, no write allocate
		mpu_reg.Enable = MPU_REGION_ENABLE;
		mpu_reg.Number = MPU_REGION_NUMBER5;
		mpu_reg.BaseAddress = 0x30000000;
		mpu_reg.Size = MPU_REGION_SIZE_128KB;
		mpu_reg.SubRegionDisable = 0x00;
		mpu_reg.AccessPermission = MPU_REGION_FULL_ACCESS;
		mpu_reg.TypeExtField = MPU_TEX_LEVEL0;
		mpu_reg.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;
		mpu_reg.IsCacheable = MPU_ACCESS_CACHEABLE;
		mpu_reg.IsBufferable = MPU_ACCESS_BUFFERABLE;
		mpu_reg.IsShareable = MPU_ACCESS_NOT_SHAREABLE;
		HAL_MPU_ConfigRegion(&mpu_reg);

		// AHB_D2_SRAM2
		// Write-back, no write allocate
		mpu_reg.Enable = MPU_REGION_ENABLE;
		mpu_reg.Number = MPU_REGION_NUMBER6;
		mpu_reg.BaseAddress = 0x30020000;
		mpu_reg.Size = MPU_REGION_SIZE_128KB;
		mpu_reg.SubRegionDisable = 0x00;
		mpu_reg.AccessPermission = MPU_REGION_FULL_ACCESS;
		mpu_reg.TypeExtField = MPU_TEX_LEVEL0;
		mpu_reg.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;
		mpu_reg.IsCacheable = MPU_ACCESS_CACHEABLE;
		mpu_reg.IsBufferable = MPU_ACCESS_BUFFERABLE;
		mpu_reg.IsShareable = MPU_ACCESS_NOT_SHAREABLE;
		HAL_MPU_ConfigRegion(&mpu_reg);

		// AHB_D2_SRAM3
		// Normal, Non-cacheable
		mpu_reg.Enable = MPU_REGION_ENABLE;
		mpu_reg.Number = MPU_REGION_NUMBER7;
		mpu_reg.BaseAddress = 0x30040000;
		mpu_reg.Size = MPU_REGION_SIZE_32KB;
		mpu_reg.SubRegionDisable = 0x00;
		mpu_reg.AccessPermission = MPU_REGION_FULL_ACCESS;
		mpu_reg.TypeExtField = MPU_TEX_LEVEL1;
		mpu_reg.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;
		mpu_reg.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
		mpu_reg.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;
		mpu_reg.IsShareable = MPU_ACCESS_NOT_SHAREABLE;
		HAL_MPU_ConfigRegion(&mpu_reg);

		// AHB_D3_SRAM4
		// Write-back, no write allocate
		mpu_reg.Enable = MPU_REGION_ENABLE;
		mpu_reg.Number = MPU_REGION_NUMBER8;
		mpu_reg.BaseAddress = 0x38000000;
		mpu_reg.Size = MPU_REGION_SIZE_64KB;
		mpu_reg.SubRegionDisable = 0x00;
		mpu_reg.AccessPermission = MPU_REGION_FULL_ACCESS;
		mpu_reg.TypeExtField = MPU_TEX_LEVEL0;
		mpu_reg.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;
		mpu_reg.IsCacheable = MPU_ACCESS_CACHEABLE;
		mpu_reg.IsBufferable = MPU_ACCESS_BUFFERABLE;
		mpu_reg.IsShareable = MPU_ACCESS_NOT_SHAREABLE;
		HAL_MPU_ConfigRegion(&mpu_reg);

		// BBSRAM
		// Write-back, no write allocate
		mpu_reg.Enable = MPU_REGION_ENABLE;
		mpu_reg.Number = MPU_REGION_NUMBER9;
		mpu_reg.BaseAddress = 0x38800000;
		mpu_reg.Size = MPU_REGION_SIZE_4KB;
		mpu_reg.SubRegionDisable = 0x00;
		mpu_reg.AccessPermission = MPU_REGION_FULL_ACCESS;
		mpu_reg.TypeExtField = MPU_TEX_LEVEL0;
		mpu_reg.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;
		mpu_reg.IsCacheable = MPU_ACCESS_CACHEABLE;
		mpu_reg.IsBufferable = MPU_ACCESS_BUFFERABLE;
		mpu_reg.IsShareable = MPU_ACCESS_NOT_SHAREABLE;
		HAL_MPU_ConfigRegion(&mpu_reg);

		// QUADSPI
		// Write through, no write allocate
		mpu_reg.Enable = MPU_REGION_ENABLE;
		mpu_reg.Number = MPU_REGION_NUMBER10;
		mpu_reg.BaseAddress = 0x90000000;
		mpu_reg.Size = MPU_REGION_SIZE_16MB;
		mpu_reg.SubRegionDisable = 0x00;
		mpu_reg.AccessPermission = MPU_REGION_FULL_ACCESS;
		mpu_reg.TypeExtField = MPU_TEX_LEVEL0;
		mpu_reg.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;
		mpu_reg.IsCacheable = MPU_ACCESS_CACHEABLE;
		mpu_reg.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;
		mpu_reg.IsShareable = MPU_ACCESS_NOT_SHAREABLE;
		HAL_MPU_ConfigRegion(&mpu_reg);

		// Peripherals
		// Strongly Ordered
		/*
		mpu_reg.Enable = MPU_REGION_ENABLE;
		mpu_reg.Number = MPU_REGION_NUMBER11;
		mpu_reg.BaseAddress = 0x40000000;
		mpu_reg.Size = MPU_REGION_SIZE_512MB;
		mpu_reg.SubRegionDisable = 0x00;
		mpu_reg.AccessPermission = MPU_REGION_FULL_ACCESS;
		mpu_reg.TypeExtField = MPU_TEX_LEVEL0;
		mpu_reg.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;
		mpu_reg.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
		mpu_reg.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;
		mpu_reg.IsShareable = MPU_ACCESS_NOT_SHAREABLE;
		HAL_MPU_ConfigRegion(&mpu_reg);
		*/
		// Non-shareable device 
		mpu_reg.Enable = MPU_REGION_ENABLE;
		mpu_reg.Number = MPU_REGION_NUMBER11;
		mpu_reg.BaseAddress = 0x40000000;
		mpu_reg.Size = MPU_REGION_SIZE_512MB;
		mpu_reg.SubRegionDisable = 0x00;
		mpu_reg.AccessPermission = MPU_REGION_FULL_ACCESS;
		mpu_reg.TypeExtField = MPU_TEX_LEVEL2;
		mpu_reg.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;
		mpu_reg.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
		mpu_reg.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;
		mpu_reg.IsShareable = MPU_ACCESS_NOT_SHAREABLE;
		HAL_MPU_ConfigRegion(&mpu_reg);

		// Privledged code may use background mem map
		HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);

		//No background mem map
		//MPU enabled during MMI
		// HAL_MPU_Enable(MPU_HARDFAULT_NMI);
		
	}

	SCB_EnableICache();

	SCB_EnableDCache();

	HAL_Init();

	set_all_gpio_low_power();

	SystemClock_Config();

	//Enable backup domain in standby and Vbat mode
	HAL_PWREx_EnableBkUpReg();

	{
		std::array<char, 25> id_str;
		get_unique_id_str(&id_str);

		set_usb_serial_number(id_str.data());
	}

	MX_GPIO_Init();
	MX_USART1_UART_Init();
	// MX_FDCAN1_Init();
	MX_CRC_Init();
	MX_HASH_Init();
	MX_RTC_Init();
	MX_RNG_Init();
	// MX_TIM3_Init();
	// MX_QUADSPI_Init();

	if(0)
	{
		/*Configure GPIO pin : PA8 */
		GPIO_InitTypeDef GPIO_InitStruct = {0};
		GPIO_InitStruct.Pin = GPIO_PIN_8;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
		GPIO_InitStruct.Alternate = GPIO_AF0_MCO;
		HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

		HAL_RCC_MCOConfig(RCC_MCO1, RCC_MCO1SOURCE_HSE, RCC_MCODIV_1);
	}

	{
		std::array<char, 25> id_str;
		get_unique_id_str(&id_str);
		uart1_log<64>(LOG_LEVEL::INFO, "main", "Initialing");
		uart1_log<64>(LOG_LEVEL::INFO, "main", "CAN FD <-> USB Adapter");
		uart1_log<64>(LOG_LEVEL::INFO, "main", "P/N: SM-1301");
		uart1_log<64>(LOG_LEVEL::INFO, "main", "S/N: %s", id_str.data());
	
		const uint32_t* main_ptr = reinterpret_cast<const uint32_t*>(&main);
		uart1_log<64>(LOG_LEVEL::DEBUG, "main", "main: 0x%08" PRIX32, main);

		const uint32_t stack_ptr = __get_MSP();
		uart1_log<64>(LOG_LEVEL::DEBUG, "main", "msp:  0x%08" PRIX32, stack_ptr);
	}

	main_task.launch("main_task", 15);

	vTaskStartScheduler();

	for(;;)
	{

	}
}
