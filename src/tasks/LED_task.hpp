#pragma once

class LED_task : public Task_static<512>
{
public:

	void work() override
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
};