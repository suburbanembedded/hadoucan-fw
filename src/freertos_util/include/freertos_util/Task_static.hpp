#pragma once

#include "freertos_util/Task_base.hpp"

#include <cstring>

template<size_t StackDepth>
class Task_static : public Task_base
{
public:

	Task_static()
	{
		m_tcb = {0};
	}

	~Task_static() override
	{

	}

	bool launch(const char* name, const UBaseType_t priority)
	{
		m_handle = xTaskCreateStatic(FreeRTOS_task_dispatch, name, StackDepth, this, priority, m_stack, &m_tcb);
		return true;
	}

protected:

#if portSTACK_GROWTH < 0
	StackType_t m_stack[StackDepth];
	StaticTask_t m_tcb;
#else
	StaticTask_t m_tcb;
	StackType_t m_stack[StackDepth];
#endif
};