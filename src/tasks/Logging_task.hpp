#pragma once

#include "freertos_cpp_util/Task_static.hpp"
#include "freertos_cpp_util/logging/Logger.hpp"

class UART1_sink : public freertos_util::logging::Log_sink_base
{
public:
	bool handle_log(freertos_util::logging::String_type* const log) override;

protected:
};

class Logging_task : public Task_static<2048>
{
public:
	void work() override;
protected:
	UART1_sink m_sink;

	freertos_util::logging::Logger m_logs;
};
