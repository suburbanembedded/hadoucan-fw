#include "USB_rx_buffer_task.hpp"

#include "freertos_cpp_util/logging/Global_logger.hpp"

#include "tusb.h"

void USB_rx_buffer_task::work()
{
	while( ! tud_cdc_n_ready(0) )
	{
		vTaskDelay(pdMS_TO_TICKS(250));
	}

	freertos_util::logging::Logger* const logger = freertos_util::logging::Global_logger::get();
	using freertos_util::logging::LOG_LEVEL;

	std::vector<uint8_t> m_packet_buf;
	m_packet_buf.reserve(512);

	for(;;)
	{
		{
			m_packet_buf.resize(512);
			uint32_t ret = tud_cdc_n_read(0, m_packet_buf.data(), m_packet_buf.size());
			m_packet_buf.resize(ret);

			volatile uint8_t* in_ptr = m_packet_buf.data();
			if(m_packet_buf.size())
			{
				std::unique_lock<Mutex_static> lock(m_rx_buf_mutex);

				//if we are very full, wait for some space
				if((m_rx_buf.size() + m_packet_buf.size()) > BUFFER_HIGH_WATERMARK)
				{
					do
					{
						m_rx_buf_read_condvar.wait(lock);
					} while((m_rx_buf.size() + m_packet_buf.size()) > BUFFER_LOW_WATERMARK);
				}

				m_rx_buf.insert(m_rx_buf.end(), in_ptr, in_ptr + m_packet_buf.size());
			}
		}

		logger->log(LOG_LEVEL::TRACE, "USB_rx_buffer_task", "added buf to stream");

		m_rx_buf_write_condvar.notify_one();
	}
}