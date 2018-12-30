#include "USB_RX_task.hpp"

#include "main.h"
#include "usb_device.h"
#include "usbd_cdc_if.h"

#include "hal_inst.hpp"
#include "uart1_printf.hpp"

#include "freertos_cpp_util/Critical_section.hpp"

#include <algorithm>
#include <cinttypes>

USB_RX_task::USB_RX_task()
{
	m_active_buf = nullptr;
}

void USB_RX_task::handle_init_callback()
{
	USBD_CDC_SetTxBuffer(&hUsbDeviceHS, nullptr, 0);
	USBD_CDC_SetRxBuffer(&hUsbDeviceHS, m_active_buf.load()->buf.data());

	m_init_complete.give_from_isr();
}

USB_RX_task::USB_rx_buf_ptr USB_RX_task::get_rx_buffer()
{
	USB_buf* usb_buffer = nullptr;
	while(!m_full_buffers.pop_front(&usb_buffer, portMAX_DELAY))
	{

	}

	return USB_rx_buf_ptr(usb_buffer);
}

void USB_RX_task::work()
{
	m_active_buf = rx_buf_pool.allocate();

	MX_USB_DEVICE_Init();

	m_init_complete.take();

	for(;;)
	{
		m_rx_complete.take();

		//if this is null, there was an underflow
		//alloc a new buffer and restart tx
		//normally the isr does this
		USB_buf* active_buf = m_active_buf.load();
		if(active_buf == nullptr)
		{
			//wait for a buffer
			do
			{
				active_buf = rx_buf_pool.try_allocate_for_ticks(pdMS_TO_TICKS(1000));

				if(!active_buf)
				{
					uart1_print<64>("rx could not alloc\r\n");
				}

			} while(!active_buf);

			//invalidate cache so when dma fills it we will read it
			active_buf->invalidate_cache();
			
			USBD_CDC_SetRxBuffer(&hUsbDeviceHS, active_buf->buf.data());

			m_active_buf.store(active_buf);
			USBD_CDC_ReceivePacket(&hUsbDeviceHS);		
		}
	}
}

int8_t USB_RX_task::handle_rx_callback(uint8_t* in_buf, uint32_t in_buf_len)
{
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;

	//cache the active ptr
	USB_buf* active_buf = m_active_buf.load();

	active_buf->len = in_buf_len;

	m_full_buffers.push_back_isr(active_buf, &xHigherPriorityTaskWoken);

	//if allocation fails, stall the host by not starting a new rx
	active_buf = rx_buf_pool.try_allocate_isr(&xHigherPriorityTaskWoken);
	if(active_buf)
	{
		//invalidate cache so when dma fills it we will read it
		active_buf->invalidate_cache();

		USBD_CDC_SetRxBuffer(&hUsbDeviceHS, active_buf->buf.data());
		USBD_CDC_ReceivePacket(&hUsbDeviceHS);		
	}
	else
	{
		//wake up our user thread
		//it will continue waiting for a buffer
		m_rx_complete.give_from_isr();
	}

	//update the active ptr
	m_active_buf.store(active_buf);

	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);

	return (USBD_OK);
}
