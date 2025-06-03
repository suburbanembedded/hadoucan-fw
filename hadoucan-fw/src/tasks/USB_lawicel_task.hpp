#pragma once

#include "USB_rx_buffer_task.hpp"

#include "tasks/STM32_fdcan_rx.hpp"
#include "STM32_fdcan_tx.hpp"

#include "lawicel/Lawicel_parser.hpp"
#include "lawicel/Lawicel_parser_stm32.hpp"

#include "freertos_cpp_util/Task_static.hpp"
#include "freertos_cpp_util/logging/Global_logger.hpp"

#include "stm32h7xx_hal.h"

class USB_lawicel_task : public Task_static<2048>
{
public:

	USB_lawicel_task();

	static bool usb_input_drop(uint8_t c);

	void set_can_tx(STM32_fdcan_tx* const can_tx);
	void set_usb_rx(USB_rx_buffer_task* const usb_rx_buffer);
	bool write_string_usb(const char* str);

	void work() override;

	STM32_fdcan_tx* m_can;

	Lawicel_parser* get_lawicel()
	{
		return &m_parser;
	}

protected:

	Lawicel_parser_stm32 m_parser;

	USB_rx_buffer_task* m_usb_rx_buffer;

	std::function<bool(void)> m_has_line_pred;
};
