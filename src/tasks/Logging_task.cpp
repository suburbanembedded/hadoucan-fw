#include "Logging_task.hpp"

#include "uart1_printf.hpp"

bool UART1_sink::handle_log(freertos_util::logging::String_type* const log)
{
	return uart1_puts(log->c_str());
}

void Logging_task::work()
{
	m_logs.set_sink(&m_sink);

	for(;;)
	{
		m_logs.process_one();
	}
}