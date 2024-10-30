#include "UART.h"


char *msg = "Hello World\r\n" ;
int main(void) {

    HAL_Init();
    System_Clock_Init();


    UART2_Init();



    while (1) {

        USART_Write(USART2, (uint8_t*)msg, strlen(msg));
        HAL_Delay(1000);
    }
}


