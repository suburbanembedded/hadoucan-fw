#include "USB_lawicel_task.hpp"

#include "tusb.h"

#include <vector>
#include <algorithm>

#include <cstring>

USB_lawicel_task::USB_lawicel_task()
{
	m_usb_rx_buffer = nullptr;
}

bool USB_lawicel_task::usb_input_drop(uint8_t c)
{
	switch(c)
	{
		// case '\r':
		// {
		// 	return true;
		// }
		case '\n':
		{
			return true;
		}
		default:
		{
			return false;
		}
	}

	return false;
}

void USB_lawicel_task::set_can_tx(STM32_fdcan_tx* const can_tx)
{
	m_can = can_tx;
}

void USB_lawicel_task::set_usb_rx(USB_rx_buffer_task* const usb_rx_buffer)
{
	m_usb_rx_buffer = usb_rx_buffer;

	m_has_line_pred = std::bind(&USB_rx_buffer_task::has_line, m_usb_rx_buffer);
}

bool USB_lawicel_task::write_string_usb(const char* str)
{
	const uint32_t num_to_write = strlen(str);
	const uint32_t ret = tud_cdc_n_write(0, str, num_to_write);

	if(ret)
	{
		tud_cdc_n_write_flush(0);
	}

	return ret == num_to_write;
}

void USB_lawicel_task::work()
{
	freertos_util::logging::Logger* const logger = freertos_util::logging::Global_logger::get();
	using freertos_util::logging::LOG_LEVEL;

	m_parser.set_can(m_can);
	m_parser.set_write_string_func(
		std::bind(&USB_lawicel_task::write_string_usb, this, std::placeholders::_1)
		);

	//a null terminated string
	//maybe using std::string is a better idea, or a stringstream....
	std::vector<uint8_t> usb_line;
	usb_line.reserve(128);

	for(;;)
	{
		{
			logger->log(LOG_LEVEL::TRACE, "USB_lawicel_task", "wait(lock, has_line_pred)");
			std::unique_lock<Mutex_static> lock(m_usb_rx_buffer->get_mutex());
			m_usb_rx_buffer->get_cv().wait(lock, std::cref(m_has_line_pred));
			logger->log(LOG_LEVEL::TRACE, "USB_lawicel_task", "woke");

			if(!m_usb_rx_buffer->get_line(&usb_line))
			{
				continue;
			}
		}
		//we unlock lock so buffering can continue

		//drop what usb_input_drop says we should drop
		auto end_it = std::remove_if(usb_line.begin(), usb_line.end(), &usb_input_drop);
		usb_line.erase(end_it, usb_line.end());

		//drop lines that are now empty
		if(strnlen((char*)usb_line.data(), usb_line.size()) == 0)
		{
			logger->log(LOG_LEVEL::WARN, "USB_lawicel_task", "Empty line");
			continue;
		}

		//drop lines that are only '\r'
		if(usb_line.front() == '\r')
		{
			logger->log(LOG_LEVEL::WARN, "USB_lawicel_task", "Line only contains \\r");
			continue;
		}

		logger->log(LOG_LEVEL::TRACE, "USB_lawicel_task", "got line: [%s]", usb_line.data());

		//process line
		if(!m_parser.parse_string((char*)usb_line.data()))
		{
			logger->log(LOG_LEVEL::ERROR, "USB_lawicel_task", "parse error");
		}
		else
		{
			logger->log(LOG_LEVEL::TRACE, "USB_lawicel_task", "ok");
		}
	}
}
