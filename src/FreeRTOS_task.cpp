#include "FreeRTOS_task.hpp"

bool FreeRTOS_task::launch(const char* name, const size_t StackDepth, const UBaseType_t priority)
{
	BaseType_t ret = xTaskCreate(FreeRTOS_task_dispatch, name, StackDepth, this, priority, &m_handle);

	return ret == pdPASS;
}