#include "freertos_util/Task_heap.hpp"

bool Task_heap::launch(const char* name, const size_t StackDepth, const UBaseType_t priority)
{
	BaseType_t ret = xTaskCreate(FreeRTOS_task_dispatch, name, StackDepth, this, priority, &m_handle);

	return ret == pdPASS;
}