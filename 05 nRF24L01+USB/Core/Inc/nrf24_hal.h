#ifndef __NRF24_HAL_H
#define __NRF24_HAL_H

#include "stdint.h" /* JSB */
#include "stm32f1xx_hal.h" /* JSB */
#include "main.h" /* JSB */

// Hardware abstraction layer for NRF24L01+ transceiver (hardware depended functions)
// GPIO pins definition
// GPIO pins initialization and control functions
// SPI transmit functions

// Peripheral libraries
//JSB #include <stm32f10x_gpio.h>
//JSB #include <stm32f10x_spi.h>

// SPI port peripheral
#define nRF24_SPI_PORT             SPI2

// CE (chip enable) pin (PB11)
#define nRF24_CE_PORT              nRF24_CE_GPIO_Port
#define nRF24_CE_PIN               nRF24_CE_Pin
#define nRF24_CE_L() 			   HAL_GPIO_WritePin(nRF24_CE_PORT, nRF24_CE_PIN, GPIO_PIN_RESET)
#define nRF24_CE_H()               HAL_GPIO_WritePin(nRF24_CE_PORT, nRF24_CE_PIN, GPIO_PIN_SET)

// CSN (chip select negative) pin (PB12)
#define nRF24_CSN_PORT             nRF24_CSN_GPIO_Port
#define nRF24_CSN_PIN              nRF24_CSN_Pin
#define nRF24_CSN_L()              HAL_GPIO_WritePin(nRF24_CSN_PORT, nRF24_CSN_PIN, GPIO_PIN_RESET)
#define nRF24_CSN_H()              HAL_GPIO_WritePin(nRF24_CSN_PORT, nRF24_CSN_PIN, GPIO_PIN_SET)

// IRQ pin (PB10)
#define nRF24_IRQ_PORT             GPIOB
#define nRF24_IRQ_PIN              GPIO_Pin_10

// Function prototypes
void nRF24_GPIO_Init(void);
HAL_StatusTypeDef nRF24_LL_RW(uint8_t *pTxData, uint8_t *pRxData, uint16_t NumBytes);

#endif // __NRF24_HAL_H
