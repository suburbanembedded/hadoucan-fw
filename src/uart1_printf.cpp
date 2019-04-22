#include "uart1_printf.hpp"

Mutex_static m_uart1_mutex;

const char* LOG_LEVEL_to_str(const LOG_LEVEL level)
{
	switch(level)
	{
		case LOG_LEVEL::FATAL:
			return "FATAL";
		case LOG_LEVEL::ERROR:
			return "ERROR";
		case LOG_LEVEL::WARN:
			return "WARN";
		case LOG_LEVEL::INFO:
			return "INFO";
		case LOG_LEVEL::DEBUG:
			return "DEBUG";
		case LOG_LEVEL::TRACE:
			return "TRACE";
		default:
			return "UNKNOWN";
	}

	return "UNKNOWN";
}

bool uart1_puts(const char* str)
{
	const size_t num_to_print = strlen(str);

	HAL_StatusTypeDef uartret;
	{
		std::lock_guard<Mutex_static> lock(m_uart1_mutex);
		uartret = HAL_UART_Transmit(&huart1, reinterpret_cast<uint8_t*>(const_cast<char*>(str)), num_to_print, -1);
	}

	return uartret == HAL_OK;
}