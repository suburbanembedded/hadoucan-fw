#include "USB_tx_buffer_task.hpp"

#include "uart1_printf.hpp"

constexpr size_t USB_tx_buffer_task::BUFFER_HIGH_WATERMARK;

constexpr uint32_t USB_tx_buffer_task::USB_HS_PACKET_WAIT_MS;

void USB_tx_buffer_task::work()
{
	std::vector<uint8_t> m_packet_buf;
	m_packet_buf.reserve(512);

	std::function<bool(void)> has_buffer_pred = std::bind(&USB_tx_buffer_task::has_buffer, this);

	for(;;)
	{
		m_packet_buf.clear();

		{
			std::unique_lock<Mutex_static> lock(m_tx_buf_mutex);
			m_tx_buf_condvar.wait_for(lock, std::chrono::milliseconds(USB_HS_PACKET_WAIT_MS), std::cref(has_buffer_pred));
			
			//might be empty if timed out
			if(!m_tx_buf.empty())
			{
				auto first = m_tx_buf.begin();
				auto last  = std::next(first, std::min<size_t>(512, m_tx_buf.size()));
				m_packet_buf.insert(m_packet_buf.begin(), first, last);

				m_tx_buf.erase(first, last);
			}
		}

		//we may have woke early and there is no data
		if(!m_packet_buf.empty())
		{
			//notify we drained some from the buffer
			m_tx_buf_drain_condvar.notify_one();

			uart1_log<64>(LOG_LEVEL::TRACE, "USB_tx_buffer_task", "Waiting for buffer");
			
			//wait for a free tx buffer from driver
			Buffer_adapter_base* tx_buf = m_usb_driver->wait_tx_buffer(0x81);

			uart1_log<64>(LOG_LEVEL::TRACE, "USB_tx_buffer_task", "Got buffer");

			tx_buf->reset();
			tx_buf->insert(m_packet_buf.data(), m_packet_buf.size());

			//give full tx buffer to the driver
			if(!m_usb_driver->enqueue_tx_buffer(0x81, tx_buf))
			{
				uart1_log<64>(LOG_LEVEL::ERROR, "USB_tx_buffer_task", "failed to enqueue tx buffer");
			}

			uart1_log<64>(LOG_LEVEL::TRACE, "USB_tx_buffer_task", "Sent buffer: %.*s", tx_buf->size(), tx_buf->data());
		}
	}
}
