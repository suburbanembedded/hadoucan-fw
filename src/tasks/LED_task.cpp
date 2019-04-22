#include "LED_task.hpp"

#include "stm32h7xx_hal.h"
#include "stm32h7xx_hal_gpio.h"

#include "main.h"

void LED_task::work()
{
	for(;;)
	{
		HAL_GPIO_WritePin(GPIOD, RED1_Pin, GPIO_PIN_SET);
		vTaskDelay(250);
		HAL_GPIO_WritePin(GPIOD, RED1_Pin, GPIO_PIN_RESET);
		vTaskDelay(250);
		HAL_GPIO_WritePin(GPIOD, RED1_Pin, GPIO_PIN_SET);

		HAL_GPIO_WritePin(GPIOD, GREEN1_Pin, GPIO_PIN_SET);
		vTaskDelay(250);
		HAL_GPIO_WritePin(GPIOD, GREEN1_Pin, GPIO_PIN_RESET);
		vTaskDelay(250);
		HAL_GPIO_WritePin(GPIOD, GREEN1_Pin, GPIO_PIN_SET);

		HAL_GPIO_WritePin(GPIOD, RED2_Pin, GPIO_PIN_SET);
		vTaskDelay(250);
		HAL_GPIO_WritePin(GPIOD, RED2_Pin, GPIO_PIN_RESET);
		vTaskDelay(250);
		HAL_GPIO_WritePin(GPIOD, RED2_Pin, GPIO_PIN_SET);

		HAL_GPIO_WritePin(GPIOD, GREEN2_Pin, GPIO_PIN_SET);
		vTaskDelay(250);
		HAL_GPIO_WritePin(GPIOD, GREEN2_Pin, GPIO_PIN_RESET);
		vTaskDelay(250);
		HAL_GPIO_WritePin(GPIOD, GREEN2_Pin, GPIO_PIN_SET);
	}
}
