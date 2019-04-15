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

#include "tasks/USB_lawicel_task.hpp"
#include "tasks/LED_task.hpp"
#include "tasks/Timesync_task.hpp"

#include "freertos_cpp_util/Task_static.hpp"
#include "freertos_cpp_util/BSema_static.hpp"
#include "freertos_cpp_util/Mutex_static.hpp"
#include "freertos_cpp_util/Condition_variable.hpp"
#include "freertos_cpp_util/object_pool/Object_pool.hpp"

#include "common_util/Byte_util.hpp"

#include "W25Q16JV.hpp"
#include "W25Q16JV_conf_region.hpp"

#include "CAN_USB_app.hpp"

#include "../external/tinyxml2/tinyxml2.h"

#include <array>
#include <map>
#include <vector>
#include <algorithm>
#include <cstdio>
#include <cinttypes>

CAN_USB_app can_usb_app __attribute__(( section(".ram_dtcm_noload") ));

USB_RX_task usb_rx_task __attribute__(( section(".ram_dtcm_noload") ));
USB_TX_task usb_tx_task __attribute__(( section(".ram_dtcm_noload") ));

USB_rx_buffer_task usb_rx_buffer_task __attribute__(( section(".ram_dtcm_noload") ));
USB_tx_buffer_task usb_tx_buffer_task __attribute__(( section(".ram_dtcm_noload") ));

LED_task led_task __attribute__(( section(".ram_dtcm_noload") ));

USB_lawicel_task usb_lawicel_task __attribute__(( section(".ram_dtcm_noload") ));

Timesync_task timesync_task __attribute__(( section(".ram_dtcm_noload") ));

STM32_fdcan_tx can_tx;

bool can_rx_to_lawicel(const std::string& str)
{
	return usb_lawicel_task.get_lawicel()->queue_rx_packet(str);
}

class Main_task : public Task_static<1024>
{
public:
	void work() override
	{
		mount_fs();
		load_config();

		CAN_USB_app_config::Config_Set config_struct;
		can_usb_app.get_config(&config_struct);
		CAN_USB_app_bitrate_table bitrate_table;
		can_usb_app.get_bitrate_tables(&bitrate_table);
		
		can_tx.set_config(config_struct);
		can_tx.set_bitrate_table(bitrate_table);

		//init
		usb_rx_buffer_task.set_usb_rx(&usb_rx_task);
		usb_tx_buffer_task.set_usb_tx(&usb_tx_task);

		usb_lawicel_task.set_can_tx(&can_tx);
		usb_lawicel_task.set_usb_tx(&usb_tx_buffer_task);
		usb_lawicel_task.set_usb_rx(&usb_rx_buffer_task);

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
		timesync_task.launch("timesync", 1);

		uart1_log<64>(LOG_LEVEL::INFO, "main", "Ready");

		for(;;)
		{
			vTaskSuspend(nullptr);
		}
	}

	bool mount_fs()
	{
		uart1_log<64>(LOG_LEVEL::INFO, "main", "Ready");

		{
			std::lock_guard<Mutex_static_recursive> lock(can_usb_app.get_mutex());

			W25Q16JV& m_qspi = can_usb_app.get_flash();
			W25Q16JV_conf_region& m_fs = can_usb_app.get_fs();

			m_qspi.set_handle(&hqspi);

			if(!m_qspi.init())
			{
				uart1_log<64>(LOG_LEVEL::ERROR, "main", "m_qspi.init failed");

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
				uart1_log<128>(LOG_LEVEL::INFO, "main", "flash mfg id %02" PRIX32, uint32_t(mfg_id));
				uart1_log<128>(LOG_LEVEL::INFO, "main", "flash pn %04" PRIX32, uint32_t(flash_pn));
			}
			else
			{
				uart1_log<64>(LOG_LEVEL::ERROR, "main", "get_jdec_id failed");
			}

			uint64_t unique_id = 0;
			if(m_qspi.get_unique_id(&unique_id))
			{
				// uart1_log<128>(LOG_LEVEL::INFO, "main", "flash sn %016" PRIX64, unique_id);
				//aparently PRIX64 is broken
				uart1_log<128>(LOG_LEVEL::INFO, "main", "flash sn %08" PRIX32 "%08" PRIX32, Byte_util::get_upper_half(unique_id), Byte_util::get_lower_half(unique_id));
			}
			else
			{
				uart1_log<64>(LOG_LEVEL::ERROR, "main", "get_unique_id failed");
			}

			uart1_log<64>(LOG_LEVEL::INFO, "main", "Mounting flash fs");
			int mount_ret = m_fs.mount();
			if(mount_ret != SPIFFS_OK)
			{
				uart1_log<128>(LOG_LEVEL::ERROR, "main", "Flash mount failed: %d", mount_ret);
				uart1_log<128>(LOG_LEVEL::ERROR, "main", "You will need to reload the config");

				uart1_log<128>(LOG_LEVEL::INFO, "main", "Format flash");
				int format_ret = m_fs.format();
				if(format_ret != SPIFFS_OK)
				{
					uart1_log<128>(LOG_LEVEL::FATAL, "main", "Flash format failed: %d", format_ret);
					uart1_log<128>(LOG_LEVEL::FATAL, "main", "Try a power cycle, your board may be broken");

					for(;;)
					{
						vTaskSuspend(nullptr);
					}
				}

				uart1_log<64>(LOG_LEVEL::INFO, "main", "Mounting flash fs");
				mount_ret = m_fs.mount();
				if(mount_ret != SPIFFS_OK)
				{
					uart1_log<128>(LOG_LEVEL::FATAL, "main", "Flash mount failed right after we formatted it: %d", mount_ret);
					uart1_log<128>(LOG_LEVEL::FATAL, "main", "Try a power cycle, your board may be broken");

					for(;;)
					{
						vTaskSuspend(nullptr);
					}
				}
				else
				{
					uart1_log<64>(LOG_LEVEL::INFO, "main", "Flash mount ok");
				}

				//write default config
				if(!can_usb_app.write_default_config())
				{
					uart1_log<64>(LOG_LEVEL::FATAL, "main", "Writing default config failed");

					for(;;)
					{
						vTaskSuspend(nullptr);
					}
				}
				if(!can_usb_app.write_default_bitrate_table())
				{
					uart1_log<64>(LOG_LEVEL::FATAL, "main", "Writing default bitrate table failed");

					for(;;)
					{
						vTaskSuspend(nullptr);
					}
				}
			}
			else
			{
				uart1_log<64>(LOG_LEVEL::INFO, "main", "Flash mount ok");
			}
		}

		return true;
	}

	bool load_config()
	{
		uart1_log<64>(LOG_LEVEL::INFO, "main", "Load config");
		if(!can_usb_app.load_config())
		{
			uart1_log<64>(LOG_LEVEL::WARN, "main", "Config load failed, restoring default");

			if(!can_usb_app.write_default_config())
			{
				uart1_log<64>(LOG_LEVEL::FATAL, "main", "Writing default config load failed");

				for(;;)
				{
					vTaskSuspend(nullptr);
				}
			}
			else
			{
				uart1_log<64>(LOG_LEVEL::WARN, "main", "Default config wrote ok");
			}
		}
		else
		{
			uart1_log<64>(LOG_LEVEL::INFO, "main", "Config ok");
		}

		if(!can_usb_app.load_bitrate_table())
		{
			uart1_log<64>(LOG_LEVEL::WARN, "main", "Bitrate table load failed, restoring default");

			if(!can_usb_app.write_default_bitrate_table())
			{
				uart1_log<64>(LOG_LEVEL::FATAL, "main", "Writing default bitrate table failed");

				for(;;)
				{
					vTaskSuspend(nullptr);
				}
			}
			else
			{
				uart1_log<64>(LOG_LEVEL::WARN, "main", "Default bitrate table wrote ok");
			}
		}
		else
		{
			uart1_log<64>(LOG_LEVEL::INFO, "main", "Bitrate table ok");
		}

		return true;
	}
protected:
	


};

Main_task main_task __attribute__(( section(".ram_dtcm_noload") ));



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

	//enable high speed for now
	//TODO: make this config
	if(1)
	{
		GPIO_InitTypeDef GPIO_InitStruct = {0};
		GPIO_InitStruct.Pin = CAN_SLOPE_Pin;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
		HAL_GPIO_Init(CAN_SLOPE_GPIO_Port, &GPIO_InitStruct);

		HAL_GPIO_WritePin(CAN_SLOPE_GPIO_Port, CAN_SLOPE_Pin, GPIO_PIN_SET);
	}

	{
		std::array<char, 25> id_str;
		get_unique_id_str(&id_str);
		uart1_log<64>(LOG_LEVEL::INFO, "main", "Initialing");
		uart1_log<64>(LOG_LEVEL::INFO, "main", "CAN FD <-> USB Adapter");
		uart1_log<64>(LOG_LEVEL::INFO, "main", "P/N: SM-1301");
		uart1_log<64>(LOG_LEVEL::INFO, "main", "S/N: %s", id_str.data());
	}

	// Switch to heap5?
	// AHB_D2_SRAM1: 0x30000000, 128KB
	// AHB_D2_SRAM2: 0x30020000, 128KB

	main_task.launch("main_task", 15);

	vTaskStartScheduler();

	for(;;)
	{

	}
}
