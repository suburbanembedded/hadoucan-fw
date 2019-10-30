#pragma once

#include "freertos_cpp_util/Task_static.hpp"
#include "freertos_cpp_util/logging/Logger.hpp"

class UART1_sink : public freertos_util::logging::Log_sink_base
{
public:
	bool handle_log(freertos_util::logging::String_type* const log) override;

protected:
};

// extern freertos_util::logging::Logger global_logs;

class Logging_task : public Task_static<1024>
{
public:
	Logging_task()
	{
		get_logger().set_sink(&m_sink);
	}

	void work() override;

	freertos_util::logging::Logger& get_logger()
	{
		return global_logs;
	}

protected:

	freertos_util::logging::Logger global_logs;

	UART1_sink m_sink;
};
