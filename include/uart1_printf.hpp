#pragma once

#include "hal_inst.hpp"
#include "stm32h7xx_hal.h"

#include "freertos_cpp_util/Mutex_static.hpp"

#include <cstdarg>
#include <mutex>

template<size_t LEN>
bool uart1_print(const char* fmt, ...)
{
	std::array<uint8_t, LEN> m_buf;

	va_list args;
	va_start (args, fmt);
	int ret = vsnprintf(reinterpret_cast<char*>(m_buf.data()), m_buf.size(), fmt, args);
	va_end(args);

	if(ret < 0)	
	{
		return false;
	}

	size_t num_to_print = 0;
	if(ret > 0)	
	{
		num_to_print = std::min<size_t>(ret, m_buf.size()-1);
	}

	HAL_StatusTypeDef uartret;
	{
		std::lock_guard<Mutex_static> lock(m_uart1_mutex);
		uartret = HAL_UART_Transmit(&huart1, m_buf.data(), num_to_print, -1);
	}

	return uartret == HAL_OK;
}