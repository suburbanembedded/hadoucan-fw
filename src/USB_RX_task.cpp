#include "USB_RX_task.hpp"

#include "main.h"

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
	m_init_complete.give_from_isr();
}

USB_RX_task::USB_rx_buf_ptr USB_RX_task::get_rx_buffer()
{
	USB_buf_rx* usb_buffer = nullptr;
	while(!m_full_buffers.pop_front(&usb_buffer, portMAX_DELAY))
	{

	}

	return USB_rx_buf_ptr(usb_buffer);
}

void USB_RX_task::work()
{
	m_active_buf = rx_buf_pool.allocate();

	m_init_complete.take();

	for(;;)
	{
		m_rx_complete.take();

		//if this is null, there was an underflow
		//alloc a new buffer and restart tx
		//normally the isr does this
		USB_buf_rx* active_buf = m_active_buf.load();
		if(active_buf == nullptr)
		{
			//wait for a buffer
			do
			{
				active_buf = rx_buf_pool.try_allocate_for_ticks(pdMS_TO_TICKS(1000));

				if(!active_buf)
				{
					uart1_log<128>(LOG_LEVEL::ERROR, "USB_RX_task", "Could not alloc rx buffer in 1000ms");
				}

			} while(!active_buf);

			//invalidate cache so when dma fills it we will read it
			// active_buf->clean_invalidate_cache();
			// active_buf->clean_cache();
		}
	}
}

int8_t USB_RX_task::handle_rx_callback(uint8_t* in_buf, uint32_t in_buf_len)
{
	return 0;
}
