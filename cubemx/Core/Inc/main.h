/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
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
#include "stm32f4xx_hal.h"

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
#define SMPS_ENABLE_Pin GPIO_PIN_4
#define SMPS_ENABLE_GPIO_Port GPIOC
#define LDO_ENABLE_Pin GPIO_PIN_10
#define LDO_ENABLE_GPIO_Port GPIOE
#define PWR_GSM_EN_Pin GPIO_PIN_12
#define PWR_GSM_EN_GPIO_Port GPIOE
#define P3V3PER_ENABLE_Pin GPIO_PIN_13
#define P3V3PER_ENABLE_GPIO_Port GPIOE
#define GSM_RESET_Pin GPIO_PIN_0
#define GSM_RESET_GPIO_Port GPIOD
#define GSM_ONOROFF_Pin GPIO_PIN_1
#define GSM_ONOROFF_GPIO_Port GPIOD
#define GSM_DCD_Pin GPIO_PIN_11
#define GSM_DCD_GPIO_Port GPIOG
#define GSM_DCDG15_Pin GPIO_PIN_15
#define GSM_DCDG15_GPIO_Port GPIOG
#define GSM_RI_Pin GPIO_PIN_3
#define GSM_RI_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
