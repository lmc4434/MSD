/*
 * control.c
 *
 *  Created on: Nov 13, 2024
 *      Author: Logan Culver
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

#define TILT_DIR_PIN GPIO_PIN_4 		//A
#define TILT_DIR_PORT GPIOB				//A
#define TILT_STEP_PIN GPIO_PIN_5		//B
#define TILT_STEP_PORT GPIOB			//B
#define TILT_EN_PIN GPIO_PIN_3			//En
#define TILT_EN_PORT GPIOB				//En

#define PANEL_DIR_PIN GPIO_PIN_9 		//A
#define PANEL_DIR_PORT GPIOA			//A
#define PANEL_STEP_PIN GPIO_PIN_8		//B
#define PANEL_STEP_PORT GPIOA			//B
#define PANEL_EN_PIN GPIO_PIN_10		//En
#define PANEL_EN_PORT GPIOB				//En
#define PANEL_SW_OPEN_PIN GPIO_PIN_2
#define PANEL_SW_OPEN_PORT GPIOC
#define PANEL_SW_CLOSE_PIN GPIO_PIN_3
#define PANEL_SW_CLOSE_PORT GPIOC

#define DUST_PIN GPIO_PIN_10
#define DUST_PORT GPIOA

int stepDelay = 100;

float Solar_Panel_Voltage = 0.0;
int battery_var = 0;
int power_generation = 0;

float prev_Solar_Panel_Voltage = 0.0;
int prev_battery_var = 0;
int prev_power_generation = 0;
int prev_tilt_angle_var = 0;

int tilt_angle_var = 0;
char tilt_angle_display_var[20] = "";
int panel_open = 0;
int mode = 0;

xTaskHandle UpdateandSendThread_Handler;
xTaskHandle UpdateFromHeaderThread_Handler;

void generate_random_values();
void check_and_send_updates(void *argument);
void step_from_ang(int angle);
void open_panel(int state);
void update_variable_from_header(void *argument);
void run(void);
void clear_dust(void);

void generate_random_values() {

	Solar_Panel_Voltage = ((float)(rand() % 1200)) / 100.0;  // Random voltage 0.0 to 30.0 volts
	battery_var = rand() % 101;                              // Random battery percentage 0 to 100
	power_generation = rand() % 20;                         // Random power generation 0 to 500 watts


}


void check_and_send_updates(void *argument) {
    while (1) {
    	generate_random_values();
        char uartBuffer[64];

        if (Solar_Panel_Voltage != prev_Solar_Panel_Voltage) {
            snprintf(uartBuffer, sizeof(uartBuffer), "VV:%.2f\r\n", Solar_Panel_Voltage);
            UART_SendString(uartBuffer);
            prev_Solar_Panel_Voltage = Solar_Panel_Voltage;
        }

        if (battery_var != prev_battery_var) {
            snprintf(uartBuffer, sizeof(uartBuffer), "BP:%d\r\n", battery_var);
            UART_SendString(uartBuffer);
            prev_battery_var = battery_var;
        }

        if (power_generation != prev_power_generation) {
            snprintf(uartBuffer, sizeof(uartBuffer), "PG:%d\r\n", power_generation);
            UART_SendString(uartBuffer);
            prev_power_generation = power_generation;
        }
        if (tilt_angle_var != prev_tilt_angle_var) {
            snprintf(uartBuffer, sizeof(uartBuffer), "TA:%d\r\n", tilt_angle_var);
            UART_SendString(uartBuffer);
            prev_tilt_angle_var = tilt_angle_var;
        }

        vTaskDelay(pdMS_TO_TICKS(1500));  // Ensure task yields periodically
    }
}

void update_variable_from_header(void *argument) {
    char identifier[3];
    char value_string[20];
    char received_string[64];
    char uartBuffer[64];

    while (1) {
        memset(received_string, 0, sizeof(received_string));  // Clear buffer

        UART_ReadLine(received_string, sizeof(received_string));  // Read line, no return check

        // If the string is not empty
        if (received_string[0] != '\0') {
            char *colon_pos = strchr(received_string, ':');
            if (colon_pos != NULL) {
                strncpy(identifier, received_string, 2);
                identifier[2] = '\0';

                strncpy(value_string, colon_pos + 1, 19);
                value_string[19] = '\0';

                if (strcmp(identifier, "VV") == 0) {
                    Solar_Panel_Voltage = atof(value_string);
                } else if (strcmp(identifier, "BP") == 0) {
                    battery_var = atoi(value_string);
                } else if (strcmp(identifier, "TA") == 0) {
                    tilt_angle_var = atoi(value_string);
                    step_from_ang(tilt_angle_var);
                } else if (strcmp(identifier, "PO") == 0) {
                    panel_open = atoi(value_string);
                    open_panel(panel_open);
                } else if (strcmp(identifier, "CM") == 0) {
                    mode = atoi(value_string);
                } else if (strcmp(identifier, "CD") == 0) {
                	clear_dust();
                } else if (strcmp(identifier, "ST") == 0) {
                    //Run the function
                	int i = 1; //placeholder so no tnull
                	i +=1;
                }else {
                    UART_SendString("Unknown Identifier\r\n");
                }
            } else {
                UART_SendString("Invalid Command Format\r\n");
            }
        }

        vTaskDelay(pdMS_TO_TICKS(1));  // Prevent task blocking forever
    }
}

void step_from_ang(int angle){
      int x;
	  int stopstep;

	  HAL_GPIO_WritePin(TILT_EN_PORT, TILT_EN_PIN, GPIO_PIN_SET);

	  if (angle < 0){
		  angle = angle * (-1);
    	  HAL_GPIO_WritePin(TILT_DIR_PORT, TILT_DIR_PIN, GPIO_PIN_RESET);
	  }
	  else{

    	  HAL_GPIO_WritePin(TILT_DIR_PORT, TILT_DIR_PIN, GPIO_PIN_SET);
	  }


      microDelay(stepDelay);
      microDelay(stepDelay);


	  stopstep = 60*(int)(((double)angle/360.0) * 10000.0);

	      for(x=0; x<stopstep; x=x+1)
	      {

	        HAL_GPIO_WritePin(TILT_STEP_PORT, TILT_STEP_PIN, GPIO_PIN_SET);
	        microDelay(stepDelay);
	        HAL_GPIO_WritePin(TILT_STEP_PORT, TILT_STEP_PIN, GPIO_PIN_RESET);
	        microDelay(stepDelay);

	      }


	  HAL_GPIO_WritePin(TILT_EN_PORT, TILT_EN_PIN, GPIO_PIN_RESET);
	  HAL_GPIO_WritePin(TILT_DIR_PORT, TILT_DIR_PIN, GPIO_PIN_RESET);
      HAL_GPIO_WritePin(TILT_STEP_PORT, TILT_STEP_PIN, GPIO_PIN_RESET);


}



void open_panel(int state){
	if (state == 1){

		HAL_GPIO_WritePin(PANEL_EN_PORT, PANEL_EN_PIN, GPIO_PIN_SET);
		HAL_GPIO_WritePin(PANEL_DIR_PORT, PANEL_DIR_PIN, GPIO_PIN_SET);

        UART_SendString("INITAL\r\n");

	    microDelay(stepDelay);
	    microDelay(stepDelay);

		while(HAL_GPIO_ReadPin(PANEL_SW_CLOSE_PORT, PANEL_SW_CLOSE_PIN) == GPIO_PIN_SET){

            UART_SendString("LOOP\r\n");

	        HAL_GPIO_WritePin(PANEL_STEP_PORT, PANEL_STEP_PIN, GPIO_PIN_SET);
	        microDelay(stepDelay);
	        HAL_GPIO_WritePin(PANEL_STEP_PORT, PANEL_STEP_PIN, GPIO_PIN_RESET);
	        microDelay(stepDelay);

		}

        UART_SendString("END\r\n");

		HAL_GPIO_WritePin(PANEL_EN_PORT, PANEL_EN_PIN, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(PANEL_DIR_PORT, PANEL_DIR_PIN, GPIO_PIN_RESET);
	    HAL_GPIO_WritePin(PANEL_STEP_PORT, PANEL_STEP_PIN, GPIO_PIN_RESET);

	}else if(state == 0){
		HAL_GPIO_WritePin(PANEL_EN_PORT, PANEL_EN_PIN, GPIO_PIN_SET);
		HAL_GPIO_WritePin(PANEL_DIR_PORT, PANEL_DIR_PIN, GPIO_PIN_RESET);

        UART_SendString("INITAL\r\n");

	    microDelay(stepDelay);
	    microDelay(stepDelay);

		while(HAL_GPIO_ReadPin(PANEL_SW_CLOSE_PORT, PANEL_SW_CLOSE_PIN) == GPIO_PIN_SET){

            //UART_SendString("LOOP\r\n");

	        HAL_GPIO_WritePin(PANEL_STEP_PORT, PANEL_STEP_PIN, GPIO_PIN_SET);
	        microDelay(stepDelay);
	        HAL_GPIO_WritePin(PANEL_STEP_PORT, PANEL_STEP_PIN, GPIO_PIN_RESET);
	        microDelay(stepDelay);

		}

        UART_SendString("END\r\n");

		HAL_GPIO_WritePin(PANEL_EN_PORT, PANEL_EN_PIN, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(PANEL_DIR_PORT, PANEL_DIR_PIN, GPIO_PIN_RESET);
	    HAL_GPIO_WritePin(PANEL_STEP_PORT, PANEL_STEP_PIN, GPIO_PIN_RESET);
	}else{
		/*if(HAL_GPIO_ReadPin(PANEL_SW_CLOSE_PORT, PANEL_SW_CLOSE_PIN) == GPIO_PIN_RESET){

	        UART_SendString("LIMIT SWITCH RESET\r\n");
		}
		else if((HAL_GPIO_ReadPin(PANEL_SW_CLOSE_PORT, PANEL_SW_CLOSE_PIN) == GPIO_PIN_SET)){
			UART_SendString("LIMIT SWITCH SET\r\n");
		}
		else{
			 UART_SendString("UNKNOWN\r\n");
		}*/
	}

}

void clear_dust(void){
	 HAL_GPIO_WritePin(DUST_PORT, DUST_PIN, GPIO_PIN_SET);
	 HAL_GPIO_WritePin(DUST_PORT, DUST_PIN, GPIO_PIN_RESET);
}

void run(void) {

    xTaskCreate(update_variable_from_header, "Update", 256, NULL, 1, &UpdateFromHeaderThread_Handler);
    xTaskCreate(check_and_send_updates, "CheckandSend", 256, NULL, 2, &UpdateandSendThread_Handler);

    vTaskStartScheduler();
}
