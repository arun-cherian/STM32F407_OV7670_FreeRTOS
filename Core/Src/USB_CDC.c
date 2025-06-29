/*
 * usb_cdc_handler.c
 *
 * Created on: Jun 5, 2025
 * Author: Arun
 */

#include "main.h" // This is the key! It brings in all types and externs.
#include "usbd_cdc_if.h"
#include "usb_device.h"

/* USER CODE BEGIN Includes */
#include <stdbool.h>
/* USER CODE END Includes */


// NEW: Bring in RTOS objects defined in main.c


// NEW: Bring in the USB device handle
extern USBD_HandleTypeDef hUsbDeviceFS;
volatile uint32_t usbTimestamp = 0;
// NEW: Prototype for the function we call


/**
  * @brief  Data received over USB OUT endpoint are sent over CDC interface
  * through this function.
  *
  * @note
  * This function will issue a NAK packet on any OUT packet received on
  * USB endpoint until exiting this function. If you exit this function
  * before transfer is complete on CDC interface (i.e. using DMA controller)
  * it will result in receiving more data while previous ones are still
  * not sent.
  *
  * @param  Buf: Buffer of data to be received
  * @param  Len: Number of data received (in bytes)
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
/*int8_t CDC_Receive_FS(uint8_t* Buf, uint32_t *Len) {
    USBCommand cmd;

    if(*Len >= 3 && Buf[0] == 'W') {  // Write register: 'W' <reg> <val>
        cmd.type = CMD_SET_REGISTER;
        cmd.reg_addr = Buf[1];
        cmd.value = Buf[2];
        xQueueSend(xUSBCommandQueue, &cmd, 10);
    }
    else if(*Len == 1) {
        if(Buf[0] == 'S') {        // Start stream
            cmd.type = CMD_START_STREAM;
            xQueueSend(xUSBCommandQueue, &cmd, 10);
        }
        else if(Buf[0] == 'X') {   // Stop stream
            cmd.type = CMD_STOP_STREAM;
            xQueueSend(xUSBCommandQueue, &cmd, 10);
        }
    }

    // Prepare to receive the next packet
    USBD_CDC_SetRxBuffer(&hUsbDeviceFS, Buf);
    USBD_CDC_ReceivePacket(&hUsbDeviceFS);
    return (USBD_OK);
}*/




// Assumes the global frame buffer is declared somewhere visible, e.g., in dcmi_driver.h
extern volatile uint16_t frame_buffer[];
#define FRAME_BUFFER_SIZE_BYTES (IMG_ROWS * IMG_COLUMNS * 2)
char sd_read_buffer[SD_READ_BUFFER_SIZE_BYTES];

/**
 * @brief  Waits for a notification, then reads an image from the SD card in chunks
 *         and streams it over USB CDC.
 * @param  argument: Not used in this implementation.
 */
void USBStreamFromSDTask(void *argument)
{
    // Store our own task handle so the ISR can notify us.
    hUsbStreamTask = xTaskGetCurrentTaskHandle();

    for (;;)
    {
        // =========================================================================
        // STEP 1: WAIT FOR THE "GO" SIGNAL FROM ANOTHER TASK
        // The task sleeps here consuming no CPU until another task calls
        // xTaskNotifyGive(hUsbStreamTask).
        // =========================================================================
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        printf("\n>>> [USBStream] Notification received. Attempting to lock SD card...\r\n");

        // =========================================================================
        // STEP 2: ACQUIRE THE MUTEX TO GAIN EXCLUSIVE ACCESS TO THE SD CARD
        // =========================================================================
        if (xSemaphoreTake(sdCardMutex, portMAX_DELAY) == pdTRUE)
        {
            printf("[USBStream] Mutex Acquired. Starting stream.\r\n");
            printf("[USBStream] Target image size: %lu bytes.\r\n", (uint32_t)IMAGE_SIZE_BYTES);

            // --- Initialize state for this transfer ---
            uint32_t total_bytes_sent = 0;
            uint32_t sd_read_address_blocks = 0; // Start reading from block 0
            uint32_t loop_iteration = 0;

            // Start timing the transfer
            uint32_t start_time = DWT->CYCCNT;

            // Send a Start-of-Frame marker to the PC application
            const char* sof_marker = "START";
            while (CDC_Transmit_FS((uint8_t*)sof_marker, strlen(sof_marker)) != USBD_OK)
            {
                taskYIELD(); // Let other tasks run if USB is busy
            }

            // =========================================================================
            // STEP 3: MAIN TRANSFER LOOP
            // Read one chunk from SD, wait for DMA, send over USB. Repeat.
            // =========================================================================
            while (total_bytes_sent < IMAGE_SIZE_BYTES)
            {
                printf("\r\n--- [USBStream] Iteration %lu ---\r\n", loop_iteration);

                // A: Calculate the size of the next chunk to read from the SD card
                uint32_t bytes_to_read = IMAGE_SIZE_BYTES - total_bytes_sent;
                if (bytes_to_read > SD_READ_BUFFER_SIZE_BYTES) {
                    bytes_to_read = SD_READ_BUFFER_SIZE_BYTES;
                }
                uint32_t blocks_to_read = bytes_to_read / 512;
                printf("[USBStream] Preparing to read %lu bytes (%lu blocks) from SD address %lu.\r\n",
                       bytes_to_read, blocks_to_read, sd_read_address_blocks);

                // B: Start the non-blocking SD Card DMA Read
                HAL_StatusTypeDef result = HAL_SD_ReadBlocks_DMA(&hsd, sd_read_buffer, sd_read_address_blocks, blocks_to_read);
                if (result != HAL_OK) {
                    printf("FATAL [USBStream]: HAL_SD_ReadBlocks_DMA() failed to START! Aborting.\r\n");
                    break; // Exit the while loop
                }

                // C: Wait efficiently for the DMA to complete. The task sleeps until the
                //    HAL_SD_RxCpltCallback ISR sends a notification.
                if (ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(5000)) == 0) {
                    printf("FATAL [USBStream]: Timed out waiting for SD Read DMA completion!\r\n");
                    HAL_SD_Abort(&hsd); // Clean up the failed DMA
                    break; // Exit the while loop
                }

                // D: If we get here, the DMA is done. Stream the buffer over USB.
                printf("[USBStream] DMA Read complete. Streaming %lu bytes to USB...\r\n", bytes_to_read);
                uint32_t ptr = 0;
                while (ptr < bytes_to_read)
                {
                    const uint16_t usb_packet_size = 64; // USB FS Max Packet Size
                    uint16_t size_to_send = (bytes_to_read - ptr > usb_packet_size) ? usb_packet_size : (bytes_to_read - ptr);
                    while (CDC_Transmit_FS(&sd_read_buffer[ptr], size_to_send) != USBD_OK) {
                        taskYIELD(); // USB is busy, yield CPU
                    }
                    ptr += size_to_send;
                }
                printf("[USBStream] ...USB chunk sent.\r\n");

                // E: Update progress counters for the next loop
                total_bytes_sent += bytes_to_read;
                sd_read_address_blocks += blocks_to_read;
                loop_iteration++;
            } // End of while(total_bytes_sent < IMAGE_SIZE_BYTES)

            // F: Finalize and print statistics
            uint32_t end_time = DWT->CYCCNT;
            float time_ms = (end_time - start_time) / (SystemCoreClock / 1000.0f);
            float speed_kbps = (total_bytes_sent / 1024.0f) / (time_ms / 1000.0f);
            printf("\n>>> [USBStream] Stream Complete. Total Sent: %lu bytes, Time: %.2f ms, Speed: %.2f KB/s <<<\r\n",
                   total_bytes_sent, time_ms, speed_kbps);

            // =========================================================================
            // STEP 4: RELEASE THE MUTEX SO OTHER TASKS CAN USE THE SD CARD
            // =========================================================================
            xSemaphoreGive(sdCardMutex);
            printf("[USBStream] Mutex Released.\r\n");
        } // End of if (xSemaphoreTake ...
    } // End of for(;;)
}

/*void USBFrameSendTask(void) // Can be called from your FrameProcessTask
{
    const uint16_t chunk_size = 64; // USB FS Max Packet Size
    uint32_t ptr = 0;

    // ⏱️ Start timing
    uint32_t start = DWT->CYCCNT;
    const char* sof_marker = "START";
    CDC_Transmit_FS((uint8_t*)sof_marker, strlen(sof_marker));
    while (ptr < FRAME_BUFFER_SIZE_BYTES)
    {
        uint16_t remaining = FRAME_BUFFER_SIZE_BYTES - ptr;
        uint16_t size_to_send = (remaining > chunk_size) ? chunk_size : remaining;

        // THE FIX IS HERE:
        // We cast the frame_buffer to a byte pointer and add the offset 'ptr'.
        uint8_t* data_source = (uint8_t*)frame_buffer + ptr;

        // Keep trying to send until the USB endpoint is free
        while (CDC_Transmit_FS(data_source, size_to_send) != USBD_OK)
        {
            // Give other tasks a chance to run if the USB is busy
            taskYIELD();
        }

        ptr += size_to_send;
    }

    // ⏱️ Stop timing
    uint32_t end = DWT->CYCCNT;
    float time_ms = (end - start) / (SystemCoreClock / 1000.0f);
    float speed_kbps = (FRAME_BUFFER_SIZE_BYTES / 1024.0f) / (time_ms / 1000.0f);

    printf("USB Transfer Complete. Time: %.2f ms, Speed: %.2f KB/s\r\n", time_ms, speed_kbps);
}
*/

