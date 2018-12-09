#include "FreeRTOS_task_base.hpp"

extern "C"
{
	void FreeRTOS_task_dispatch(void* ctx)
	{
		FreeRTOS_task_base* inst = static_cast<FreeRTOS_task_base*>(ctx);

		inst->work();
	}
}

FreeRTOS_task_base::FreeRTOS_task_base()
{
	m_handle = nullptr;
}

FreeRTOS_task_base::~FreeRTOS_task_base()
{
	if(m_handle)
	{
		vTaskDelete(m_handle);
		m_handle = nullptr;
	}
}

void FreeRTOS_task_base::work()
{
	for(;;)
	{
		vTaskSuspend(nullptr);
	}
}
