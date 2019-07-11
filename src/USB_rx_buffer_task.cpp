#include "USB_rx_buffer_task.hpp"

#include "uart1_printf.hpp"

void USB_rx_buffer_task::work()
{
	for(;;)
	{
		{
			//wait for driver to have data on the OUT ep
			Buffer_adapter_base* in_buf = m_usb_driver->wait_rx_buffer(0x01);
			// in_buf->clean_invalidate_cache();
			// in_buf->invalidate_cache();

			uart1_log<64>(LOG_LEVEL::INFO, "USB_rx_buffer_task", "got buf");

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
			m_usb_driver->release_rx_buffer(0x01, in_buf);
		}

		uart1_log<64>(LOG_LEVEL::INFO, "USB_rx_buffer_task", "added buf to stream");

		m_rx_buf_write_condvar.notify_one();
	}
}