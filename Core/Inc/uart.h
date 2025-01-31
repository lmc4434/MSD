#ifndef __STM32L476G_DISCOVERY_UART_H
#define __STM32L476G_DISCOVERY_UART_H

#include "stm32l476xx.h"

// HAL definitions
#define GPIO_AF7_USART2        	    ((uint8_t)0x07) /*!< USART2 Alternate Function mapping     */
#define UART_WORDLENGTH_8B          0x00000000U     /*!< 8-bit long UART frame */
#define UART_PARITY_NONE            0x00000000U     /*!< No parity   */
#define UART_OVERSAMPLING_16        0x00000000U     /*!< Oversampling by 16 */
#define UART_STOPBITS_1             0x00000000U     /*!< UART frame with 1 stop bit    */
#define UART_HWCONTROL_NONE         0x00000000U     /*!< No hardware control       */



void UART_Init(void);
void UART_SendString(char* str);
char UART_ReceiveChar(void);
void UART_SendChar(char c);
void UART_Delay(uint32_t us);
void UART_ReadLine(char *buffer, int bufferSize);

#endif /* __STM32L476G_DISCOVERY_UART_H */
