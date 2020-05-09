#pragma once

#include "freertos_cpp_util/Task_static.hpp"

extern "C"
{
	// configGENERATE_RUN_TIME_STATS
	// portCONFIGURE_TIMER_FOR_RUN_TIME_STATS
	// portGET_RUN_TIME_COUNTER_VALUE
	void freertos_config_runtime_stat_timer();
	uint32_t freertos_get_runtime_stat_timer();
}

class System_mon_task : public Task_static<1024>
{
public:
	void work() override;
protected:
	static const char* state_to_str(const eTaskState state);

	static constexpr bool compare_TaskStatus_t_less(const TaskStatus_t lhs, const TaskStatus_t rhs)
	{
		return lhs.xTaskNumber < rhs.xTaskNumber;
	}

	static constexpr uint32_t calculate_runtime_delta(const uint32_t a, const uint32_t b)
	{
		return b - a;
	}

};