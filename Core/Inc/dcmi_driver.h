/*
 * dcmi_driver.h
 *
 *  Created on: Jun 10, 2025
 *      Author: Arun
 */
#ifndef DCMI_DRIVER_H
#define DCMI_DRIVER_H

#include "main.h"


#ifdef __cplusplus
extern "C" {
#endif



void DCMI_Start_Capture(void);
void DCMI_CaptureComplete_Callback(void);
void DCMI_CaptureError_Callback(void);


#ifdef __cplusplus
}
#endif

#endif // DCMI_DRIVER_H
