#include "Status_task.hpp"

#include "uart1_printf.hpp"

#include <vector>

Status_task::Status_task()
{

}

void Status_task::work()
{
	std::vector<TaskStatus_t> task_status_list;
	task_status_list.reserve(uxTaskGetNumberOfTasks());
		
	for(;;)
	{

		task_status_list.resize(uxTaskGetNumberOfTasks());
		UBaseType_t ret = uxTaskGetSystemState(task_status_list.data(), task_status_list.size(), nullptr);
		if(ret != task_status_list.size())
		{

		}

		uart1_log<128>(LOG_LEVEL::INFO, "Status_task", "");
		for(size_t i = 0; i < task_status_list.size(); i++)
		{
			uart1_printf<128>("\tTask %s, %s\r\n", task_status_list[i].pcTaskName, state_to_str(task_status_list[i].eCurrentState));
		}

		vTaskDelay(pdMS_TO_TICKS(500));
	}
}


const char* Status_task::state_to_str(const eTaskState state)
{
	char const * str = nullptr;
	switch(state)
	{
		case eReady:
		{
			str = "Ready";
			break;
		}
		case eRunning:
		{
			str = "Running";
			break;
		}
		case eBlocked:
		{
			str = "Blocked";
			break;	
		}
		case eSuspended:
		{
			str = "Suspended";
			break;
		}
		case eDeleted:
		{
			str = "Deleted";
			break;
		}
		default:
		{
			str = "Unknown";
			break;
		}
	}
		return str;
}