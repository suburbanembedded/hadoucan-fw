#include "USB_rx_buffer_task.hpp"

#include "uart1_printf.hpp"

void USB_rx_buffer_task::work()
{
	for(;;)
	{
		USB_RX_task::USB_rx_buf_ptr in_buf = m_usb_rx_task->get_rx_buffer();
		// in_buf->clean_invalidate_cache();
		// in_buf->invalidate_cache();

		uart1_log<64>(LOG_LEVEL::TRACE, "USB_rx_buffer_task", "got buf");

		volatile uint8_t* in_ptr = in_buf->buf.data();
		{
			std::unique_lock<Mutex_static> lock(m_rx_buf_mutex);
			m_rx_buf.insert(m_rx_buf.end(), in_ptr, in_ptr + in_buf->len);
		}

		uart1_log<64>(LOG_LEVEL::TRACE, "USB_rx_buffer_task", "added buf to stream");

		m_rx_buf_condvar.notify_one();
	}
}