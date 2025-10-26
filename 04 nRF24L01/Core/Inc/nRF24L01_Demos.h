#ifndef NRF24L01_DEMOS_H_
#define NRF24L01_DEMOS_H_

#include "stm32f1xx_hal.h"

extern SPI_HandleTypeDef nRF24L01_SPI_Handle;

void nRF24L01_Init();

void RxSingle();
void RxMulti();
void RxSolar();
void TxSingle();
void TxMulti();
void RxSingleESB();
void TxSIngleESB();

#endif
