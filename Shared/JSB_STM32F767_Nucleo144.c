#include "stdio.h"
#include "string.h"
#include "stdarg.h"
#include "stm32f7xx_hal.h"
#include "stm32f7xx_nucleo_144.h"
//
#include "JSB_STM32F767_Nucleo144.h"
#include "JSB_General.h"

#if JSBSTM32F767ZINucleo144_FreeRTOS
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

/**
 * @brief  System Clock Configuration
 *         The system Clock is configured as follow :
 *            System Clock source            = PLL (HSE)

 *            SYSCLK(Hz)                     = 200000000 // JSB: Limited to 200MHz for SDRAM compatibility. See project "readme.txt".
 *            HCLK(Hz)                       = 200000000
 *            AHB Prescaler                  = 1
 *            APB1 Prescaler                 = 4
 *            APB2 Prescaler                 = 2
 *            HSE Frequency(Hz)              = 8000000
 *            PLL_M                          = 8
 *            PLL_N                          = 400
 *            PLL_P                          = 2
 *            PLL_Q                          = 9
 *            PLL_R                          = 7
 *            VDD(V)                         = 3.3
 *            Main regulator output voltage  = Scale1 mode
 *            Flash Latency(WS)              = 7
 * @param  None
 * @retval None
 */
void JSB_InitSystemClock_200MHz()
{
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_OscInitTypeDef RCC_OscInitStruct;

  /* Enable HSE Oscillator and activate PLL with HSE as source */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
  RCC_OscInitStruct.HSIState = RCC_HSI_OFF;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 400;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 9;
  RCC_OscInitStruct.PLL.PLLR = 7;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    JSB_ErrorHandler();

  /* Activate overdrive */
  if (HAL_PWREx_EnableOverDrive() != HAL_OK)
    JSB_ErrorHandler();

  /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2 clocks dividers */
  RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
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
}

/**
 * @brief  System Clock Configuration
 *         The system Clock is configured as follow :
 *            System Clock source            = PLL (HSE)
 *            SYSCLK(Hz)                     = 216000000
 *            HCLK(Hz)                       = 216000000
 *            AHB Prescaler                  = 1
 *            APB1 Prescaler                 = 4
 *            APB2 Prescaler                 = 2
 *            HSE Frequency(Hz)              = 8000000
 *            PLL_M                          = 8
 *            PLL_N                          = 432
 *            PLL_P                          = 2
 *            PLL_Q                          = 9
 *            PLL_R                          = 7
 *            VDD(V)                         = 3.3
 *            Main regulator output voltage  = Scale1 mode
 *            Flash Latency(WS)              = 7
 * @param  None
 * @retval None
 */
void JSB_InitSystemClock_216MHz()
{
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_OscInitTypeDef RCC_OscInitStruct;

  /* Enable HSE Oscillator and activate PLL with HSE as source */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
  RCC_OscInitStruct.HSIState = RCC_HSI_OFF;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 432;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 9;
  RCC_OscInitStruct.PLL.PLLR = 7;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    JSB_ErrorHandler();

  /* Activate overdrive */
  if (HAL_PWREx_EnableOverDrive() != HAL_OK)
    JSB_ErrorHandler();

  /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2 clocks dividers */
  RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
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

#if JSBSTM32F767ZINucleo144_DebuggingUART

/* Definition for USARTx clock resources */
#define USARTx                           USART3
#define USARTx_CLK_ENABLE()              __HAL_RCC_USART3_CLK_ENABLE();
#define USARTx_RX_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOD_CLK_ENABLE()
#define USARTx_TX_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOD_CLK_ENABLE()

#define USARTx_FORCE_RESET()             __HAL_RCC_USART3_FORCE_RESET()
#define USARTx_RELEASE_RESET()           __HAL_RCC_USART3_RELEASE_RESET()

/* Definition for USARTx Pins */
#define USARTx_TX_PIN                    GPIO_PIN_8
#define USARTx_TX_GPIO_PORT              GPIOD
#define USARTx_TX_AF                     GPIO_AF7_USART3
#define USARTx_RX_PIN                    GPIO_PIN_9
#define USARTx_RX_GPIO_PORT              GPIOD
#define USARTx_RX_AF                     GPIO_AF7_USART3

UART_HandleTypeDef UartHandle;
#if JSBSTM32F767ZINucleo144_FreeRTOS
osMutexDef(DebugPrintfMutex);
osMutexId DebugPrintfMutex;
#endif

void JSB_InitDebuggingUART()
{
  UartHandle.Instance = USARTx;
  UartHandle.Init.BaudRate = 115200;
  UartHandle.Init.WordLength = UART_WORDLENGTH_9B;
  UartHandle.Init.StopBits = UART_STOPBITS_1;
  UartHandle.Init.Parity = UART_PARITY_ODD;
  UartHandle.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  UartHandle.Init.Mode = UART_MODE_TX_RX;
  UartHandle.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&UartHandle) != HAL_OK)
    JSB_ErrorHandler();

#if JSBSTM32F767ZINucleo144_FreeRTOS
  DebugPrintfMutex = osMutexCreate(osMutex(DebugPrintfMutex));
#endif

  printf("printf() redirected to UART.\r\n");
  fflush(stdout);
}

/**
 * @brief UART MSP Initialization
 *        This function configures the hardware resources used in this example:
 *           - Peripheral's clock enable
 *           - Peripheral's GPIO Configuration
 * @param huart: UART handle pointer
 * @retval None
 */
void HAL_UART_MspInit(UART_HandleTypeDef *huart)
{
  GPIO_InitTypeDef GPIO_InitStruct;

  RCC_PeriphCLKInitTypeDef RCC_PeriphClkInit;

  /*##-1- Enable peripherals and GPIO Clocks #################################*/
  /* Enable GPIO TX/RX clock */
  USARTx_TX_GPIO_CLK_ENABLE()
        ;
  USARTx_RX_GPIO_CLK_ENABLE()
        ;

  /* Select SysClk as source of USART1 clocks */
  RCC_PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART1;
  RCC_PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_SYSCLK;
  HAL_RCCEx_PeriphCLKConfig(&RCC_PeriphClkInit);

  /* Enable USARTx clock */
  USARTx_CLK_ENABLE()
        ;

  /*##-2- Configure peripheral GPIO ##########################################*/
  /* UART TX GPIO pin configuration  */
  GPIO_InitStruct.Pin = USARTx_TX_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = USARTx_TX_AF;

  HAL_GPIO_Init(USARTx_TX_GPIO_PORT, &GPIO_InitStruct);

  /* UART RX GPIO pin configuration  */
  GPIO_InitStruct.Pin = USARTx_RX_PIN;
  GPIO_InitStruct.Alternate = USARTx_RX_AF;

  HAL_GPIO_Init(USARTx_RX_GPIO_PORT, &GPIO_InitStruct);
}

void HAL_UART_MspDeInit(UART_HandleTypeDef *huart)
{
  USARTx_FORCE_RESET();
  USARTx_RELEASE_RESET();

  HAL_GPIO_DeInit(USARTx_TX_GPIO_PORT, USARTx_TX_PIN);
  HAL_GPIO_DeInit(USARTx_RX_GPIO_PORT, USARTx_RX_PIN);
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

// 29/9/2017: This thread safety does not appear to be necessary anymore. Possibly thanks to the recently updated toolchain.
// Certainly the "STM32F767 Nucleo-144 => RTOS" project now appears to run correctly without this threadsafety.
//
//int JSB_DebugPrintf(const char *format, ...)
//// printf does not appear to work correctly when called from multiple threads, hence this mutex version.
//// Requires UART debugging.
////
//{
//  va_list args;
//  va_start(args, format);
//  int Result;
//
//#if JSBSTM32F767ZINucleo144_FreeRTOS
//  osMutexWait(DebugPrintfMutex, osWaitForever);
//#endif
//  Result = vprintf(format, args);
//#if JSBSTM32F767ZINucleo144_FreeRTOS
//  osMutexRelease(DebugPrintfMutex);
//#endif
//
//  va_end(args);
//
//  return Result;
//}

#endif /* JSBSTM32F767ZINucleo144_DebuggingUART */

///////////////////////////////////////////////////////////////////////////////

#if JSBSTM32F767ZINucleo144_Timers

int JSB_Timer_InitPWM(TIM_HandleTypeDef *pTimerHandle, uint32_t CounterMode, uint32_t Prescale, uint32_t Period)
// Requires the following function to enable the timer (using __HAL_RCC_TIMx_CLK_ENABLE) and setup the used pins:
// => void HAL_TIM_PWM_MspInit(TIM_HandleTypeDef *htim);
{
  memset(&pTimerHandle->Init, 0, sizeof(pTimerHandle->Init));
  pTimerHandle->Init.CounterMode = CounterMode;
  pTimerHandle->Init.Prescaler = Prescale - 1;
  pTimerHandle->Init.Period = Period - 1;
  pTimerHandle->Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
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
        BSP_LED_On(LED3);
      }

      for (int Duration = 0; Duration < 1000000; ++Duration)
      {
        BSP_LED_Off(LED1);
        BSP_LED_Off(LED2);
        BSP_LED_Off(LED3);
      }
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
