/*
 * control.h
 *
 *  Created on: Nov 13, 2024
 *      Author: Logan CUlver
 */

void SystemClock_Config(void);
//static void MX_GPIO_Init(void);
void AddCustomer_Task(void *argument);
void Teller_Task1(void *argument);
void Teller_Task2(void *argument);
void Teller_Task3(void *argument);
void PrintStatus_Task(void *argument);
void Seven_Segment_Task(void *argument);
void update_time(void);  // Add this prototype
void vApplicationIdleHook( void );
void run( void );
