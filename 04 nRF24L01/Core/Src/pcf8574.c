/*
 * pcf8574.c
 *
 *  Created on: Dec 30, 2014
 *      Author: peter
 */

#include "pcf8574.h"

PCF8574_RESULT PCF8574_Write(PCF8574_HandleTypeDef* handle, uint8_t val)
{
  if (HAL_I2C_Master_Transmit(handle->pI2C, (handle->PCF_I2C_ADDRESS << 1), &val, 1, handle->PCF_I2C_TIMEOUT) != HAL_OK)
  {
    handle->errorCallback(PCF8574_ERROR);
    return PCF8574_ERROR;
  }
  return PCF8574_OK;
}

PCF8574_RESULT PCF8574_Read(PCF8574_HandleTypeDef* handle, uint8_t* val)
{
  if (HAL_I2C_Master_Receive(handle->pI2C, (handle->PCF_I2C_ADDRESS << 1), val, 1, handle->PCF_I2C_TIMEOUT) != HAL_OK)
  {
    handle->errorCallback(PCF8574_ERROR);
    return PCF8574_ERROR;
  }
  return PCF8574_OK;
}
