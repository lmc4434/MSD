/*
 * 7segment.h
 *
 *  Created on: Nov 13, 2024
 *      Author: Logan CUlver
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef MFS_H_
#define MFS_H_

#include "stm32l476xx.h"


/// This must be called prior to any other MFS functions.
void MFS_init(void);
void MFS_print_int(int integer);
void MFS_7seg_refresh();

#endif /* MFS_H_ */
