#pragma once

#include "FreeRTOS_task_base.hpp"

#include <cstring>

template<size_t StackDepth>
class FreeRTOS_static_task : public FreeRTOS_task_base
{
public:

	FreeRTOS_static_task()
	{
		memset(&m_tcb, 0, sizeof(m_tcb));
	}

	~FreeRTOS_static_task() override
	{

	}

	bool launch(const char* name, const UBaseType_t priority)
	{
		m_handle = xTaskCreateStatic(FreeRTOS_task_dispatch, name, StackDepth, this, priority, m_stack, &m_tcb);
		return true;
	}

protected:

	StaticTask_t m_tcb;
	StackType_t m_stack[StackDepth];

};