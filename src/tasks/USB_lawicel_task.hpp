#pragma once

#include "uart1_printf.hpp"

#include "USB_rx_buffer_task.hpp"
#include "USB_tx_buffer_task.hpp"

#include "tasks/STM32_fdcan_rx.hpp"
#include "STM32_fdcan_tx.hpp"

#include "lawicel/Lawicel_parser.hpp"
#include "lawicel/Lawicel_parser_stm32.hpp"

#include "freertos_cpp_util/Task_static.hpp"

#include "stm32h7xx_hal.h"

#include <cstring>

#include <algorithm>
#include <vector>

class USB_lawicel_task : public Task_static<2048>
{
public:

	USB_lawicel_task()
	{
		m_usb_tx_buffer = nullptr;
	}

	static bool usb_input_drop(uint8_t c)
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

	void set_can_tx(STM32_fdcan_tx* const can_tx)
	{
		m_can = can_tx;
	}

	void set_usb_tx(USB_tx_buffer_task* const usb_tx_buffer)
	{
		m_usb_tx_buffer = usb_tx_buffer;
	}

	bool write_string_usb(const char* str)
	{
		m_usb_tx_buffer->write(str);
		return true;
	}

	void set_usb_rx(USB_rx_buffer_task* const usb_rx_buffer)
	{
		m_usb_rx_buffer = usb_rx_buffer;

		m_has_line_pred = std::bind(&USB_rx_buffer_task::has_line, m_usb_rx_buffer);
	}

	void work() override
	{
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
				uart1_log<64>(LOG_LEVEL::TRACE, "USB_lawicel_task", "wait(lock, has_line_pred)");
				std::unique_lock<Mutex_static> lock(m_usb_rx_buffer->get_mutex());
				m_usb_rx_buffer->get_cv().wait(lock, std::cref(m_has_line_pred));
				uart1_log<64>(LOG_LEVEL::TRACE, "USB_lawicel_task", "woke");

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
				uart1_log<64>(LOG_LEVEL::WARN, "USB_lawicel_task", "Empty line");
				continue;
			}

			//drop lines that are only '\r'
			if(usb_line.front() == '\r')
			{
				uart1_log<64>(LOG_LEVEL::WARN, "USB_lawicel_task", "Line only contains \\r");
				continue;
			}

			uart1_log<64>(LOG_LEVEL::TRACE, "USB_lawicel_task", "got line: [%s]", usb_line.data());

			//process line
			if(!m_parser.parse_string((char*)usb_line.data()))
			{
				uart1_log<64>(LOG_LEVEL::ERROR, "USB_lawicel_task", "parse error");
			}
			else
			{
				uart1_log<64>(LOG_LEVEL::TRACE, "USB_lawicel_task", "ok");
			}
		}
	}

	STM32_fdcan_tx* m_can;

	Lawicel_parser* get_lawicel()
	{
		return &m_parser;
	}

protected:

	Lawicel_parser_stm32 m_parser;

	USB_tx_buffer_task* m_usb_tx_buffer;
	USB_rx_buffer_task* m_usb_rx_buffer;

	std::function<bool(void)> m_has_line_pred;
};
