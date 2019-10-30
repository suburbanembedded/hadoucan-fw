#pragma once

#include "freertos_cpp_util/Task_static.hpp"

#include <vector>

class System_mon_task
{
public:
	void work() override
	{
		for(;;)
		{	
			uxTaskGetNumberOfTasks()
			std::vector<TaskStatus_t> tasks_a;
			unsigned long runtime_a;
			const UBaseType_t reta = uxTaskGetSystemState(tasks_a.data(), tasks_a.size(), &runtime_a);

			vTaskDelay(pdMS_to_TICKS(1000));

			std::vector<TaskStatus_t> tasks_b;
			unsigned long runtime_b;
			const UBaseType_t retb = uxTaskGetSystemState(tasks_b.data(), tasks_b.size(), &runtime_b)

			for(size_t i = 0; i < tasks_a.size(); i++)
			{
				const TaskHandle_t curr_handle = tasks_a[i].xHandle;

				bool found_match = false;
				for(size_t j = 0; j < tasks_b.size(); j++)
				{
					if(curr_handle == tasks_a[j].xHandle)
					{
						found_match = true;

						// TaskHandle_t xHandle;
						// const signed char *pcTaskName;
						// UBaseType_t xTaskNumber;
						// eTaskState eCurrentState;
						// UBaseType_t uxCurrentPriority;
						// UBaseType_t uxBasePriority;
						// unsigned long ulRunTimeCounter;
						// StackType_t *pxStackBase;
						// configSTACK_DEPTH_TYPE usStackHighWaterMark;
					}
				}

				if(!found_match)
				{
					//log failure
				}
			}
		}
	}
};