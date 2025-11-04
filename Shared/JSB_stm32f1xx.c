 // NB: Requries this in the project's stm32f1xx_hal_conf.h file: #include "main.h"
 #include "stm32f1xx_hal.h"
#include "JSB_stm32f1xx.h"

///////////////////////////////////////////////////////////////////////////////

void JSB_SetLEDState(uint8_t On)
{
  HAL_GPIO_WritePin(UserLED_GPIO_Port, UserLED_Pin, On ? GPIO_PIN_RESET : GPIO_PIN_SET);
}

void JSB_ToggleLEDState()
{
  HAL_GPIO_TogglePin(UserLED_GPIO_Port, UserLED_Pin);
}

void JSB_ErrorHandler()
{
  while (1)
  {
	JSB_SetLEDState(1);
    HAL_Delay(100);
    JSB_SetLEDState(0);
    HAL_Delay(100);
    JSB_SetLEDState(1);
    HAL_Delay(100);
    JSB_SetLEDState(0);
    HAL_Delay(100);
    JSB_SetLEDState(1);
    HAL_Delay(100);
    JSB_SetLEDState(0);
    HAL_Delay(1000);
  }
}

///////////////////////////////////////////////////////////////////////////////
