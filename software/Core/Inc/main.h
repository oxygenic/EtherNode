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

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define DIGI_OUT_4_Pin GPIO_PIN_15
#define DIGI_OUT_4_GPIO_Port GPIOC
#define DIGI_OUT_5_Pin GPIO_PIN_0
#define DIGI_OUT_5_GPIO_Port GPIOC
#define SET_DEFAULT_Pin GPIO_PIN_2
#define SET_DEFAULT_GPIO_Port GPIOC
#define DIGI_IN_0_Pin GPIO_PIN_3
#define DIGI_IN_0_GPIO_Port GPIOC
#define DIGI_OUT_0_Pin GPIO_PIN_3
#define DIGI_OUT_0_GPIO_Port GPIOA
#define AIN1_Pin GPIO_PIN_0
#define AIN1_GPIO_Port GPIOB
#define AIN0_Pin GPIO_PIN_1
#define AIN0_GPIO_Port GPIOB
#define DIGI_OUT_6_Pin GPIO_PIN_11
#define DIGI_OUT_6_GPIO_Port GPIOD
#define DIGI_OUT_7_Pin GPIO_PIN_12
#define DIGI_OUT_7_GPIO_Port GPIOD
#define DIGI_OUT_1_Pin GPIO_PIN_13
#define DIGI_OUT_1_GPIO_Port GPIOD
#define DIGI_IN_7_Pin GPIO_PIN_10
#define DIGI_IN_7_GPIO_Port GPIOC
#define DIGI_IN_3_Pin GPIO_PIN_1
#define DIGI_IN_3_GPIO_Port GPIOD
#define DIGI_IN_2_Pin GPIO_PIN_2
#define DIGI_IN_2_GPIO_Port GPIOD
#define DIGI_IN_1OLD_Pin GPIO_PIN_3
#define DIGI_IN_1OLD_GPIO_Port GPIOD
#define DIGI_IN_0OLD_Pin GPIO_PIN_4
#define DIGI_IN_0OLD_GPIO_Port GPIOD
#define DIGI_IN_1_Pin GPIO_PIN_7
#define DIGI_IN_1_GPIO_Port GPIOD
#define DIGI_OUT_3_Pin GPIO_PIN_5
#define DIGI_OUT_3_GPIO_Port GPIOB
#define DIGI_IN_6_Pin GPIO_PIN_6
#define DIGI_IN_6_GPIO_Port GPIOB
#define DIGI_IN_5_Pin GPIO_PIN_7
#define DIGI_IN_5_GPIO_Port GPIOB
#define DIGI_IN_4_Pin GPIO_PIN_8
#define DIGI_IN_4_GPIO_Port GPIOB
#define DIGI_OUT_2_Pin GPIO_PIN_9
#define DIGI_OUT_2_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */
extern ADC_HandleTypeDef hadc1;
extern ADC_HandleTypeDef hadc2;
extern LPTIM_HandleTypeDef hlptim1;
extern LPTIM_HandleTypeDef hlptim5;

extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim5;
extern TIM_HandleTypeDef htim6;
extern UART_HandleTypeDef huart1;
/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
