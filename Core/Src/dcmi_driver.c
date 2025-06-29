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
//__attribute__((aligned(32))) volatile uint16_t frame_buffer1[IMG_COLUMNS/2 * IMG_ROWS];
//__attribute__((aligned(32))) volatile uint16_t frame_buffer2[IMG_COLUMNS/2 * IMG_ROWS];
__attribute__((aligned(32))) volatile uint16_t frame_buffer[IMG_COLUMNS * IMG_ROWS];
// Assume a FreeRTOS queue is created in main.c
// extern QueueHandle_t xFrameQueue;

HAL_StatusTypeDef HAL_DCMI_Start_DMA_DoubleBuffer(
    DCMI_HandleTypeDef *hdcmi,
    uint32_t DCMI_Mode,
    uint32_t pData1,
    uint32_t pData2,
    uint32_t Length)
{
    HAL_StatusTypeDef status;

    /* Set mode */
    hdcmi->Instance->CR &= ~(DCMI_CR_CM);
    hdcmi->Instance->CR |= DCMI_Mode;

    /* Start MultiBuffer DMA transfer */
    status = HAL_DMAEx_MultiBufferStart(
        hdcmi->DMA_Handle,
        (uint32_t)&hdcmi->Instance->DR,
        pData1,
        pData2,
        Length);

    if (status != HAL_OK) return status;

    __HAL_DMA_ENABLE(hdcmi->DMA_Handle);
    __HAL_DCMI_ENABLE(hdcmi);

    return HAL_OK;
}
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

void HAL_DMA_XferCpltCallback(DMA_HandleTypeDef *hdma)
{
    if (hdma == &hdma_dcmi)
    {
        // Buffer1 filled
        printf("buffer1\r\n");
    }
}

void HAL_DMA_XferM1CpltCallback(DMA_HandleTypeDef *hdma)
{
    if (hdma == &hdma_dcmi)
    {
        // Buffer0 filled
    	printf("buffer2\r\n");
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



/*void HAL_DCMI_VsyncEventCallback(DCMI_HandleTypeDef *hdcmi)
{
    static TickType_t prev = 0;
    line_num=0;
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
}*/
void HAL_DCMI_DmaXferCpltCallback(DCMI_HandleTypeDef *hdcmi)
{
	DCMI_Message_t vsync_event;
    vsync_event.type = DCMI_EVENT_FRAME;
    // This variable is required for the FromISR function
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    // Send the message to the queue.
    xQueueSendToBackFromISR(xFrameChunkQueue, &vsync_event, &xHigherPriorityTaskWoken);

    // If sending the message unblocked a higher-priority task, yield.
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}
void HAL_DCMI_VsyncEventCallback(DCMI_HandleTypeDef *hdcmi)
{
    EventBits_t bits = xEventGroupGetBitsFromISR(appEventGroup);
    if(bits&EVENT_BIT_VSYNC){
    // Prepare the VSYNC event message
	DCMI_Message_t vsync_event;
    vsync_event.type = DCMI_EVENT_VSYNC;
    // This variable is required for the FromISR function
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    // Send the message to the queue.
    xQueueSendToBackFromISR(xFrameChunkQueue, &vsync_event, &xHigherPriorityTaskWoken);

    // If sending the message unblocked a higher-priority task, yield.
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}

void HAL_DCMI_LineEventCallback(DCMI_HandleTypeDef *hdcmi)
{
	static uint8_t count=0;
    EventBits_t bits = xEventGroupGetBitsFromISR(appEventGroup);
    if(bits&EVENT_BIT_VSYNC){
	 //printf("%lu\r\n", (unsigned long)hdma_dcmi.Instance->NDTR);
	DCMI_Message_t line_event;
    line_event.type = DCMI_EVENT_LINE;
    count==30?count=0:count++;
    line_event.value=count;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    // Send the message to the queue
    xQueueSendToBackFromISR(xFrameChunkQueue, &line_event, &xHigherPriorityTaskWoken);

    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}

void DCMI_CaptureError_Callback(void)
{
    // This is a custom function you can call if HAL_DCMI_Start_DMA fails
    printf("Frame capture failed to start\r\n");
}
