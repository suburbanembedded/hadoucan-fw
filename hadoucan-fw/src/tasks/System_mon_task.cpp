#include "System_mon_task.hpp"
#include "Task_instances.hpp"

#include "common_util/Stack_string.hpp"
#include "common_util/Insertion_sort.hpp"

#include "stm32h7xx_hal.h"
#include "stm32h7xx_hal_tim.h"

#include <cinttypes>

#include <vector>

extern "C"
{
	// configGENERATE_RUN_TIME_STATS
	// portCONFIGURE_TIMER_FOR_RUN_TIME_STATS
	// portGET_RUN_TIME_COUNTER_VALUE

	TIM_HandleTypeDef htim2;

	//TIM2 or TIM5, APB1
	void freertos_config_runtime_stat_timer()
	{
		__HAL_RCC_TIM2_CLK_ENABLE();
		
		htim2 = TIM_HandleTypeDef();

		const uint32_t apb1_freq = HAL_RCC_GetPCLK1Freq();
		const uint32_t apb1_1MHz_pre = (apb1_freq / 1000000) - 1;
   
		htim2.Instance = TIM2;
		htim2.Init.Period = 0xFFFFFFFF;
		htim2.Init.Prescaler = apb1_1MHz_pre;
		htim2.Init.ClockDivision = 0;
		htim2.Init.CounterMode = TIM_COUNTERMODE_UP;

		if(HAL_TIM_Base_Init(&htim2) != HAL_OK)
		{
			//abort...
		}
		HAL_TIM_Base_Start_IT(&htim2);
	}
	uint32_t freertos_get_runtime_stat_timer()
	{
		// return 0xFFFFFF - SysTick->VAL;
		return __HAL_TIM_GET_COUNTER(&htim2);
	}
}

const char* System_mon_task::state_to_str(const eTaskState state)
{
	switch(state)
	{
		case eReady:
		{
			return "Ready";
			break;
		}
		case eRunning:
		{
			return "Running";
			break;
		}
		case eBlocked:
		{
			return "Blocked";
			break;
		}
		case eSuspended:
		{
			return "Suspended";
			break;
		}
		case eDeleted:
		{
			return "Deleted";
			break;
		}
		default:
		{
			return "Unknown";
			break;
		}
	}

	return "Unknown";
}

void System_mon_task::work()
{
	std::vector<TaskStatus_t> tasks_a;
	std::vector<TaskStatus_t> tasks_b;

	Stack_string<128> task_msg;

	for(;;)
	{	
		tasks_a.resize(uxTaskGetNumberOfTasks());

		uint32_t runtime_a = 0;
		const UBaseType_t reta = uxTaskGetSystemState(tasks_a.data(), tasks_a.size(), &runtime_a);
		if(reta == 0)
		{
			continue;
		}

		vTaskDelay(pdMS_TO_TICKS(5000));

		tasks_b.resize(uxTaskGetNumberOfTasks());
		uint32_t runtime_b = 0;
		const UBaseType_t retb = uxTaskGetSystemState(tasks_b.data(), tasks_b.size(), &runtime_b);
		if(retb == 0)
		{
			continue;
		}

		//could sort or index to speed this up
		insertion_sort(tasks_a.begin(), tasks_a.end(), &System_mon_task::compare_TaskStatus_t_less);
		insertion_sort(tasks_b.begin(), tasks_b.end(), &System_mon_task::compare_TaskStatus_t_less);

		for(size_t i = 0; i < tasks_a.size(); i++)
		{
			const TaskHandle_t curr_handle = tasks_a[i].xHandle;

			bool found_match = false;
			for(size_t j = 0; j < tasks_b.size(); j++)
			{
				if(curr_handle != tasks_b[j].xHandle)
				{
					continue;
				}

				found_match = true;

				TaskStatus_t const * const prev_state = &tasks_a[i];
				TaskStatus_t const * const curr_state = &tasks_b[j];

				const uint32_t cpu_dt = calculate_runtime_delta(runtime_a, runtime_b);
				const uint32_t task_dt = curr_state->ulRunTimeCounter - prev_state->ulRunTimeCounter;

				const float task_dt_per = float(task_dt) / float(cpu_dt) * 100.0f;

				task_msg.clear();
				task_msg.sprintf("name: %s, id: %u, state: %s, runtime: %" PRIu32 ", run_per: %.2f, stack_depth: %" PRIu32,
					curr_state->pcTaskName,
					curr_state->xTaskNumber,
					state_to_str(curr_state->eCurrentState),
					task_dt,
					task_dt_per,
					uint32_t(curr_state->usStackHighWaterMark)
					);

				// Stack_string<128> msg4;

				if(logging_task.get_logger().log(freertos_util::logging::LOG_LEVEL::INFO, "SysMon", "%s", task_msg.c_str()))
				{
					// msg4.append("System_mon_task::work log PASS\r\n");
					// HAL_UART_Transmit(&huart1, reinterpret_cast<uint8_t*>(const_cast<char*>(msg4.c_str())), msg4.size(), -1);
				}
				else
				{
					// msg4.append("System_mon_task::work log FAIL\r\n");
					// HAL_UART_Transmit(&huart1, reinterpret_cast<uint8_t*>(const_cast<char*>(msg4.c_str())), msg4.size(), -1);
				}

				// HAL_UART_Transmit(&huart1, reinterpret_cast<uint8_t*>(const_cast<char*>(task_msg.c_str())), task_msg.size(), -1);

				// msg4.clear();
				// msg4.append("\r\n");
				// HAL_UART_Transmit(&huart1, reinterpret_cast<uint8_t*>(const_cast<char*>(msg4.c_str())), msg4.size(), -1);

				// msg4.clear();
				// msg4.sprintf("m_record_buffer\r\n\tsize: %" PRIu32 "\r\n\tcap: %" PRIu32 "\r\n\tfull: %s\r\n\tempty: %s\r\n", 
				// 	logging_task.get_logger().m_record_buffer.size(),
				// 	logging_task.get_logger().m_record_buffer.capacity(),
				// 	logging_task.get_logger().m_record_buffer.full()  ? "true" : "false",
				// 	logging_task.get_logger().m_record_buffer.empty() ? "true" : "false"
				// 	);
				// HAL_UART_Transmit(&huart1, reinterpret_cast<uint8_t*>(const_cast<char*>(msg4.c_str())), msg4.size(), -1);

				// TaskHandle_t xHandle;
				// const signed char *pcTaskName;
				// UBaseType_t xTaskNumber;
				// eTaskState eCurrentState;
				// UBaseType_t uxCurrentPriority;
				// UBaseType_t uxBasePriority;
				// uint32_t ulRunTimeCounter;
				// StackType_t *pxStackBase;
				// configSTACK_DEPTH_TYPE usStackHighWaterMark;
				break;
			}

			if(!found_match)
			{
				//log failure
			}
		}

		task_msg.clear();
		task_msg.sprintf("heap used: %" PRIu32 " / %" PRIu32, configTOTAL_HEAP_SIZE - xPortGetFreeHeapSize(), configTOTAL_HEAP_SIZE);
		if(logging_task.get_logger().log(freertos_util::logging::LOG_LEVEL::INFO, "SysMon", "%s", task_msg.c_str()))
		{

		}
		else
		{

		}
	}
}
