/*
 * dcmi_driver.c
 *
 * Created on: Jun 10, 2025
 * Author: Arun
 */

#include "dcmi_driver.h"
#include <stdio.h>
#include "main.h" // Include main.h for IMG_... defines and handles

volatile uint32_t dmaTimestamp = 0;
int line_num=0;

// It's good practice to define a struct for passing frame chunks


// Make sure the buffer is 32-byte aligned for best DMA performance
__attribute__((aligned(32))) volatile uint16_t frame_buffer[IMG_COLUMNS * IMG_ROWS];

// Assume a FreeRTOS queue is created in main.c
// extern QueueHandle_t xFrameQueue;


void DCMI_Start_Capture(void)
{
    HAL_StatusTypeDef ret;

    printf("Starting DCMI capture in continuous mode...\r\n");

    // The length for HAL_DCMI_Start_DMA is the number of 32-bit words.
    // For 16-bit pixels (uint16_t), this is TotalPixels / 2.
    uint32_t transfer_length_in_words = (IMG_ROWS * IMG_COLUMNS) / 2;

    ret = HAL_DCMI_Start_DMA(&hdcmi, DCMI_MODE_CONTINUOUS, (uint32_t)frame_buffer, transfer_length_in_words);

    if (ret != HAL_OK)
    {
        printf("DCMI DMA Start FAILED with code %d\r\n", ret);
        //DCMI_CaptureError_Callback();
    }
    else
    {
        printf("DCMI DMA Start SUCCESSFUL\r\n");
    }
}



/**
  * @brief  Half DMA transfer callback.
  * @param  hdcmi: DCMI handle
  * @retval None
  */
#include "main.h" // Or your relevant header

// This is called on DMA Half-Transfer Complete
void My_DMA_HalfTransfer_Callback(DMA_HandleTypeDef *hdma)
{
	//printf("The DMA delta: %u\r\n",dmaTimestamp-DWT->CYCCNT);
	//HAL_DCMI_Stop(hdma);
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    // Notify bit 0 => half-frame
    vTaskNotifyGiveFromISR(xFrameTaskHandle, &xHigherPriorityTaskWoken);
    xTaskNotifyFromISR(xFrameTaskHandle,
                       0x01,
                       eSetBits,
                       &xHigherPriorityTaskWoken);

    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}



void HAL_DCMI_VsyncEventCallback(DCMI_HandleTypeDef *hdcmi)
{
    static TickType_t prev = 0;
    TickType_t now = xTaskGetTickCountFromISR();
    TickType_t delta = now - prev;
    prev = now;

    float delta_ms = delta * portTICK_PERIOD_MS;
    float fps = 1000.0f / delta_ms;

    printf("Frame Interval: %.2f ms | FPS: %.2f\r\n", delta_ms, fps);

    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
   //HAL_DCMI_Stop(hdcmi);
    xTaskNotifyFromISR(xFrameTaskHandle,
                       0x02,
                       eSetBits,
                       &xHigherPriorityTaskWoken);

    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}


/*void HAL_DCMI_LineEventCallback(DCMI_HandleTypeDef *hdcmi)
{
printf("line event: %d\r\n",line_num++);
}*/

void DCMI_CaptureError_Callback(void)
{
    // This is a custom function you can call if HAL_DCMI_Start_DMA fails
    printf("Frame capture failed to start\r\n");
}
