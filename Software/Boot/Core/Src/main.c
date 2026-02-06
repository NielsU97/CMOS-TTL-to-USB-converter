#include "main.h"

/* Application start address - must match your Appli linker script */
#define APP_START_ADDRESS   0x08100000U

void SystemClock_Config(void);

static void BootToApplication(void)
{
  uint32_t *appVectorTable = (uint32_t *)APP_START_ADDRESS;

  /* Check if app is valid (stack pointer should point to RAM) */
  if ((appVectorTable[0] & 0x20000000U) == 0U)
  {
    while (1) {}  /* No valid app - hang here */
  }

  /* Set stack pointer and jump to app */
  __set_MSP(appVectorTable[0]);
  ((void (*)())appVectorTable[1])();
}

int main(void)
{
  HAL_Init();
  SystemClock_Config();

  /* Jump to application immediately */
  BootToApplication();

  while (1) {}
}

void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState       = RCC_HSE_BYPASS;
  RCC_OscInitStruct.PLL1.PLLState  = RCC_PLL_ON;
  RCC_OscInitStruct.PLL1.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL1.PLLM      = 2;
  RCC_OscInitStruct.PLL1.PLLN      = 125;
  RCC_OscInitStruct.PLL1.PLLP      = 2;
  RCC_OscInitStruct.PLL1.PLLQ      = 5;
  RCC_OscInitStruct.PLL1.PLLR      = 2;
  RCC_OscInitStruct.PLL1.PLLFractional = 0;

  HAL_RCC_OscConfig(&RCC_OscInitStruct);
}

void Error_Handler(void)
{
  __disable_irq();
  while (1) {}
}
