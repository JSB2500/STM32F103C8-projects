// Copyright 2017 J S Bladen.

#ifndef __MCP444X_H
#define __MCP444X_H

#include "stdint.h"

typedef enum
{
  Potentiometer_0,
  Potentiometer_1,
  Potentiometer_2,
  Potentiometer_3
} MCP444X_Potentiometer_t;

uint8_t MCP444X_IsDevicePresent(uint8_t DeviceIndex);
uint16_t MCP444X_GetPotentiometerPosition_Volatile(uint8_t DeviceIndex, MCP444X_Potentiometer_t Potentiometer);
void MCP444X_SetPotentiometerPosition_Volatile(uint8_t DeviceIndex, MCP444X_Potentiometer_t Potentiometer, uint16_t Position);
uint16_t MCP444X_GetPotentiometerPosition_NonVolatile(uint8_t DeviceIndex, MCP444X_Potentiometer_t Potentiometer);
void MCP444X_SetPotentiometerPosition_NonVolatile(uint8_t DeviceIndex, MCP444X_Potentiometer_t Potentiometer, uint16_t Position);
void MCP444X_ReadTCON(uint8_t DeviceIndex, uint16_t *pTCON0Value, uint16_t *pTCON1Value);
void MCP444X_WriteTCON(uint8_t DeviceIndex, uint16_t TCON0Value, uint16_t TCON1Value);
uint16_t MCP444X_ReadNonvolatileMemoryValue(uint8_t DeviceIndex, uint8_t Address); // 9 bit value.
void MCP444X_WriteNonvolatileMemoryValue(uint8_t DeviceIndex, uint8_t Address, uint16_t Value); // 9 bit value.

#endif

