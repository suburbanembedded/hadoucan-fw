#include "USB_rx_buffer_task.hpp"

#include "freertos_cpp_util/logging/Global_logger.hpp"

void USB_rx_buffer_task::work()
{
	freertos_util::logging::Logger* const logger = freertos_util::logging::Global_logger::get();
	using freertos_util::logging::LOG_LEVEL;

	for(;;)
	{
		{
			//wait for driver to have data on the OUT ep
			Buffer_adapter_base* in_buf = m_usb_driver->wait_rx_buffer(0x01);
			// in_buf->clean_invalidate_cache();
			// in_buf->invalidate_cache();

			logger->log(LOG_LEVEL::TRACE, "USB_rx_buffer_task", "got buf");

			volatile uint8_t* in_ptr = in_buf->data();
			{
				std::unique_lock<Mutex_static> lock(m_rx_buf_mutex);

				//if we are very full, wait for some space
				if((m_rx_buf.size() + in_buf->size()) > BUFFER_HIGH_WATERMARK)
				{
					do
					{
						m_rx_buf_read_condvar.wait(lock);
					} while((m_rx_buf.size() + in_buf->size()) > BUFFER_LOW_WATERMARK);
				}

				m_rx_buf.insert(m_rx_buf.end(), in_ptr, in_ptr + in_buf->size());
			}

			//return buffer to the driver
			logger->log(LOG_LEVEL::TRACE, "USB_rx_buffer_task", "release buf");
			m_usb_driver->release_rx_buffer(0x01, in_buf);
		}

		logger->log(LOG_LEVEL::TRACE, "USB_rx_buffer_task", "added buf to stream");

		m_rx_buf_write_condvar.notify_one();
	}
}