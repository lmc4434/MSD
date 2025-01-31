/*
 * control.c
 *
 *  Created on: Nov 13, 2024
 *      Author: Logan CUlver
 */


#include "main.h"
#include "cmsis_os.h"
#include "FreeRTOS.h"
#include "string.h"
#include <stdio.h>
#include <stdlib.h>
#include "task.h"
#include "queue.h"
#include "uart.h"
#include "time.h"
#include "gpio.h"
#include "7segment.h"
#include <sys/time.h>
#include <errno.h>



#include <string.h>
#include <stdlib.h>

#define DIR_PIN GPIO_PIN_2
#define DIR_PORT GPIOC
#define STEP_PIN GPIO_PIN_3
#define STEP_PORT GPIOC


int stepDelay = 1000;
float voltage_var = 0.0;
int battery_var = 0;
int tilt_angle_var = 0;
char tilt_angle_display_var[20] = "";
int panel_open = 0;
int mode = 0;

void microDelay (uint16_t delay)
{
  __HAL_TIM_SET_COUNTER(&htim1, 0);
  while (__HAL_TIM_GET_COUNTER(&htim1) < delay);
}

void step_from_ang(int tilt_angle_var){
	  int stopstep;

	  if (angle < 0){
		  angle = angle * (-1);
    	  HAL_GPIO_WritePin(DIR_PORT, DIR_PIN, GPIO_PIN_RESET);
	  }
	  else{

    	  HAL_GPIO_WritePin(DIR_PORT, DIR_PIN, GPIO_PIN_SET);
	  }
	  stopstep = (int)(((double)angle/360.0) * 512.0);

	      for(x=0; x<stopstep; x=x+1)
	      {
	        HAL_GPIO_WritePin(STEP_PORT, STEP_PIN, GPIO_PIN_SET);
	        microDelay(stepDelay);
	        HAL_GPIO_WritePin(STEP_PORT, STEP_PIN, GPIO_PIN_RESET);
	        microDelay(stepDelay);
	      }

}


void update_variable_from_header(char* received_string) {
    char identifier[3];
    char value_string[20];
    float value;


    char *colon_pos = strchr(received_string, ':');
    if (colon_pos != NULL) {

        strncpy(identifier, received_string, 2);
        identifier[2] = '\0';

        strncpy(value_string, colon_pos + 1, 19);
        value_string[19] = '\0';


        if (identifier[0] == 'V' && identifier[1] == 'V') {

            voltage_var = atof(value_string);

        }
        else if (identifier[0] == 'B' && identifier[1] == 'P') {

            battery_var = atoi(value_string);

        }
        else if (identifier[0] == 'T' && identifier[1] == 'A') {

            tilt_angle_var = atoi(value_string);

        }
        else if (identifier[0] == 'T' && identifier[1] == 'D') {

            strncpy(tilt_angle_display_var, value_string, sizeof(tilt_angle_display_var) - 1);
            tilt_angle_display_var[sizeof(tilt_angle_display_var) - 1] = '\0';

        }
        else if (identifier[0] == 'P' && identifier[1] == 'O') {

            panel_open = atoi(value_string);

        }
        else if (identifier[0] == 'C' && identifier[1] == 'M') {

            mode = atoi(value_string);

        }
        else {
            UART_SendString("Unknown Identifier\n");
        }
    } else {
        UART_SendString("Invalid Command Format\n");
    }
}


int run(void) {
	char uartBuffer[64];


    while (1) {


    	UART_ReadLine(uartBuffer, 64);

        update_variable_from_header(uartBuffer);

        char tilt_angle_str[20];


        snprintf(tilt_angle_str, sizeof(tilt_angle_str), "Tilt Angle %d", tilt_angle_var);

        step_from_ang(tilt_angle_var);

        UART_SendString(tilt_angle_str);
        UART_SendString("\r\n");
    }

}

