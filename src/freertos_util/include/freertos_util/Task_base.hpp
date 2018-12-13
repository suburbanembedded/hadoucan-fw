#pragma once

#include "FreeRTOS.h"
#include "task.h"

extern "C"
{
	void FreeRTOS_task_dispatch(void* ctx);
}

class Task_base
{
public:

	Task_base();
	virtual ~Task_base();

	virtual void work();

protected:
	TaskHandle_t m_handle;
};