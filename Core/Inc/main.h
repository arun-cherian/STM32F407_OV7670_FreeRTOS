/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdbool.h>
#include "FreeRTOS.h"
#include "task.h"
#include "event_groups.h"
#include "queue.h"
#include "semphr.h"
//#include "sd_diskio.h"
//#include "bsp_driver_sd.h"



/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */
#define IMG_COLUMNS 160
#define IMG_ROWS    120
#define OV7670_I2C_ADDR 0x21 // 8-bit write address
#define STREAM_ENABLED     (1 << 0)
#define CAMERA_READY       (1 << 1)
#define OV7670_REG_NUM 89
#define FRAME_BUFFER_SIZE IMG_COLUMNS*IMG_ROWS
#define BSP_FUNC 0

// Shared structure for USB commands
typedef enum {
    CMD_START_STREAM,
    CMD_STOP_STREAM,
    CMD_SET_REGISTER
} USBCommandType;

typedef struct {
    USBCommandType type;
    uint8_t reg_addr;
    uint8_t value;
} USBCommand;

// Shared structure for Frame Chunks
typedef struct {
    uint8_t* buffer;      // Pointer to the start of the data chunk
    uint32_t size_in_bytes; // Size of the chunk
} FrameChunk_t;

// Declare the queue handle globally

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */
void USBCommandTask(void *pvParameters);
void USBFrameSendTask(void);
void demo(void);
bool SCCB_write_reg(uint8_t reg_addr, uint8_t value);
void sd_raw_test(void);
/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/

/* USER CODE BEGIN Private defines */
extern volatile uint16_t frame_buffer[IMG_COLUMNS * IMG_ROWS];
extern EventGroupHandle_t xSystemEvents;
extern QueueHandle_t xFrameChunkQueue;
extern QueueHandle_t xUSBCommandQueue;
extern SemaphoreHandle_t xI2CSemaphore;
extern UART_HandleTypeDef huart2;
extern TaskHandle_t xSDTaskHandle;

// HAL Handles
extern DCMI_HandleTypeDef hdcmi;
extern I2C_HandleTypeDef hi2c1;
extern TaskHandle_t xFrameTaskHandle;
extern SD_HandleTypeDef hsd;

extern int line_num;
/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
