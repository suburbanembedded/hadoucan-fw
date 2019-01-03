#include "STM32_fdcan_rx.hpp"

#include "uart1_printf.hpp"

STM32_fdcan_rx stm32_fdcan_rx_task;

void STM32_fdcan_rx::work()
{
	CAN_fd_packet pk;

	for(;;)
	{
		if(!m_can_fd_queue.pop_front(&pk))
		{
			continue;
		}

		if(!m_usb_tx_buffer)
		{
			continue;	
		}
	}
}

bool STM32_fdcan_rx::insert_packet_isr(CAN_fd_packet& pk)
{
	return m_can_fd_queue.push_back_isr(pk);
}

extern "C"
{
	void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs)
	{
		if((RxFifo0ITs & FDCAN_IT_RX_FIFO0_NEW_MESSAGE) != RESET)
		{
			STM32_fdcan_rx::CAN_fd_packet pk;

			if(HAL_FDCAN_GetRxMessage(hfdcan, FDCAN_RX_FIFO0, &pk.rxheader, pk.data.data()) == HAL_OK)
			{
				stm32_fdcan_rx_task.insert_packet_isr(pk);
			}
			else
			{

			}

			if(HAL_FDCAN_ActivateNotification(hfdcan, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0) == HAL_OK)
			{

			}
			else
			{

			}
		}
	}
	
	void HAL_FDCAN_RxFifo1Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo1ITs)
	{
		if((RxFifo1ITs & FDCAN_IT_RX_FIFO1_NEW_MESSAGE) != RESET)
		{
			STM32_fdcan_rx::CAN_fd_packet pk;

			if(HAL_FDCAN_GetRxMessage(hfdcan, FDCAN_RX_FIFO1, &pk.rxheader, pk.data.data()) == HAL_OK)
			{
				stm32_fdcan_rx_task.insert_packet_isr(pk);
			}
			else
			{

			}

			if(HAL_FDCAN_ActivateNotification(hfdcan, FDCAN_IT_RX_FIFO1_NEW_MESSAGE, 0) == HAL_OK)
			{

			}
			else
			{

			}
		}
	}
	
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
