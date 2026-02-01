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
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Application start address — adjust to match your Appli linker script */
#define APP_START_ADDRESS   0x08100000U

/**
  * @brief  Read the stack pointer and reset handler from the App vector table,
  *         then jump to the application.
  */
static void BootToApplication(void)
{
  /* Pointer to the application's vector table */
  uint32_t *appVectorTable = (uint32_t *)APP_START_ADDRESS;

  /* Sanity check: make sure the app vector table looks valid
   * (stack pointer should point into RAM) */
  if ((appVectorTable[0] & 0x20000000U) == 0U)
  {
    /* No valid application found — stay in bootloader */
    while (1) {}
  }

  /* Set the stack pointer to the application's initial stack pointer */
  __set_MSP(appVectorTable[0]);

  /* Jump to the application's reset handler (index 1 in the vector table) */
  ((void (*)())appVectorTable[1])();
}

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  HAL_Init();
  SystemClock_Config();

  /* Jump to the application */
  BootToApplication();

  /* Should never reach here */
  while (1) {}
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  /* Use the same clock config as your Appli project, or a minimal one here.
   * Minimal example using HSE bypass (matches your Appli project): */
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState      = RCC_HSE_BYPASS;

  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
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
  while (1) {}
}
