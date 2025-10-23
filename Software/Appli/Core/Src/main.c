/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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

/* STM32H7S3L8 USB UVC Webcam Implementation with DCMI
 *
 * Hardware Configuration:
 * - PB7: DCMI_VSYNC
 * - PA6: DCMI_PIXCLK
 * - PC6: DCMI_D0
 * - USB: PA11 (USB_DM), PA12 (USB_DP)
 *
 * Camera: 400x400 pixels @ 30fps
 * Format: YUY2 (YUYV) - 2 bytes per pixel
 */

#include "main.h"
#include "usbd_core.h"
#include "usbd_desc.h"
#include "usbd_video.h"

/* Camera Configuration */
#define CAM_WIDTH           400
#define CAM_HEIGHT          400
#define CAM_FPS             30
#define CAM_FRAME_SIZE      (CAM_WIDTH * CAM_HEIGHT * 2)  // YUY2 format

/* Double buffering for smooth operation */
#define NUM_BUFFERS         2
uint8_t camera_buffer[NUM_BUFFERS][CAM_FRAME_SIZE] __attribute__((aligned(32)));
volatile uint8_t active_buffer = 0;
volatile uint8_t ready_buffer = 0xFF;
volatile uint8_t frame_ready = 0;

/* Handles */
DCMI_HandleTypeDef hdcmi;
DMA_HandleTypeDef hdma_dcmi;
USBD_HandleTypeDef hUsbDeviceHS;

/* Function Prototypes */
void SystemClock_Config(void);
void GPIO_Init(void);
void DCMI_Init(void);
void DMA_Init(void);
void USB_Init(void);
void Error_Handler(void);

int main(void)
{
    /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
    HAL_Init();

    /* Configure the system clock */
    SystemClock_Config();

    /* Initialize all configured peripherals */
    GPIO_Init();
    DMA_Init();
    DCMI_Init();
    USB_Init();

    /* Start DCMI capture in continuous mode with DMA */
    HAL_DCMI_Start_DMA(&hdcmi, DCMI_MODE_CONTINUOUS,
                       (uint32_t)camera_buffer[active_buffer],
                       CAM_FRAME_SIZE / 4);  // Size in words

    while (1)
    {
        /* Main loop - USB and DMA handle frame transfers */
        HAL_Delay(10);
    }
}

void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    /* Supply configuration update enable */
    HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY);

    /* Configure the main internal regulator output voltage */
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE0);

    while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}

    /* Configure HSE and PLL */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLM = 4;
    RCC_OscInitStruct.PLL.PLLN = 250;
    RCC_OscInitStruct.PLL.PLLP = 2;
    RCC_OscInitStruct.PLL.PLLQ = 5;
    RCC_OscInitStruct.PLL.PLLR = 2;
    RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_1;
    RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
    RCC_OscInitStruct.PLL.PLLFRACN = 0;

    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        Error_Handler();
    }

    /* Configure system clocks */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK |
                                  RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2 |
                                  RCC_CLOCKTYPE_PCLK3;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB3CLKDivider = RCC_HCLK_DIV2;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
    {
        Error_Handler();
    }
}

void GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* Enable GPIO clocks */
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();

    /* DCMI GPIO Configuration:
     * PA6  -> DCMI_PIXCLK
     * PB7  -> DCMI_VSYNC
     * PC6  -> DCMI_D0
     */

    /* PA6 - DCMI_PIXCLK */
    GPIO_InitStruct.Pin = GPIO_PIN_6;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF13_DCMI;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* PB7 - DCMI_VSYNC */
    GPIO_InitStruct.Pin = GPIO_PIN_7;
    GPIO_InitStruct.Alternate = GPIO_AF13_DCMI;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* PC6 - DCMI_D0 */
    GPIO_InitStruct.Pin = GPIO_PIN_6;
    GPIO_InitStruct.Alternate = GPIO_AF13_DCMI;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    /* Note: Add remaining DCMI data pins (D1-D7) and HSYNC here if available */

    /* USB GPIO Configuration (if needed - usually handled by USB HAL) */
    /* PA11 -> USB_DM, PA12 -> USB_DP */
}

void DMA_Init(void)
{
    /* Enable DMA clock */
    __HAL_RCC_GPDMA1_CLK_ENABLE();

    /* Configure DMA for DCMI */
    hdma_dcmi.Instance = GPDMA1_Channel0;
    hdma_dcmi.Init.Request = GPDMA1_REQUEST_DCMI;
    hdma_dcmi.Init.BlkHWRequest = DMA_BREQ_SINGLE_BURST;
    hdma_dcmi.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_dcmi.Init.SrcInc = DMA_SINC_FIXED;
    hdma_dcmi.Init.DestInc = DMA_DINC_INCREMENTED;
    hdma_dcmi.Init.SrcDataWidth = DMA_SRC_DATAWIDTH_WORD;
    hdma_dcmi.Init.DestDataWidth = DMA_DEST_DATAWIDTH_WORD;
    hdma_dcmi.Init.Priority = DMA_HIGH_PRIORITY;
    hdma_dcmi.Init.SrcBurstLength = 4;
    hdma_dcmi.Init.DestBurstLength = 4;
    hdma_dcmi.Init.TransferAllocatedPort = DMA_SRC_ALLOCATED_PORT0 | DMA_DEST_ALLOCATED_PORT1;
    hdma_dcmi.Init.TransferEventMode = DMA_TCEM_BLOCK_TRANSFER;
    hdma_dcmi.Init.Mode = DMA_NORMAL;

    if (HAL_DMA_Init(&hdma_dcmi) != HAL_OK)
    {
        Error_Handler();
    }

    /* Link DMA to DCMI */
    __HAL_LINKDMA(&hdcmi, DMA_Handle, hdma_dcmi);

    /* NVIC configuration for DMA */
    HAL_NVIC_SetPriority(GPDMA1_Channel0_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(GPDMA1_Channel0_IRQn);
}

void DCMI_Init(void)
{
    /* Enable DCMI clock */
    __HAL_RCC_DCMI_CLK_ENABLE();

    /* Configure DCMI */
    hdcmi.Instance = DCMI;
    hdcmi.Init.SynchroMode = DCMI_SYNCHRO_HARDWARE;
    hdcmi.Init.PCKPolarity = DCMI_PCKPOLARITY_RISING;
    hdcmi.Init.VSPolarity = DCMI_VSPOLARITY_HIGH;
    hdcmi.Init.HSPolarity = DCMI_HSPOLARITY_LOW;
    hdcmi.Init.CaptureRate = DCMI_CR_ALL_FRAME;
    hdcmi.Init.ExtendedDataMode = DCMI_EXTEND_DATA_8B;
    hdcmi.Init.JPEGMode = DCMI_JPEG_DISABLE;
    hdcmi.Init.ByteSelectMode = DCMI_BSM_ALL;
    hdcmi.Init.ByteSelectStart = DCMI_OEBS_ODD;
    hdcmi.Init.LineSelectMode = DCMI_LSM_ALL;
    hdcmi.Init.LineSelectStart = DCMI_OELS_ODD;

    if (HAL_DCMI_Init(&hdcmi) != HAL_OK)
    {
        Error_Handler();
    }

    /* NVIC configuration for DCMI */
    HAL_NVIC_SetPriority(DCMI_PSSI_IRQn, 1, 0);
    HAL_NVIC_EnableIRQ(DCMI_PSSI_IRQn);
}

void USB_Init(void)
{
    /* Init USB device Library */
    if (USBD_Init(&hUsbDeviceHS, &VIDEO_Desc, DEVICE_HS) != USBD_OK)
    {
        Error_Handler();
    }

    /* Register the VIDEO class */
    if (USBD_RegisterClass(&hUsbDeviceHS, &USBD_VIDEO) != USBD_OK)
    {
        Error_Handler();
    }

    /* Start Device */
    if (USBD_Start(&hUsbDeviceHS) != USBD_OK)
    {
        Error_Handler();
    }
}

/* DCMI Frame Event Callback */
void HAL_DCMI_FrameEventCallback(DCMI_HandleTypeDef *hdcmi)
{
    /* Frame captured - mark buffer as ready */
    ready_buffer = active_buffer;
    frame_ready = 1;

    /* Switch to next buffer */
    active_buffer = (active_buffer + 1) % NUM_BUFFERS;

    /* Restart capture with new buffer */
    HAL_DCMI_Start_DMA(hdcmi, DCMI_MODE_CONTINUOUS,
                       (uint32_t)camera_buffer[active_buffer],
                       CAM_FRAME_SIZE / 4);
}

/* DMA Transfer Complete Callback */
void HAL_DCMI_VsyncEventCallback(DCMI_HandleTypeDef *hdcmi)
{
    /* VSYNC detected - new frame starting */
}

/* Get frame buffer for USB transmission */
uint8_t* USB_GetFrameBuffer(void)
{
    if (frame_ready)
    {
        return camera_buffer[ready_buffer];
    }
    return NULL;
}

/* Acknowledge frame sent */
void USB_FrameSent(void)
{
    frame_ready = 0;
}

/* IRQ Handlers */
void GPDMA1_Channel0_IRQHandler(void)
{
    HAL_DMA_IRQHandler(&hdma_dcmi);
}

void DCMI_PSSI_IRQHandler(void)
{
    HAL_DCMI_IRQHandler(&hdcmi);
}

void Error_Handler(void)
{
    __disable_irq();
    while (1)
    {
        /* Blink LED or handle error */
    }
}

#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line)
{
    /* User can add implementation to report file and line */
}
#endif
