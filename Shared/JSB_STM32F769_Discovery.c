#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include "stm32f7xx_hal.h"
#include "stm32f769i_discovery.h"
//
#include "JSB_General.h"
#include "JSB_STM32F769_Discovery.h"

#if JSBSTM32F769NIDiscovery_FreeRTOS
#include "cmsis_os.h"
#include "task.h"
#endif

///////////////////////////////////////////////////////////////////////////////
// CPU:

void JSB_EnableCPUCache()
{
  SCB_EnableICache();
  SCB_EnableDCache();
}

void JSB_InitSystemClock_200MHz()
{
  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;

  /** Configure the main internal regulator output voltage */
  __HAL_RCC_PWR_CLK_ENABLE();

  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the CPU, AHB and APB busses clocks */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 25;
  RCC_OscInitStruct.PLL.PLLN = 400;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    JSB_ErrorHandler();

  /* Activate overdrive */
  if (HAL_PWREx_EnableOverDrive() != HAL_OK)
    JSB_ErrorHandler();

  /** Initializes the CPU, AHB and APB busses clocks */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;
  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_7) != HAL_OK)
    JSB_ErrorHandler();

  // Check clock speed:
  uint32_t hclk = HAL_RCC_GetHCLKFreq();
  if (hclk != 200000000)
    JSB_ErrorHandler();

  //    /* Configure the Systick interrupt time */
  //    HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);
  //
  //    /* Configure the Systick */
  //    HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);
  //
  //    /* SysTick_IRQn interrupt configuration */
  //    HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

void JSB_InitSystemClock_216MHz()
{
  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;

  /** Configure the main internal regulator output voltage */
  __HAL_RCC_PWR_CLK_ENABLE();

  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the CPU, AHB and APB busses clocks */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 25;
  RCC_OscInitStruct.PLL.PLLN = 432;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    JSB_ErrorHandler();

  /* Activate overdrive */
  if (HAL_PWREx_EnableOverDrive() != HAL_OK)
    JSB_ErrorHandler();

  /** Initializes the CPU, AHB and APB busses clocks */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;
  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_7) != HAL_OK)
    JSB_ErrorHandler();

  // Check clock speed:
  uint32_t hclk = HAL_RCC_GetHCLKFreq();
  if (hclk != 216000000)
    JSB_ErrorHandler();


//    /* Configure the Systick interrupt time */
//    HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);
//
//    /* Configure the Systick */
//    HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);
//
//    /* SysTick_IRQn interrupt configuration */
//    HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

///////////////////////////////////////////////////////////////////////////////
// GPIO:

void JSB_GPIO_EnableClocks()
{
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOI_CLK_ENABLE();
  __HAL_RCC_GPIOJ_CLK_ENABLE();
  __HAL_RCC_GPIOK_CLK_ENABLE();
}

void JSB_GPIO_InitPin(GPIO_TypeDef *pPort, uint32_t Pin, uint32_t Mode, uint32_t Pull, uint32_t Speed, uint32_t Alternate)
// Also need to enable GPIO clock for given poart (__HAL_RCC_GPIOC_CLK_ENABLE() etc).
{
  GPIO_InitTypeDef GPIO_InitStructure;

  memset(&GPIO_InitStructure, 0, sizeof(GPIO_InitStructure));
  GPIO_InitStructure.Pin = Pin;
  GPIO_InitStructure.Mode = Mode;
  GPIO_InitStructure.Pull = Pull;
  GPIO_InitStructure.Speed = Speed;
  GPIO_InitStructure.Alternate = Alternate;

  HAL_GPIO_Init(pPort, &GPIO_InitStructure);
}

void JSB_InitUserButton()
{
  JSB_GPIO_InitPin(GPIOC, GPIO_PIN_13, GPIO_MODE_INPUT, GPIO_NOPULL, GPIO_SPEED_FREQ_LOW, 0);
  // Or use: BSP_PB_Init(BUTTON_USER, BUTTON_MODE_GPIO);
}

int JSB_GetUserButtonPressed()
{
  return HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13);
  // Or use: BSP_PB_GetState(BUTTON_USER);
}

///////////////////////////////////////////////////////////////////////////////
// Debugging UART:

#if JSBSTM32F769NIDiscovery_DebuggingUART

UART_HandleTypeDef UartHandle;
#if JSBSTM32F769NIDiscovery_FreeRTOS
osMutexDef(DebugPrintfMutex);
osMutexId DebugPrintfMutex;
#endif

void JSB_InitDebuggingUART()
// JSB: It may be necessary to call JSB_GPIO_EnableClocks before calling this.
{
  UartHandle.Instance = USART1;
  UartHandle.Init.BaudRate = 115200;
  UartHandle.Init.WordLength = UART_WORDLENGTH_9B;
  UartHandle.Init.StopBits = UART_STOPBITS_1;
  UartHandle.Init.Parity = UART_PARITY_ODD;
  UartHandle.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  UartHandle.Init.Mode = UART_MODE_TX_RX;
  UartHandle.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&UartHandle) != HAL_OK)
    JSB_ErrorHandler();

#if JSBSTM32F769NIDiscovery_FreeRTOS
  DebugPrintfMutex = osMutexCreate(osMutex(DebugPrintfMutex));
#endif

  JSB_DebugPrintf("Printf() redirected to UART.\r\n");
}

#define VCP_RX_Pin GPIO_PIN_10
#define VCP_RX_GPIO_Port GPIOA
#define VCP_TX_Pin GPIO_PIN_9
#define VCP_TX_GPIO_Port GPIOA

void HAL_UART_MspInit(UART_HandleTypeDef *huart)
{
  GPIO_InitTypeDef GPIO_InitStruct;
  RCC_PeriphCLKInitTypeDef RCC_PeriphClkInitStruct;

  if (huart->Instance==USART1)
  {
    RCC_PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_USART1;
    RCC_PeriphClkInitStruct.Usart1ClockSelection = RCC_USART1CLKSOURCE_SYSCLK; // JSB: Or RCC_USART1CLKSOURCE_PCLK2, as given by CubeMX. Both result in the correct baud rate.
    if (HAL_RCCEx_PeriphCLKConfig(&RCC_PeriphClkInitStruct) != HAL_OK)
      JSB_ErrorHandler();

    __HAL_RCC_USART1_CLK_ENABLE();

    /* USART1 GPIO Configuration
    PA10     ------> USART1_RX
    PA9     ------> USART1_TX
    */
    GPIO_InitStruct.Pin = VCP_RX_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART1;
    HAL_GPIO_Init(VCP_RX_GPIO_Port, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = VCP_TX_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART1;
    HAL_GPIO_Init(VCP_TX_GPIO_Port, &GPIO_InitStruct);
  }
}

void HAL_UART_MspDeInit(UART_HandleTypeDef *huart)
{
  if(huart->Instance==USART1)
  {
    __HAL_RCC_USART1_CLK_DISABLE();
    __HAL_RCC_USART1_CLK_DISABLE();
    HAL_GPIO_DeInit(GPIOA, VCP_RX_Pin | VCP_TX_Pin);
  }
}

#ifdef __GNUC__
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif

PUTCHAR_PROTOTYPE
// Hook printf.
// JSB: Requires "syscalls.c" in order to intercept printf() etc.
{
  HAL_UART_Transmit(&UartHandle, (uint8_t *)&ch, 1, 0xFFFF);
  return ch;
}

int JSB_DebugPrintf(const char *format, ...)
// printf does not appear to work correctly when called from multiple threads, hence this mutex version.
// Requires UART debugging.
{
  va_list args;
  va_start(args, format);
  int Result;

#if JSBSTM32F769NIDiscovery_FreeRTOS
  osMutexWait(DebugPrintfMutex, osWaitForever);
#endif
  Result = vprintf(format, args);
#if JSBSTM32F769NIDiscovery_FreeRTOS
  osMutexRelease(DebugPrintfMutex);
#endif

  va_end(args);

  return Result;
}

#endif /* JSBSTM32F769NIDiscovery_DebuggingUART */

///////////////////////////////////////////////////////////////////////////////

#if JSBSTM32F769NIDiscovery_Timers

int JSB_Timer_InitPWM(TIM_HandleTypeDef *pTimerHandle, uint32_t CounterMode, uint32_t Prescale, uint32_t Period)
// Requires the following function to enable the timer (using __HAL_RCC_TIMx_CLK_ENABLE) and setup the used pins:
// => void HAL_TIM_PWM_MspInit(TIM_HandleTypeDef *htim);
{
  memset(&pTimerHandle->Init, 0, sizeof(pTimerHandle->Init));
  pTimerHandle->Init.CounterMode = CounterMode;
  pTimerHandle->Init.Prescaler = Prescale - 1;
  pTimerHandle->Init.Period = Period - 1;
  pTimerHandle->Init.ClockDivision = 0;
  pTimerHandle->Init.RepetitionCounter = 0;
  pTimerHandle->Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;

  return HAL_TIM_PWM_Init(pTimerHandle);
}

int JSB_Timer_ConfigureChannel(TIM_HandleTypeDef *pTimerHandle, uint32_t Channel, uint32_t Mode, uint32_t Polarity, uint32_t PulseLength)
{
  TIM_OC_InitTypeDef OutputCompareConfiguration;

  memset(&OutputCompareConfiguration, 0, sizeof(OutputCompareConfiguration));
  OutputCompareConfiguration.OCMode = Mode;
  OutputCompareConfiguration.Pulse = PulseLength;
  OutputCompareConfiguration.OCPolarity = Polarity;
  OutputCompareConfiguration.OCNPolarity = TIM_OCNPOLARITY_HIGH;
  OutputCompareConfiguration.OCFastMode = TIM_OCFAST_DISABLE;
  OutputCompareConfiguration.OCIdleState = TIM_OCIDLESTATE_RESET;
  OutputCompareConfiguration.OCNIdleState = TIM_OCNIDLESTATE_RESET;

  return HAL_TIM_PWM_ConfigChannel(pTimerHandle, &OutputCompareConfiguration, Channel);
}

void JSB_Timer_SetCaptureCompareRegister(TIM_HandleTypeDef *pTimerHandle, uint32_t Channel, uint32_t Value)
// It would be nice if there was a pure HAL way to do this.
{
  switch (Channel)
  {
  case TIM_CHANNEL_1:
    pTimerHandle->Instance->CCR1 = Value;
    break;
  case TIM_CHANNEL_2:
    pTimerHandle->Instance->CCR2 = Value;
    break;
  case TIM_CHANNEL_3:
    pTimerHandle->Instance->CCR3 = Value;
    break;
  case TIM_CHANNEL_4:
    pTimerHandle->Instance->CCR4 = Value;
    break;
  default:
    JSB_ErrorHandler();
  }
}

#endif

///////////////////////////////////////////////////////////////////////////////

void JSB_ErrorHandler()
{
//  while (1)
//  {
//    for (int Index = 0; Index < 3; ++Index)
//    {
//      BSP_LED_On(LED1);
//      BSP_LED_On(LED2);
//      BSP_LED_On(LED3);
//      HAL_Delay(100);
//      BSP_LED_Off(LED1);
//      BSP_LED_Off(LED2);
//      BSP_LED_Off(LED3);
//      HAL_Delay(100);
//    }
//    HAL_Delay(1000);
//  }

// JSB: Version that doesn't use HAL_Delay/osDelay as these functions fail during error conditions.
// JSB: The flash rate will depend on the clock speeds, however.
  while (1)
  {
    for (int Index = 0; Index < 3; ++Index)
    {
      for (int Duration = 0; Duration < 1000000; ++Duration)
      {
        BSP_LED_On(LED1);
        BSP_LED_On(LED2);
        // BSP_LED_On(LED3); // Not available on F769I-Discovery
      }

      for (int Duration = 0; Duration < 1000000; ++Duration)
      {
        BSP_LED_Off(LED1);
        BSP_LED_Off(LED2);
        // BSP_LED_Off(LED3); // Not available on F769I-Discovery
      }
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
