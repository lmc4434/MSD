/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
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
#include "stm32l4xx_hal.h"


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
#define B1_Pin GPIO_PIN_13
#define B1_GPIO_Port GPIOC
#define SHLD_A5_Pin GPIO_PIN_0
#define SHLD_A5_GPIO_Port GPIOC
#define SHLD_A4_Pin GPIO_PIN_1
#define SHLD_A4_GPIO_Port GPIOC
#define SHLD_A0_Pin GPIO_PIN_0
#define SHLD_A0_GPIO_Port GPIOA
#define SHLD_A1_Pin GPIO_PIN_1
#define SHLD_A1_GPIO_Port GPIOA
#define USART_TX_Pin GPIO_PIN_2
#define USART_TX_GPIO_Port GPIOA
#define USART_RX_Pin GPIO_PIN_3
#define USART_RX_GPIO_Port GPIOA
#define SHLD_A2_Pin GPIO_PIN_4
#define SHLD_A2_GPIO_Port GPIOA
#define SHLD_D13_Pin GPIO_PIN_5
#define SHLD_D13_GPIO_Port GPIOA
#define SHLD_D12_Pin GPIO_PIN_6
#define SHLD_D12_GPIO_Port GPIOA
#define SHLD_D11_Pin GPIO_PIN_7
#define SHLD_D11_GPIO_Port GPIOA
#define SHLD_A3_Pin GPIO_PIN_0
#define SHLD_A3_GPIO_Port GPIOB
#define SHLD_D6_Pin GPIO_PIN_10
#define SHLD_D6_GPIO_Port GPIOB
#define SHLD_D9_Pin GPIO_PIN_7
#define SHLD_D9_GPIO_Port GPIOC
#define SHLD_D7_SEG7_Clock_Pin GPIO_PIN_8
#define SHLD_D7_SEG7_Clock_GPIO_Port GPIOA
#define SHLD_D8_SEG7_Data_Pin GPIO_PIN_9
#define SHLD_D8_SEG7_Data_GPIO_Port GPIOA
#define SHLD_D2_Pin GPIO_PIN_10
#define SHLD_D2_GPIO_Port GPIOA
#define TMS_Pin GPIO_PIN_13
#define TMS_GPIO_Port GPIOA
#define TCK_Pin GPIO_PIN_14
#define TCK_GPIO_Port GPIOA
#define SHLD_D5_Pin GPIO_PIN_4
#define SHLD_D5_GPIO_Port GPIOB
#define SHLD_D4_SEG7_Latch_Pin GPIO_PIN_5
#define SHLD_D4_SEG7_Latch_GPIO_Port GPIOB
#define SHLD_D10_Pin GPIO_PIN_6
#define SHLD_D10_GPIO_Port GPIOB
#define SHLD_D15_Pin GPIO_PIN_8
#define SHLD_D15_GPIO_Port GPIOB
#define SHLD_D14_Pin GPIO_PIN_9
#define SHLD_D14_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
