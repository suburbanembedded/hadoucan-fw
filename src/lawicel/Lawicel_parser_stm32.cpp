#include "Lawicel_parser_stm32.hpp"

#include <algorithm>
#include <stdexcept>

uint32_t stm32_get_dlc_from_dlc(const uint8_t dlc)
{
	switch(dlc)
	{
		case 0x0:
			return FDCAN_DLC_BYTES_0;
		case 0x1: 
			return FDCAN_DLC_BYTES_1;
		case 0x2: 
			return FDCAN_DLC_BYTES_2;
		case 0x3:
			return FDCAN_DLC_BYTES_3;
		case 0x4:
			return FDCAN_DLC_BYTES_4;
		case 0x5:
			return FDCAN_DLC_BYTES_5;
		case 0x6:
			return FDCAN_DLC_BYTES_6;
		case 0x7:
			return FDCAN_DLC_BYTES_7;
		case 0x8:
			return FDCAN_DLC_BYTES_8;
		case 0x9:
			return FDCAN_DLC_BYTES_12;
		case 0xA:
			return FDCAN_DLC_BYTES_16;
		case 0xB:
			return FDCAN_DLC_BYTES_20;
		case 0xC:
			return FDCAN_DLC_BYTES_24;
		case 0xD:
			return FDCAN_DLC_BYTES_32;
		case 0xE:
			return FDCAN_DLC_BYTES_48;
		case 0xF:
			return FDCAN_DLC_BYTES_64;
		default:
			throw std::domain_error("dlc not in bounds");
	}

	throw std::domain_error("dlc not in bounds");
}

bool Lawicel_parser_stm32::write_string(const char* out_str)
{
	return false;
}

bool Lawicel_parser_stm32::handle_std_baud(const uint8_t baud)
{
	return false;
}
bool Lawicel_parser_stm32::handle_cust_baud(const uint8_t b0, const uint8_t b1)
{
	return false;
}
bool Lawicel_parser_stm32::handle_open()
{
	return false;
}
bool Lawicel_parser_stm32::handle_close()
{
	return false;
}
bool Lawicel_parser_stm32::handle_tx_std(const uint32_t id, const uint8_t dlc, const uint8_t* data)
{
	FDCAN_TxHeaderTypeDef tx_head;

	tx_head.Identifier = id;
	tx_head.IdType = FDCAN_STANDARD_ID;
	tx_head.TxFrameType = FDCAN_DATA_FRAME;
	tx_head.DataLength = stm32_get_dlc_from_dlc(dlc);
	tx_head.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
	tx_head.BitRateSwitch = FDCAN_BRS_OFF;
	tx_head.FDFormat = FDCAN_CLASSIC_CAN;
	tx_head.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
	tx_head.MessageMarker = 0;

	std::array<uint8_t, 8> out_data;
	std::copy_n(data, dlc, out_data.begin());

	if(HAL_FDCAN_AddMessageToTxFifoQ(m_fdcan, &tx_head, out_data.data()) != HAL_OK)
	{
		return false;
	}

	return true;
}
bool Lawicel_parser_stm32::handle_tx_ext(const uint32_t id, const uint8_t dlc, const uint8_t* data)
{
	FDCAN_TxHeaderTypeDef tx_head;

	tx_head.Identifier = id;
	tx_head.IdType = FDCAN_EXTENDED_ID;
	tx_head.TxFrameType = FDCAN_DATA_FRAME;
	tx_head.DataLength = stm32_get_dlc_from_dlc(dlc);
	tx_head.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
	tx_head.BitRateSwitch = FDCAN_BRS_OFF;
	tx_head.FDFormat = FDCAN_CLASSIC_CAN;
	tx_head.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
	tx_head.MessageMarker = 0;

	std::array<uint8_t, 8> out_data;
	std::copy_n(data, dlc, out_data.begin());

	if(HAL_FDCAN_AddMessageToTxFifoQ(m_fdcan, &tx_head, out_data.data()) != HAL_OK)
	{
		return false;
	}

	return true;
}
bool Lawicel_parser_stm32::handle_tx_rtr_std(const uint32_t id, const uint8_t dlc, const uint8_t* data)
{
	FDCAN_TxHeaderTypeDef tx_head;

	tx_head.Identifier = id;
	tx_head.IdType = FDCAN_STANDARD_ID;
	tx_head.TxFrameType = FDCAN_REMOTE_FRAME;
	tx_head.DataLength = stm32_get_dlc_from_dlc(dlc);
	tx_head.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
	tx_head.BitRateSwitch = FDCAN_BRS_OFF;
	tx_head.FDFormat = FDCAN_CLASSIC_CAN;
	tx_head.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
	tx_head.MessageMarker = 0;

	std::array<uint8_t, 8> out_data;
	std::copy_n(data, dlc, out_data.begin());

	if(HAL_FDCAN_AddMessageToTxFifoQ(m_fdcan, &tx_head, out_data.data()) != HAL_OK)
	{
		return false;
	}

	return true;
}
bool Lawicel_parser_stm32::handle_tx_rtr_ext(const uint32_t id, const uint8_t dlc, const uint8_t* data)
{
	FDCAN_TxHeaderTypeDef tx_head;

	tx_head.Identifier = id;
	tx_head.IdType = FDCAN_EXTENDED_ID;
	tx_head.TxFrameType = FDCAN_REMOTE_FRAME;
	tx_head.DataLength = stm32_get_dlc_from_dlc(dlc);
	tx_head.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
	tx_head.BitRateSwitch = FDCAN_BRS_OFF;
	tx_head.FDFormat = FDCAN_CLASSIC_CAN;
	tx_head.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
	tx_head.MessageMarker = 0;

	std::array<uint8_t, 8> out_data;
	std::copy_n(data, dlc, out_data.begin());

	if(HAL_FDCAN_AddMessageToTxFifoQ(m_fdcan, &tx_head, out_data.data()) != HAL_OK)
	{
		return false;
	}

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
bool Lawicel_parser_stm32::handle_get_version(std::array<uint8_t, 4>* ver)
{
	return false;
}
bool Lawicel_parser_stm32::handle_get_serial(std::array<uint8_t, 4>* sn)
{
	return false;
}
bool Lawicel_parser_stm32::handle_set_timestamp(bool enable)
{
	return false;
}

