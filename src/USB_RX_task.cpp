#include "USB_RX_task.hpp"

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

void USB_RX_task::work()
{
	m_active_buf = m_rx_buf_pool.allocate();

	MX_USB_DEVICE_Init();

	m_init_complete.take();

	for(;;)
	{
		RX_buf* usb_buffer = nullptr;
		if(!m_full_buffers.pop_front(&usb_buffer, portMAX_DELAY))
		{
			continue;
		}

		m_rx_buf_pool.deallocate(usb_buffer);

		//if this is null, there was an underflow
		//alloc a new buffer and restart tx
		//normally the isr does this
		RX_buf* active_buf = m_active_buf.load();
		if(active_buf == nullptr)
		{
			//this will succeed since we just deleted one
			active_buf = m_rx_buf_pool.allocate();
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
	RX_buf* active_buf = m_active_buf.load();

	active_buf->len = in_buf_len;

	m_full_buffers.push_back_isr(active_buf, &xHigherPriorityTaskWoken);

	//if allocation fails, stall the host by not starting a new rx
	active_buf = m_rx_buf_pool.try_allocate_isr(&xHigherPriorityTaskWoken);
	if(active_buf)
	{
		USBD_CDC_SetRxBuffer(&hUsbDeviceHS, active_buf->buf.data());
		USBD_CDC_ReceivePacket(&hUsbDeviceHS);		
	}

	//update the active ptr
	m_active_buf.store(active_buf);

	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);

	return (USBD_OK);
}
