#include "uart1_printf.hpp"

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