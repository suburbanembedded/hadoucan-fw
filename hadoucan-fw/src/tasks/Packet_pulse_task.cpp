#include "Packet_pulse_task.hpp"

#include "Task_instances.hpp"

#include "freertos_cpp_util/logging/Global_logger.hpp"

using freertos_util::logging::LOG_LEVEL;
#include "main.h"
#include "hal_inst.h"
#include "stm32h7xx_hal.h"

Packet_pulse_task::Packet_pulse_task()
{
	m_local_packet_counter = 0;
}

Packet_pulse_task::~Packet_pulse_task()
{
	
}

void Packet_pulse_task::work()
{
	__HAL_RCC_GPIOA_CLK_ENABLE();

	//config sync pin as gpio
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	GPIO_InitStruct.Pin = GPIO_PIN_7;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
	HAL_GPIO_WritePin(MASTER_TIMESYNC_nOE_GPIO_Port, MASTER_TIMESYNC_nOE_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, GPIO_PIN_RESET);

	//enable CYCCNT
	CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
	DWT->LAR = 0xC5ACCE55; 
	DWT->CYCCNT = 0;
	DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;

	for(;;)
	{
		const uint32_t new_packets = stm32_fdcan_rx_task.get_reset_packet_count();
		m_local_packet_counter += new_packets;

		if(m_local_packet_counter == 0)
		{
			vTaskDelay(pdMS_TO_TICKS(5));
			continue;
		}

		while(m_local_packet_counter)
		{
			//send a pulse
			bitbang_50us_pulse();
			m_local_packet_counter--;

			taskYIELD();
		}
	}
}

void Packet_pulse_task::bitbang_50us_pulse()
{
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, GPIO_PIN_SET);
	microsleep(50);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, GPIO_PIN_RESET);
	microsleep(50);
}

void Packet_pulse_task::microsleep(const uint32_t val)
{
	const uint32_t clk_cycle_start = DWT->CYCCNT;
 
	const uint32_t microseconds = val * (HAL_RCC_GetSysClockFreq() / 1000000);
	
	while ((DWT->CYCCNT - clk_cycle_start) < microseconds);
}
