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
int8_t CDC_Receive_FS(uint8_t* Buf, uint32_t *Len) {
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
}


void USBCommandTask(void *pvParameters) {
    USBCommand cmd;
    while (1) {
    	printf("USBTaskRunning");
        if (xQueueReceive(xUSBCommandQueue, &cmd, portMAX_DELAY) == pdPASS) {
            switch(cmd.type) {
                case CMD_START_STREAM:
                    xEventGroupSetBits(xSystemEvents, STREAM_ENABLED);
                    break;

                case CMD_STOP_STREAM:
                    xEventGroupClearBits(xSystemEvents, STREAM_ENABLED);
                    break;

                case CMD_SET_REGISTER:
                    xSemaphoreTake(xI2CSemaphore, portMAX_DELAY);
                    SCCB_write_reg(cmd.reg_addr, cmd.value);
                    xSemaphoreGive(xI2CSemaphore);
                    break;
            }
        }
    }
}


