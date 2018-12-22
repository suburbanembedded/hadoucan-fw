#include "freertos_util/Task_base.hpp"

extern "C"
{
	void FreeRTOS_task_dispatch(void* ctx)
	{
		Task_base* inst = static_cast<Task_base*>(ctx);

		inst->work();

		//in case work returns, remove us from the runnable list
		//we could also self-delete, but on some combination of port and newlib that crashes
		for(;;)
		{
			vTaskSuspend(nullptr);
		}
	}
}

Task_base::Task_base()
{
	m_handle = nullptr;
}

Task_base::~Task_base()
{
	if(m_handle)
	{
		vTaskDelete(m_handle);
		m_handle = nullptr;
	}
}

void Task_base::work()
{
	for(;;)
	{
		vTaskSuspend(nullptr);
	}
}
