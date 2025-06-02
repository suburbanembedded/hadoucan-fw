#include "tasks/USB_rx_buffer_task.hpp"

#include "Task_instances.hpp"

#include "freertos_cpp_util/logging/Global_logger.hpp"

using freertos_util::logging::LOG_LEVEL;

void USB_rx_buffer_task::work()
{
	freertos_util::logging::Logger* const logger = freertos_util::logging::Global_logger::get();

	logger->log(LOG_LEVEL::info, "USB_rx_buffer_task", "Starting");

	std::vector<uint8_t> in_buf;
	in_buf.reserve(512);

	for(;;)
	{
		{
			while( ! tud_cdc_n_available(0) )
			{
				usb_core_task.wait_for_usb_rx_avail();
			}

			in_buf.resize(512);
			uint32_t ret = tud_cdc_n_read(0, in_buf.data(), in_buf.size());
			in_buf.resize(ret);

			{
				std::unique_lock<Mutex_static> lock(m_rx_buf_mutex);

				//if we are very full, wait for some space
				if((m_rx_buf.size() + in_buf.size()) > BUFFER_HIGH_WATERMARK)
				{
					do
					{
						m_rx_buf_read_condvar.wait(lock);
					} while((m_rx_buf.size() + in_buf.size()) > BUFFER_LOW_WATERMARK);
				}

				m_rx_buf.insert(m_rx_buf.end(), in_buf.data(), in_buf.data() + in_buf.size());
			}
		}

		logger->log(LOG_LEVEL::trace, "USB_rx_buffer_task", "added buf to stream");

		m_rx_buf_write_condvar.notify_one();
	}
}
