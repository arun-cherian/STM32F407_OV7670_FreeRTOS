#include "main.h"
#include "SD.h"


TaskHandle_t xSDTaskHandle;
uint16_t tx_buffer [LINES_PER_CHUNK * IMG_COLUMNS];
uint16_t rx_buffer[30 * 512];

void BSP_SD_WriteCpltCallback(void)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    // Notify the SD task that a write operation has completed.
    // Using bit 0 (value 0x01) to indicate a write completion.
    xTaskNotifyFromISR(xSDTaskHandle, 0x01, eSetBits, &xHigherPriorityTaskWoken);

    // If a higher priority task was woken, yield at the end of the ISR.
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

/**
  * @brief Read Transfer complete callback.
  * @retval None
  */
void BSP_SD_ReadCpltCallback(SD_HandleTypeDef *hsd)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    // Use the simple "give" function. This just increments the target task's
    // notification counting semaphore.
    if (hUsbStreamTask != NULL)
    {
        vTaskNotifyGiveFromISR(hUsbStreamTask, &xHigherPriorityTaskWoken);
    }

    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}
void flushFrameChunkQueue() {
    DCMI_Message_t dummy;
    while (xQueueReceive(xFrameChunkQueue, &dummy, 0) == pdTRUE);
}

uint8_t* p_frame_buffer = (uint8_t*)frame_buffer;
uint8_t* p_tx_buffer = (uint8_t*)tx_buffer;

void MeM_to_SD(void *argument) {
    const TickType_t xTicksToWait = portMAX_DELAY;
    DCMI_Message_t received_message;
    uint32_t sd_write_address = 0; // The block address on the SD card

    for (;;) {
    	xEventGroupWaitBits(appEventGroup, EVENT_BIT_BUTTON_PRESSED, pdTRUE, pdFALSE, xTicksToWait);
        // 1. Wait for the button press trigger.
        if (xSemaphoreTake(sdCardMutex, portMAX_DELAY) == pdTRUE)
        {
            xEventGroupSetBits(appEventGroup, EVENT_BIT_VSYNC);
			//printf("Button pressed. Starting capture...\r\n");
			sd_write_address = 0; // Reset SD address for the new image.
			flushFrameChunkQueue();

			// 2. Start DCMI and Synchronize with VSYNC.
			DCMI_Start_Capture(frame_buffer, sizeof(frame_buffer));
			uint16_t offset;
			//printf("Synchronizing... Waiting for VSYNC.\r\n");
			bool vsync_received = false;
			while (!vsync_received) {
				if (xQueueReceive(xFrameChunkQueue, &received_message, portMAX_DELAY) == pdPASS) {
					offset=received_message.value;
					if (received_message.type == DCMI_EVENT_VSYNC) {
						vsync_received = true;
						offset++;
					}
				}
			}
			//printf("SYNC OK. Capturing full frame.\r\n");

			// 3. Capture & Transfer Loop
			uint16_t lines_since_last_write = 0;
			uint16_t total_lines_processed = 0;
			bool frame_complete = false;

			while (!frame_complete) {
				// Wait for a line or VSYNC event from the ISR.
				if (xQueueReceive(xFrameChunkQueue, &received_message, portMAX_DELAY) == pdPASS) {
					if (received_message.type == DCMI_EVENT_LINE) {
						lines_since_last_write++;
						total_lines_processed++;

						// --- Check if a chunk is ready to be written ---
						if (lines_since_last_write == LINES_PER_CHUNK) {
							// A full chunk of lines is now in the DMA buffer. Time to write.

							// This is the starting line index of the chunk we need to read from the circular buffer.
							uint32_t chunk_start_line_idx = (offset+total_lines_processed - LINES_PER_CHUNK) % IMG_ROWS;

							//printf("Chunk ready. Lines %u-%u. Start index in buffer: %lu\r\n",
								   //total_lines_processed - LINES_PER_CHUNK + 1, total_lines_processed, chunk_start_line_idx);

							// --- Simplified Wrap-Around Logic ---
							uint32_t lines_at_end = IMG_ROWS - chunk_start_line_idx;

							if (lines_at_end >= LINES_PER_CHUNK) {
								// CASE 1: CONTIGUOUS. The whole chunk fits without wrapping.
								//printf("   -> Chunk is contiguous.\r\n");
								HAL_SD_WriteBlocks_DMA(&hsd, &p_frame_buffer[chunk_start_line_idx * IMG_WIDTH_BYTES], sd_write_address, BLOCKS_SD);
							} else {
								// CASE 2: WRAP-AROUND. The chunk is split.
								//printf("   -> Chunk has wrapped. Using staging buffer.\r\n");
								uint32_t lines_at_start = LINES_PER_CHUNK - lines_at_end;

								// 2a. Copy the first part (from the tail of the buffer)
								memcpy(p_tx_buffer, &p_frame_buffer[chunk_start_line_idx * IMG_WIDTH_BYTES], lines_at_end * IMG_WIDTH_BYTES);

								// 2b. Copy the second part (from the head of the buffer)
								memcpy(&p_tx_buffer[lines_at_end * IMG_WIDTH_BYTES], p_frame_buffer, lines_at_start * IMG_WIDTH_BYTES);

								// 2c. Write the now-contiguous staging buffer
								HAL_SD_WriteBlocks_DMA(&hsd, p_tx_buffer, sd_write_address, BLOCKS_SD);
							}

							// Wait for the SD card DMA to finish.
							ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
							while (HAL_SD_GetCardState(&hsd) != HAL_SD_CARD_TRANSFER);
							//printf("   -> SD Write Complete.\r\n");

							// Update state for the next chunk
							sd_write_address +=BLOCKS_SD;
							lines_since_last_write = 0; // Reset for next chunk
						}

						// Check if we've processed all the lines for the image


					} else if (received_message.type == DCMI_EVENT_VSYNC) {
						if(lines_since_last_write>0){
							// This is the starting line index of the chunk we need to read from the circular buffer.
							uint32_t chunk_start_line_idx = (offset+total_lines_processed - lines_since_last_write) % IMG_ROWS;

							//printf("Chunk ready. Lines %u-%u. Start index in buffer: %lu\r\n",
							//	   total_lines_processed - lines_since_last_write + 1, total_lines_processed, chunk_start_line_idx);

							// --- Simplified Wrap-Around Logic ---
							uint32_t lines_at_end = IMG_ROWS - chunk_start_line_idx-1;

							if (lines_at_end >= lines_since_last_write) {
								// CASE 1: CONTIGUOUS. The whole chunk fits without wrapping.
								//printf("   -> Chunk is contiguous.\r\n");
								HAL_SD_WriteBlocks_DMA(&hsd, &p_frame_buffer[chunk_start_line_idx * IMG_WIDTH_BYTES], sd_write_address, BLOCKS_SD);
							} else {
								// CASE 2: WRAP-AROUND. The chunk is split.
								//printf("   -> Chunk has wrapped. Using staging buffer.\r\n");
								uint32_t lines_at_start = lines_since_last_write - lines_at_end;

								// 2a. Copy the first part (from the tail of the buffer)
								memcpy(p_tx_buffer, &p_frame_buffer[chunk_start_line_idx * IMG_WIDTH_BYTES], lines_at_end * IMG_WIDTH_BYTES);

								// 2b. Copy the second part (from the head of the buffer)
								memcpy(&p_tx_buffer[lines_at_end * IMG_WIDTH_BYTES], frame_buffer, lines_at_start * IMG_WIDTH_BYTES);

								// 2c. Write the now-contiguous staging buffer
								HAL_SD_WriteBlocks_DMA(&hsd, p_tx_buffer, sd_write_address, BLOCKS_SD);
							}

							// Wait for the SD card DMA to finish.
							ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
							while (HAL_SD_GetCardState(&hsd) != HAL_SD_CARD_TRANSFER);
							//printf("   -> SD Write Complete.\r\n");

							// Update state for the next chunk
							sd_write_address +=lines_since_last_write;
							lines_since_last_write = 0; // Reset for next chunk
							// This VSYNC marks the end of our complete frame.

						}
						frame_complete = true;
						printf("VSYNC marked end of frame. Total lines processed: %u\r\n", total_lines_processed);
						xEventGroupClearBits(appEventGroup, EVENT_BIT_VSYNC);
					}
				}
			}
			// 4. Stop the capture and clean up
			HAL_DCMI_Stop(&hdcmi);
			HAL_DMA_Abort(&hdma_dcmi);

			xTaskNotify(hUsbStreamTask, 0x01, eSetBits);
			printf("Capture complete. Waiting for next button press.\r\n");
    }
		xSemaphoreGive(sdCardMutex);
    }
}



void sd_raw_test(void)
{
	uint32_t ulNotificationValue;
    const uint32_t block_count = 30;
    const uint32_t iterations = 100;
    const uint32_t start_block = 0;
    uint32_t start, end, duration;
    float speed_kb;

    // Fill TX buffer with pattern
    for (int i = 0; i < sizeof(tx_buffer); i++)
        tx_buffer[i] = (i % 255) + 1;
    memset(rx_buffer, 0, sizeof(rx_buffer));

    printf("\r\n--- SDIO DMA Multi-Block Speed Test ---\r\n");



    // --------------------- Write Test ---------------------
    printf("Writing %lu blocks per iteration, total %lu blocks (%.2f KB)...\r\n",
           block_count, block_count * iterations, (block_count * iterations * 512.0f) / 1024.0f);

    start = HAL_GetTick();
    for (uint32_t i = 0; i < iterations; i++) {
        HAL_StatusTypeDef result = HAL_SD_WriteBlocks_DMA(&hsd, tx_buffer, start_block, block_count);
        if (result != HAL_OK) {
            printf("Write DMA failed at iteration %lu. Status: %d, SD_Error: 0x%lX\r\n", i, result, HAL_SD_GetError(&hsd));
            vTaskDelete(NULL);
        }

        // Wait for transfer to complete
        ulNotificationValue = ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        while (HAL_SD_GetCardState(&hsd) != HAL_SD_CARD_TRANSFER);
    }
    end = HAL_GetTick();
    duration = end - start;
    speed_kb = (block_count * iterations * 512.0f / 1024.0f) / (duration / 1000.0f);
    printf("DMA Write OK. Time: %lu ms, Speed: %.2f KB/s\r\n", duration, speed_kb);

    HAL_Delay(10);

    // --------------------- Read Test ---------------------
    printf("Reading %lu blocks per iteration, total %lu blocks (%.2f KB)...\r\n",
           block_count, block_count * iterations, (block_count * iterations * 512.0f) / 1024.0f);

    start = HAL_GetTick();
    for (uint32_t i = 0; i < iterations; i++) {
        HAL_StatusTypeDef result = HAL_SD_ReadBlocks_DMA(&hsd, rx_buffer, start_block, block_count);
        if (result != HAL_OK) {
            printf("Read DMA failed at iteration %lu. Status: %d, SD_Error: 0x%lX\r\n", i, result, HAL_SD_GetError(&hsd));
            vTaskDelete(NULL);
        }

        // Wait for transfer to complete
        ulNotificationValue = ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        while (HAL_SD_GetCardState(&hsd) != HAL_SD_CARD_TRANSFER);
    }
    end = HAL_GetTick();
    duration = end - start;
    speed_kb = (block_count * iterations * 512.0f / 1024.0f) / (duration / 1000.0f);
    printf("DMA Read OK. Time: %lu ms, Speed: %.2f KB/s\r\n", duration, speed_kb);

    // --------------------- Verify (1st block only) ---------------------
    for (uint32_t i = 0; i < block_count * 512; i++)
    {
        if (rx_buffer[i] != ((i % 255) + 1))
        {
            printf("Data mismatch at byte %lu: expected %02X, got %02X\r\n", i, (i % 255) + 1, rx_buffer[i]);
            return;
        }
    }

    printf("Read/Write verification successful (1st block).\r\n");
    vTaskDelete(NULL);
}
