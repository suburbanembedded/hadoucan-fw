#include "USB_TX_task.hpp"

#include "main.h"
#include "hal_inst.h"
#include "usb_device.h"
#include "usbd_cdc_if.h"

#include "uart1_printf.hpp"

#include <algorithm>
#include <cinttypes>

USB_TX_task::USB_TX_task()
{
	m_tx_idle.give();

	//don't need to send this until we've sent data for the first time
	m_needs_send_null = false;
}

void USB_TX_task::handle_init_callback()
{
	m_init_complete.give_from_isr();
}

void USB_TX_task::work()
{
	m_init_complete.take();
	
	for(;;)
	{
		// uart1_print<64>("tx wait buf\r\n");

		USB_buf_tx* usb_buffer = nullptr;
		if(!m_pending_tx_buffers.pop_front(&usb_buffer, pdMS_TO_TICKS(USB_HS_PACKET_WAIT_MS)))
		{
			//if we previously sent data, and don't have more to send we should send a null packet
			//this is a hint to the OS to flush buffers / hand data to the application
			if(m_needs_send_null)
			{
				uart1_log<128>(LOG_LEVEL::TRACE, "USB_TX_task", "tx idle for 50ms, send null buf");
				const uint8_t ret = send_buffer(nullptr);

				if(ret != USBD_OK)
				{
					switch(ret)
					{
						case USBD_BUSY:
						{
							uart1_log<128>(LOG_LEVEL::ERROR, "USB_TX_task", "send null: USBD_BUSY");
							break;
						}
						case USBD_FAIL:
						{
							uart1_log<128>(LOG_LEVEL::ERROR, "USB_TX_task", "send null: USBD_FAIL");
							break;
						}
						default:
						{
							uart1_log<128>(LOG_LEVEL::ERROR, "USB_TX_task", "send null: unk");
							break;	
						}
					}
				}

				m_needs_send_null = false;
			}
			continue;
		}

		const uint8_t ret = send_buffer(usb_buffer);
		m_needs_send_null = true;
		if(ret != USBD_OK)
		{
			switch(ret)
			{
				case USBD_BUSY:
				{
					uart1_log<128>(LOG_LEVEL::ERROR, "USB_TX_task", "send usb_buffer: USBD_BUSY");
					break;
				}
				case USBD_FAIL:
				{
					uart1_log<128>(LOG_LEVEL::ERROR, "USB_TX_task", "send usb_buffer: USBD_FAIL");
					break;
				}
				default:
				{
					uart1_log<128>(LOG_LEVEL::ERROR, "USB_TX_task", "send usb_buffer: unk");
					break;	
				}
			}
		}

		Object_pool_base<USB_buf_tx>::free(usb_buffer);
	}
}

void USB_TX_task::notify_tx_complete_callback()
{
	m_tx_idle.give_from_isr();
}

bool USB_TX_task::is_tx_complete()
{
	return 0 < m_tx_idle.get_count();
}

void USB_TX_task::wait_tx_finish()
{
	#if 0
	USBD_CDC_HandleTypeDef *hcdc = (USBD_CDC_HandleTypeDef*)hUsbDeviceHS.pClassData;
	while(hcdc->TxState != 0)
	{
		m_tx_idle.take();
		m_tx_idle.give();
	}
	#else
	while(!m_tx_idle.try_take_for_ticks(pdMS_TO_TICKS(1000)))
	{
		uart1_log<128>(LOG_LEVEL::WARN, "USB_TX_task", "m_tx_idle take took longer than 1000ms, retry");
	}
	m_tx_idle.give();
	#endif
}

size_t USB_TX_task::queue_buffer_blocking(const uint8_t* buf, const size_t len)
{
	size_t num_queued = 0;

	while(num_queued < len)
	{
		USB_buf_tx* usb_buf = nullptr;
		do
		{
			usb_buf = tx_buf_pool.try_allocate_for_ticks(portMAX_DELAY);
		} while(usb_buf == nullptr);
		
		const size_t num_to_copy = std::min(len - num_queued, usb_buf->buf.size());
		std::copy_n(buf + num_queued, num_to_copy, usb_buf->buf.data() + num_queued);
		usb_buf->len = num_to_copy;
	
		if(!m_pending_tx_buffers.push_back(usb_buf))
		{
			//return it to the queue, and spin around to try again
			uart1_log<128>(LOG_LEVEL::WARN, "USB_TX_task", "m_pending_tx_buffers enqueue failed, retry");
			Object_pool_base<USB_buf_tx>::free(usb_buf);
			continue;
		}
	
		num_queued += num_to_copy;		
	}

	return num_queued;
}

size_t USB_TX_task::queue_buffer(const uint8_t* buf, const size_t len, const TickType_t xTicksToWait)
{
	size_t num_queued = 0;

	TimeOut_t tx_timeout;
	vTaskSetTimeOutState(&tx_timeout);
	TickType_t ticks_left = xTicksToWait;

	while(num_queued < len)
	{
		USB_buf_tx* usb_buf = tx_buf_pool.try_allocate_for_ticks(ticks_left);
		if(!usb_buf)
		{
			uart1_log<128>(LOG_LEVEL::ERROR, "USB_TX_task", "Could not alloc tx buffer in %" PRIu32 " ticks", uint32_t(xTicksToWait));
			return num_queued;
		}

		//update wait time for if we need to loop
		xTaskCheckForTimeOut(&tx_timeout, &ticks_left);
		
		const size_t num_to_copy = std::min(len - num_queued, usb_buf->buf.size());
		std::copy_n(buf + num_queued, num_to_copy, usb_buf->buf.data() + num_queued);
		usb_buf->len = num_to_copy;
	
		//commit to sram so dma can see it
		// usb_buf->clean_cache();

		m_pending_tx_buffers.push_back(usb_buf);
	
		num_queued += num_to_copy;		
	}

	return num_queued;
}

uint8_t USB_TX_task::send_buffer(USB_buf_tx* const buf)
{
	wait_tx_finish();
    
	m_tx_idle.take();

	//we may need to mask the usb interrupt here based on a race condition with the usb rx interrupt
	//https://github.com/GrumpyOldPizza/arduino-STM32L4/blob/master/system/STM32L4xx/Source/stm32l4_usbd_cdc.c
	//ttps://community.st.com/s/question/0D50X00009XkYM6SAN/stm32-usb-library-naks-out-packets-from-host
	//NVIC_DisableIRQ(USB_IRQn);
	// HAL_NVIC_DisableIRQ(OTG_HS_IRQn);
	asm volatile(
		"cpsid i\n"
		"dsb 0xF\n"
		"isb 0xF\n"
		: /* no out */
		: /* no in */
		: "memory"
	);
	
    if(buf)
    {
    	//ensure the store buffer is flushed before the next instruction
    	//the usb buffers are in non-cachable but buffered memory
    	__DSB();

		USBD_CDC_SetTxBuffer(&hUsbDeviceHS, buf->buf.data(), buf->len);
    }
    else
    {
		USBD_CDC_SetTxBuffer(&hUsbDeviceHS, nullptr, 0);
    }

	const uint8_t ret = USBD_CDC_TransmitPacket(&hUsbDeviceHS);

	asm volatile(
		"dsb 0xF\n"
		"isb 0xF\n"
		"cpsie i\n"
		"isb 0xF\n"
		: /* no out */
		: /* no in */
		: "memory"
	);
	//we may need to restore the usb interrupt here based on a race condition with the usb rx interrupt
	//NVIC_EnableIRQ(USB_IRQn);
	// HAL_NVIC_EnableIRQ(OTG_HS_IRQn);

	if(ret != USBD_OK)
	{
		if(ret == USBD_FAIL)
		{
			uart1_log<128>(LOG_LEVEL::ERROR, "USB_TX_task", "USBD_CDC_TransmitPacket returned USBD_FAIL");
		}
		else
		{
			uart1_log<128>(LOG_LEVEL::ERROR, "USB_TX_task", "USBD_CDC_TransmitPacket returned unk error");
		}
	}

	if(ret == USBD_OK)
	{
		//write sync
		wait_tx_finish();
	}

	return ret;
}