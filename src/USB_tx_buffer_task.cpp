#include "USB_tx_buffer_task.hpp"

#include "uart1_printf.hpp"

void USB_tx_buffer_task::work()
{
	std::vector<uint8_t> m_packet_buf;
	m_packet_buf.reserve(512);

	std::function<bool(void)> has_buffer_pred = std::bind(&USB_tx_buffer_task::has_buffer, this);

	for(;;)
	{
		m_packet_buf.clear();

		//wait for 50ms max
		vTaskSetTimeOutState(&m_tx_timeout);
		m_tx_timeout_ticks_left = pdMS_TO_TICKS(50);

		{
			std::unique_lock<Mutex_static> lock(m_tx_buf_mutex);
			m_tx_buf_condvar.wait(lock, std::cref(has_buffer_pred));
			
			auto first = m_tx_buf.begin();
			auto last  = std::next(first, std::min<size_t>(512, m_tx_buf.size()));
			m_packet_buf.insert(m_packet_buf.begin(), first, last);

			m_tx_buf.erase(first, last);
		}

		m_usb_tx_task->queue_buffer_blocking(m_packet_buf.data(), m_packet_buf.size());
	}
}
