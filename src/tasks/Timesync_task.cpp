#include "Timesync_task.hpp"

#include "main.h"

#include "uart1_printf.hpp"

#include "stm32h7xx_hal.h"

void Timesync_task::work()
{
	if(!ic_config())
	{
		uart1_log<64>(LOG_LEVEL::ERROR, "Timesync_task", "ic_config failed");
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

	return true;
}