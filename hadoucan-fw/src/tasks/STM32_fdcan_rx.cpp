#include "STM32_fdcan_rx.hpp"

#include "CAN_USB_app_config.hpp"
#include "global_app_inst.hpp"

#include "Task_instances.hpp"

#include "common_util/Byte_util.hpp"

#include "freertos_cpp_util/logging/Global_logger.hpp"
#include "freertos_cpp_util/Critical_section.hpp"

#include <string>

using freertos_util::logging::LOG_LEVEL;

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
	char get_hex_from_stm32_dlc(const uint32_t dlc)
	{
		switch(dlc)
		{
			case FDCAN_DLC_BYTES_0:
				return '0';
			case FDCAN_DLC_BYTES_1:
				return '1';
			case FDCAN_DLC_BYTES_2:
				return '2';
			case FDCAN_DLC_BYTES_3:
				return '3';
			case FDCAN_DLC_BYTES_4:
				return '4';
			case FDCAN_DLC_BYTES_5:
				return '5';
			case FDCAN_DLC_BYTES_6:
				return '6';
			case FDCAN_DLC_BYTES_7:
				return '7';
			case FDCAN_DLC_BYTES_8:
				return '8';
			case FDCAN_DLC_BYTES_12:
				return '9';
			case FDCAN_DLC_BYTES_16:
				return 'A';
			case FDCAN_DLC_BYTES_20:
				return 'B';
			case FDCAN_DLC_BYTES_24:
				return 'C';
			case FDCAN_DLC_BYTES_32:
				return 'D';
			case FDCAN_DLC_BYTES_48:
				return 'E';
			case FDCAN_DLC_BYTES_64:
				return 'F';
			default:
				throw std::domain_error("dlc not in bounds");
		}

		throw std::domain_error("dlc not in bounds");
	}
}

bool STM32_fdcan_rx::append_packet_type(const FDCAN_RxHeaderTypeDef& rxheader, std::string* const s)
{
	freertos_util::logging::Logger* const logger = freertos_util::logging::Global_logger::get();
		
	char cmd = '\0';

	if(rxheader.FDFormat == FDCAN_CLASSIC_CAN)
	{
		if(rxheader.RxFrameType == FDCAN_DATA_FRAME)
		{
			cmd = 't';
		}
		else if(rxheader.RxFrameType == FDCAN_REMOTE_FRAME)
		{
			cmd = 'r';
		}
	}
	else if(rxheader.FDFormat == FDCAN_FD_CAN)
	{
		if(rxheader.RxFrameType == FDCAN_DATA_FRAME)
		{
			if(rxheader.BitRateSwitch == FDCAN_BRS_ON)
			{
				cmd = 'b';
			}
			else
			{
				cmd = 'd';
			}
		}
	}

	const bool success = cmd != '\0';
	if(success)
	{
		if(rxheader.IdType == FDCAN_EXTENDED_ID)
		{
			//convert to uppercase for EXT ID
			cmd = Byte_util::ascii_to_upper(cmd);
		}
		
		s->push_back(cmd);
	}
	else
	{
		logger->log(LOG_LEVEL::TRACE, "STM32_fdcan_rx::append_packet_type", "Illegal frame");
	}

	return success;
}
bool STM32_fdcan_rx::append_packet_id(const FDCAN_RxHeaderTypeDef& rxheader, std::string* const s)
{
	const uint32_t x = rxheader.Identifier;

	if(rxheader.IdType == FDCAN_STANDARD_ID)
	{
		std::array<char, 3+1> id;

		Byte_util::nibble_to_hex(Byte_util::get_b1(x), id.data() + 0);
		Byte_util::u8_to_hex(Byte_util::get_b0(x), id.data() + 1);
		id.back() = '\0';

		s->append(id.data());
	}
	else if(rxheader.IdType == FDCAN_EXTENDED_ID)
	{
		std::array<char, 8+1> id;

		Byte_util::u8_to_hex(Byte_util::get_b3(x), id.data() + 0);
		Byte_util::u8_to_hex(Byte_util::get_b2(x), id.data() + 2);
		Byte_util::u8_to_hex(Byte_util::get_b1(x), id.data() + 4);
		Byte_util::u8_to_hex(Byte_util::get_b0(x), id.data() + 6);
		id.back() = '\0';

		s->append(id.data());
	}

	return true;
}
bool STM32_fdcan_rx::append_packet_dlc(const FDCAN_RxHeaderTypeDef& rxheader, std::string* const s)
{
	std::array<char, 1+1> str;

	str[0] = get_hex_from_stm32_dlc(rxheader.DataLength);
	str.back() = '\0';

	s->append(str.data());

	return true;
}
bool STM32_fdcan_rx::append_packet_data(const CAN_fd_packet& pk, std::string* const s)
{
	std::array<char, 2+1> str;

	const size_t byte_len = get_size_from_stm32_dlc(pk.rxheader.DataLength);
	for(size_t i = 0; i < byte_len; i++)
	{
		Byte_util::u8_to_hex(pk.data[i], str.data());
		str.back() = '\0';

		s->append(str.data());
	}

	return true;
}
bool STM32_fdcan_rx::append_packet_timestamp(const FDCAN_RxHeaderTypeDef& rxheader, std::string* const s)
{
	std::array<char, 4+1> str;

	Byte_util::u8_to_hex(Byte_util::get_b1(rxheader.RxTimestamp), str.data() + 0);
	Byte_util::u8_to_hex(Byte_util::get_b0(rxheader.RxTimestamp), str.data() + 2);
	str.back() = '\0';
	
	s->append(str.data());

	return true;
}
void STM32_fdcan_rx::work()
{
	freertos_util::logging::Logger* const logger = freertos_util::logging::Global_logger::get();
	using freertos_util::logging::LOG_LEVEL;

	CAN_fd_packet pk;

	std::string packet_str;
	packet_str.reserve(1+8+128+4+1);

	for(;;)
	{
		//if the fifo is empty, wait for the watermark interrupt or a timeout
		//once the timeout expires we will poll for packets under the watermark
		bool queue_set_pk = false;
		{
			//check for packets in fifo
			const uint32_t fifo0_depth = HAL_FDCAN_GetRxFifoFillLevel(m_fdcan_handle, FDCAN_RX_FIFO0);
			if(fifo0_depth == 0)
			{
				queue_set_pk = m_can_fd_queue.pop_front(&pk, pdMS_TO_TICKS(50));
			}
		}

		//check flags
		if((xTaskGetTickCount() - m_last_fifo_msg_lost_check) > pdMS_TO_TICKS(1000))
		{
			m_last_fifo_msg_lost_check = xTaskGetTickCount();
			unsigned int fifo0_msg_lost = 0;
			unsigned int fifo1_msg_lost = 0;
			{
				Critical_section crit_sec;

				fifo0_msg_lost = m_can_fifo0_msg_lost.load();
				fifo1_msg_lost = m_can_fifo1_msg_lost.load();
			}

			if(fifo0_msg_lost > 0)
			{
				m_can_fifo0_msg_lost.store(0);
				logger->log(LOG_LEVEL::ERROR, "STM32_fdcan_rx", "FIFO0 lost %d msg in the last second", fifo0_msg_lost);
			}
			if(fifo1_msg_lost > 0)
			{
				m_can_fifo1_msg_lost.store(false);
				logger->log(LOG_LEVEL::ERROR, "STM32_fdcan_rx", "FIFO1 lost %d msg in the last second", fifo1_msg_lost);
			}
		}

		//check PSR for RX errors
		const uint32_t cccr_reg = READ_REG(m_fdcan_handle->Instance->CCCR);
		if((cccr_reg & 0x00000001) == 0)//if not in init mode
		{
			const uint32_t psr_reg = READ_REG(m_fdcan_handle->Instance->PSR);

			const uint32_t dlec = (psr_reg & 0x00000700) >> 8;
			const uint32_t lec  = (psr_reg & 0x00000007) >> 0;
			
			switch(lec)
			{
				case 0:
				{
					break;
				}
				case 1:
				{
					logger->log(LOG_LEVEL::ERROR, "STM32_fdcan_rx", "lec stuff error");
					break;
				}
				case 2:
				{
					logger->log(LOG_LEVEL::ERROR, "STM32_fdcan_rx", "lec form error");
					break;
				}
				case 3:
				{
					logger->log(LOG_LEVEL::ERROR, "STM32_fdcan_rx", "lec ack error");
					break;
				}
				case 4:
				{
					logger->log(LOG_LEVEL::ERROR, "STM32_fdcan_rx", "lec bit1 error error");
					break;
				}
				case 5:
				{
					logger->log(LOG_LEVEL::ERROR, "STM32_fdcan_rx", "lec bit0 error error");
					break;
				}
				case 6:
				{
					logger->log(LOG_LEVEL::ERROR, "STM32_fdcan_rx", "lec crc error");
					break;
				}
				case 7:
				{
					break;
				}
			}
			switch(dlec)
			{
				case 0:
				{
					break;
				}
				case 1:
				{
					logger->log(LOG_LEVEL::ERROR, "STM32_fdcan_rx", "dlec stuff error");
					break;
				}
				case 2:
				{
					logger->log(LOG_LEVEL::ERROR, "STM32_fdcan_rx", "dlec form error");
					break;
				}
				case 3:
				{
					logger->log(LOG_LEVEL::ERROR, "STM32_fdcan_rx", "dlec ack error");
					break;
				}
				case 4:
				{
					logger->log(LOG_LEVEL::ERROR, "STM32_fdcan_rx", "dlec bit1 error error");
					break;
				}
				case 5:
				{
					logger->log(LOG_LEVEL::ERROR, "STM32_fdcan_rx", "dlec bit0 error error");
					break;
				}
				case 6:
				{
					logger->log(LOG_LEVEL::ERROR, "STM32_fdcan_rx", "dlec crc error");
					break;
				}
				case 7:
				{
					break;
				}
			}
		}

		//if we didn't sleep because we should poll, or we woke up due to timeout, check the fifo
		if(!queue_set_pk)
		{
			if(HAL_FDCAN_GetRxMessage(m_fdcan_handle, FDCAN_RX_FIFO0, &pk.rxheader, pk.data.data()) == HAL_OK)
			{
				//there was a packet
				//update led status
				led_task.notify_can_rx();
			}		
			else
			{
				//no data in hw fifo either, go back to sleep
				continue;
			}
		}


		//check more flags
		//re-enable fifo full after we deque to try and avoid isr race
		{
			bool fifo0_full = false;
			bool fifo1_full = false;
				
			{
				Critical_section crit_sec;

				fifo0_full = m_can_fifo0_full.load();
				fifo1_full = m_can_fifo1_full.load();
			}

			if(fifo0_full)
			{
				m_can_fifo0_full.store(false);
				logger->log(LOG_LEVEL::ERROR, "STM32_fdcan_rx", "FIFO0 full");

				if(HAL_FDCAN_ActivateNotification(m_fdcan_handle, FDCAN_IT_RX_FIFO0_FULL, 0) != HAL_OK)
				{
					logger->log(LOG_LEVEL::ERROR, "STM32_fdcan_rx", "Could not reenable FDCAN_IT_RX_FIFO0_FULL");
				}
			}

			if(fifo1_full)
			{
				m_can_fifo1_full.store(false);
				logger->log(LOG_LEVEL::ERROR, "STM32_fdcan_rx", "FIFO1 full");

				if(HAL_FDCAN_ActivateNotification(m_fdcan_handle, FDCAN_IT_RX_FIFO1_FULL, 0) != HAL_OK)
				{
					logger->log(LOG_LEVEL::ERROR, "STM32_fdcan_rx", "Could not reenable FDCAN_IT_RX_FIFO0_FULL");
				}
			}
		}

		//increment packet counter
		{
			m_local_rx_packet_counter++;
		}

		//stringify the packet
		packet_str.clear();
		if(!append_packet_type(pk.rxheader, &packet_str))
		{
			logger->log(LOG_LEVEL::ERROR, "STM32_fdcan_rx", "append_packet_type failed");
			continue;
		}
		if(!append_packet_id(pk.rxheader, &packet_str))
		{
			logger->log(LOG_LEVEL::ERROR, "STM32_fdcan_rx", "append_packet_id failed");
			continue;
		}
		if(!append_packet_dlc(pk.rxheader, &packet_str))
		{
			logger->log(LOG_LEVEL::ERROR, "STM32_fdcan_rx", "append_packet_dlc failed");
			continue;
		}
		if(!append_packet_data(pk, &packet_str))
		{
			logger->log(LOG_LEVEL::ERROR, "STM32_fdcan_rx", "append_packet_data failed");
			continue;
		}

		{
			//it seems rather expensive to get a copy of this every cycle
			bool append_timestamp = false;
			{
				std::unique_lock<Mutex_static_recursive> lock;
				append_timestamp = can_usb_app.get_config(&lock).timestamp_enable;
			}
			if(append_timestamp)
			{
				if(!append_packet_timestamp(pk.rxheader, &packet_str))
				{
					logger->log(LOG_LEVEL::ERROR, "STM32_fdcan_rx", "append_packet_timestamp failed");
					continue;
				}
			}
		}
		packet_str.push_back('\r');

		if(m_rx_callback)
		{
			//send the packet to a stream, that will try to coalesce a USB HS packet
			//m_usb_tx_buffer->write(packet_str.begin(), packet_str.end());
			if(!m_rx_callback(packet_str))
			{
				logger->log(LOG_LEVEL::ERROR, "STM32_fdcan_rx", "m_rx_callback failed");
			}
		}
		else
		{
			logger->log(LOG_LEVEL::ERROR, "STM32_fdcan_rx", "m_rx_callback is nullptr");
		}
	}
}

void STM32_fdcan_rx::can_fifo0_callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs)
{
	freertos_util::logging::Logger* const logger = freertos_util::logging::Global_logger::get();

	if((RxFifo0ITs & FDCAN_IT_RX_FIFO0_WATERMARK) != 0U)
	{
		//update led status
		led_task.notify_can_rx();

		//insert as many as we can
		//if we can't drain below the watermark, the isr will get called again...
		//TODO: in that case we turn off the isr and fall back to thread-mode drain, then re-enable
		CAN_fd_packet pk;
		while(!m_can_fd_queue.full_isr())
		{
			if(HAL_FDCAN_GetRxMessage(hfdcan, FDCAN_RX_FIFO0, &pk.rxheader, pk.data.data()) == HAL_OK)
			{
				//this should not fail, since we checked it was not full
				if(!m_can_fd_queue.push_back_isr(pk))
				{
					//TODO: assert? WD reset?
					logger->log(LOG_LEVEL::ERROR, "STM32_fdcan_rx", "can_fifo0_callback m_can_fd_queue push failed");
				}
			}
			else
			{
				//no more in fifo
				break;
			}
		}

		//turn isr back on
		if(HAL_FDCAN_ActivateNotification(hfdcan, FDCAN_IT_RX_FIFO0_WATERMARK, 0) != HAL_OK)
		{
			logger->log(LOG_LEVEL::ERROR, "STM32_fdcan_rx", "HAL_FDCAN_ActivateNotification FDCAN_IT_RX_FIFO0_WATERMARK failed");
		}
	}

	if((RxFifo0ITs & FDCAN_IT_RX_FIFO0_FULL) != 0U)
	{
		m_can_fifo0_full.store(true);
		//this is turned back on in the rx thread, after the fifo is drained
	}

	if((RxFifo0ITs & FDCAN_IT_RX_FIFO0_MESSAGE_LOST) != RESET)
	{
		m_can_fifo0_msg_lost++;
		if(HAL_FDCAN_ActivateNotification(hfdcan, FDCAN_IT_RX_FIFO0_MESSAGE_LOST, 0) != HAL_OK)
		{
			logger->log(LOG_LEVEL::ERROR, "STM32_fdcan_rx", "HAL_FDCAN_ActivateNotification FDCAN_IT_RX_FIFO0_MESSAGE_LOST failed");
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
