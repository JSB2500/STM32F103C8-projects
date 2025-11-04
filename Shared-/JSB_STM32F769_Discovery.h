#pragma once

///////////////////////////////////////////////////////////////////////////////
// JSBSTM32F769NIDiscovery_DebuggingUART:
// => Requires:
//    => HAL_UART_MODULE_ENABLED to be defined in stm32f7xx_hal_conf.h
//    => The following to be added to Drivers\STM32F7xx_HAL_Driver project folder:
//       => stm32f7xx_hal_uart.c
//       => stm32f7xx_hal_rcc.c
//       => stm32f7xx_hal_rcc_ex.c
//    => The following to be added to STM32 project folder:
//       => syscalls.c
// => Notes:
//    => Lines will not normally appear on PuTTY until "\r\n" is sent.
///////////////////////////////////////////////////////////////////////////////

#include "JSB_STM32F769_DiscoveryOptions.h"
#include "stm32f769xx.h"

#ifdef __cplusplus
extern "C" {
#endif

// CPU:
void JSB_EnableCPUCache();
void JSB_InitSystemClock_200MHz();
void JSB_InitSystemClock_216MHz();

// GPIO:
void JSB_GPIO_EnableClocks();
void JSB_GPIO_InitPin(GPIO_TypeDef *pPort, uint32_t Pin, uint32_t Mode, uint32_t Pull, uint32_t Speed, uint32_t Alternate);
void JSB_InitUserButton();
int JSB_GetUserButtonPressed();

// Debugging:
#if JSBSTM32F769NIDiscovery_DebuggingUART
  void JSB_InitDebuggingUART();
  int JSB_DebugPrintf(const char *format, ...);
#endif

// Timers:
#if JSBSTM32F769NIDiscovery_Timers
  int JSB_Timer_InitPWM(TIM_HandleTypeDef *pTimerHandle, uint32_t CounterMode, uint32_t Prescale, uint32_t Period);
  int JSB_Timer_ConfigureChannel(TIM_HandleTypeDef *pTimerHandle, uint32_t Channel, uint32_t Mode, uint32_t Polarity, uint32_t PulseLength);
  void JSB_Timer_SetCaptureCompareRegister(TIM_HandleTypeDef *pTimerHandle, uint32_t Channel, uint32_t Value);
#endif

void JSB_ErrorHandler();

#ifdef __cplusplus
}
#endif
