#include "Lawicel_parser_stm32.hpp"

#include "bootloader_util/Bootloader_key.hpp"
#include "common_util/Byte_util.hpp"

#include "global_app_inst.hpp"
#include "tasks/Task_instances.hpp"

#include "uart1_printf.hpp"

#include <mbedtls/md5.h>
#include <crc/crc_16_ccitt.hpp>

#include <algorithm>

bool Lawicel_parser_stm32::handle_std_baud(const CAN_NOM_BPS baud)
{
	unsigned stm32_baud = 0;
	switch(baud)
	{
		case CAN_NOM_BPS::bps_10k:
		{
			stm32_baud = 10000U;
			break;
		}
		case CAN_NOM_BPS::bps_20k:
		{
			stm32_baud = 20000U;
			break;
		}
		case CAN_NOM_BPS::bps_50k:
		{
			stm32_baud = 50000U;
			break;
		}
		case CAN_NOM_BPS::bps_100k:
		{
			stm32_baud = 100000U;
			break;
		}
		case CAN_NOM_BPS::bps_125k:
		{
			stm32_baud = 125000U;
			break;
		}
		case CAN_NOM_BPS::bps_250k:
		{
			stm32_baud = 250000U;
			break;
		}
		case CAN_NOM_BPS::bps_500k:
		{
			stm32_baud = 500000U;
			break;
		}
		case CAN_NOM_BPS::bps_800k:
		{
			stm32_baud = 800000U;
			break;
		}
		case CAN_NOM_BPS::bps_1M:
		{
			stm32_baud = 1000000U;
			break;
		}
		default:
		{
			return false;
		}
	}
	
	//update config
	CAN_USB_app_config config;
	can_usb_app.get_config(&config);

	if(config.get_config().bitrate_nominal != stm32_baud)
	{
		uart1_log<128>(LOG_LEVEL::INFO, "Lawicel_parser_stm32::handle_std_baud", "Updating baud");

		config.get_config().bitrate_nominal = stm32_baud;

		//write new config to flash
		if(!can_usb_app.write_config(config))
		{
			uart1_log<128>(LOG_LEVEL::ERROR, "Lawicel_parser_stm32::handle_std_baud", "Writing config.xml failed");
			return false;
		}
	}
	else
	{
		uart1_log<128>(LOG_LEVEL::INFO, "Lawicel_parser_stm32::handle_std_baud", "Requested baud already set");
	}

	return true;
}
bool Lawicel_parser_stm32::handle_cust_baud(const uint8_t b0, const uint8_t b1)
{
	return false;
}
bool Lawicel_parser_stm32::handle_open()
{
	if(!m_fdcan->open())
	{
		uart1_log<128>(LOG_LEVEL::ERROR, "Lawicel_parser_stm32::handle_open", "fdcan->open() failed");
		return false;
	}

	uart1_log<128>(LOG_LEVEL::ERROR, "Lawicel_parser_stm32::handle_open", "fdcan->open() ok");

	return true;
}
bool Lawicel_parser_stm32::handle_open_listen()
{
	return false;
}
bool Lawicel_parser_stm32::handle_close()
{
	if(!m_fdcan->close())
	{
		uart1_log<128>(LOG_LEVEL::ERROR, "Lawicel_parser_stm32::handle_open", "fdcan->close() failed");
		return false;
	}

	uart1_log<128>(LOG_LEVEL::ERROR, "Lawicel_parser_stm32::handle_open", "fdcan->close() ok");

	return true;
}
bool Lawicel_parser_stm32::handle_tx_std(const uint32_t id, const uint8_t data_len, const uint8_t* data)
{
	if(!m_fdcan->tx_std(id, data_len, data))
	{
		return false;
	}

	//update led status
	led_task.notify_can_tx();
	return true;
}
bool Lawicel_parser_stm32::handle_tx_ext(const uint32_t id, const uint8_t data_len, const uint8_t* data)
{
	if(m_fdcan->tx_ext(id, data_len, data))
	{
		return false;
	}

	//update led status
	led_task.notify_can_tx();
	return true;
}
bool Lawicel_parser_stm32::handle_tx_rtr_std(const uint32_t id, const uint8_t data_len)
{
	if(m_fdcan->tx_std_rtr(id, data_len))
	{
		return false;
	}

	//update led status
	led_task.notify_can_tx();
	return true;
}
bool Lawicel_parser_stm32::handle_tx_rtr_ext(const uint32_t id, const uint8_t data_len)
{
	if(m_fdcan->tx_ext_rtr(id, data_len))
	{
		return false;
	}

	//update led status
	led_task.notify_can_tx();
	return true;
}
bool Lawicel_parser_stm32::handle_tx_fd_std(const uint32_t id, const uint8_t data_len, const uint8_t* data)
{
	if(m_fdcan->tx_fd_std(id, STM32_fdcan_tx::BRS::ON, STM32_fdcan_tx::ESI::ACTIVE, data_len, data))
	{
		return false;
	}

	//update led status
	led_task.notify_can_tx();
	return true;
}
bool Lawicel_parser_stm32::handle_tx_fd_ext(const uint32_t id, const uint8_t data_len, const uint8_t* data)
{
	if(m_fdcan->tx_fd_ext(id, STM32_fdcan_tx::BRS::ON, STM32_fdcan_tx::ESI::ACTIVE, data_len, data))
	{
		return false;
	}

	//update led status
	led_task.notify_can_tx();
	return true;
}
bool Lawicel_parser_stm32::handle_get_flags()
{
	return false;
}
bool Lawicel_parser_stm32::handle_set_accept_code(const uint32_t code)
{
	return false;
}
bool Lawicel_parser_stm32::handle_set_accept_mask(const uint32_t mask)
{
	return false;
}
bool Lawicel_parser_stm32::handle_get_version(std::array<uint8_t, 4>* const ver)
{
	const std::array<char, 2> hw_ver = {'0', '1'};
	const std::array<char, 2> sw_ver = {'0', '2'};
	
	ver->data()[0] = static_cast<uint8_t>(hw_ver[0]);
	ver->data()[1] = static_cast<uint8_t>(hw_ver[1]);

	ver->data()[2] = static_cast<uint8_t>(sw_ver[0]);
	ver->data()[3] = static_cast<uint8_t>(sw_ver[1]);

	return true;
}
bool Lawicel_parser_stm32::handle_get_serial(std::array<uint8_t, 4>* const sn)
{
	//CAN232 has a 4 hex digit serial number, eg 2 bytes
	//hash down our real id to 2 bytes, as uniform as we can
	//this is not globally unique
	//TODO: add extended serial number that is globally unique

	//get stm32 id, 12 bytes globally unique. something like fab + date + wafer location i think
	std::array<uint32_t, 3> id;
	CAN_USB_app::get_unique_id(&id);

	//the 2 byte output
	std::array<uint8_t, 2> sn_bin;
	sn_bin.fill(0);

	//md5 hash to mix
	mbedtls_md5_context md5_ctx;
	mbedtls_md5_init(&md5_ctx);
	mbedtls_md5_starts_ret(&md5_ctx);

	mbedtls_md5_update_ret(&md5_ctx, reinterpret_cast<uint8_t*>(id.data()), id.size() * sizeof(uint32_t) / sizeof(uint8_t));

	std::array<uint8_t, 16> md5_output;
	mbedtls_md5_finish_ret(&md5_ctx, md5_output.data());
	mbedtls_md5_free(&md5_ctx);

	for(size_t i = 0; i < 8; i++)
	{
		sn_bin[0] ^= md5_output[0 + i];
		sn_bin[1] ^= md5_output[8 + i];
	}

	//convert to hex
	Byte_util::u8_to_hex(sn_bin[0], reinterpret_cast<char*>(sn->data() + 0));
	Byte_util::u8_to_hex(sn_bin[1], reinterpret_cast<char*>(sn->data() + 2));

	return true;
}
bool Lawicel_parser_stm32::handle_set_timestamp(const bool enable)
{
	return false;
}

bool Lawicel_parser_stm32::handle_ext_config(const std::vector<char>& config_str)
{
	tinyxml2::XMLDocument config_doc;
	tinyxml2::XMLError err = config_doc.Parse(config_str.data(), config_str.size());
	if(err != tinyxml2::XML_SUCCESS)
	{
		uart1_log<128>(LOG_LEVEL::ERROR, "Lawicel_parser_stm32::handle_ext_config", "config_doc.Parse failed, %s", tinyxml2::XMLDocument::ErrorIDToName(err));
		return false;
	}

	CAN_USB_app_config config;
	if(!config.from_xml(config_doc))
	{
		uart1_log<128>(LOG_LEVEL::ERROR, "Lawicel_parser_stm32::handle_ext_config", "config.from_xml failed");
		return false;
	}

	return can_usb_app.write_config(config);
}
bool Lawicel_parser_stm32::handle_ext_print_config()
{
	CAN_USB_app_config config;
	can_usb_app.get_config(&config);

	tinyxml2::XMLDocument config_doc;
	if(!config.to_xml(&config_doc))
	{
		return false;
	}

	tinyxml2::XMLPrinter xml_printer(nullptr, true, 0);
	config_doc.Print(&xml_printer);
	const char* doc_str = xml_printer.CStr();

	uart1_log<128>(LOG_LEVEL::INFO, "Lawicel_parser_stm32", "Printing config file");
	// print to debug header
	// uart1_puts(doc_str);
	// print to usb
	write_string(doc_str);
	uart1_log<128>(LOG_LEVEL::INFO, "Lawicel_parser_stm32", "End of config file");

	return true;
}
bool Lawicel_parser_stm32::handle_ext_bitrate_table(const std::vector<char>& table_str)
{
	tinyxml2::XMLDocument table_doc;
	tinyxml2::XMLError err = table_doc.Parse(table_str.data(), table_str.size());
	if(err != tinyxml2::XML_SUCCESS)
	{
		return false;
	}

	CAN_USB_app_bitrate_table table;
	if(!table.from_xml(table_doc))
	{
		return false;
	}

	return can_usb_app.write_bitrate_table(table);
}
bool Lawicel_parser_stm32::handle_ext_print_bitrate_table()
{
	CAN_USB_app_bitrate_table table;
	can_usb_app.get_bitrate_tables(&table);

	tinyxml2::XMLDocument table_doc;
	if(!table.to_xml(&table_doc))
	{
		return false;
	}

	tinyxml2::XMLPrinter xml_printer(nullptr, true, 0);
	table_doc.Print(&xml_printer);
	const char* doc_str = xml_printer.CStr();

	uart1_log<128>(LOG_LEVEL::INFO, "Lawicel_parser_stm32", "Printing bitrate table");
	// print to debug header
	// uart1_puts(doc_str);
	// print to usb
	write_string(doc_str);
	uart1_log<128>(LOG_LEVEL::INFO, "Lawicel_parser_stm32", "End of bitrate table");

	return true;
}
bool Lawicel_parser_stm32::handle_ext_defconfig()
{
	if(!can_usb_app.write_default_config())
	{
		return false;
	}
	if(!can_usb_app.write_default_bitrate_table())
	{
		return false;
	}

	return true;
}
bool Lawicel_parser_stm32::handle_ext_bootloader()
{
	const Bootloader_key key = Bootloader_key::get_key_boot();
	key.to_addr(reinterpret_cast<uint8_t*>(0x38800000));

	//Disable ISR, sync
	asm volatile(
		"cpsid i\n"
		"dsb 0xF\n"
		"isb 0xF\n"
		: /* no out */
		: /* no in */
		: "memory"
		);

	//reboot
	NVIC_SystemReset();

	for(;;)
	{

	}

	return true;
}

bool Lawicel_parser_stm32::handle_ext_serial()
{
	std::array<char, 25> id_str;
	CAN_USB_app::get_unique_id_str(&id_str);

	write_string(id_str.data());

	return true;
}
