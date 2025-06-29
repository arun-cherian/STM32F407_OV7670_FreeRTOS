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
#include "cmsis_os.h"
//#include "sd_diskio.h"
//#include "bsp_driver_sd.h"



/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */


#define IMG_COLUMNS 640
#define IMG_ROWS    70
#define OV7670_I2C_ADDR 0x21// 8-bit write address
#define STREAM_ENABLED     (1 << 0)
#define CAMERA_READY       (1 << 1)
#define OV7670_REG_NUM 89
#define FRAME_BUFFER_SIZE IMG_COLUMNS*IMG_ROWS
#define FRAME_BUFFER_WORD_SIZE IMG_COLUMNS*IMG_ROWS/2
#define BSP_FUNC 0

#define EVENT_BIT_BUTTON_PRESSED    (1 << 0)  // Bit 0
#define EVENT_BIT_VSYNC             (1 << 1)  // Bit 1
#define EVENT_BIT_FRAME_READY       (1 << 2)  // Bit 2
#define IMG_WIDTH_BYTES  2*IMG_COLUMNS
#define LINES_PER_CHUNK     4
#define BLOCKS_SD 			5
#define BYTES_PER_PIXEL 2   // Common for RGB565 format
#define IMAGE_SIZE_BYTES (IMG_COLUMNS * 200 * BYTES_PER_PIXEL)

// Define the size of our intermediate RAM buffer.
// It MUST be a multiple of the SD card block size (512 bytes).
// A size of 4KB to 8KB is a good balance between RAM usage and efficiency.
#define SD_READ_BUFFER_SIZE_BYTES (8 * 512) // 4096 bytes



// Calculate the number of lines per transfer chunk.
// This is your 'iteration' variable, but calculated more clearly.
//#define LINES_PER_CHUNK     (lcm(512, IMG_WIDTH_BYTES) / IMG_WIDTH_BYTES)


// Shared structure for USB commands

typedef enum {
    DCMI_EVENT_LINE,   // A single line of the image has been received
	DCMI_EVENT_VSYNC,
	DCMI_EVENT_FRAME,// A new frame is starting (VSYNC pulse)
} DCMI_EventType;

// 2. Define the message structure that will be sent to the queue.
//    This is what goes inside sizeof().
typedef struct {
    DCMI_EventType type;
    uint8_t value;
    // You could add more data here in the future, e.g., a timestamp.
} DCMI_Message_t;
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
void USBFrameSendTask(void);
void demo(void);
bool SCCB_write_reg(uint8_t reg_addr, uint8_t value);
void sd_raw_test(void);
void Start_DMA_in_double_buffer(void);
int gcd(int, int);
int lcm(int, int);
/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/

/* USER CODE BEGIN Private defines */
extern volatile uint16_t frame_buffer[IMG_COLUMNS * IMG_ROWS];
extern EventGroupHandle_t appEventGroup;
extern QueueHandle_t xFrameChunkQueue;
extern QueueHandle_t xUSBCommandQueue;
extern SemaphoreHandle_t xI2CSemaphore;
extern UART_HandleTypeDef huart2;
extern TaskHandle_t xSDTaskHandle;
extern TaskHandle_t hUsbStreamTask;
extern SemaphoreHandle_t sdCardMutex;

// HAL Handles
extern DMA_HandleTypeDef hdma_dcmi;
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
