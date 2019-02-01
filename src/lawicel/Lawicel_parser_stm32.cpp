#include "Lawicel_parser_stm32.hpp"

#include <algorithm>
#include <stdexcept>

bool Lawicel_parser_stm32::handle_std_baud(const CAN_NOM_BPS baud)
{
	STM32_fdcan_tx::STD_BAUD stm32_baud;
	switch(baud)
	{
		case CAN_NOM_BPS::bps_10k:
		{
			stm32_baud = STM32_fdcan_tx::STD_BAUD::B10000;
			break;
		}
		case CAN_NOM_BPS::bps_20k:
		{
			stm32_baud = STM32_fdcan_tx::STD_BAUD::B20000;
			break;
		}
		case CAN_NOM_BPS::bps_50k:
		{
			stm32_baud = STM32_fdcan_tx::STD_BAUD::B50000;
			break;
		}
		case CAN_NOM_BPS::bps_100k:
		{
			stm32_baud = STM32_fdcan_tx::STD_BAUD::B100000;
			break;
		}
		case CAN_NOM_BPS::bps_125k:
		{
			stm32_baud = STM32_fdcan_tx::STD_BAUD::B125000;
			break;
		}
		case CAN_NOM_BPS::bps_250k:
		{
			stm32_baud = STM32_fdcan_tx::STD_BAUD::B250000;
			break;
		}
		case CAN_NOM_BPS::bps_500k:
		{
			stm32_baud = STM32_fdcan_tx::STD_BAUD::B500000;
			break;
		}
		case CAN_NOM_BPS::bps_800k:
		{
			stm32_baud = STM32_fdcan_tx::STD_BAUD::B800000;
			break;
		}
		case CAN_NOM_BPS::bps_1M:
		{
			stm32_baud = STM32_fdcan_tx::STD_BAUD::B1000000;
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
bool Lawicel_parser_stm32::handle_tx_fd_rtr_std(const uint32_t id, const uint8_t data_len)
{
	return m_fdcan->tx_fd_rtr_std(id, STM32_fdcan_tx::ESI::ACTIVE, data_len);
}
bool Lawicel_parser_stm32::handle_tx_fd_rtr_ext(const uint32_t id, const uint8_t data_len)
{
	return m_fdcan->tx_fd_rtr_ext(id, STM32_fdcan_tx::ESI::ACTIVE, data_len);
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
