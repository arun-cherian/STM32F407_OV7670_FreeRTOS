#include "main.h"
#include "SD.h"
TaskHandle_t xSDTaskHandle;
uint8_t tx_buffer[30 * 512];
uint8_t rx_buffer[30 * 512];

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
void BSP_SD_ReadCpltCallback(void)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    // Notify the SD task that a read operation has completed.
    // Using bit 1 (value 0x02) to indicate a read completion.
    xTaskNotifyFromISR(xSDTaskHandle, 0x02, eSetBits, &xHigherPriorityTaskWoken);

    // Yield if a higher priority task has been woken.
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
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
