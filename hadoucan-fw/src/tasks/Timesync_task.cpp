#include "Timesync_task.hpp"

#include "main.h"
#include "global_app_inst.hpp"

#include "freertos_cpp_util/logging/Global_logger.hpp"

#include "hal_inst.h"
#include "stm32h7xx_hal.h"

using freertos_util::logging::LOG_LEVEL;

void Timesync_task::work()
{
	freertos_util::logging::Logger* const logger = freertos_util::logging::Global_logger::get();

	CAN_USB_app_config::TIMESYNC_MODE m_timesync_mode = CAN_USB_app_config::TIMESYNC_MODE::SLAVE;

	{
		std::unique_lock<Mutex_static_recursive> config_lock;
		const CAN_USB_app_config::Config_Set& m_config = can_usb_app.get_config(&config_lock);

		m_timesync_mode = m_config.timesync_mode;
	}

	__HAL_RCC_TIM3_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();

	switch(m_timesync_mode)
	{
		case CAN_USB_app_config::TIMESYNC_MODE::MASTER:
		{
			logger->log(LOG_LEVEL::info, "Timesync_task", "Timesync master mode");
			if(!oc_config())
			{
				logger->log(LOG_LEVEL::error, "Timesync_task", "oc_config failed");
			}
			break;
		}
		case CAN_USB_app_config::TIMESYNC_MODE::SLAVE:
		{
			logger->log(LOG_LEVEL::info, "Timesync_task", "Timesync slave mode");
			if(!ic_config())
			{
				logger->log(LOG_LEVEL::error, "Timesync_task", "ic_config failed");
			}
			break;
		}
		default:
		{
			logger->log(LOG_LEVEL::error, "Timesync_task", "unknown time sync mode, disabling");
			break;
		}
	}

	
	for(;;)
	{
		vTaskSuspend(nullptr);
	}
}

bool Timesync_task::oc_config()
{
	// 100 MHz input clock
	// 2000 prescaler -> 20us per tick
	// 50000 counts -> 1s overflow

	htim3.Instance = TIM3;
	htim3.Init.Prescaler = 2000;
	htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim3.Init.Period = 50000;
	htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
	if(HAL_TIM_Base_Init(&htim3) != HAL_OK)
	{
		return false;
	}

	TIM_ClockConfigTypeDef sClockSourceConfig = {0};
	sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
	if(HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
	{
		return false;
	}

	TIM_MasterConfigTypeDef sMasterConfig = {0};
	sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	if(HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
	{
		return false;
	}

	HAL_TIM_Base_Start(&htim3);

	if(HAL_TIM_PWM_Init(&htim3) != HAL_OK)
	{
		return false;
	}

	TIM_OC_InitTypeDef sConfigOC = {0};
	sConfigOC.OCMode = TIM_OCMODE_PWM1;
	sConfigOC.Pulse = 5;
	sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
	sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
	if(HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
	{
		return false;
	}

	HAL_GPIO_WritePin(MASTER_TIMESYNC_nOE_GPIO_Port, MASTER_TIMESYNC_nOE_Pin, GPIO_PIN_RESET);

	HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);

	GPIO_InitTypeDef GPIO_InitStruct = {0};
	GPIO_InitStruct.Pin = GPIO_PIN_7;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull = GPIO_PULLDOWN;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	GPIO_InitStruct.Alternate = GPIO_AF2_TIM3;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	HAL_GPIO_WritePin(MASTER_TIMESYNC_nOE_GPIO_Port, MASTER_TIMESYNC_nOE_Pin, GPIO_PIN_RESET);

	return true;
}

bool Timesync_task::ic_config()
{
	// 100 MHz input clock
	// 50000 prescaler -> 500us per tick
	// 50000 counts -> 25s overflow

	htim3.Instance = TIM3;
	htim3.Init.Prescaler = 50000;
	htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim3.Init.Period = 50000;
	htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
	if(HAL_TIM_Base_Init(&htim3) != HAL_OK)
	{
		return false;
	}

	TIM_ClockConfigTypeDef sClockSourceConfig = {0};
	sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
	if(HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
	{
		return false;
	}

	if(HAL_TIM_IC_Init(&htim3) != HAL_OK)
	{
		return false;
	}

	TIM_SlaveConfigTypeDef sSlaveConfig = {0};
	sSlaveConfig.SlaveMode = TIM_SLAVEMODE_RESET;
	sSlaveConfig.InputTrigger = TIM_TS_TI1FP1;
	sSlaveConfig.TriggerPolarity = TIM_INPUTCHANNELPOLARITY_RISING;
	sSlaveConfig.TriggerFilter = 0;
	if(HAL_TIM_SlaveConfigSynchronization(&htim3, &sSlaveConfig) != HAL_OK)
	{
		return false;
	}

	TIM_MasterConfigTypeDef sMasterConfig = {0};
	sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	if(HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
	{
		return false;
	}

	TIM_IC_InitTypeDef sConfigIC = {0};
	sConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_RISING;
	sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;
	sConfigIC.ICPrescaler = TIM_ICPSC_DIV1;
	sConfigIC.ICFilter = 0;
	if(HAL_TIM_IC_ConfigChannel(&htim3, &sConfigIC, TIM_CHANNEL_1) != HAL_OK)
	{
		return false;
	}

	HAL_GPIO_WritePin(MASTER_TIMESYNC_nOE_GPIO_Port, MASTER_TIMESYNC_nOE_Pin, GPIO_PIN_SET);

	HAL_TIM_Base_Start(&htim3);

	HAL_TIM_IC_Start(&htim3, TIM_CHANNEL_1);

	GPIO_InitTypeDef GPIO_InitStruct = {0};
	GPIO_InitStruct.Pin = GPIO_PIN_6;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull = GPIO_PULLDOWN;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	GPIO_InitStruct.Alternate = GPIO_AF2_TIM3;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	return true;
}