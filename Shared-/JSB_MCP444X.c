// Copyright 2017 J S Bladen.

#include "stm32f1xx_hal.h"
#include "i2c.h"
#include "JSB_MCP444X.h"

#define MCP444X_I2CBaseAddress (0x2C)  /* This is [A6..A0]. It must be shifted left by one in the control byte that is sent to the DeviceIndex */

static uint8_t GetI2CAddress(uint8_t DeviceIndex)
{
  return MCP444X_I2CBaseAddress | (DeviceIndex & 0x03);
}

static HAL_StatusTypeDef ReadRegister(uint8_t DeviceIndex, uint8_t RegisterIndex, uint16_t *pResult)
{
  uint8_t I2CAddress, CommandByte;
  uint8_t RxData[2];
  HAL_StatusTypeDef HALResult;

  I2CAddress = GetI2CAddress(DeviceIndex);

  CommandByte = ((RegisterIndex & 0x0F) << 4) | 0x0C;
  // HAL_I2C_Mem_Read achieves the restart between the command write and the data read, as specified in the MCP444X datasheet,
  // rather than the stop-start that would occur if HAL_I2C_Master_Transmit() and HAL_I2C_Master_Receive() were used.
  HALResult = HAL_I2C_Mem_Read(&hi2c1, I2CAddress << 1, CommandByte, I2C_MEMADD_SIZE_8BIT, RxData, 2, 1000);
  if (HALResult != HAL_OK)
    return HALResult;

  *pResult = (RxData[0] << 8) | RxData[1];

  return HAL_OK;
}

static HAL_StatusTypeDef WriteRegister(uint8_t DeviceIndex, uint8_t RegisterIndex, uint16_t Value)
{
  uint8_t I2CAddress;
  uint8_t CommandByte, TxData;

  I2CAddress = GetI2CAddress(DeviceIndex);

  CommandByte = ((RegisterIndex & 0x0F) << 4) | ((Value & 0x100) >> 8); // Includes bit 8 of Value.
  TxData = (Value & 0xFF);
  return HAL_I2C_Mem_Write(&hi2c1, I2CAddress << 1, CommandByte, I2C_MEMADD_SIZE_8BIT, &TxData, 1, 1000);
}

uint8_t MCP444X_IsDevicePresent(uint8_t DeviceIndex)
{
  uint16_t RegisterValue;

  return (ReadRegister(DeviceIndex, 0x00, &RegisterValue) == HAL_OK);
}

static uint8_t GetPotentiometerRegisterIndex(MCP444X_Potentiometer_t Potentiometer, uint8_t NonVolatile)
{
  uint8_t Result;

  switch (Potentiometer)
  {
    case Potentiometer_0:
      Result = 0x00;
      break;
    case Potentiometer_1:
      Result = 0x01;
      break;
    case Potentiometer_2:
      Result = 0x06;
      break;
    case Potentiometer_3:
      Result = 0x07;
      break;
    default:
      Error_Handler();
      return 0;
  }

  if (NonVolatile)
    Result += 2;

  return Result;
}

uint16_t MCP444X_GetPotentiometerPosition_Volatile(uint8_t DeviceIndex, MCP444X_Potentiometer_t Potentiometer)
{
  uint8_t RegisterIndex;
  uint16_t Position;

  RegisterIndex = GetPotentiometerRegisterIndex(Potentiometer, 0);
  if (ReadRegister(DeviceIndex, RegisterIndex, &Position) != HAL_OK)
    Error_Handler();
  return Position;
}

void MCP444X_SetPotentiometerPosition_Volatile(uint8_t DeviceIndex, MCP444X_Potentiometer_t Potentiometer, uint16_t Position)
{
  uint8_t RegisterIndex;

  RegisterIndex = GetPotentiometerRegisterIndex(Potentiometer, 0);
  if (WriteRegister(DeviceIndex, RegisterIndex, Position) != HAL_OK)
    Error_Handler();
}

uint16_t MCP444X_GetPotentiometerPosition_NonVolatile(uint8_t DeviceIndex, MCP444X_Potentiometer_t Potentiometer)
{
  uint8_t RegisterIndex;
  uint16_t Position;

  RegisterIndex = GetPotentiometerRegisterIndex(Potentiometer, 1);
  if (ReadRegister(DeviceIndex, RegisterIndex, &Position) != HAL_OK)
    Error_Handler();
  return Position;
}

void MCP444X_SetPotentiometerPosition_NonVolatile(uint8_t DeviceIndex, MCP444X_Potentiometer_t Potentiometer, uint16_t Position)
{
  uint8_t RegisterIndex;

  RegisterIndex = GetPotentiometerRegisterIndex(Potentiometer, 1);
  if (WriteRegister(DeviceIndex, RegisterIndex, Position) != HAL_OK)
    Error_Handler();
  HAL_Delay(10);
}

void MCP444X_ReadTCON(uint8_t DeviceIndex, uint16_t *pTCON0Value, uint16_t *pTCON1Value)
{
  if (ReadRegister(DeviceIndex, 0x04, pTCON0Value) != HAL_OK)
    Error_Handler();
  if (ReadRegister(DeviceIndex, 0x0A, pTCON1Value) != HAL_OK)
    Error_Handler();
}

void MCP444X_WriteTCON(uint8_t DeviceIndex, uint16_t TCON0Value, uint16_t TCON1Value)
{
  if (WriteRegister(DeviceIndex, 0x04, TCON0Value) != HAL_OK)
    Error_Handler();
  if (WriteRegister(DeviceIndex, 0x0A, TCON1Value) != HAL_OK)
    Error_Handler();
}

uint16_t MCP444X_ReadNonvolatileMemoryValue(uint8_t DeviceIndex, uint8_t Address)
// 9 bit value.
{
  uint8_t RegisterIndex;
  uint16_t Result;

  if (Address > 4)
    Error_Handler();

  RegisterIndex = 0x0B + Address;
  if (ReadRegister(DeviceIndex, RegisterIndex, &Result) != HAL_OK)
    Error_Handler();

  return Result;
}

void MCP444X_WriteNonvolatileMemoryValue(uint8_t DeviceIndex, uint8_t Address, uint16_t Value)
// 9 bit value.
{
  uint8_t RegisterIndex;

  if (Address > 4)
    Error_Handler();

  RegisterIndex = 0x0B + Address;
  if (WriteRegister(DeviceIndex, RegisterIndex, Value) != HAL_OK)
    Error_Handler();

  HAL_Delay(10);
}
