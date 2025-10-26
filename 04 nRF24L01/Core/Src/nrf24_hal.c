#include "nrf24_hal.h"

SPI_HandleTypeDef nRF24L01_SPI_Handle;

HAL_StatusTypeDef nRF24_LL_RW(uint8_t *pTxData, uint8_t *pRxData, uint16_t NumBytes)
{
  return HAL_SPI_TransmitReceive(&nRF24L01_SPI_Handle, pTxData, pRxData, NumBytes, 100); // JSB: Guessed timeout.
}
