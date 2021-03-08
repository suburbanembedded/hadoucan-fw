#include "LED_task.hpp"

#include "stm32h7xx_hal.h"
#include "stm32h7xx_hal_gpio.h"

#include "main.h"

LED_task::LED_task()
{
	set_mode_boot();
	handle_boot_mode();
}

void LED_task::set_mode_boot()
{
	m_mode = LED_MODE::BOOT;
}
void LED_task::set_mode_normal()
{
	m_mode = LED_MODE::NORMAL;
}
void LED_task::set_mode_error()
{
	m_mode = LED_MODE::ERROR;
}

void LED_task::all_off()
{
	HAL_GPIO_WritePin(GPIOD, GREEN1_Pin | GREEN2_Pin | RED1_Pin | RED2_Pin, GPIO_PIN_SET);
}

void LED_task::all_on()
{
	HAL_GPIO_WritePin(GPIOD, GREEN1_Pin | GREEN2_Pin | RED1_Pin | RED2_Pin, GPIO_PIN_RESET);
}

void LED_task::notify_can_rx()
{
	m_can_rx_activity = true;
}
void LED_task::notify_can_tx()
{
	m_can_tx_activity = true;
}

void LED_task::notify_usb_rx()
{
	m_usb_activity = true;
}
void LED_task::notify_usb_tx()
{
	m_usb_activity = true;
}

void LED_task::work()
{
	for(;;)
	{
		switch(m_mode)
		{
			case LED_MODE::BOOT:
			{
				handle_boot_mode();
				break;
			}
			case LED_MODE::NORMAL:
			{
				handle_normal_mode();
				break;
			}
			case LED_MODE::ERROR:
			{
				handle_error_mode();
				break;
			}
			default:
			{
				m_mode = LED_MODE::ERROR;
			}
		}

		vTaskDelay(pdMS_TO_TICKS(50));
	}
}

void LED_task::handle_boot_mode()
{
	all_on();
}
void LED_task::handle_normal_mode()
{
	HAL_GPIO_WritePin(GPIOD, GREEN1_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOD, RED1_Pin,   GPIO_PIN_SET);

	if(m_can_tx_activity)
	{
		m_can_tx_activity = false;

		if(HAL_GPIO_ReadPin(GPIOD, RED2_Pin) == GPIO_PIN_RESET)
		{
			HAL_GPIO_WritePin(GPIOD, RED2_Pin, GPIO_PIN_SET);
		}
		else
		{
			HAL_GPIO_WritePin(GPIOD, RED2_Pin, GPIO_PIN_RESET);	
		}
	}
	else
	{
		if(HAL_GPIO_ReadPin(GPIOD, RED2_Pin) == GPIO_PIN_RESET)
		{
			HAL_GPIO_WritePin(GPIOD, RED2_Pin, GPIO_PIN_SET);
		}
	}

	if(m_can_rx_activity)
	{
		m_can_rx_activity = false;
		
		if(HAL_GPIO_ReadPin(GPIOD, GREEN2_Pin) == GPIO_PIN_RESET)
		{
			HAL_GPIO_WritePin(GPIOD, GREEN2_Pin, GPIO_PIN_SET);
		}
		else
		{
			HAL_GPIO_WritePin(GPIOD, GREEN2_Pin, GPIO_PIN_RESET);	
		}
	}
	else
	{
		if(HAL_GPIO_ReadPin(GPIOD, GREEN2_Pin) == GPIO_PIN_RESET)
		{
			HAL_GPIO_WritePin(GPIOD, GREEN2_Pin, GPIO_PIN_SET);
		}
	}
}
void LED_task::handle_error_mode()
{
	all_off();

	HAL_GPIO_WritePin(GPIOD, RED1_Pin,   GPIO_PIN_RESET);
}