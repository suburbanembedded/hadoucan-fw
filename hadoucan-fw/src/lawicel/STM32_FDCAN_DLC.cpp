#include "STM32_FDCAN_DLC.hpp"

#include <stdexcept>

size_t STM32_FDCAN_DLC::to_len() const
{
	switch(m_dlc)
	{
		case FDCAN_DLC_BYTES_0:
			return 0;
		case FDCAN_DLC_BYTES_1:
			return 1;
		case FDCAN_DLC_BYTES_2:
			return 2;
		case FDCAN_DLC_BYTES_3:
			return 3;
		case FDCAN_DLC_BYTES_4:
			return 4;
		case FDCAN_DLC_BYTES_5:
			return 5;
		case FDCAN_DLC_BYTES_6:
			return 6;
		case FDCAN_DLC_BYTES_7:
			return 7;
		case FDCAN_DLC_BYTES_8:
			return 8;
		case FDCAN_DLC_BYTES_12:
			return 12;
		case FDCAN_DLC_BYTES_16:
			return 16;
		case FDCAN_DLC_BYTES_20:
			return 20;
		case FDCAN_DLC_BYTES_24:
			return 24;
		case FDCAN_DLC_BYTES_32:
			return 32;
		case FDCAN_DLC_BYTES_48:
			return 48;
		case FDCAN_DLC_BYTES_64:
			return 64;
		default:
			throw std::domain_error("dlc not in bounds");
	}

	throw std::domain_error("dlc not in bounds");
}
bool STM32_FDCAN_DLC::from_len(uint8_t len)
{
	switch(len)
	{
		case 0:
			m_dlc = FDCAN_DLC_BYTES_0;
			break;
		case 1:
			m_dlc = FDCAN_DLC_BYTES_1;
			break;
		case 2:
			m_dlc = FDCAN_DLC_BYTES_2;
			break;
		case 3:
			m_dlc = FDCAN_DLC_BYTES_3;
			break;
		case 4:
			m_dlc = FDCAN_DLC_BYTES_4;
			break;
		case 5:
			m_dlc = FDCAN_DLC_BYTES_5;
			break;
		case 6:
			m_dlc = FDCAN_DLC_BYTES_6;
			break;
		case 7:
			m_dlc = FDCAN_DLC_BYTES_7;
			break;
		case 8:
			m_dlc = FDCAN_DLC_BYTES_8;
			break;
		case 12:
			m_dlc = FDCAN_DLC_BYTES_12;
			break;
		case 16:
			m_dlc = FDCAN_DLC_BYTES_16;
			break;
		case 20:
			m_dlc = FDCAN_DLC_BYTES_20;
			break;
		case 24:
			m_dlc = FDCAN_DLC_BYTES_24;
			break;
		case 32:
			m_dlc = FDCAN_DLC_BYTES_32;
			break;
		case 48:
			m_dlc = FDCAN_DLC_BYTES_48;
			break;
		case 64:
			m_dlc = FDCAN_DLC_BYTES_64;
			break;
		default:
			return false;
	}

	return true;
}

CAN_DLC STM32_FDCAN_DLC::to_can_dlc() const
{
	CAN_DLC dlc;

	switch(m_dlc)
	{
		case FDCAN_DLC_BYTES_0:
			dlc.set_can_dlc(0x00);
			return dlc;
		case FDCAN_DLC_BYTES_1:
			dlc.set_can_dlc(0x01);
			return dlc;
		case FDCAN_DLC_BYTES_2:
			dlc.set_can_dlc(0x02);
			return dlc;
		case FDCAN_DLC_BYTES_3:
			dlc.set_can_dlc(0x03);
			return dlc;
		case FDCAN_DLC_BYTES_4:
			dlc.set_can_dlc(0x04);
			return dlc;
		case FDCAN_DLC_BYTES_5:
			dlc.set_can_dlc(0x05);
			return dlc;
		case FDCAN_DLC_BYTES_6:
			dlc.set_can_dlc(0x06);
			return dlc;
		case FDCAN_DLC_BYTES_7:
			dlc.set_can_dlc(0x07);
			return dlc;
		case FDCAN_DLC_BYTES_8:
			dlc.set_can_dlc(0x08);
			return dlc;
		case FDCAN_DLC_BYTES_12:
			dlc.set_can_dlc(0x09);
			return dlc;
		case FDCAN_DLC_BYTES_16:
			dlc.set_can_dlc(0x0A);
			return dlc;
		case FDCAN_DLC_BYTES_20:
			dlc.set_can_dlc(0x0B);
			return dlc;
		case FDCAN_DLC_BYTES_24:
			dlc.set_can_dlc(0x0C);
			return dlc;
		case FDCAN_DLC_BYTES_32:
			dlc.set_can_dlc(0x0D);
			return dlc;
		case FDCAN_DLC_BYTES_48:
			dlc.set_can_dlc(0x0E);
			return dlc;
		case FDCAN_DLC_BYTES_64:
			dlc.set_can_dlc(0x0F);
			return dlc;
		default:
			throw std::domain_error("dlc not in bounds");
	}

	throw std::domain_error("dlc not in bounds");
}

bool STM32_FDCAN_DLC::from_can_dlc(const CAN_DLC& dlc)
{
	switch(dlc.get_can_dlc())
	{
		case 0x00:
			m_dlc = FDCAN_DLC_BYTES_0;
			break;
		case 0x01:
			m_dlc = FDCAN_DLC_BYTES_1;
			break;
		case 0x02:
			m_dlc = FDCAN_DLC_BYTES_2;
			break;
		case 0x03:
			m_dlc = FDCAN_DLC_BYTES_3;
			break;
		case 0x04:
			m_dlc = FDCAN_DLC_BYTES_4;
			break;
		case 0x05:
			m_dlc = FDCAN_DLC_BYTES_5;
			break;
		case 0x06:
			m_dlc = FDCAN_DLC_BYTES_6;
			break;
		case 0x07:
			m_dlc = FDCAN_DLC_BYTES_7;
			break;
		case 0x08:
			m_dlc = FDCAN_DLC_BYTES_8;
			break;
		case 0x09:
			m_dlc = FDCAN_DLC_BYTES_12;
			break;
		case 0x0A:
			m_dlc = FDCAN_DLC_BYTES_16;
			break;
		case 0x0B:
			m_dlc = FDCAN_DLC_BYTES_20;
			break;
		case 0x0C:
			m_dlc = FDCAN_DLC_BYTES_24;
			break;
		case 0x0D:
			m_dlc = FDCAN_DLC_BYTES_32;
			break;
		case 0x0E:
			m_dlc = FDCAN_DLC_BYTES_48;
			break;
		case 0x0F:
			m_dlc = FDCAN_DLC_BYTES_64;
			break;
		default:
			return false;
	}

	return true;
}

