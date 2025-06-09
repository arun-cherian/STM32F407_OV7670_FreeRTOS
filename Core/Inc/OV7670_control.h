/*
 * OV7670_control.h
 *
 *  Created on: Jun 7, 2025
 *      Author: Arun
 */

#ifndef INC_OV7670_CONTROL_H_
#define INC_OV7670_CONTROL_H_


#include "stm32f4xx.h"
#include <stdbool.h>

// SCCB write address
#define SCCB_REG_ADDR 			0x01

// OV7670 camera settings
//#define OV7670_REG_NUM 			124

/*
// Image settings
#define IMG_ROWS   					320
#define IMG_COLUMNS   			240
*/

/*
#define IMG_ROWS   					320
#define IMG_COLUMNS   			120
*/
// Image buffer

/*
* Initialize MCO1 (XCLK)
*/
extern void MCO1_init(void);

/*
* Initialize SCCB
*/
extern void SCCB_init(void);

/*
* Initialize OV7670 camera
*/
extern bool OV7670_init(void);

extern bool OV7670_init_v2(void);
bool OV7670_init(void);

/*
* Initialize DCMI bus, DMA transfer
*/
extern void DCMI_DMA_init();

extern uint8_t init_count;


#endif /* INC_OV7670_CONTROL_H_ */
