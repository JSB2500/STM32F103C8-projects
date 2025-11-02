#ifndef __JSB_DWT_H
#define __JSB_DWT_H

#include "stdint.h"

void Enable_DWT_Delay();
uint32_t DWT_GetNumProcessorClockCycles();
double DWT_GetCycleTime();
float DWT_GetElapsedTime();
void DWT_Delay_ProcessorClockCycles(uint32_t Value);
void DWT_Delay_us(uint32_t Value);

inline void Enable_DWT_Delay()
{
  CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk; // Enable DWT.
  DWT->LAR = 0xC5ACCE55; // Unlock register access.
  DWT->CTRL = 0; // Reset the counter.
  DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk; // Enable CPU_CYCLES.
}

inline uint32_t DWT_GetNumProcessorClockCycles()
{
  return DWT->CYCCNT;
}

inline double DWT_GetCycleTime()
{
  return 1.0f/(float)HAL_RCC_GetSysClockFreq();
}

inline float DWT_GetElapsedTime()
// Warning: Only around 10.7 seconds max count with 400 MHz system clock.
{
  return (float)DWT->CYCCNT/(float)HAL_RCC_GetSysClockFreq();
}

inline void DWT_Delay_ProcessorClockCycles(uint32_t Value)
{
 uint32_t NumCycles_Start = DWT->CYCCNT;
 while ((DWT->CYCCNT - NumCycles_Start) < Value);
}

inline void DWT_Delay_us(uint32_t Value)
{
 uint32_t NumCycles_Start = DWT->CYCCNT;
 uint32_t NumCyclesWanted = (HAL_RCC_GetSysClockFreq() / 1000000) * Value;
 while ((DWT->CYCCNT - NumCycles_Start) < NumCyclesWanted);
}

#endif
