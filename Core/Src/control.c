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
#define PANEL_SW_CLOSE_PIN GPIO_PIN_7
#define PANEL_SW_CLOSE_PORT GPIOC

#define DUST_PIN GPIO_PIN_10
#define DUST_PORT GPIOA

#define TILT_STEP_ANGLE 5

int tiltDelay = 550;
int openDelay = 400;

extern ADC_HandleTypeDef hadc1;
uint16_t value_adc;


float Solar_Panel_Voltage = 0.0;
int battery_var = 0;
int power_generation = 0;
int solar_tracking = 0;
int angle_moved = 0;
int panel_angle_moved = 0;
int panel_angle_var = 0;

float prev_Solar_Panel_Voltage = 0.0;
int prev_battery_var = 0;
int prev_power_generation = 0;
int prev_tilt_angle_var = 0;
int prev_solar_tracking = 0;
int prev_panel_angle_var = 0;

int test_index = 0;
int test_power_outputs[] = {100, 150, 200, 250, 300, 250, 200, 150, 100};  // Simulated power outputs
int test_length = sizeof(test_power_outputs) / sizeof(test_power_outputs[0]);  // Length of test array


int tilt_angle_var = 0;
char tilt_angle_display_var[20] = "";
int mode = 0;
int panel_open = 0;

int current_angle = 0;
int current_panel_angle = 0;

xTaskHandle UpdateandSendThread_Handler;
xTaskHandle UpdateFromHeaderThread_Handler;
xTaskHandle HillClimb_Handler;

void check_and_send_updates(void *argument);
void step_from_ang(int angle);
void open_panel(int state);
void open_panel_angle(int angle);
void step_from_panel_ang(int angle);
void tilt_panel(int angle, int in_tracking);
void update_variable_from_header(void *argument);
void run(void);
void clear_dust(void);
void send_values(void);
void automode(void);




void check_and_send_updates(void *argument) {
    while (1) {
        char uartBuffer[64];

        if (Solar_Panel_Voltage != prev_Solar_Panel_Voltage) {
            snprintf(uartBuffer, sizeof(uartBuffer), "VV:%.2f\r\n", Solar_Panel_Voltage);
            //UART_SendString(uartBuffer);
            prev_Solar_Panel_Voltage = Solar_Panel_Voltage;
        }

        if (battery_var != prev_battery_var) {
            snprintf(uartBuffer, sizeof(uartBuffer), "BP:%d\r\n", battery_var);
            //UART_SendString(uartBuffer);
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
        if (panel_angle_var != prev_panel_angle_var) {
            snprintf(uartBuffer, sizeof(uartBuffer), "PA:%d\r\n", panel_angle_var);
            UART_SendString(uartBuffer);
            prev_panel_angle_var = panel_angle_var;
        }
        if (solar_tracking != prev_solar_tracking) {
            snprintf(uartBuffer, sizeof(uartBuffer), "ST:%d\r\n", solar_tracking);
            UART_SendString(uartBuffer);
            prev_solar_tracking = solar_tracking;
        }

        vTaskDelay(pdMS_TO_TICKS(1500));  // Ensure task yields periodically
    }
}


void send_values(){
    char uartBuffer[64];
    solar_tracking = 0;
    snprintf(uartBuffer, sizeof(uartBuffer), "VV:%.2f\r\n", Solar_Panel_Voltage);
    prev_Solar_Panel_Voltage = Solar_Panel_Voltage;
    snprintf(uartBuffer, sizeof(uartBuffer), "BP:%d\r\n", battery_var);
    prev_battery_var = battery_var;
    snprintf(uartBuffer, sizeof(uartBuffer), "PG:%d\r\n", power_generation);
    UART_SendString(uartBuffer);
    prev_power_generation = power_generation;
    snprintf(uartBuffer, sizeof(uartBuffer), "TA:%d\r\n", tilt_angle_var);
    UART_SendString(uartBuffer);
    prev_tilt_angle_var = tilt_angle_var;
    snprintf(uartBuffer, sizeof(uartBuffer), "PA:%d\r\n", panel_angle_var);
    UART_SendString(uartBuffer);
    prev_panel_angle_var = panel_angle_var;
    snprintf(uartBuffer, sizeof(uartBuffer), "PO:%d\r\n", panel_open);
    UART_SendString(uartBuffer);
    snprintf(uartBuffer, sizeof(uartBuffer), "CM:%d\r\n", mode);
    UART_SendString(uartBuffer);
	snprintf(uartBuffer, sizeof(uartBuffer), "FI:%d\r\n", 0);
    UART_SendString(uartBuffer);


}


void update_variable_from_header(void *argument) {
    char identifier[3];
    char value_string[20];
    char received_string[64];
    //char uartBuffer[64];

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
                    tilt_panel(tilt_angle_var, 0);
                } else if (strcmp(identifier, "PA") == 0) {
                	panel_angle_var = atoi(value_string);
                    open_panel_angle(panel_angle_var);
                } else if (strcmp(identifier, "PO") == 0) {
                    panel_open = atoi(value_string);
                    open_panel(panel_open);
                } else if (strcmp(identifier, "CM") == 0) {
                	mode = atoi(value_string);
                    automode();
                } else if (strcmp(identifier, "ST") == 0) {
                	solar_tracking = atoi(value_string);
                } else if (strcmp(identifier, "CD") == 0) {
                	clear_dust();
                } else if (strcmp(identifier, "SU") == 0) {
                	send_values();

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

void automode(void){


	microDelay(10000);

	clear_dust();
	solar_tracking = 1;
	char uartBuffer[64];
	snprintf(uartBuffer, sizeof(uartBuffer), "FI:%d\r\n", 0);
	UART_SendString(uartBuffer);
}

void tilt_panel(int dest_angle, int in_tracking){
	if (dest_angle < -22){
		dest_angle = -22;
	}else if (dest_angle > 22){
		dest_angle = 22;
	}


	angle_moved = dest_angle - current_angle;
	current_angle = dest_angle;
	tilt_angle_var = current_angle;

	step_from_ang(angle_moved);

	if (!in_tracking){
		char uartBuffer[64];
		snprintf(uartBuffer, sizeof(uartBuffer), "FI:%d\r\n", 0);
		UART_SendString(uartBuffer);
	}

}

void open_panel_angle(int dest_angle){
	if (dest_angle < 0){
		dest_angle = 0;
	}else if (dest_angle > 64){
		dest_angle = 64;
	}


	panel_angle_moved = dest_angle - current_panel_angle;
	current_panel_angle = dest_angle;
	panel_angle_var = current_panel_angle;

	step_from_panel_ang(panel_angle_moved);
	char uartBuffer[64];
    snprintf(uartBuffer, sizeof(uartBuffer), "FI:%d\r\n", 0);
    UART_SendString(uartBuffer);

}


void step_from_ang(int angle){
      int x;
	  int stopstep = 0;
	  char uartBuffer[64];


	  HAL_GPIO_WritePin(TILT_EN_PORT, TILT_EN_PIN, GPIO_PIN_SET);

	  if (angle < 0){
		  angle = angle * (-1);
    	  HAL_GPIO_WritePin(TILT_DIR_PORT, TILT_DIR_PIN, GPIO_PIN_RESET);
	  }
	  else{
    	  HAL_GPIO_WritePin(TILT_DIR_PORT, TILT_DIR_PIN, GPIO_PIN_SET);
	  }


      microDelay(tiltDelay*4);

	  stopstep = stopstep + (int)(3200.0*(((double)angle/360.0)));
	  stopstep = stopstep * 60;

	      for(x=0; x<stopstep; x=x+1)
	      {
	        HAL_GPIO_WritePin(TILT_STEP_PORT, TILT_STEP_PIN, GPIO_PIN_SET);
	        microDelay(tiltDelay);
	        HAL_GPIO_WritePin(TILT_STEP_PORT, TILT_STEP_PIN, GPIO_PIN_RESET);
	        microDelay(tiltDelay);
	      }

	  HAL_GPIO_WritePin(TILT_EN_PORT, TILT_EN_PIN, GPIO_PIN_RESET);
	  HAL_GPIO_WritePin(TILT_DIR_PORT, TILT_DIR_PIN, GPIO_PIN_RESET);
      HAL_GPIO_WritePin(TILT_STEP_PORT, TILT_STEP_PIN, GPIO_PIN_RESET);


}

void step_from_panel_ang(int angle){
      int x;
	  int stopstep = 0;
	  char uartBuffer[64];



	  HAL_GPIO_WritePin(PANEL_EN_PORT, PANEL_EN_PIN, GPIO_PIN_SET);

	  if (angle < 0){
		  angle = angle * (-1);
    	  HAL_GPIO_WritePin(PANEL_DIR_PORT, PANEL_DIR_PIN, GPIO_PIN_RESET);
	  }
	  else{
    	  HAL_GPIO_WritePin(PANEL_DIR_PORT, PANEL_DIR_PIN, GPIO_PIN_SET);
	  }


      microDelay(openDelay*4);

	  stopstep = stopstep + (int)(3200.0*(((double)angle/360.0)));
	  stopstep = stopstep * 60;

	      for(x=0; x<stopstep; x=x+1)
	      {
	        HAL_GPIO_WritePin(PANEL_STEP_PORT, PANEL_STEP_PIN, GPIO_PIN_SET);
	        microDelay(openDelay);
	        HAL_GPIO_WritePin(PANEL_STEP_PORT, PANEL_STEP_PIN, GPIO_PIN_RESET);
	        microDelay(openDelay);
	      }

	  HAL_GPIO_WritePin(PANEL_EN_PORT, PANEL_EN_PIN, GPIO_PIN_RESET);
	  HAL_GPIO_WritePin(PANEL_DIR_PORT, PANEL_DIR_PIN, GPIO_PIN_RESET);
      HAL_GPIO_WritePin(PANEL_STEP_PORT, PANEL_STEP_PIN, GPIO_PIN_RESET);

}



void open_panel(int state){
	if (state == 1){
		open_panel_angle(360);
	}else if(state == 0){
		HAL_GPIO_WritePin(PANEL_EN_PORT, PANEL_EN_PIN, GPIO_PIN_SET);
		HAL_GPIO_WritePin(PANEL_DIR_PORT, PANEL_DIR_PIN, GPIO_PIN_RESET);


	    microDelay(openDelay);
	    microDelay(openDelay);

		while(HAL_GPIO_ReadPin(PANEL_SW_CLOSE_PORT, PANEL_SW_CLOSE_PIN) == GPIO_PIN_SET){


	        HAL_GPIO_WritePin(PANEL_STEP_PORT, PANEL_STEP_PIN, GPIO_PIN_SET);
	        microDelay(openDelay);
	        HAL_GPIO_WritePin(PANEL_STEP_PORT, PANEL_STEP_PIN, GPIO_PIN_RESET);
	        microDelay(openDelay);

		}


		HAL_GPIO_WritePin(PANEL_EN_PORT, PANEL_EN_PIN, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(PANEL_DIR_PORT, PANEL_DIR_PIN, GPIO_PIN_RESET);
	    HAL_GPIO_WritePin(PANEL_STEP_PORT, PANEL_STEP_PIN, GPIO_PIN_RESET);


	    current_panel_angle = 0;
	    panel_angle_var = 0;

	}
}

void clear_dust(void){
	 HAL_GPIO_WritePin(DUST_PORT, DUST_PIN, GPIO_PIN_SET);
	 HAL_GPIO_WritePin(DUST_PORT, DUST_PIN, GPIO_PIN_RESET);
	 char uartBuffer[64];
	 snprintf(uartBuffer, sizeof(uartBuffer), "FI:%d\r\n", 0);
	 UART_SendString(uartBuffer);
}


void hill_climb_for_optimal_tilt(void *argument) {
		int variable_array[44] = {0};
	    int last_power;
	    int first_run = 0;
	    int direction = 1;
	    int improved = 0;
	    int last_angle;
	    int current_power;
	    while (1) {
	    	if(first_run == 0 && solar_tracking == 1){
	    		HAL_ADC_Start(&hadc1);
	    		HAL_ADC_PollForConversion(&hadc1, 20);
	    		current_power = HAL_ADC_GetValue(&hadc1);
	            variable_array[tilt_angle_var + 22] = current_power;
	            first_run = 1;
	    	}

	        if (solar_tracking == 1) {
	            last_angle = tilt_angle_var;
	            tilt_panel(tilt_angle_var + (5 * direction), 1);

	            char uartBuffer[64];
	            snprintf(uartBuffer, sizeof(uartBuffer), "Line 367: %i ", tilt_angle_var + (5 * direction));
	            UART_SendString(uartBuffer);

	    		HAL_ADC_Start(&hadc1);
	    		HAL_ADC_PollForConversion(&hadc1, 20);
	    		current_power = HAL_ADC_GetValue(&hadc1);

	            variable_array[tilt_angle_var+22] = current_power;
	            if (variable_array[tilt_angle_var+22] < variable_array[last_angle+22]) {
	                direction = -1;
	                if(improved || tilt_angle_var > 22 || tilt_angle_var < -22){
	                	for (int i = 0; i < 45; i++){
		    	            char uartBuffer[64];
		    	            snprintf(uartBuffer, sizeof(uartBuffer), "%i: %i, \r\n", i, variable_array[i]);
		    	            UART_SendString(uartBuffer);
		    	            break;
	                	}

	    	            improved = 0;
	    	            direction = 1;
	    	            solar_tracking = 0;
	    	            first_run = 0;
	                }

	            } else if(variable_array[tilt_angle_var+22] > variable_array[last_angle+22]){
	                improved = 1;
	            }

	        }

	        vTaskDelay(pdMS_TO_TICKS(3000));

	    }
    	char uartBuffer[64];
        snprintf(uartBuffer, sizeof(uartBuffer), "FI:%d\r\n", 0);
        UART_SendString(uartBuffer);

	}

void run(void) {

    xTaskCreate(update_variable_from_header, "Update", 256, NULL, 1, &UpdateFromHeaderThread_Handler);
    xTaskCreate(check_and_send_updates, "CheckandSend", 256, NULL, 2, &UpdateandSendThread_Handler);
    xTaskCreate(hill_climb_for_optimal_tilt, "HillClimb", 256, NULL, 3, &HillClimb_Handler);

    vTaskStartScheduler();
}
