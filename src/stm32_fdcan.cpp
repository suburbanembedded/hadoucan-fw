#include "stm32_fdcan.hpp"

#include "uart1_printf.hpp"

#include <array>
#include <algorithm>
#include <stdexcept>

namespace
{
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
}

bool stm32_fdcan::init()
{
	HAL_StatusTypeDef ret = HAL_OK;

	m_hfdcan = FDCAN_HandleTypeDef();

	m_hfdcan.Instance = m_fdcan;
	m_hfdcan.Init.FrameFormat = FDCAN_FRAME_CLASSIC;
	m_hfdcan.Init.Mode = FDCAN_MODE_NORMAL;
	m_hfdcan.Init.AutoRetransmission = ENABLE;
	m_hfdcan.Init.TransmitPause = DISABLE;
	m_hfdcan.Init.ProtocolException = ENABLE;

	// 100 MHz CAN Clk
	// tq = NominalPrescaler x (1/fdcan_ker_ck)
	// tq = 25 x (1/100MHz) = 250ns
	// m_hfdcan.Init.NominalPrescaler = 5 - 1;//1-512 -1
	// m_hfdcan.Init.NominalSyncJumpWidth = 8 - 1;//1-128  -1
	// // NominalTimeSeg1 = Propagation_segment + Phase_segment_1
	// m_hfdcan.Init.NominalTimeSeg1 = 139 - 1;  //1-256 - 1
	// m_hfdcan.Init.NominalTimeSeg2 = 20  - 1;   //1-128 - 1

	m_hfdcan.Init.NominalPrescaler = 5;//1-512 -1
	m_hfdcan.Init.NominalSyncJumpWidth = 8;//1-128  -1
	// NominalTimeSeg1 = Propagation_segment + Phase_segment_1
	m_hfdcan.Init.NominalTimeSeg1 = 139;  //1-256 - 1
	m_hfdcan.Init.NominalTimeSeg2 = 20;   //1-128 - 1

	m_hfdcan.Init.DataPrescaler = 5-1;//1-32
	m_hfdcan.Init.DataSyncJumpWidth = 8 - 1;//1-16
	m_hfdcan.Init.DataTimeSeg1 = 32 - 1;//1-32
	m_hfdcan.Init.DataTimeSeg2 = 16  - 1;//1-16

	m_hfdcan.Init.MessageRAMOffset = 0;//0 - 2560
	m_hfdcan.Init.StdFiltersNbr = 1;
	m_hfdcan.Init.ExtFiltersNbr = 0;
	m_hfdcan.Init.RxFifo0ElmtsNbr = 1;
	m_hfdcan.Init.RxFifo0ElmtSize = FDCAN_DATA_BYTES_8;
	m_hfdcan.Init.RxFifo1ElmtsNbr = 0;
	m_hfdcan.Init.RxBuffersNbr = 0;

	m_hfdcan.Init.TxEventsNbr = 0;
	m_hfdcan.Init.TxBuffersNbr = 0;
	m_hfdcan.Init.TxFifoQueueElmtsNbr = 1;
	m_hfdcan.Init.TxFifoQueueMode = FDCAN_TX_FIFO_OPERATION;
	m_hfdcan.Init.TxElmtSize = FDCAN_DATA_BYTES_8;

	ret = HAL_FDCAN_Init(&m_hfdcan);
	if(ret != HAL_OK)
	{
		uart1_print<128>("[stm32_fdcan::init] HAL_FDCAN_Init failed\r\n");
		return false;
	}

	// Configure Rx filter
	FDCAN_FilterTypeDef sFilterConfig;
	sFilterConfig.IdType = FDCAN_STANDARD_ID;
	sFilterConfig.FilterIndex = 0;
	sFilterConfig.FilterType = FDCAN_FILTER_MASK;
	sFilterConfig.FilterConfig = FDCAN_FILTER_TO_RXFIFO0;
	sFilterConfig.FilterID1 = 0x321;
	sFilterConfig.FilterID2 = 0x7FF;
	ret = HAL_FDCAN_ConfigFilter(&m_hfdcan, &sFilterConfig);
	if(ret != HAL_OK)
	{
		uart1_print<128>("[stm32_fdcan::init] HAL_FDCAN_ConfigFilter failed\r\n");
		return false;
	}

	return true;
}

bool stm32_fdcan::open()
{
	HAL_StatusTypeDef ret = HAL_OK;

	if(!init())
	{
		uart1_print<128>("[stm32_fdcan::open] stm32_fdcan::init failed\r\n");
		return false;
	}

	ret = HAL_FDCAN_ActivateNotification(&m_hfdcan, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0);
	if(ret != HAL_OK)
	{
		uart1_print<128>("[stm32_fdcan::open] HAL_FDCAN_ActivateNotification failed\r\n");
		return false;
	}

	ret = HAL_FDCAN_Start(&m_hfdcan);
	if(ret != HAL_OK)
	{
		uart1_print<128>("[stm32_fdcan::open] HAL_FDCAN_Start failed\r\n");
		return false;
	}

	return true;
}
bool stm32_fdcan::close()
{
	if(HAL_FDCAN_DeactivateNotification(&m_hfdcan, FDCAN_IT_RX_FIFO0_NEW_MESSAGE) != HAL_OK)
	{
		return false;	
	}

	if(HAL_FDCAN_Stop(&m_hfdcan) != HAL_OK)
	{
		return false;
	}

	return true;
}

bool stm32_fdcan::tx_std(const uint32_t id, const uint8_t dlc, const uint8_t* data)
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

	if(HAL_FDCAN_AddMessageToTxFifoQ(&m_hfdcan, &tx_head, out_data.data()) != HAL_OK)
	{
		uart1_print<128>("[stm32_fdcan::tx_std] HAL_FDCAN_AddMessageToTxFifoQ failed\r\n");
		return false;
	}

	return true;
}

bool stm32_fdcan::tx_ext(const uint32_t id, const uint8_t dlc, const uint8_t* data)
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

	if(HAL_FDCAN_AddMessageToTxFifoQ(&m_hfdcan, &tx_head, out_data.data()) != HAL_OK)
	{
		uart1_print<128>("[stm32_fdcan::tx_ext] HAL_FDCAN_AddMessageToTxFifoQ failed\r\n");
		return false;
	}

	return true;
}

extern "C"
{
	void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs)
	{
		if((RxFifo0ITs & FDCAN_IT_RX_FIFO0_NEW_MESSAGE) != RESET)
		{
			FDCAN_RxHeaderTypeDef RxHeader;
			uint8_t RxData[8];

			if(HAL_FDCAN_GetRxMessage(hfdcan, FDCAN_RX_FIFO0, &RxHeader, RxData) != HAL_OK)
			{

			}

			if(HAL_FDCAN_ActivateNotification(hfdcan, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0) != HAL_OK)
			{

			}
		}
	}

	/*
	void HAL_FDCAN_RxFifo1Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs)
	{
		if((RxFifo0ITs & FDCAN_IT_RX_FIFO0_NEW_MESSAGE) != RESET)
		{
			FDCAN_RxHeaderTypeDef RxHeader;
			uint8_t RxData[8];

			if(HAL_FDCAN_GetRxMessage(hfdcan, FDCAN_RX_FIFO1, &RxHeader, RxData) != HAL_OK)
			{

			}

			if(HAL_FDCAN_ActivateNotification(hfdcan, FDCAN_IT_RX_FIFO1_NEW_MESSAGE, 0) != HAL_OK)
			{

			}
		}
	}
	*/

	// void HAL_FDCAN_ClockCalibrationCallback(FDCAN_HandleTypeDef *hfdcan, uint32_t ClkCalibrationITs);
	// void HAL_FDCAN_TxEventFifoCallback(FDCAN_HandleTypeDef *hfdcan, uint32_t TxEventFifoITs);
	// void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs);
	// void HAL_FDCAN_RxFifo1Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo1ITs);
	// void HAL_FDCAN_TxFifoEmptyCallback(FDCAN_HandleTypeDef *hfdcan);
	// void HAL_FDCAN_TxBufferCompleteCallback(FDCAN_HandleTypeDef *hfdcan, uint32_t BufferIndexes);
	// void HAL_FDCAN_TxBufferAbortCallback(FDCAN_HandleTypeDef *hfdcan, uint32_t BufferIndexes);
	// void HAL_FDCAN_RxBufferNewMessageCallback(FDCAN_HandleTypeDef *hfdcan);
	// void HAL_FDCAN_HighPriorityMessageCallback(FDCAN_HandleTypeDef *hfdcan);
	// void HAL_FDCAN_TimestampWraparoundCallback(FDCAN_HandleTypeDef *hfdcan);
	// void HAL_FDCAN_TimeoutOccurredCallback(FDCAN_HandleTypeDef *hfdcan);
	// void HAL_FDCAN_ErrorCallback(FDCAN_HandleTypeDef *hfdcan);
}