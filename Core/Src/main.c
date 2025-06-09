/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body for DCMI test
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "usb_device.h"
#include "cmsis_os.h" // Keep for HAL types, but RTOS is not used

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <stdbool.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
// Use QVGA resolution for a standard test
#define IMG_COLUMNS 320
#define IMG_ROWS    240
#define OV7670_I2C_ADDR (0x21 << 1) // 7-bit address 0x21
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
DCMI_HandleTypeDef hdcmi;
DMA_HandleTypeDef hdma_dcmi;
I2C_HandleTypeDef hi2c1;
UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
volatile uint16_t frame_buffer[IMG_COLUMNS * IMG_ROWS];
volatile bool g_frame_ready = false; // Flag to signal frame capture
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_DCMI_Init(void);
static void MX_I2C1_Init(void);
static void MX_USART2_UART_Init(void);

/* USER CODE BEGIN PFP */
bool OV7670_init(void);
bool SCCB_Write(uint8_t reg, uint8_t data);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

#ifdef __GNUC__
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif
PUTCHAR_PROTOTYPE
{
  HAL_UART_Transmit(&huart2, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
  return ch;
}

// Minimal register list for QVGA RGB565
const uint8_t ov7670_init_reg_tbl[][2] = {
	{0x12, 0x80}, // Reset registers
	{0x11, 0x80}, // CLKRC, Internal clock pre-scaler
	{0x3a, 0x04}, // TSLB,
	{0x12, 0x00}, // COM7, QVGA output
	{0x17, 0x13}, // HSTART,
	{0x18, 0x01}, // HSTOP,
	{0x32, 0xb6}, // HREF,
	{0x19, 0x02}, // VSTRT,
	{0x1a, 0x7a}, // VSTOP,
	{0x03, 0x0a}, // VREF,
	{0x0c, 0x00}, // COM3,
	{0x3e, 0x00}, // COM14,
	{0x70, 0x3a}, // SCALING_XSC,
	{0x71, 0x35}, // SCALING_YSC,
	{0x72, 0x11}, // SCALING_DCWCTR,
	{0x73, 0xf0}, // SCALING_PCLK_DIV,
	{0xa2, 0x02}, // SCALING_PCLK_DELAY,
	{0x15, 0x02}, // COM10, PCLK does not toggle on HBLANK
	{0x40, 0xc0}, // COM15, Full range output, RGB565
	{0x7a, 0x20}, // TSLB,
	{0x13, 0xe0}, // COM8, Enable AWB, AGC, AEC
	{0x00, 0x00}, // GAIN
	{0x10, 0x00}, // AECH
	{0x0e, 0x61}, // COM5
	{0x0f, 0x4b}, // COM6
	{0x16, 0x02}, // COM11
	{0x22, 0x91}, // BDBase
	{0x29, 0x07}, // BDMStep
	{0x3b, 0x0a}, // COM12
	{0x3c, 0x78}, // DBLV
	{0x3d, 0x40}, // AWBCTR3
	{0x69, 0x00}, // GFIX
	{0x09, 0x10}, // COM2
	{0x14, 0x1a}, // COM9 - Max AGC value
	{0x4f, 0x80}, // MTX1
	{0x50, 0x80}, // MTX2
	{0x51, 0x00}, // MTX3
	{0x52, 0x22}, // MTX4
	{0x53, 0x5e}, // MTX5
	{0x54, 0x80}, // MTX6
	{0x58, 0x9e}, // MTXS
	{0x13, 0xe7}, // COM8 - Enable AWB, AGC, AEC
	{0, 0}       // End of table
};

bool SCCB_Write(uint8_t reg, uint8_t data) {
    uint8_t tx_buffer[2] = {reg, data};
    if (HAL_I2C_Master_Transmit(&hi2c1, OV7670_I2C_ADDR, tx_buffer, 2, 100) != HAL_OK) {
        return false; // Error
    }
    return true; // Success
}

bool OV7670_init(void) {
    printf("  Checking for OV7670 camera...\r\n");
    if (HAL_I2C_IsDeviceReady(&hi2c1, OV7670_I2C_ADDR, 3, 100) != HAL_OK) {
        printf("  ERROR: OV7670 not found on I2C bus.\r\n");
        return false;
    }
    printf("  OV7670 detected. Configuring registers...\r\n");

    for (int i = 0; ov7670_init_reg_tbl[i][0]; i++) {
        if (!SCCB_Write(ov7670_init_reg_tbl[i][0], ov7670_init_reg_tbl[i][1])) {
            printf("  ERROR: Failed to write register 0x%02X\r\n", ov7670_init_reg_tbl[i][0]);
            return false;
        }
        HAL_Delay(1); // Small delay between writes
    }
    printf("  OV7670 configuration complete.\r\n");
    return true;
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* MCU Configuration--------------------------------------------------------*/
  HAL_Init();
  SystemClock_Config();

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_DCMI_Init();
  MX_I2C1_Init();
  MX_USART2_UART_Init();

  printf("\r\n--- DCMI Hardware Test ---\r\n");

  // Initialize the camera
  if (!OV7670_init()) {
      printf("FATAL: Camera initialization failed. Halting.\r\n");
      Error_Handler();
  }

  printf("Starting DCMI in snapshot mode. Waiting for one frame...\r\n");
  g_frame_ready = false; // Ensure flag is false before starting

  // Start the DCMI and DMA to capture one frame
  if (HAL_DCMI_Start_DMA(&hdcmi, DCMI_MODE_SNAPSHOT, (uint32_t)frame_buffer, IMG_COLUMNS * IMG_ROWS) != HAL_OK) {
      printf("ERROR: HAL_DCMI_Start_DMA failed!\r\n");
      Error_Handler();
  }

  uint32_t timeout_counter = 0;
  const uint32_t TIMEOUT_LIMIT = 5000; // ~5 seconds

  while (1)
  {
    if (g_frame_ready) {
        printf("\r\nSUCCESS: Frame captured! DCMI is working.\r\n");

        // Optional: Check a few pixel values to see if they are not zero
        printf("  - Pixel at [0][0]: 0x%04X\r\n", frame_buffer[0]);
        printf("  - Pixel at [10][10]: 0x%04X\r\n", frame_buffer[10 * IMG_COLUMNS + 10]);
        printf("\r\nTest finished.\r\n");
        while(1) {
            // Halt here after success
        }
    }

    // Check for timeout
    if (timeout_counter++ > TIMEOUT_LIMIT) {
        printf("\r\nTIMEOUT: No frame received!\r\n");
        printf("  - Check camera XCLK, PCLK, VSYNC, HREF signals.\r\n");
        printf("  - Verify camera power and I2C connection.\r\n");
        printf("Test failed. Halting.\r\n");
        HAL_DCMI_Stop(&hdcmi);
        Error_Handler();
    }

    HAL_Delay(1); // Main loop delay
  }
}

/**
  * @brief  Frame event callback.
  * @param  hdcmi: DCMI handle
  * @retval None
  * @note   This callback is executed when the DCMI VSYNC signal is received,
  * signifying the end of a frame capture.
  */
void HAL_DCMI_FrameEventCallback(DCMI_HandleTypeDef *hdcmi)
{
  g_frame_ready = true;
}

// System configuration functions (SystemClock_Config, MX_..._Init)
// remain the same as in your original file.
// Make sure they are correctly implemented below.

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 192; // For 96MHz SysClk, assuming 8MHz HSE from ST-Link MCO
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4; // For USB
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
  {
    Error_Handler();
  }
    // MCO1 output on PA8 to provide XCLK for the camera
  HAL_RCC_MCOConfig(RCC_MCO1, RCC_MCO1SOURCE_PLLCLK, RCC_MCODIV_4); // e.g. 96MHz / 4 = 24MHz
}

/**
  * @brief DCMI Initialization Function
  */
static void MX_DCMI_Init(void)
{
  hdcmi.Instance = DCMI;
  hdcmi.Init.SynchroMode = DCMI_SYNCHRO_HARDWARE;
  hdcmi.Init.PCKPolarity = DCMI_PCKPOLARITY_RISING; // OV7670 can be rising or falling
  hdcmi.Init.VSPolarity = DCMI_VSPOLARITY_LOW;
  hdcmi.Init.HSPolarity = DCMI_HSPOLARITY_LOW;
  hdcmi.Init.CaptureRate = DCMI_CR_ALL_FRAME;
  hdcmi.Init.ExtendedDataMode = DCMI_EXTEND_DATA_8B;
  hdcmi.Init.JPEGMode = DCMI_JPEG_DISABLE;
  if (HAL_DCMI_Init(&hdcmi) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief I2C1 Initialization Function
  */
static void MX_I2C1_Init(void)
{
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 100000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief USART2 Initialization Function
  */
static void MX_USART2_UART_Init(void)
{
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{
  __HAL_RCC_DMA2_CLK_ENABLE();
  /* DMA interrupt init */
  /* DMA2_Stream1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Stream1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream1_IRQn);
}

/**
  * @brief GPIO Initialization Function
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* GPIO Ports Clock Enable */
  __HAL_RCC_HSE_CLK_ENABLE(); // For HSE oscillator on PH0/PH1
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin : PA8 for MCO1 (Camera XCLK) */
  GPIO_InitStruct.Pin = GPIO_PIN_8;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF0_MCO;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}


/**
  * @brief  This function is executed in case of error occurrence.
  */
void Error_Handler(void)
{
  __disable_irq();
  while (1)
  {
  }
}

#ifdef  USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line)
{
  printf("Wrong parameters value: file %s on line %lu\r\n", file, line);
}
#endif /* USE_FULL_ASSERT */
