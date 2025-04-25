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


#define TILT_DIR_PIN GPIO_PIN_4 		//A D5
#define TILT_DIR_PORT GPIOB				//A D5
#define TILT_STEP_PIN GPIO_PIN_5		//B D4
#define TILT_STEP_PORT GPIOB			//B D4
#define TILT_EN_PIN GPIO_PIN_3			//En D3
#define TILT_EN_PORT GPIOB				//En D3

#define PANEL_DIR_PIN GPIO_PIN_9 		//A
#define PANEL_DIR_PORT GPIOA			//A
#define PANEL_STEP_PIN GPIO_PIN_8		//B
#define PANEL_STEP_PORT GPIOA			//B
#define PANEL_SW_OPEN_PIN GPIO_PIN_10		//En
#define PANEL_SW_OPEN_PORT GPIOB				//En
#define PANEL_SW_CLOSE_PIN GPIO_PIN_7
#define PANEL_SW_CLOSE_PORT GPIOC

#define DUST_PIN GPIO_PIN_10
#define DUST_PORT GPIOA

#define TILT_STEP_ANGLE 5

int tiltDelay = 550;
int openDelay = 300;

extern ADC_HandleTypeDef hadc1;
extern RTC_HandleTypeDef hrtc;
uint16_t value_adc;


uint16_t ADC_VAL[3];
int isADCFinished = 0;

int tracker = 0;

float Solar_Panel_Voltage = 0.0;
float battery_var = 0.0;
float power_generation = 0.0;
int solar_tracking = 0;
int angle_moved = 0;
int panel_angle_moved = 0;
int panel_angle_var = 0;

float prev_Solar_Panel_Voltage = 0.0;
float prev_battery_var = 0.0;
float prev_power_generation = 0.0;
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
int prev_panel_open = 0;

int current_angle = 0;
int current_panel_angle = 0;


int solar_tracker = 0;

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
void automode(void *argument);


void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc){
	 char uartBuffer[64];
	 int  volt_adc = ADC_VAL[0];
	 int  batt_charge_adc = ADC_VAL[1];
	 int  curr_adc = ADC_VAL[2];
	 float  real_curr, real_volt;
	 float lowest_voltage = 8.6;
	 float highest_voltage = 12.6;
	 /*
	 real_curr = ((3.3/4096.0)*(float)curr_adc)/0.3;
	 real_volt = ((3.3/4096.0)*((float)volt_adc))/0.2294;

	 power_generation = real_curr * real_volt;
	 Solar_Panel_Voltage = real_volt;
	 battery_var = ((real_volt - lowest_voltage) / (highest_voltage - lowest_voltage)) * 100;
	*/
	 /*
	 snprintf(uartBuffer, sizeof(uartBuffer), "IN ADC INTERUPT: \r\n");
	 UART_SendString(uartBuffer);
     snprintf(uartBuffer, sizeof(uartBuffer), "ADC CHANNEL 1:%d\r\n", batt_charge_adc);
     UART_SendString(uartBuffer);
     snprintf(uartBuffer, sizeof(uartBuffer), "ADC CHANNEL 5:%d\r\n", curr_adc);
     UART_SendString(uartBuffer);
     snprintf(uartBuffer, sizeof(uartBuffer), "ADC CHANNEL 2:%d\r\n", volt_adc);
     UART_SendString(uartBuffer);
     snprintf(uartBuffer, sizeof(uartBuffer), "\r\n");
	 UART_SendString(uartBuffer);*/

     isADCFinished = 1;
}



void check_and_send_updates(void *argument) {
    while (1) {
        char uartBuffer[64];
        HAL_ADC_Start_DMA(&hadc1, ADC_VAL, 3);
    	if(isADCFinished){
			if (Solar_Panel_Voltage != prev_Solar_Panel_Voltage) {
				snprintf(uartBuffer, sizeof(uartBuffer), "VV:%.2f\r\n", Solar_Panel_Voltage);
				UART_SendString(uartBuffer);
				prev_Solar_Panel_Voltage = Solar_Panel_Voltage;
			}

			if (battery_var != prev_battery_var) {
				snprintf(uartBuffer, sizeof(uartBuffer), "BP:%.2f\r\n", battery_var);
				UART_SendString(uartBuffer);
				prev_battery_var = battery_var;
			}

			if (power_generation != prev_power_generation) {
				snprintf(uartBuffer, sizeof(uartBuffer), "PG:%.2f\r\n", power_generation);
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
			if (panel_open != prev_panel_open) {
				snprintf(uartBuffer, sizeof(uartBuffer), "PO:%d\r\n", panel_open);
				UART_SendString(uartBuffer);
				prev_panel_open = panel_open;
			}

			isADCFinished = 0;
    	}

        vTaskDelay(pdMS_TO_TICKS(750));  // Ensure task yields periodically was 1500
    }
}


void send_values(){
    char uartBuffer[64];
    solar_tracking = 0;

    snprintf(uartBuffer, sizeof(uartBuffer), "VV:%.2f\r\n", Solar_Panel_Voltage);
    prev_Solar_Panel_Voltage = Solar_Panel_Voltage;
    snprintf(uartBuffer, sizeof(uartBuffer), "BP:%.2f\r\n", battery_var);
    prev_battery_var = battery_var;
    snprintf(uartBuffer, sizeof(uartBuffer), "PG:%.2f\r\n", power_generation);
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

        /*if(HAL_GPIO_ReadPin(PANEL_SW_CLOSE_PORT, PANEL_SW_CLOSE_PIN) == GPIO_PIN_SET)
        {

    		char uartBuffer[64];
    	   snprintf(uartBuffer, sizeof(uartBuffer), "SWITCH SET: \r\n");
    	   UART_SendString(uartBuffer);
        }else {
        	char uartBuffer[64];
        	    	   snprintf(uartBuffer, sizeof(uartBuffer), "SWITCH RESET: \r\n");
        	    	   UART_SendString(uartBuffer);
        }*/
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
                	if (mode == 0){
                		char uartBuffer[64];
                		snprintf(uartBuffer, sizeof(uartBuffer), "FI:%d\r\n", 0);
                		UART_SendString(uartBuffer);
                	}
                } else if (strcmp(identifier, "ST") == 0) {
                	solar_tracking = atoi(value_string);
                } else if (strcmp(identifier, "CD") == 0) {
                	clear_dust();
                } else if (strcmp(identifier, "SU") == 0) {
                	send_values();
                } else if (strcmp(identifier, "GS") == 0) {
                    tilt_panel(0, 0);
                    open_panel(0);

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


void HAL_RTC_AlarmAEventCallback(RTC_HandleTypeDef *hrtc) {
  RTC_AlarmTypeDef sAlarm;
  char uartBuffer[64];

  HAL_RTC_GetAlarm(hrtc,&sAlarm,RTC_ALARM_A,FORMAT_BIN);
  if(sAlarm.AlarmTime.Seconds>29) {
    sAlarm.AlarmTime.Seconds=0;
  }else{
    sAlarm.AlarmTime.Seconds=sAlarm.AlarmTime.Seconds+30;
  }
  while(HAL_RTC_SetAlarm_IT(hrtc, &sAlarm, FORMAT_BIN)!=HAL_OK){}


  if(mode){

	 if(tracker == 1){
		  snprintf(uartBuffer, sizeof(uartBuffer), "Opening Panels\r\n");
		  UART_SendString(uartBuffer);
		  open_panel(1);
		  snprintf(uartBuffer, sizeof(uartBuffer), "Tilting to 22\r\n");
		  UART_SendString(uartBuffer);
		  tilt_panel(22,1);
		  snprintf(uartBuffer, sizeof(uartBuffer), "Tilting to -22\r\n");
		  UART_SendString(uartBuffer);
		  tilt_panel(-22,1);
		  tilt_panel(0,1);
		  snprintf(uartBuffer, sizeof(uartBuffer), "Clearing Dust\r\n");
		  UART_SendString(uartBuffer);
		  clear_dust();
		  for(int i = 0; i < 100000; i++){}
		  snprintf(uartBuffer, sizeof(uartBuffer), "Running Solar Tracking\r\n");
		  UART_SendString(uartBuffer);
		  hill_climb();
		  snprintf(uartBuffer, sizeof(uartBuffer), "Resetting Panel State\r\n");
		  UART_SendString(uartBuffer);
		  tilt_panel(0, 1);
		  open_panel(0);
		  tracker = 0;
	 } else{
		 tracker++;
	 }

	  /*
	  if(panel_open == 0){

		  panel_open = 1;
		  open_panel(panel_open);
	  }


	  if(solar_tracker == 1){


		  snprintf(uartBuffer, sizeof(uartBuffer), "Running Solar Tracking\r\n");
		  UART_SendString(uartBuffer);
		  solar_tracking = 1;

		  snprintf(uartBuffer, sizeof(uartBuffer), "Clearing Dust\r\n");
		  UART_SendString(uartBuffer);
		  clear_dust();

		  solar_tracker = 0;
	  } else
	  {
      	snprintf(uartBuffer, sizeof(uartBuffer), "FI:%d\r\n", 0);
      	UART_SendString(uartBuffer);
		  clear_dust();

		  solar_tracker = 1;
	  }*/

  }

}

/*void automode(void *arugment){

	RTC_TimeTypeDef sTime;
	char uartBuffer[64];
	while(1){
		while(mode){
			HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);


			//if(sTime.Seconds % 5 == 0){

				snprintf(uartBuffer, sizeof(uartBuffer), "Time:%02d:%02d:%02d\r\n", sTime.Hours, sTime.Minutes, sTime.Seconds);
				UART_SendString(uartBuffer);
			//}

			vTaskDelay(pdMS_TO_TICKS(3000));
		}

		vTaskDelay(pdMS_TO_TICKS(3000));
	}

	*/
	/*microDelay(10000);

	clear_dust();
	solar_tracking = 1;
	char uartBuffer[64];
	snprintf(uartBuffer, sizeof(uartBuffer), "FI:%d\r\n", 0);
	UART_SendString(uartBuffer);
}*/

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
	}else if (dest_angle > 70){
		dest_angle = 70;
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


	  //HAL_GPIO_WritePin(TILT_EN_PORT, TILT_EN_PIN, GPIO_PIN_SET);


		// for(int i = 0; i < 30000000; i++){}

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

	  //HAL_GPIO_WritePin(TILT_EN_PORT, TILT_EN_PIN, GPIO_PIN_RESET);
	  HAL_GPIO_WritePin(TILT_DIR_PORT, TILT_DIR_PIN, GPIO_PIN_RESET);
      HAL_GPIO_WritePin(TILT_STEP_PORT, TILT_STEP_PIN, GPIO_PIN_RESET);


}

void step_from_panel_ang(int angle){
      int x;
	  int stopstep = 0;
	  char uartBuffer[64];



	  //HAL_GPIO_WritePin(PANEL_EN_PORT, PANEL_EN_PIN, GPIO_PIN_SET);


		//for(int i = 0; i < 30000000; i++){}

	  if (angle < 0){
		  angle = angle * (-1);
    	  HAL_GPIO_WritePin(PANEL_DIR_PORT, PANEL_DIR_PIN, GPIO_PIN_SET);
	  }
	  else{
    	  HAL_GPIO_WritePin(PANEL_DIR_PORT, PANEL_DIR_PIN, GPIO_PIN_RESET);
	  }


      microDelay(openDelay*4);

	  stopstep = stopstep + (int)(3200.0*(((double)angle/360.0)));
	  stopstep = stopstep * 60;

	      for(x=0; x<stopstep; x=x+1)
	      {
	        HAL_GPIO_WritePin(PANEL_STEP_PORT, PANEL_STEP_PIN, GPIO_PIN_SET);
	        microDelay(openDelay*2);
	        HAL_GPIO_WritePin(PANEL_STEP_PORT, PANEL_STEP_PIN, GPIO_PIN_RESET);
	        microDelay(openDelay*2);
	      }

	  //HAL_GPIO_WritePin(PANEL_EN_PORT, PANEL_EN_PIN, GPIO_PIN_RESET);
	  HAL_GPIO_WritePin(PANEL_DIR_PORT, PANEL_DIR_PIN, GPIO_PIN_RESET);
      HAL_GPIO_WritePin(PANEL_STEP_PORT, PANEL_STEP_PIN, GPIO_PIN_RESET);

}



void open_panel(int state){
	if (state == 1){//HAL_GPIO_WritePin(PANEL_EN_PORT, PANEL_EN_PIN, GPIO_PIN_SET);
		HAL_GPIO_WritePin(PANEL_DIR_PORT, PANEL_DIR_PIN, GPIO_PIN_RESET);



		 //for(int i = 0; i < 30000000; i++){}

	    microDelay(openDelay);
	    microDelay(openDelay);

		for (int i = 0; i < 5000; i++){
	        HAL_GPIO_WritePin(PANEL_STEP_PORT, PANEL_STEP_PIN, GPIO_PIN_SET);
	        microDelay(openDelay*2);
	        HAL_GPIO_WritePin(PANEL_STEP_PORT, PANEL_STEP_PIN, GPIO_PIN_RESET);
	        microDelay(openDelay*2);
		}

			while(HAL_GPIO_ReadPin(PANEL_SW_CLOSE_PORT, PANEL_SW_CLOSE_PIN) == GPIO_PIN_RESET){


				HAL_GPIO_WritePin(PANEL_STEP_PORT, PANEL_STEP_PIN, GPIO_PIN_SET);
				microDelay(openDelay*2);
				HAL_GPIO_WritePin(PANEL_STEP_PORT, PANEL_STEP_PIN, GPIO_PIN_RESET);
				microDelay(openDelay*2);

			}



		for (int i = 0; i < 1000; i++){
	        HAL_GPIO_WritePin(PANEL_STEP_PORT, PANEL_STEP_PIN, GPIO_PIN_SET);
	        microDelay(openDelay*2);
	        HAL_GPIO_WritePin(PANEL_STEP_PORT, PANEL_STEP_PIN, GPIO_PIN_RESET);
	        microDelay(openDelay*2);
		}

		//HAL_GPIO_WritePin(PANEL_EN_PORT, PANEL_EN_PIN, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(PANEL_DIR_PORT, PANEL_DIR_PIN, GPIO_PIN_RESET);
	    HAL_GPIO_WritePin(PANEL_STEP_PORT, PANEL_STEP_PIN, GPIO_PIN_RESET);


	    current_panel_angle = 70;
	    panel_angle_var = 70;


		char uartBuffer[64];
	    snprintf(uartBuffer, sizeof(uartBuffer), "FI:%d\r\n", 0);
	    UART_SendString(uartBuffer);
		/*
		//open_panel_angle(70);
		HAL_GPIO_WritePin(PANEL_DIR_PORT, PANEL_DIR_PIN, GPIO_PIN_RESET);



		 //for(int i = 0; i < 30000000; i++){}

	   microDelay(openDelay);
	   microDelay(openDelay);

		while(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_10) == GPIO_PIN_SET){


		   HAL_GPIO_WritePin(PANEL_STEP_PORT, PANEL_STEP_PIN, GPIO_PIN_SET);
		   microDelay(openDelay*2);
		   HAL_GPIO_WritePin(PANEL_STEP_PORT, PANEL_STEP_PIN, GPIO_PIN_RESET);
		   microDelay(openDelay*2);

		}

		for (int i = 0; i < 1000; i++){
		   HAL_GPIO_WritePin(PANEL_STEP_PORT, PANEL_STEP_PIN, GPIO_PIN_SET);
		   microDelay(openDelay*2);
		   HAL_GPIO_WritePin(PANEL_STEP_PORT, PANEL_STEP_PIN, GPIO_PIN_RESET);
		   microDelay(openDelay*2);
		}

		//HAL_GPIO_WritePin(PANEL_EN_PORT, PANEL_EN_PIN, GPIO_PIN_RESET);

		HAL_GPIO_WritePin(PANEL_DIR_PORT, PANEL_DIR_PIN, GPIO_PIN_RESET);
	   HAL_GPIO_WritePin(PANEL_STEP_PORT, PANEL_STEP_PIN, GPIO_PIN_RESET);
	*/
	}else if(state == 0){
		//HAL_GPIO_WritePin(PANEL_EN_PORT, PANEL_EN_PIN, GPIO_PIN_SET);
		HAL_GPIO_WritePin(PANEL_DIR_PORT, PANEL_DIR_PIN, GPIO_PIN_SET);



		 //for(int i = 0; i < 30000000; i++){}

	    microDelay(openDelay);
	    microDelay(openDelay);

		for (int i = 0; i < 5000; i++){
		        HAL_GPIO_WritePin(PANEL_STEP_PORT, PANEL_STEP_PIN, GPIO_PIN_SET);
		        microDelay(openDelay*2);
		        HAL_GPIO_WritePin(PANEL_STEP_PORT, PANEL_STEP_PIN, GPIO_PIN_RESET);
		        microDelay(openDelay*2);
			}


		while(HAL_GPIO_ReadPin(PANEL_SW_CLOSE_PORT, PANEL_SW_CLOSE_PIN) == GPIO_PIN_RESET){


	        HAL_GPIO_WritePin(PANEL_STEP_PORT, PANEL_STEP_PIN, GPIO_PIN_SET);
	        microDelay(openDelay*2);
	        HAL_GPIO_WritePin(PANEL_STEP_PORT, PANEL_STEP_PIN, GPIO_PIN_RESET);
	        microDelay(openDelay*2);

		}

		for (int i = 0; i < 1000; i++){
	        HAL_GPIO_WritePin(PANEL_STEP_PORT, PANEL_STEP_PIN, GPIO_PIN_SET);
	        microDelay(openDelay*2);
	        HAL_GPIO_WritePin(PANEL_STEP_PORT, PANEL_STEP_PIN, GPIO_PIN_RESET);
	        microDelay(openDelay*2);
		}

		//HAL_GPIO_WritePin(PANEL_EN_PORT, PANEL_EN_PIN, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(PANEL_DIR_PORT, PANEL_DIR_PIN, GPIO_PIN_RESET);
	    HAL_GPIO_WritePin(PANEL_STEP_PORT, PANEL_STEP_PIN, GPIO_PIN_RESET);


	    current_panel_angle = 0;
	    panel_angle_var = 0;


		char uartBuffer[64];
	    snprintf(uartBuffer, sizeof(uartBuffer), "FI:%d\r\n", 0);
	    UART_SendString(uartBuffer);

	}
}

void clear_dust(void){
	 HAL_GPIO_WritePin(DUST_PORT, DUST_PIN, GPIO_PIN_SET);
	 for(int i = 0; i < 10000000; i++){}
	 HAL_GPIO_WritePin(DUST_PORT, DUST_PIN, GPIO_PIN_RESET);
	 char uartBuffer[64];
	 snprintf(uartBuffer, sizeof(uartBuffer), "FI:%d\r\n", 0);
	 UART_SendString(uartBuffer);
}

void hill_climb(void){
	tilt_panel(-5,1);
			   	 for(int i = 0; i < 10000000; i++){}
			    	tilt_panel(0,1);
			   	 for(int i = 0; i < 10000000; i++){}
			    	tilt_panel(5,1);
			   	 for(int i = 0; i < 10000000; i++){}
			    	tilt_panel(10,1);
			   	 for(int i = 0; i < 10000000; i++){}
			    	tilt_panel(15,1);
			   	 for(int i = 0; i < 10000000; i++){}
			    	tilt_panel(10,1);
			    for(int i = 0; i < 20000000; i++){}
					char uartBuffer[64];
					snprintf(uartBuffer, sizeof(uartBuffer), "Optimal Hill Climb Angle Found\r\n");
					UART_SendString(uartBuffer);
}


void hill_climb_for_optimal_tilt(void *argument) {
		float variable_array[44] = {0.0};
	    int last_power;
	    int first_run = 0;
	    int direction = 1;
	    int improved = 0;
	    int last_angle;
	    float current_power;
	    while (1) {
	    	if(solar_tracking == 1){
		    	tilt_panel(-5,1);
		   	 for(int i = 0; i < 5000000; i++){}
		    	tilt_panel(0,1);
		   	 for(int i = 0; i < 5000000; i++){}
		    	tilt_panel(5,1);
		   	 for(int i = 0; i < 5000000; i++){}
		    	tilt_panel(10,1);
		   	 for(int i = 0; i < 5000000; i++){}
		    	tilt_panel(15,1);
		   	 for(int i = 0; i < 5000000; i++){}
		    	tilt_panel(10,1);
		    	solar_tracking == 0;
	    	}

	    	/*
	    	if(first_run == 0 && solar_tracking == 1){
	    		//HAL_ADC_Start(&hadc1, ADC_VAL, 4);
	    		//HAL_ADC_PollForConversion(&hadc1, 20);
	    		//current_power = HAL_ADC_GetValue(&hadc1);
	            variable_array[tilt_angle_var + 22] = power_generation;
	            first_run = 1;
	    	}

	        if (solar_tracking == 1) {
	            last_angle = tilt_angle_var;
	            tilt_panel(tilt_angle_var + (5 * direction), 1);

	            //char uartBuffer[64];
	            //snprintf(uartBuffer, sizeof(uartBuffer), "Line 367: %i ", tilt_angle_var + (5 * direction));
	            //UART_SendString(uartBuffer);

	    		//HAL_ADC_Start(&hadc1);
	    		//HAL_ADC_PollForConversion(&hadc1, 20);
	    		//current_power = HAL_ADC_GetValue(&hadc1);

	            variable_array[tilt_angle_var+22] = power_generation;
	            if (variable_array[tilt_angle_var+22] < variable_array[last_angle+22]) {
	                direction = -1;
	                if(improved || tilt_angle_var >= 22 || tilt_angle_var <= -22){
	                	for (int i = 0; i < 45; i++){
		    	            char uartBuffer[64];
		    	            snprintf(uartBuffer, sizeof(uartBuffer), "%i: %i, \r\n", i, variable_array[i]);
		    	            UART_SendString(uartBuffer);
		    	            break;
	                	}
						char uartBuffer[64];
						snprintf(uartBuffer, sizeof(uartBuffer), "FI:%d\r\n", 0);
						UART_SendString(uartBuffer);

	    	            improved = 0;
	    	            direction = 1;
	    	            solar_tracking = 0;
	    	            first_run = 0;


	                }

	            } else if(variable_array[tilt_angle_var+22] > variable_array[last_angle+22]){
	                improved = 1;
	            }
	            else{
	            	if(tilt_angle_var == -22){
	            		direction = 1;
	            	}
	            	else{
	            		direction = -1;
	            	}
					char uartBuffer[64];
					snprintf(uartBuffer, sizeof(uartBuffer), "FI:%d\r\n", 0);
					UART_SendString(uartBuffer);

    	            improved = 0;
    	            solar_tracking = 0;
    	            first_run = 0;
	            }

	        }*/

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
    //xTaskCreate(automode, "AutoMode", 256, NULL, 4, &AutoMode_Handler);

    vTaskStartScheduler();
}
