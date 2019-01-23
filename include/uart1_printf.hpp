#pragma once

#include "hal_inst.h"
#include "stm32h7xx_hal.h"

#include "freertos_cpp_util/Mutex_static.hpp"

#include <cstdarg>
#include <cinttypes>
#include <mutex>
#include <string>

extern Mutex_static m_uart1_mutex;

template<size_t LEN>
bool uart1_printf(const char* fmt, ...)
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

enum class LOG_LEVEL
{
	FATAL,
	ERROR,
	WARN,
	INFO,
	DEBUG,
	TRACE
};

const char* LOG_LEVEL_to_str(const LOG_LEVEL level);

template<size_t LEN>
bool uart1_log(const LOG_LEVEL level, const char* module_name, const char* fmt, ...)
{
	if(level > LOG_LEVEL::DEBUG)
	{
		return true;
	}

	std::array<char, LEN> m_buf;

	va_list args;
	va_start (args, fmt);
	int ret = vsnprintf(m_buf.data(), m_buf.size(), fmt, args);
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

	std::array<char, 8+2+1> time_str;
	time_str.fill(0);
{
	TickType_t tick_count = xTaskGetTickCount();
	static_assert(sizeof(TickType_t) <= sizeof(uint32_t));
	snprintf(time_str.data(), time_str.size(), "0x%08" PRIX32, uint32_t(tick_count));
}

	std::string s;
	s.reserve(time_str.size() + 6 + 10 + 20 + num_to_print);
	s.append("[");
	s.append(time_str.data());
	s.append("]");
	s.append("[");
	s.append(LOG_LEVEL_to_str(level));
	s.append("][");
	s.append(module_name);
	s.append("]");
	s.append(m_buf.data(), num_to_print);
	s.append("\r\n");

	HAL_StatusTypeDef uartret = HAL_OK;
	{
		std::lock_guard<Mutex_static> lock(m_uart1_mutex);
		uartret = HAL_UART_Transmit(&huart1, (uint8_t*)s.data(), s.size(), -1);
	}

	return uartret == HAL_OK;
}