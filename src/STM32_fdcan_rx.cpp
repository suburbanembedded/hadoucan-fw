#include "STM32_fdcan_rx.hpp"

#include "uart1_printf.hpp"

#include "freertos_cpp_util/Critical_section.hpp"

#include <string>

STM32_fdcan_rx stm32_fdcan_rx_task;

namespace
{
uint32_t get_size_from_stm32_dlc(const uint32_t dlc)
{
	switch(dlc)
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
}

bool STM32_fdcan_rx::append_packet_type(const FDCAN_RxHeaderTypeDef& rxheader, std::string* const s)
{
	bool success = false;

	if(rxheader.FDFormat == FDCAN_CLASSIC_CAN)
	{
		if(rxheader.RxFrameType == FDCAN_DATA_FRAME)
		{
			if(rxheader.IdType == FDCAN_STANDARD_ID)
			{
				s->push_back('t');
				success = true;
			}
			else if(rxheader.IdType == FDCAN_EXTENDED_ID)
			{
				s->push_back('T');
				success = true;
			}
		}
		else if(rxheader.RxFrameType == FDCAN_REMOTE_FRAME)
		{
			if(rxheader.IdType == FDCAN_STANDARD_ID)
			{
				s->push_back('r');
				success = true;
			}
			else if(rxheader.IdType == FDCAN_EXTENDED_ID)
			{
				s->push_back('R');
				success = true;
			}
		}
	}
	else if(rxheader.FDFormat == FDCAN_FD_CAN)
	{
		if(rxheader.RxFrameType == FDCAN_DATA_FRAME)
		{
			if(rxheader.IdType == FDCAN_STANDARD_ID)
			{
				s->push_back('f');
				success = true;
			}
			else if(rxheader.IdType == FDCAN_EXTENDED_ID)
			{
				s->push_back('F');
				success = true;
			}
		}
		else if(rxheader.RxFrameType == FDCAN_REMOTE_FRAME)
		{
			if(rxheader.IdType == FDCAN_STANDARD_ID)
			{
				s->push_back('u');
				success = true;
			}
			else if(rxheader.IdType == FDCAN_EXTENDED_ID)
			{
				s->push_back('U');
				success = true;
			}
		}
	}

	return success;
}
bool STM32_fdcan_rx::append_packet_id(const FDCAN_RxHeaderTypeDef& rxheader, std::string* const s)
{
	const unsigned int x = rxheader.Identifier;

	if(rxheader.IdType == FDCAN_STANDARD_ID)
	{
		std::array<char, 3+1> id;

		int ret = snprintf(id.data(), id.size(), "%03X", x);
		
		if(ret < 0)
		{
			return false;
		}

		if(ret > 3)
		{
			return false;
		}

		s->append(id.data());
	}
	else if(rxheader.IdType == FDCAN_EXTENDED_ID)
	{
		std::array<char, 8+1> id;

		int ret = snprintf(id.data(), id.size(), "%08X", x);

		if(ret < 0)
		{
			return false;
		}
		
		if(ret > 8)
		{
			return false;
		}

		s->append(id.data());
	}

	return true;
}
bool STM32_fdcan_rx::append_packet_data(const CAN_fd_packet& pk, std::string* const s)
{
	size_t byte_len = get_size_from_stm32_dlc(pk.rxheader.DataLength);
	for(size_t i = 0; i < byte_len; i++)
	{
		std::array<char, 2+1> str;

		unsigned int x = pk.data[i];
		int ret = snprintf(str.data(), str.size(), "%02X", x);
		if(ret < 0)
		{
			uart1_log<64>(LOG_LEVEL::ERROR, "STM32_fdcan_rx", "snprintf failed");
			return false;
		}
		if(ret > 2)
		{
			uart1_log<64>(LOG_LEVEL::ERROR, "STM32_fdcan_rx", "snprintf truncated");
			return false;
		}

		s->append(str.data());
	}

	return true;
}

void STM32_fdcan_rx::work()
{
	CAN_fd_packet pk;

	std::string packet_str;
	packet_str.reserve(1+8+128+1);

	for(;;)
	{
		if(!m_usb_tx_buffer)
		{
			uart1_log<64>(LOG_LEVEL::ERROR, "STM32_fdcan_rx", "usb_tx_buffer is nullptr");
			vTaskDelay(pdMS_TO_TICKS(500));
			continue;	
		}

		//check for packets in fifo
		const uint32_t fifo0_depth = HAL_FDCAN_GetRxFifoFillLevel(m_fdcan_handle, FDCAN_RX_FIFO0);

		//if the fifo is empty, wait for the watermark interrupt or a timeout
		//once the timeout expires we will poll for packets under the watermark
		bool queue_set_pk = false;
		if(fifo0_depth == 0)
		{
			queue_set_pk = m_can_fd_queue.pop_front(&pk, pdMS_TO_TICKS(50));
		}

		{
			Critical_section crit_sec;

			if(m_can_fifo0_full.load())
			{
				m_can_fifo0_full.store(false);
				uart1_log<64>(LOG_LEVEL::ERROR, "STM32_fdcan_rx", "FIFO0 full");
			}
			if(m_can_fifo0_msg_lost.load())
			{
				m_can_fifo0_msg_lost.store(false);
				uart1_log<64>(LOG_LEVEL::ERROR, "STM32_fdcan_rx", "FIFO0 msg lost");
			}
			if(m_can_fifo1_full.load())
			{
				m_can_fifo1_full.store(false);
				uart1_log<64>(LOG_LEVEL::ERROR, "STM32_fdcan_rx", "FIFO1 full");
			}
			if(m_can_fifo1_msg_lost.load())
			{
				m_can_fifo1_msg_lost.store(false);
				uart1_log<64>(LOG_LEVEL::ERROR, "STM32_fdcan_rx", "FIFO1 msg lost");
			}

			// uint32_t RxFifo0ITs = m_RxFifo0ITs.load();
			// uart1_log<64>(LOG_LEVEL::TRACE, "STM32_fdcan_rx", "RxFifo0ITs is %08" PRIX32, RxFifo0ITs);
			// uart1_log<64>(LOG_LEVEL::TRACE, "STM32_fdcan_rx", "FDCAN_IT_RX_FIFO0_WATERMARK is %08" PRIX32, FDCAN_IT_RX_FIFO0_WATERMARK);
			// uart1_log<64>(LOG_LEVEL::TRACE, "STM32_fdcan_rx", "FDCAN_IT_RX_FIFO0_FULL is %08" PRIX32, FDCAN_IT_RX_FIFO0_FULL);
			// uart1_log<64>(LOG_LEVEL::TRACE, "STM32_fdcan_rx", "FDCAN_IT_RX_FIFO0_MESSAGE_LOST is %08" PRIX32, FDCAN_IT_RX_FIFO0_MESSAGE_LOST);
			// m_RxFifo0ITs.store(0);
		}

		//if we didn't sleep because we should poll, or we woke up due to timeout, check the fifo
		if(!queue_set_pk)
		{
			if(HAL_FDCAN_GetRxMessage(m_fdcan_handle, FDCAN_RX_FIFO0, &pk.rxheader, pk.data.data()) != HAL_OK)
			{
				//no data in hw fifo either, go back to sleep
				continue;
			}			
		}

		packet_str.clear();
		if(!append_packet_type(pk.rxheader, &packet_str))
		{
			uart1_log<64>(LOG_LEVEL::ERROR, "STM32_fdcan_rx", "append_packet_type failed");
			continue;
		}
		if(!append_packet_id(pk.rxheader, &packet_str))
		{
			uart1_log<64>(LOG_LEVEL::ERROR, "STM32_fdcan_rx", "append_packet_id failed");
			continue;
		}
		if(!append_packet_data(pk, &packet_str))
		{
			uart1_log<64>(LOG_LEVEL::ERROR, "STM32_fdcan_rx", "append_packet_data failed");
			continue;
		}
		packet_str.push_back('\r');

		m_usb_tx_buffer->write(packet_str.begin(), packet_str.end());
	}
}

void STM32_fdcan_rx::can_fifo0_callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs)
{
	m_RxFifo0ITs.store(RxFifo0ITs);

	if((RxFifo0ITs & FDCAN_IT_RX_FIFO0_WATERMARK) != 0U)
	{
		HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_13);

		//insert as many as we can
		//if we can't drain below the watermark, the isr will get called again...
		//TODO: in that case we turn off the isr and fall back to thread-mode drain, then re-enable
		CAN_fd_packet pk;
		while(!m_can_fd_queue.full_isr())
		{
			if(HAL_FDCAN_GetRxMessage(hfdcan, FDCAN_RX_FIFO0, &pk.rxheader, pk.data.data()) != HAL_OK)
			{
				//this should not fail, since we checked it was not full
				if(!m_can_fd_queue.push_back_isr(pk))
				{
					HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_12);
				}
			}
			else
			{
				break;
			}
		}

		//turn isr back on
		if(HAL_FDCAN_ActivateNotification(hfdcan, FDCAN_IT_RX_FIFO0_WATERMARK, 0) != HAL_OK)
		{

		}
	}

	if((RxFifo0ITs & FDCAN_IT_RX_FIFO0_FULL) != 0U)
	{
		m_can_fifo0_full.store(true);
		if(HAL_FDCAN_ActivateNotification(hfdcan, FDCAN_IT_RX_FIFO0_FULL, 0) != HAL_OK)
		{

		}
	}

	if((RxFifo0ITs & FDCAN_IT_RX_FIFO0_MESSAGE_LOST) != RESET)
	{
		m_can_fifo0_msg_lost.store(true);
		if(HAL_FDCAN_ActivateNotification(hfdcan, FDCAN_IT_RX_FIFO0_MESSAGE_LOST, 0) != HAL_OK)
		{

		}
	}
}
#if 0
void STM32_fdcan_rx::can_fifo1_callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo1ITs)
{
	if((RxFifo1ITs & FDCAN_IT_RX_FIFO1_WATERMARK) != RESET)
	{
		HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_15);

		STM32_fdcan_rx::CAN_fd_packet pk;

		if(HAL_FDCAN_GetRxMessage(hfdcan, FDCAN_RX_FIFO1, &pk.rxheader, pk.data.data()) == HAL_OK)
		{
			if(!insert_packet_isr(pk))
			{
				HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_14);
			}
		}
		else
		{
			HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_14);
		}

		if(HAL_FDCAN_ActivateNotification(hfdcan, FDCAN_IT_RX_FIFO1_WATERMARK, 0) != HAL_OK)
		{

		}
	}

	if((RxFifo1ITs & FDCAN_IT_RX_FIFO1_FULL) != RESET)
	{
		m_can_fifo1_full.store(true);
		if(HAL_FDCAN_ActivateNotification(hfdcan, FDCAN_IT_RX_FIFO1_FULL, 0) != HAL_OK)
		{
		
		}
	}

	if((RxFifo1ITs & FDCAN_IT_RX_FIFO1_MESSAGE_LOST) != RESET)
	{
		m_can_fifo1_msg_lost.store(true);
		if(HAL_FDCAN_ActivateNotification(hfdcan, FDCAN_IT_RX_FIFO1_MESSAGE_LOST, 0) != HAL_OK)
		{

		}
	}
}
#endif

extern "C"
{
	void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef* hfdcan, uint32_t RxFifo0ITs)
	{
		stm32_fdcan_rx_task.can_fifo0_callback(hfdcan, RxFifo0ITs);
	}
	
	#if 0
	void HAL_FDCAN_RxFifo1Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo1ITs)
	{
		stm32_fdcan_rx_task.can_fifo1_callback(hfdcan, RxFifo1ITs);
	}
	#endif
	
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
