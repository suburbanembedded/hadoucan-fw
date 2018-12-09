#pragma once

#include "FreeRTOS.h"
#include "task.h"

class FreeRTOS_task_base;

extern "C"
{
	void FreeRTOS_task_dispatch(void* ctx);
}

class FreeRTOS_task_base
{
public:

	FreeRTOS_task_base();
	virtual ~FreeRTOS_task_base();

	virtual void work();

protected:
	TaskHandle_t m_handle;
};