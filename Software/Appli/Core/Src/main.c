/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body for STM32H7S3L8 UVC Webcam
  * @details        : DCMI + DMA + USB UVC implementation for 400x400@30fps camera
  ******************************************************************************
  */

#include "main.h"
#include "usbd_core.h"
#include "usbd_desc.h"
#include "usbd_video.h"
#include "usbd_video_if.h"
#include "stm32h7rsxx_hal_pwr_ex.h"

/* Private defines -----------------------------------------------------------*/
#define CAMERA_WIDTH           320 //400
#define CAMERA_HEIGHT          240 //400
#define CAMERA_FPS             30
#define CAMERA_FRAME_SIZE      (CAMERA_WIDTH * CAMERA_HEIGHT * 2)  // MONOCHROME_8B

/* Private variables ---------------------------------------------------------*/
DCMIPP_HandleTypeDef phdcmipp;
DMA_HandleTypeDef hdma_dcmipp;
USBD_HandleTypeDef hUsbDeviceFS;

/* Frame buffers - double buffering for continuous capture */
__attribute__((section(".axisram"))) uint8_t frameBuffer1[CAMERA_FRAME_SIZE];
__attribute__((section(".axisram"))) uint8_t frameBuffer2[CAMERA_FRAME_SIZE];

volatile uint8_t *activeBuffer = frameBuffer1;
volatile uint8_t *readyBuffer = NULL;
volatile uint8_t frameReady = 0;

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_DCMIPP_Init(void);
static void MX_USB_DEVICE_Init(void);
static void MPU_Config(void);
static void CPU_CACHE_Enable(void);

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* Configure the MPU */
  MPU_Config();

  /* Enable the CPU Cache */
  CPU_CACHE_Enable();

  /* MCU Configuration--------------------------------------------------------*/
  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* Configure the system clock */
  SystemClock_Config();

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_DCMIPP_Init();
  MX_USB_DEVICE_Init();

  /* Start DCMIPP Pipe0 capture in continuous mode */
  HAL_DCMIPP_PIPE_Start(&phdcmipp, DCMIPP_PIPE0, (uint32_t)activeBuffer, DCMIPP_MODE_CONTINUOUS);

  /* Infinite loop */
  while (1)
  {
    /* Check if a frame is ready to be sent via USB */
    if (frameReady && readyBuffer != NULL)
    {
      /* Send frame via USB UVC */
      USBD_VIDEO_SendFrame(&hUsbDeviceFS, (uint8_t *)readyBuffer, CAMERA_FRAME_SIZE);

      frameReady = 0;
    }
  }
}

/**
  * @brief DCMIPP Initialization Function
  * @param None
  * @retval None
  */
static void MX_DCMIPP_Init(void)
{
	DCMIPP_ParallelConfTypeDef ParallelConfig = {0};
	DCMIPP_PipeConfTypeDef PipeConfig = {0};
	phdcmipp.Instance = DCMIPP;

	if (HAL_DCMIPP_Init(&phdcmipp) != HAL_OK)
	{
	   Error_Handler();
	}
	ParallelConfig.PCKPolarity = DCMIPP_PCKPOLARITY_RISING ;
	ParallelConfig.HSPolarity = DCMIPP_HSPOLARITY_LOW ;
	ParallelConfig.VSPolarity = DCMIPP_VSPOLARITY_HIGH ;
	ParallelConfig.ExtendedDataMode = DCMIPP_INTERFACE_8BITS;
	ParallelConfig.Format = DCMIPP_FORMAT_MONOCHROME_8B;
	ParallelConfig.SwapBits = DCMIPP_SWAPBITS_DISABLE;
	ParallelConfig.SwapCycles = DCMIPP_SWAPCYCLES_DISABLE;
	ParallelConfig.SynchroMode = DCMIPP_SYNCHRO_HARDWARE;

	HAL_DCMIPP_PARALLEL_SetConfig(&phdcmipp, &ParallelConfig);

	/* Configure DCMIPP Pipe0 for main capture */
	PipeConfig.FrameRate = DCMIPP_FRAME_RATE_ALL;  // Capture all frames

  if (HAL_DCMIPP_PIPE_SetConfig(&phdcmipp, DCMIPP_PIPE0, &PipeConfig) != HAL_OK)
  {
    Error_Handler();
  }

}

/**
  * @brief DMA Initialization Function
  * @param None
  * @retval None
  */
static void MX_DMA_Init(void)
{
  /* DMA controller clock enable */
  __HAL_RCC_HPDMA1_CLK_ENABLE();

  /* HPDMA1 interrupt init */
  HAL_NVIC_SetPriority(HPDMA1_Channel0_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(HPDMA1_Channel0_IRQn);
}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();

  /* Configure GPIO pins for DCMIPP:
   * PA6  -> DCMIPP_PIXCLK
   * PB7  -> DCMIPP_VSYNC
   * PC6  -> DCMIPP_D0
   *
   * Note: You'll need to add D1-D7 pins based on your camera connection
   */

  /* DCMIPP_PIXCLK - PA6 */
  GPIO_InitStruct.Pin = GPIO_PIN_6;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF13_DCMIPP;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* DCMIPP_VSYNC - PB7 */
  GPIO_InitStruct.Pin = GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF13_DCMIPP;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* DCMIPP_D0 - PC6 */
  GPIO_InitStruct.Pin = GPIO_PIN_6;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF13_DCMIPP;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : PB15 */
  GPIO_InitStruct.Pin = GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}

/**
  * @brief USB Device Initialization Function
  * @param None
  * @retval None
  */
static void MX_USB_DEVICE_Init(void)
{
  /* Init Device Library */
  USBD_Init(&hUsbDeviceFS, &CUSTOMHID_Desc, DEVICE_FS);

  /* Enable USB power domain (VBUS detection on the STM32H7S3) */
  HAL_PWREx_EnableUSBReg();

  /* Wait until the USB regulator is ready */
  while (!__HAL_PWR_GET_FLAG(PWR_FLAG_USB33RDY)) {}

  if (USBD_Init(&hUsbDeviceFS, &CUSTOMHID_Desc, DEVICE_FS) != USBD_OK)
   {
     Error_Handler();
   }

  /* Add Supported Class */
  USBD_RegisterClass(&hUsbDeviceFS, &USBD_VIDEO);

  /* Add Interface callbacks for VIDEO Class */
  USBD_VIDEO_RegisterInterface(&hUsbDeviceFS, &USBD_VIDEO_fops_FS);

  /* Start Device Process */
  USBD_Start(&hUsbDeviceFS);
}

/**
  * @brief  Frame Event callback - Called when a complete frame is captured
  * @param  hdcmipp: pointer to a DCMIPP_HandleTypeDef structure
  * @param  Pipe: DCMIPP pipe (PIPE0 in our case)
  * @retval None
  */
void HAL_DCMIPP_PIPE_FrameEventCallback(DCMIPP_HandleTypeDef *hdcmipp, uint32_t Pipe)
{
  if (Pipe == DCMIPP_PIPE0)
  {
    /* Mark current buffer as ready for USB transmission */
    readyBuffer = activeBuffer;
    frameReady = 1;

    /* Switch to the other buffer for next capture */
    if (activeBuffer == frameBuffer1)
    {
      activeBuffer = frameBuffer2;
    }
    else
    {
      activeBuffer = frameBuffer1;
    }

    /* Restart capture to the new buffer */
    HAL_DCMIPP_PIPE_Start(hdcmipp, DCMIPP_PIPE0, (uint32_t)activeBuffer, DCMIPP_MODE_CONTINUOUS);
  }
}

/**
  * @brief  DCMIPP error callback
  * @param  hdcmipp: pointer to a DCMIPP_HandleTypeDef structure
  * @param  Pipe: DCMIPP pipe
  * @retval None
  */
void HAL_DCMIPP_PIPE_ErrorCallback(DCMIPP_HandleTypeDef *hdcmipp, uint32_t Pipe)
{
  if (Pipe == DCMIPP_PIPE0)
  {
    /* Restart capture on error */
    HAL_DCMIPP_PIPE_Start(hdcmipp, DCMIPP_PIPE0, (uint32_t)activeBuffer, DCMIPP_MODE_CONTINUOUS);
  }
}

/**
  * @brief  VSYNC Event callback
  * @param  hdcmipp: pointer to a DCMIPP_HandleTypeDef structure
  * @retval None
  */
void HAL_DCMIPP_VsyncEventCallback(DCMIPP_HandleTypeDef *hdcmipp)
{
  /* VSYNC received - new frame starting */
  /* Can be used for synchronization or timing if needed */
	(void)hdcmipp;
}

/**
  * @brief  Configure the MPU attributes
  * @param  None
  * @retval None
  */
static void MPU_Config(void)
{
  MPU_Region_InitTypeDef MPU_InitStruct;

  /* Disable the MPU */
  HAL_MPU_Disable();

  /* Configure the MPU for SDRAM */
  MPU_InitStruct.Enable = MPU_REGION_ENABLE;
  MPU_InitStruct.BaseAddress = 0x24000000; //Axisram 0x24000000 sdram 0x70000000
  MPU_InitStruct.Size = MPU_REGION_SIZE_512KB;
  MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
  MPU_InitStruct.IsBufferable = MPU_ACCESS_BUFFERABLE;
  MPU_InitStruct.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
  MPU_InitStruct.IsShareable = MPU_ACCESS_SHAREABLE;
  MPU_InitStruct.Number = MPU_REGION_NUMBER0;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL1;
  MPU_InitStruct.SubRegionDisable = 0x00;
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;

  HAL_MPU_ConfigRegion(&MPU_InitStruct);

  /* Enable the MPU */
  HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);
}

/**
  * @brief  CPU L1-Cache enable
  * @param  None
  * @retval None
  */
static void CPU_CACHE_Enable(void)
{
  /* Enable I-Cache */
  SCB_EnableICache();

  /* Enable D-Cache */
  SCB_EnableDCache();
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  __HAL_RCC_FMC_CLK_ENABLE();

  /** Configure the main internal regulator output voltage
  */
  //__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE0);

  while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}

  /* Configure HSE oscillator and PLL1 */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState       = RCC_HSE_BYPASS;  // HSE comes from ST-LINK
  RCC_OscInitStruct.PLL1.PLLState  = RCC_PLL_ON;
  RCC_OscInitStruct.PLL1.PLLSource = RCC_PLLSOURCE_HSE;

  /* PLL1 configuration for 250 MHz SYSCLK from 8 MHz HSE:
   * VCO input frequency  = HSE / PLLM = 8 MHz / 2 = 4 MHz
   * VCO output frequency = VCO input * PLLN = 4 MHz * 125 = 500 MHz
   * SYSCLK (PLL1P)       = VCO / PLLP = 500 MHz / 2 = 250 MHz
   * PLL1Q                = VCO / PLLQ = 500 MHz / 5 = 100 MHz (optional)
   * PLL1R                = VCO / PLLR = 500 MHz / 2 = 250 MHz (for DCMIPP)
   */
  RCC_OscInitStruct.PLL1.PLLM         = 2;
  RCC_OscInitStruct.PLL1.PLLN         = 125;
  RCC_OscInitStruct.PLL1.PLLP         = 2;
  RCC_OscInitStruct.PLL1.PLLQ         = 5;
  RCC_OscInitStruct.PLL1.PLLR         = 2;
  RCC_OscInitStruct.PLL1.PLLFractional = 0;

  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /* Configure CPU and bus clocks */
  RCC_ClkInitStruct.ClockType      = RCC_CLOCKTYPE_HCLK  | RCC_CLOCKTYPE_SYSCLK |
                                     RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2  |
                                     RCC_CLOCKTYPE_PCLK4 | RCC_CLOCKTYPE_PCLK5;
  RCC_ClkInitStruct.SYSCLKSource   = RCC_SYSCLKSOURCE_PLLCLK;  // Use PLL1P
  RCC_ClkInitStruct.SYSCLKDivider  = RCC_SYSCLK_DIV1;          // SYSCLK = 250 MHz
  RCC_ClkInitStruct.AHBCLKDivider  = RCC_HCLK_DIV2;            // HCLK = 125 MHz
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;            // APB1 = 62.5 MHz
  RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2;            // APB2 = 62.5 MHz
  RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2;            // APB4 = 62.5 MHz
  RCC_ClkInitStruct.APB5CLKDivider = RCC_APB5_DIV2;            // APB5 = 62.5 MHz

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure DCMIPP clock source */
  //PeriphClkInit.PeriphClockSelection  = RCC_PERIPHCLK_DCMIPP | RCC_PERIPHCLK_USB;
  //PeriphClkInit.DcmippClockSelection  = RCC_DCMIPPCLKSOURCE_PLL1R;
  //PeriphClkInit.UsbClockSelection     = RCC_USBCLKSOURCE_HSE;   // 8 MHz; internal USB PLL scales to 48

  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  __disable_irq();
  while (1)
  {
    /* Error - stay here */
  }
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* User can add implementation to report error */
}
#endif /* USE_FULL_ASSERT */
