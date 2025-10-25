/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file         stm32h7rsxx_hal_msp.c
  * @brief        This file provides code for the MSP Initialization
  *               and de-Initialization codes.
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

/* Includes ------------------------------------------------------------------*/
#include "main.h"
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN TD */

/* USER CODE END TD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN Define */

/* USER CODE END Define */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN Macro */

/* USER CODE END Macro */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* External functions --------------------------------------------------------*/
/* USER CODE BEGIN ExternalFunctions */

/* USER CODE END ExternalFunctions */

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */
/**
  * Initializes the Global MSP.
  */
void HAL_MspInit(void)
{
RCC_OscInitTypeDef RCC_OscInitStruct = {0};

  /* Configure the system Power Supply */

  if (HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY) != HAL_OK)
  {
    /* Initialization error */
    Error_Handler();
  }

  /* USER CODE BEGIN MspInit 0 */

  /* USER CODE END MspInit 0 */
  PWR_PVDTypeDef sConfigPVD = {0};

  __HAL_RCC_SBS_CLK_ENABLE();

  /* System interrupt init*/

  /** PVD Configuration
  */
  sConfigPVD.PVDLevel = PWR_PVDLEVEL_EXT_VOL;
  sConfigPVD.Mode = PWR_PVD_MODE_NORMAL;
  HAL_PWR_ConfigPVD(&sConfigPVD);

  /** Enable the PVD Output
  */
  HAL_PWR_EnablePVD();

  /* Enable the XSPIM_P2 interface */
  HAL_PWREx_EnableXSPIM2();

  /* The CSI is used by the compensation cells and must be enabled before enabling the
     compensation cells.
     For more details refer to RM0477 [SBS I/O compensation cell management] chapter.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_CSI;
  RCC_OscInitStruct.CSIState = RCC_CSI_ON;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /* Configure the compensation cell */
  HAL_SBS_ConfigCompensationCell(SBS_IO_XSPI2_CELL, SBS_IO_CELL_CODE, 0U, 0U);

  /* Enable compensation cell */
  HAL_SBS_EnableCompensationCell(SBS_IO_XSPI2_CELL);

  /* wait ready before enabled IO */
  while(HAL_SBS_GetCompensationCellReadyStatus(SBS_IO_XSPI2_CELL_READY) != 1U);

  /* high speed low voltage config */
  HAL_SBS_EnableIOSpeedOptimize(SBS_IO_XSPI2_HSLV);

  /* USER CODE BEGIN MspInit 1 */

  /* USER CODE END MspInit 1 */
}

/**
  * @brief DCMIPP MSP Initialization
  * This function configures the hardware resources used in this example
  * @param hdcmipp: DCMIPP handle pointer
  * @retval None
  */
void HAL_DCMIPP_MspInit(DCMIPP_HandleTypeDef* hdcmipp)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(hdcmipp->Instance==DCMIPP)
  {
    /* USER CODE BEGIN DCMIPP_MspInit 0 */

    /* USER CODE END DCMIPP_MspInit 0 */
    /* Peripheral clock enable */
    __HAL_RCC_DCMIPP_CLK_ENABLE();

    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    /**DCMIPP GPIO Configuration
    PD3     ------> DCMIPP_D5
    PB8     ------> DCMIPP_D6
    PC11     ------> DCMIPP_D4
    PB9     ------> DCMIPP_D7
    PB7     ------> DCMIPP_VSYNC
    PD5     ------> DCMIPP_PIXCLK
    */
    GPIO_InitStruct.Pin = GPIO_PIN_3;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Alternate = GPIO_AF13_DCMIPP;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Alternate = GPIO_AF13_DCMIPP;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_11;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Alternate = GPIO_AF13_DCMIPP;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_5;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Alternate = GPIO_AF5_DCMIPP;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

    /* USER CODE BEGIN DCMIPP_MspInit 1 */

    /* USER CODE END DCMIPP_MspInit 1 */

  }

}

/**
  * @brief DCMIPP MSP De-Initialization
  * This function freeze the hardware resources used in this example
  * @param hdcmipp: DCMIPP handle pointer
  * @retval None
  */
void HAL_DCMIPP_MspDeInit(DCMIPP_HandleTypeDef* hdcmipp)
{
  if(hdcmipp->Instance==DCMIPP)
  {
    /* USER CODE BEGIN DCMIPP_MspDeInit 0 */

    /* USER CODE END DCMIPP_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_DCMIPP_CLK_DISABLE();

    /**DCMIPP GPIO Configuration
    PD3     ------> DCMIPP_D5
    PB8     ------> DCMIPP_D6
    PC11     ------> DCMIPP_D4
    PB9     ------> DCMIPP_D7
    PB7     ------> DCMIPP_VSYNC
    PD5     ------> DCMIPP_PIXCLK
    */
    HAL_GPIO_DeInit(GPIOD, GPIO_PIN_3|GPIO_PIN_5);

    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_7);

    HAL_GPIO_DeInit(GPIOC, GPIO_PIN_11);

    /* USER CODE BEGIN DCMIPP_MspDeInit 1 */

    /* USER CODE END DCMIPP_MspDeInit 1 */
  }

}

/**
  * @brief XSPI MSP Initialization
  * This function configures the hardware resources used in this example
  * @param hxspi: XSPI handle pointer
  * @retval None
  */
void HAL_XSPI_MspInit(XSPI_HandleTypeDef* hxspi)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};
  if(hxspi->Instance==XSPI2)
  {
    /* USER CODE BEGIN XSPI2_MspInit 0 */

    /* USER CODE END XSPI2_MspInit 0 */

  /** Initializes the peripherals clock
  */
    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_XSPI2;
    PeriphClkInit.Xspi2ClockSelection = RCC_XSPI2CLKSOURCE_PLL2S;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
    {
      Error_Handler();
    }

    /* Peripheral clock enable */
    __HAL_RCC_XSPIM_CLK_ENABLE();
    __HAL_RCC_XSPI2_CLK_ENABLE();

    __HAL_RCC_GPION_CLK_ENABLE();
    /**XSPI2 GPIO Configuration
    PN1     ------> XSPIM_P2_NCS1
    PN3     ------> XSPIM_P2_IO1
    PN0     ------> XSPIM_P2_DQS0
    PN11     ------> XSPIM_P2_IO7
    PN10     ------> XSPIM_P2_IO6
    PN9     ------> XSPIM_P2_IO5
    PN2     ------> XSPIM_P2_IO0
    PN6     ------> XSPIM_P2_CLK
    PN8     ------> XSPIM_P2_IO4
    PN4     ------> XSPIM_P2_IO2
    PN5     ------> XSPIM_P2_IO3
    */
    GPIO_InitStruct.Pin = GPIO_PIN_1|GPIO_PIN_3|GPIO_PIN_0|GPIO_PIN_11
                          |GPIO_PIN_10|GPIO_PIN_9|GPIO_PIN_2|GPIO_PIN_6
                          |GPIO_PIN_8|GPIO_PIN_4|GPIO_PIN_5;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF9_XSPIM_P2;
    HAL_GPIO_Init(GPION, &GPIO_InitStruct);

    /* USER CODE BEGIN XSPI2_MspInit 1 */

    /* USER CODE END XSPI2_MspInit 1 */

  }

}

/**
  * @brief XSPI MSP De-Initialization
  * This function freeze the hardware resources used in this example
  * @param hxspi: XSPI handle pointer
  * @retval None
  */
void HAL_XSPI_MspDeInit(XSPI_HandleTypeDef* hxspi)
{
  if(hxspi->Instance==XSPI2)
  {
    /* USER CODE BEGIN XSPI2_MspDeInit 0 */

    /* USER CODE END XSPI2_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_XSPIM_CLK_DISABLE();
    __HAL_RCC_XSPI2_CLK_DISABLE();

    /**XSPI2 GPIO Configuration
    PN1     ------> XSPIM_P2_NCS1
    PN3     ------> XSPIM_P2_IO1
    PN0     ------> XSPIM_P2_DQS0
    PN11     ------> XSPIM_P2_IO7
    PN10     ------> XSPIM_P2_IO6
    PN9     ------> XSPIM_P2_IO5
    PN2     ------> XSPIM_P2_IO0
    PN6     ------> XSPIM_P2_CLK
    PN8     ------> XSPIM_P2_IO4
    PN4     ------> XSPIM_P2_IO2
    PN5     ------> XSPIM_P2_IO3
    */
    HAL_GPIO_DeInit(GPION, GPIO_PIN_1|GPIO_PIN_3|GPIO_PIN_0|GPIO_PIN_11
                          |GPIO_PIN_10|GPIO_PIN_9|GPIO_PIN_2|GPIO_PIN_6
                          |GPIO_PIN_8|GPIO_PIN_4|GPIO_PIN_5);

    /* USER CODE BEGIN XSPI2_MspDeInit 1 */

    /* USER CODE END XSPI2_MspDeInit 1 */
  }

}

static uint32_t FMC_Initialized = 0;

static void HAL_FMC_MspInit(void){
  /* USER CODE BEGIN FMC_MspInit 0 */

  /* USER CODE END FMC_MspInit 0 */
  GPIO_InitTypeDef GPIO_InitStruct ={0};
  if (FMC_Initialized) {
    return;
  }
  FMC_Initialized = 1;
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the peripherals clock
  */
    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_FMC;
    PeriphClkInit.FmcClockSelection = RCC_FMCCLKSOURCE_HCLK;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
    {
      Error_Handler();
    }

  /* Peripheral clock enable */
  __HAL_RCC_FMC_CLK_ENABLE();

  /** FMC GPIO Configuration
  PE0   ------> FMC_D9
  PF3   ------> FMC_A11
  PF2   ------> FMC_A10
  PG3   ------> FMC_D21
  PE14   ------> FMC_SDNE0
  PE2   ------> FMC_D11
  PB5   ------> FMC_D12
  PF0   ------> FMC_A8
  PD7   ------> FMC_D8
  PE13   ------> FMC_SDNCAS
  PD1   ------> FMC_A7
  PE1   ------> FMC_D10
  PB3(JTDO/TRACESWO)   ------> FMC_D14
  PF1   ------> FMC_A9
  PE11   ------> FMC_SDNWE
  PD0   ------> FMC_A6
  PA12   ------> FMC_D0
  PA11   ------> FMC_D1
  PB4(NJTRST)   ------> FMC_D13
  PE15   ------> FMC_SDCKE0
  PA9   ------> FMC_D3
  PE6   ------> FMC_D15
  PE12   ------> FMC_SDNRAS
  PA10   ------> FMC_D2
  PG14   ------> FMC_D31
  PG12   ------> FMC_D29
  PG11   ------> FMC_D28
  PA8   ------> FMC_D4
  PG15   ------> FMC_NBL3
  PG13   ------> FMC_D30
  PC0   ------> FMC_NBL1
  PC1   ------> FMC_A0
  PC2   ------> FMC_A1
  PC3   ------> FMC_A2
  PA2   ------> FMC_D5
  PC4   ------> FMC_A3
  PD15   ------> FMC_D18
  PD14   ------> FMC_D17
  PA0   ------> FMC_D7
  PA1   ------> FMC_D6
  PE7   ------> FMC_A4
  PG6   ------> FMC_NBL2
  PD8   ------> FMC_NBL0
  PD11   ------> FMC_D16
  PC5   ------> FMC_A5
  PG4   ------> FMC_D22
  PG7   ------> FMC_D24
  PF13   ------> FMC_D20
  PE9   ------> FMC_BA0
  PG5   ------> FMC_D23
  PG8   ------> FMC_D25
  PD9   ------> FMC_SDCLK
  PF12   ------> FMC_D19
  PE8   ------> FMC_A12
  PE10   ------> FMC_BA1
  PG9   ------> FMC_D26
  PG10   ------> FMC_D27
  */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_14|GPIO_PIN_2|GPIO_PIN_13
                          |GPIO_PIN_1|GPIO_PIN_11|GPIO_PIN_15|GPIO_PIN_6
                          |GPIO_PIN_12|GPIO_PIN_7|GPIO_PIN_8;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF12_FMC;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = GPIO_PIN_3|GPIO_PIN_2|GPIO_PIN_0|GPIO_PIN_1
                          |GPIO_PIN_13|GPIO_PIN_12;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF12_FMC;
  HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = GPIO_PIN_3|GPIO_PIN_14|GPIO_PIN_12|GPIO_PIN_11
                          |GPIO_PIN_15|GPIO_PIN_13|GPIO_PIN_6|GPIO_PIN_4
                          |GPIO_PIN_7|GPIO_PIN_5|GPIO_PIN_8|GPIO_PIN_9
                          |GPIO_PIN_10;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF12_FMC;
  HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = GPIO_PIN_5|GPIO_PIN_3|GPIO_PIN_4;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF12_FMC;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = GPIO_PIN_7|GPIO_PIN_1|GPIO_PIN_0|GPIO_PIN_15
                          |GPIO_PIN_14|GPIO_PIN_8|GPIO_PIN_11|GPIO_PIN_9;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF12_FMC;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = GPIO_PIN_12|GPIO_PIN_11|GPIO_PIN_9|GPIO_PIN_10
                          |GPIO_PIN_8|GPIO_PIN_2|GPIO_PIN_0|GPIO_PIN_1;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF12_FMC;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3
                          |GPIO_PIN_4|GPIO_PIN_5;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF12_FMC;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = GPIO_PIN_9|GPIO_PIN_10;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF14_FMC;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /* USER CODE BEGIN FMC_MspInit 1 */

  /* USER CODE END FMC_MspInit 1 */
}

void HAL_SDRAM_MspInit(SDRAM_HandleTypeDef* hsdram){
  /* USER CODE BEGIN SDRAM_MspInit 0 */

  /* USER CODE END SDRAM_MspInit 0 */
  HAL_FMC_MspInit();
  /* USER CODE BEGIN SDRAM_MspInit 1 */

  /* USER CODE END SDRAM_MspInit 1 */
}

static uint32_t FMC_DeInitialized = 0;

static void HAL_FMC_MspDeInit(void){
  /* USER CODE BEGIN FMC_MspDeInit 0 */

  /* USER CODE END FMC_MspDeInit 0 */
  if (FMC_DeInitialized) {
    return;
  }
  FMC_DeInitialized = 1;
  /* Peripheral clock enable */
  __HAL_RCC_FMC_CLK_DISABLE();

  /** FMC GPIO Configuration
  PE0   ------> FMC_D9
  PF3   ------> FMC_A11
  PF2   ------> FMC_A10
  PG3   ------> FMC_D21
  PE14   ------> FMC_SDNE0
  PE2   ------> FMC_D11
  PB5   ------> FMC_D12
  PF0   ------> FMC_A8
  PD7   ------> FMC_D8
  PE13   ------> FMC_SDNCAS
  PD1   ------> FMC_A7
  PE1   ------> FMC_D10
  PB3(JTDO/TRACESWO)   ------> FMC_D14
  PF1   ------> FMC_A9
  PE11   ------> FMC_SDNWE
  PD0   ------> FMC_A6
  PA12   ------> FMC_D0
  PA11   ------> FMC_D1
  PB4(NJTRST)   ------> FMC_D13
  PE15   ------> FMC_SDCKE0
  PA9   ------> FMC_D3
  PE6   ------> FMC_D15
  PE12   ------> FMC_SDNRAS
  PA10   ------> FMC_D2
  PG14   ------> FMC_D31
  PG12   ------> FMC_D29
  PG11   ------> FMC_D28
  PA8   ------> FMC_D4
  PG15   ------> FMC_NBL3
  PG13   ------> FMC_D30
  PC0   ------> FMC_NBL1
  PC1   ------> FMC_A0
  PC2   ------> FMC_A1
  PC3   ------> FMC_A2
  PA2   ------> FMC_D5
  PC4   ------> FMC_A3
  PD15   ------> FMC_D18
  PD14   ------> FMC_D17
  PA0   ------> FMC_D7
  PA1   ------> FMC_D6
  PE7   ------> FMC_A4
  PG6   ------> FMC_NBL2
  PD8   ------> FMC_NBL0
  PD11   ------> FMC_D16
  PC5   ------> FMC_A5
  PG4   ------> FMC_D22
  PG7   ------> FMC_D24
  PF13   ------> FMC_D20
  PE9   ------> FMC_BA0
  PG5   ------> FMC_D23
  PG8   ------> FMC_D25
  PD9   ------> FMC_SDCLK
  PF12   ------> FMC_D19
  PE8   ------> FMC_A12
  PE10   ------> FMC_BA1
  PG9   ------> FMC_D26
  PG10   ------> FMC_D27
  */
  HAL_GPIO_DeInit(GPIOE, GPIO_PIN_0|GPIO_PIN_14|GPIO_PIN_2|GPIO_PIN_13
                          |GPIO_PIN_1|GPIO_PIN_11|GPIO_PIN_15|GPIO_PIN_6
                          |GPIO_PIN_12|GPIO_PIN_7|GPIO_PIN_9|GPIO_PIN_8
                          |GPIO_PIN_10);

  HAL_GPIO_DeInit(GPIOF, GPIO_PIN_3|GPIO_PIN_2|GPIO_PIN_0|GPIO_PIN_1
                          |GPIO_PIN_13|GPIO_PIN_12);

  HAL_GPIO_DeInit(GPIOG, GPIO_PIN_3|GPIO_PIN_14|GPIO_PIN_12|GPIO_PIN_11
                          |GPIO_PIN_15|GPIO_PIN_13|GPIO_PIN_6|GPIO_PIN_4
                          |GPIO_PIN_7|GPIO_PIN_5|GPIO_PIN_8|GPIO_PIN_9
                          |GPIO_PIN_10);

  HAL_GPIO_DeInit(GPIOB, GPIO_PIN_5|GPIO_PIN_3|GPIO_PIN_4);

  HAL_GPIO_DeInit(GPIOD, GPIO_PIN_7|GPIO_PIN_1|GPIO_PIN_0|GPIO_PIN_15
                          |GPIO_PIN_14|GPIO_PIN_8|GPIO_PIN_11|GPIO_PIN_9);

  HAL_GPIO_DeInit(GPIOA, GPIO_PIN_12|GPIO_PIN_11|GPIO_PIN_9|GPIO_PIN_10
                          |GPIO_PIN_8|GPIO_PIN_2|GPIO_PIN_0|GPIO_PIN_1);

  HAL_GPIO_DeInit(GPIOC, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3
                          |GPIO_PIN_4|GPIO_PIN_5);

  /* USER CODE BEGIN FMC_MspDeInit 1 */

  /* USER CODE END FMC_MspDeInit 1 */
}

void HAL_SDRAM_MspDeInit(SDRAM_HandleTypeDef* hsdram){
  /* USER CODE BEGIN SDRAM_MspDeInit 0 */

  /* USER CODE END SDRAM_MspDeInit 0 */
  HAL_FMC_MspDeInit();
  /* USER CODE BEGIN SDRAM_MspDeInit 1 */

  /* USER CODE END SDRAM_MspDeInit 1 */
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
