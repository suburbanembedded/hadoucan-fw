#include "Lawicel_parser_stm32.hpp"

#include "bootloader_util/Bootloader_key.hpp"
#include "common_util/Byte_util.hpp"
#include "common_util/Comparison_util.hpp"

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
		uart1_log<128>(LOG_LEVEL::INFO, "Lawicel_parser_stm32::handle_std_baud", "Updating baud to %u", stm32_baud);

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
bool Lawicel_parser_stm32::handle_cust_baud(const uint8_t BTR0, const uint8_t BTR1)
{
	uart1_log<128>(LOG_LEVEL::DEBUG, "Lawicel_parser_stm32::handle_cust_baud", "");

	//number of clock cycles a bit period maybe shortened or lengthened
	const uint8_t sjw = (BTR0 & 0xC0) >> 6;

	//s031C = 125kbps according to datasheet
	//brp = 4
	//tseg1 = 13
	//tseg2 = 2
	//so 8MHz external osc
	const uint32_t CAN232_clock = 8000000U;

	//tsci = 2*tclk*(brp+1)
	//fsci = fxtal / ( 2 * brp + 1)
	//fxtal 24MHz max
	const uint8_t brp = ((BTR0 & 0x3F) >> 0) + 1;

	// samp = 1 means triple sampling for low / medium speed
	// samp = 1 means single sampling for high speed
	// we will ignore
	// const uint8_t samp  = (BTR1 & 0x80) >> 7;
	const uint8_t tseg2 = ((BTR1 & 0x70) >> 4) + 1;
	const uint8_t tseg1 = ((BTR1 & 0x0F) >> 0) + 1;

	CAN_USB_app_config config;
	can_usb_app.get_config(&config);
	const uint32_t kernel_clock = config.get_config().can_clock;	

	//calculate the desired kernel clock
	const uint32_t prescaler_multiplier = kernel_clock / CAN232_clock;
	const uint32_t prescaler_multiplier_remainder = kernel_clock % CAN232_clock;

	if(prescaler_multiplier_remainder != 0)
	{
		uart1_log<128>(LOG_LEVEL::ERROR, "Lawicel_parser_stm32::handle_cust_baud", "CAN kernel clock is not a multiple of 8MHz so cannot use SJA1000 bitrate commands");
		return false;
	}

	//what prescaler of our higher speed clock is this?
	//either we should use a 8MHz kernel, or use a 24MHz kernel and mul the divider by 3

	CAN_USB_app_bitrate_table::Bitrate_Table_Entry new_baud;
	new_baud.pre = brp * prescaler_multiplier;//1-512
	new_baud.sjw = sjw;//1-128
	new_baud.tseg1 = tseg1;//1-256 
	new_baud.tseg2 = tseg2;//1-128

	if(!Comparison_util::is_within_inclusive(new_baud.pre, 1, 512))
	{
		uart1_log<128>(LOG_LEVEL::ERROR, "Lawicel_parser_stm32::handle_cust_baud", "PRE out of range");
		return false;
	}
	if(!Comparison_util::is_within_inclusive(new_baud.sjw, 1, 128))
	{
		uart1_log<128>(LOG_LEVEL::ERROR, "Lawicel_parser_stm32::handle_cust_baud", "SJW out of range");	
		return false;
	}
	if(!Comparison_util::is_within_inclusive(new_baud.tseg1, 1, 256))
	{
		uart1_log<128>(LOG_LEVEL::ERROR, "Lawicel_parser_stm32::handle_cust_baud", "TSEG1 out of range");
		return false;
	}
	if(!Comparison_util::is_within_inclusive(new_baud.tseg2, 1, 128))
	{
		uart1_log<128>(LOG_LEVEL::ERROR, "Lawicel_parser_stm32::handle_cust_baud", "TSEG2 out of range");
		return false;
	}

	return false;
}
bool Lawicel_parser_stm32::handle_open()
{
	uart1_log<128>(LOG_LEVEL::DEBUG, "Lawicel_parser_stm32::handle_open", "");

	//check if we need to update the default mode
	{
		CAN_USB_app_config config;
		can_usb_app.get_config(&config);

		if(config.get_config().listen_only)
		{
			config.get_config().timestamp_enable = false;
		
			if(!can_usb_app.write_config(config))
			{
				uart1_log<128>(LOG_LEVEL::ERROR, "Lawicel_parser_stm32::handle_open", "config update failed");
				return false;
			}
		}
	}

	if(!m_fdcan->open())
	{
		uart1_log<128>(LOG_LEVEL::ERROR, "Lawicel_parser_stm32::handle_open", "fdcan->open() failed");
		return false;
	}

	uart1_log<128>(LOG_LEVEL::INFO, "Lawicel_parser_stm32::handle_open", "fdcan->open() ok");

	return true;
}
bool Lawicel_parser_stm32::handle_open_listen()
{
	uart1_log<128>(LOG_LEVEL::DEBUG, "Lawicel_parser_stm32::handle_open_listen", "");

	//check if we need to update the default mode
	{
		CAN_USB_app_config config;
		can_usb_app.get_config(&config);

		if(!config.get_config().listen_only)
		{
			config.get_config().timestamp_enable = true;
			
			if(!can_usb_app.write_config(config))
			{
				uart1_log<128>(LOG_LEVEL::ERROR, "Lawicel_parser_stm32::handle_open_listen", "config update failed");
				return false;
			}
		}
	}

	if(!m_fdcan->open())
	{
		uart1_log<128>(LOG_LEVEL::ERROR, "Lawicel_parser_stm32::handle_open_listen", "fdcan->open() failed");
		return false;
	}

	return false;
}
bool Lawicel_parser_stm32::handle_close()
{
	uart1_log<128>(LOG_LEVEL::DEBUG, "Lawicel_parser_stm32::handle_close", "");

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
	uart1_log<128>(LOG_LEVEL::TRACE, "Lawicel_parser_stm32::handle_tx_std", "");

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
	uart1_log<128>(LOG_LEVEL::TRACE, "Lawicel_parser_stm32::handle_tx_ext", "");

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
	uart1_log<128>(LOG_LEVEL::TRACE, "Lawicel_parser_stm32::handle_tx_rtr_std", "");

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
	uart1_log<128>(LOG_LEVEL::TRACE, "Lawicel_parser_stm32::handle_tx_rtr_ext", "");

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
	uart1_log<128>(LOG_LEVEL::TRACE, "Lawicel_parser_stm32::handle_tx_fd_std", "");

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
	uart1_log<128>(LOG_LEVEL::TRACE, "Lawicel_parser_stm32::handle_tx_fd_ext", "");

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
	uart1_log<128>(LOG_LEVEL::DEBUG, "Lawicel_parser_stm32::handle_get_flags", "");

	return false;
}
bool Lawicel_parser_stm32::handle_set_accept_code(const uint32_t code)
{
	uart1_log<128>(LOG_LEVEL::DEBUG, "Lawicel_parser_stm32::handle_set_accept_code", "");

	return false;
}
bool Lawicel_parser_stm32::handle_set_accept_mask(const uint32_t mask)
{
	uart1_log<128>(LOG_LEVEL::DEBUG, "Lawicel_parser_stm32::handle_set_accept_mask", "");

	return false;
}
bool Lawicel_parser_stm32::handle_get_version(std::array<uint8_t, 4>* const ver)
{
	uart1_log<128>(LOG_LEVEL::DEBUG, "Lawicel_parser_stm32::handle_get_version", "");

	const std::array<char, 2> hw_ver = {'0', '1'};
	const std::array<char, 2> sw_ver = {'0', '5'};
	
	ver->data()[0] = static_cast<uint8_t>(hw_ver[0]);
	ver->data()[1] = static_cast<uint8_t>(hw_ver[1]);

	ver->data()[2] = static_cast<uint8_t>(sw_ver[0]);
	ver->data()[3] = static_cast<uint8_t>(sw_ver[1]);

	return true;
}
bool Lawicel_parser_stm32::handle_get_serial(std::array<uint8_t, 4>* const sn)
{
	uart1_log<128>(LOG_LEVEL::DEBUG, "Lawicel_parser_stm32::handle_get_serial", "");

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
	uart1_log<128>(LOG_LEVEL::DEBUG, "Lawicel_parser_stm32::handle_set_timestamp", "");

	CAN_USB_app_config config;
	can_usb_app.get_config(&config);

	config.get_config().timestamp_enable = enable;

	return can_usb_app.write_config(config);
}

bool Lawicel_parser_stm32::handle_set_autostartup(const bool enable)
{
	uart1_log<128>(LOG_LEVEL::DEBUG, "Lawicel_parser_stm32::handle_set_autostartup", "");

	CAN_USB_app_config config;
	can_usb_app.get_config(&config);

	config.get_config().auto_startup = enable;

	return can_usb_app.write_config(config);	
}

bool Lawicel_parser_stm32::handle_ext_config(const std::vector<char>& config_str)
{
	uart1_log<128>(LOG_LEVEL::DEBUG, "Lawicel_parser_stm32::handle_ext_config", "");

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
	uart1_log<128>(LOG_LEVEL::DEBUG, "Lawicel_parser_stm32::handle_ext_print_config", "");

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
	uart1_log<128>(LOG_LEVEL::DEBUG, "Lawicel_parser_stm32::handle_ext_bitrate_table", "");

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
	uart1_log<128>(LOG_LEVEL::DEBUG, "Lawicel_parser_stm32::handle_ext_print_bitrate_table", "");

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
	uart1_log<128>(LOG_LEVEL::DEBUG, "Lawicel_parser_stm32::handle_ext_defconfig", "");

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
	uart1_log<128>(LOG_LEVEL::DEBUG, "Lawicel_parser_stm32::handle_ext_bootloader", "");

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
	uart1_log<128>(LOG_LEVEL::DEBUG, "Lawicel_parser_stm32::handle_ext_serial", "");

	std::array<char, 25> id_str;
	CAN_USB_app::get_unique_id_str(&id_str);

	write_string(id_str.data());

	return true;
}

bool Lawicel_parser_stm32::handle_ext_version()
{
	uart1_log<128>(LOG_LEVEL::DEBUG, "Lawicel_parser_stm32::handle_ext_version", "");
	
	std::array<uint8_t, 4> ver;
	handle_get_version(&ver);

	std::array<uint8_t, 7> resp;
	resp[0] = 'V';
	std::copy_n(ver.data(), 4, resp.data()+1);
	resp[5] = '\r';
	resp[6] = '\0';

	write_string((char*)(resp.data()));

	return true;
}
