/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32h7xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define ULPI_CLK_EN_Pin GPIO_PIN_0
#define ULPI_CLK_EN_GPIO_Port GPIOA
#define ULPI_RESETB_Pin GPIO_PIN_1
#define ULPI_RESETB_GPIO_Port GPIOA
#define MASTER_TIMESYNC_nOE_Pin GPIO_PIN_4
#define MASTER_TIMESYNC_nOE_GPIO_Port GPIOC
#define CAN_nSILENT_Pin GPIO_PIN_14
#define CAN_nSILENT_GPIO_Port GPIOB
#define CAN_nSTDBY_Pin GPIO_PIN_15
#define CAN_nSTDBY_GPIO_Port GPIOB
#define RED1_Pin GPIO_PIN_12
#define RED1_GPIO_Port GPIOD
#define GREEN1_Pin GPIO_PIN_13
#define GREEN1_GPIO_Port GPIOD
#define RED2_Pin GPIO_PIN_14
#define RED2_GPIO_Port GPIOD
#define GREEN2_Pin GPIO_PIN_15
#define GREEN2_GPIO_Port GPIOD
#define CAN_SLOPE_Pin GPIO_PIN_8
#define CAN_SLOPE_GPIO_Port GPIOA

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
