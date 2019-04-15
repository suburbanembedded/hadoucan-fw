#include "Lawicel_parser_stm32.hpp"

#include "bootloader_util/Bootloader_key.hpp"

#include "global_app_inst.hpp"

#include <algorithm>

bool Lawicel_parser_stm32::handle_std_baud(const CAN_NOM_BPS baud)
{
	int stm32_baud = 0;
	switch(baud)
	{
		case CAN_NOM_BPS::bps_10k:
		{
			stm32_baud = 10000;
			break;
		}
		case CAN_NOM_BPS::bps_20k:
		{
			stm32_baud = 20000;
			break;
		}
		case CAN_NOM_BPS::bps_50k:
		{
			stm32_baud = 50000;
			break;
		}
		case CAN_NOM_BPS::bps_100k:
		{
			stm32_baud = 100000;
			break;
		}
		case CAN_NOM_BPS::bps_125k:
		{
			stm32_baud = 125000;
			break;
		}
		case CAN_NOM_BPS::bps_250k:
		{
			stm32_baud = 250000;
			break;
		}
		case CAN_NOM_BPS::bps_500k:
		{
			stm32_baud = 500000;
			break;
		}
		case CAN_NOM_BPS::bps_800k:
		{
			stm32_baud = 800000;
			break;
		}
		case CAN_NOM_BPS::bps_1M:
		{
			stm32_baud = 1000000;
			break;
		}
		default:
		{
			return false;
		}
	}
	
	if(!m_fdcan->set_baud(stm32_baud))
	{
		return false;
	}

	return true;
}
bool Lawicel_parser_stm32::handle_cust_baud(const uint8_t b0, const uint8_t b1)
{
	return false;
}
bool Lawicel_parser_stm32::handle_open()
{
	return m_fdcan->open();
}
bool Lawicel_parser_stm32::handle_open_listen()
{
	return false;
}
bool Lawicel_parser_stm32::handle_close()
{
	return m_fdcan->close();
}
bool Lawicel_parser_stm32::handle_tx_std(const uint32_t id, const uint8_t data_len, const uint8_t* data)
{
	return m_fdcan->tx_std(id, data_len, data);
}
bool Lawicel_parser_stm32::handle_tx_ext(const uint32_t id, const uint8_t data_len, const uint8_t* data)
{
	return m_fdcan->tx_ext(id, data_len, data);
}
bool Lawicel_parser_stm32::handle_tx_rtr_std(const uint32_t id, const uint8_t data_len)
{
	return m_fdcan->tx_std_rtr(id, data_len);
}
bool Lawicel_parser_stm32::handle_tx_rtr_ext(const uint32_t id, const uint8_t data_len)
{
	return m_fdcan->tx_ext_rtr(id, data_len);
}
bool Lawicel_parser_stm32::handle_tx_fd_std(const uint32_t id, const uint8_t data_len, const uint8_t* data)
{
	return m_fdcan->tx_fd_std(id, STM32_fdcan_tx::BRS::ON, STM32_fdcan_tx::ESI::ACTIVE, data_len, data);
}
bool Lawicel_parser_stm32::handle_tx_fd_ext(const uint32_t id, const uint8_t data_len, const uint8_t* data)
{
	return m_fdcan->tx_fd_ext(id, STM32_fdcan_tx::BRS::ON, STM32_fdcan_tx::ESI::ACTIVE, data_len, data);
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
	ver->fill(0);
	return true;
}
bool Lawicel_parser_stm32::handle_get_serial(std::array<uint8_t, 4>* const sn)
{
	sn->fill(0);
	return true;
}
bool Lawicel_parser_stm32::handle_set_timestamp(const bool enable)
{
	return false;
}

bool Lawicel_parser_stm32::handle_ext_config()
{
	tinyxml2::XMLDocument config_doc;

	CAN_USB_app_config config;
	if(!config.from_xml(config_doc))
	{

	}

	return can_usb_app.write_config(config);
}
bool Lawicel_parser_stm32::handle_ext_defconfig()
{
	return can_usb_app.write_default_config();
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