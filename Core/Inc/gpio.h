/*
 * gpio.h
 *
 *  Created on: Nov 13, 2024
 *      Author: Logan CUlver
 */

#ifndef INC_GPIO_H_
#define INC_GPIO_H_

#include "stm32l476xx.h"
#include "stm32l4xx_hal.h"

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

void MX_GPIO_Init(void);
int check_button1(void);
int check_button2(void);
int check_button3(void);
#endif /* INC_GPIO_H_ */
