#include "CAN_DLC.hpp"

#include <stdexcept>

size_t CAN_DLC::to_len() const
{
	switch(m_dlc)
	{
		case 0x0:
			return 0;
		case 0x1: 
			return 1;
		case 0x2: 
			return 2;
		case 0x3:
			return 3;
		case 0x4:
			return 4;
		case 0x5:
			return 5;
		case 0x6:
			return 6;
		case 0x7:
			return 7;
		case 0x8:
			return 8;
		case 0x9:
			return 12;
		case 0xA:
			return 16;
		case 0xB:
			return 20;
		case 0xC:
			return 24;
		case 0xD:
			return 32;
		case 0xE:
			return 48;
		case 0xF:
			return 64;
		default:
			throw std::domain_error("dlc not in bounds");
	}

	throw std::domain_error("dlc not in bounds");
}
bool CAN_DLC::from_len(uint8_t len)
{
	switch(len)
	{
		case 0:
			m_dlc = 0x00;
			break;
		case 1: 
			m_dlc = 0x01;
			break;
		case 2: 
			m_dlc = 0x02;
			break;
		case 3:
			m_dlc = 0x03;
			break;
		case 4:
			m_dlc = 0x04;
			break;
		case 5:
			m_dlc = 0x05;
			break;
		case 6:
			m_dlc = 0x06;
			break;
		case 7:
			m_dlc = 0x07;
			break;
		case 8:
			m_dlc = 0x08;
			break;
		case 12:
			m_dlc = 0x09;
			break;
		case 16:
			m_dlc = 0x0A;
			break;
		case 20:
			m_dlc = 0x0B;
			break;
		case 24:
			m_dlc = 0x0C;
			break;
		case 32:
			m_dlc = 0x0D;
			break;
		case 48:
			m_dlc = 0x0E;
			break;
		case 64:
			m_dlc = 0x0F;
			break;
		default:
			return false;
	}

	return true;
}

bool CAN_DLC::from_ascii(char c)
{
	switch(c)
	{
		case '0':
			m_dlc = 0;
			break;
		case '1': 
			m_dlc = 1;
			break;
		case '2': 
			m_dlc = 2;
			break;
		case '3':
			m_dlc = 3;
			break;
		case '4':
			m_dlc = 4;
			break;
		case '5':
			m_dlc = 5;
			break;
		case '6':
			m_dlc = 6;
			break;
		case '7':
			m_dlc = 7;
			break;
		case '8':
			m_dlc = 8;
			break;
		case '9':
			m_dlc = 12;
			break;
		case 'A':
			m_dlc = 16;
			break;
		case 'B':
			m_dlc = 20;
			break;
		case 'C':
			m_dlc = 24;
			break;
		case 'D':
			m_dlc = 32;
			break;
		case 'E':
			m_dlc = 48;
			break;
		case 'F':
			m_dlc = 64;
			break;
		default:
			return false;
	}

	return true;
}
