#ifndef __NRF24_H
#define __NRF24_H

#include "stdint.h" /* JSB */

// Interface to specific hardware:
#include "nrf24_hal.h"

// nRF24L01+ register bits:
//
// STATUS:
#define nRF24_STATUS_RX_DR           (uint8_t)0x40 // RX_DR bit (data ready RX FIFO interrupt)
#define nRF24_STATUS_TX_DS           (uint8_t)0x20 // TX_DS bit (data sent TX FIFO interrupt)
#define nRF24_STATUS_MAX_RT          (uint8_t)0x10 // MAX_RT bit (maximum number of TX retransmits interrupt)

// Retransmit delay
typedef enum
{
  nRF24_ARD_NONE = (uint8_t) 0x00, // Dummy value for case when retransmission is not used
  nRF24_ARD_250us = (uint8_t) 0x00,
  nRF24_ARD_500us = (uint8_t) 0x01,
  nRF24_ARD_750us = (uint8_t) 0x02,
  nRF24_ARD_1000us = (uint8_t) 0x03,
  nRF24_ARD_1250us = (uint8_t) 0x04,
  nRF24_ARD_1500us = (uint8_t) 0x05,
  nRF24_ARD_1750us = (uint8_t) 0x06,
  nRF24_ARD_2000us = (uint8_t) 0x07,
  nRF24_ARD_2250us = (uint8_t) 0x08,
  nRF24_ARD_2500us = (uint8_t) 0x09,
  nRF24_ARD_2750us = (uint8_t) 0x0A,
  nRF24_ARD_3000us = (uint8_t) 0x0B,
  nRF24_ARD_3250us = (uint8_t) 0x0C,
  nRF24_ARD_3500us = (uint8_t) 0x0D,
  nRF24_ARD_3750us = (uint8_t) 0x0E,
  nRF24_ARD_4000us = (uint8_t) 0x0F
} nRF24_ARD;

typedef enum
{
  nRF24_DataRate_250kbps = (uint8_t) 0x20, // 250kbps data rate
  nRF24_DataRate_1Mbps = (uint8_t) 0x00, // 1Mbps data rate
  nRF24_DataRate_2Mbps = (uint8_t) 0x08  // 2Mbps data rate
} nRF24_DataRate;

// RF output power in TX mode
typedef enum
{
  nRF24_TXPWR_Minus18dBm = (uint8_t) 0x00, // -18dBm
  nRF24_TXPWR_Minus12dBm = (uint8_t) 0x02, // -12dBm
  nRF24_TXPWR_Minus6dBm = (uint8_t) 0x04, //  -6dBm
  nRF24_TXPWR_0dBm = (uint8_t) 0x06  //   0dBm
} nRF24_TXPWR;

// CRC encoding scheme
typedef enum
{
  nRF24_CRC_off = (uint8_t) 0x00, // CRC disabled
  nRF24_CRC_1byte = (uint8_t) 0x08, // 1-byte CRC
  nRF24_CRC_2byte = (uint8_t) 0x0c  // 2-byte CRC
} nRF24_CRC;

// nRF24L01 power control
typedef enum
{
  nRF24_PWR_UP = (uint8_t) 0x02, // Power up
  nRF24_PWR_DOWN = (uint8_t) 0x00  // Power down
} nRF24_PWR;

// Transceiver mode
typedef enum
{
  nRF24_MODE_RX = (uint8_t) 0x01, // PRX
  nRF24_MODE_TX = (uint8_t) 0x00  // PTX
} nRF24_MODE;

// Enumeration of RX pipe addresses and TX address
typedef enum
{
  nRF24_PIPE0 = (uint8_t) 0x00, // pipe0
  nRF24_PIPE1 = (uint8_t) 0x01, // pipe1
  nRF24_PIPE2 = (uint8_t) 0x02, // pipe2
  nRF24_PIPE3 = (uint8_t) 0x03, // pipe3
  nRF24_PIPE4 = (uint8_t) 0x04, // pipe4
  nRF24_PIPE5 = (uint8_t) 0x05, // pipe5
  nRF24_PIPETX = (uint8_t) 0x06  // TX address (not a pipe in fact)
} nRF24_PIPE;

// State of auto acknowledgement for specified pipe
typedef enum
{
  nRF24_AA_OFF = (uint8_t) 0x00,
  nRF24_AA_ON = (uint8_t) 0x01
} nRF24_AA;

// Status of the RX FIFO
typedef enum
{
  nRF24_STATUS_RXFIFO_DATA = (uint8_t) 0x00, // The RX FIFO contains data and available locations
  nRF24_STATUS_RXFIFO_EMPTY = (uint8_t) 0x01, // The RX FIFO is empty
  nRF24_STATUS_RXFIFO_FULL = (uint8_t) 0x02, // The RX FIFO is full
  nRF24_STATUS_RXFIFO_ERROR = (uint8_t) 0x03  // Impossible state: RX FIFO cannot be empty and full at the same time
} nRF24_STATUS_RXFIFO;

// Status of the TX FIFO
typedef enum
{
  nRF24_STATUS_TXFIFO_DATA = (uint8_t) 0x00, // The TX FIFO contains data and available locations
  nRF24_STATUS_TXFIFO_EMPTY = (uint8_t) 0x01, // The TX FIFO is empty
  nRF24_STATUS_TXFIFO_FULL = (uint8_t) 0x02, // The TX FIFO is full
  nRF24_STATUS_TXFIFO_ERROR = (uint8_t) 0x03  // Impossible state: TX FIFO cannot be empty and full at the same time
} nRF24_STATUS_TXFIFO;

// Result of RX FIFO reading
typedef enum
{
  nRF24_RX_PIPE0 = (uint8_t) 0x00, // Packet received from the PIPE#0
  nRF24_RX_PIPE1 = (uint8_t) 0x01, // Packet received from the PIPE#1
  nRF24_RX_PIPE2 = (uint8_t) 0x02, // Packet received from the PIPE#2
  nRF24_RX_PIPE3 = (uint8_t) 0x03, // Packet received from the PIPE#3
  nRF24_RX_PIPE4 = (uint8_t) 0x04, // Packet received from the PIPE#4
  nRF24_RX_PIPE5 = (uint8_t) 0x05, // Packet received from the PIPE#5
  nRF24_RX_EMPTY = (uint8_t) 0xff  // The RX FIFO is empty
} nRF24_RXResult;

// Function prototypes
void nRF24_Init(void);
uint8_t nRF24_Check(void);

int8_t nRF24_ConfigRegisterIsZero();

void nRF24_EnableInterrupts(uint8_t RX_DR, uint8_t TX_DS, uint8_t MAX_RT);
void nRF24_SetPowerMode(nRF24_PWR value);
void nRF24_SetOperationalMode(nRF24_MODE value);
void nRF24_SetRFChannel(uint8_t channel);
void nRF24_GetAutoRetransmission(nRF24_ARD *ard, uint8_t *arc);
void nRF24_SetAutoRetransmission(nRF24_ARD ard, uint8_t arc);
uint8_t nRF24_GetAddressWidth(); // JSB: Added.
void nRF24_SetAddressWidth(uint8_t addr_width);
uint64_t nRF24_GetAddress(nRF24_PIPE pipe); // JSB: Added.
void nRF24_SetAddress(nRF24_PIPE pipe, const uint64_t address); // JSB: Modified.
nRF24_TXPWR nRF24_GetTXPower();
void nRF24_GetTXPowerAsString(char *S);
void nRF24_SetTXPower(nRF24_TXPWR value);
nRF24_DataRate nRF24_GetDataRate();
void nRF24_GetDataRateAsString(char *S);
void nRF24_SetDataRate(nRF24_DataRate data_rate);
void nRF24_SetCRCScheme(nRF24_CRC value);
void nRF24_SetRXPipe(nRF24_PIPE pipe, nRF24_AA aa_state, uint8_t payload_len);
void nRF24_ClosePipe(nRF24_PIPE pipe);
void nRF24_EnableAA(nRF24_PIPE pipe);
void nRF24_DisableAA(nRF24_PIPE pipe);

uint8_t nRF24_GetStatus(void);
uint8_t nRF24_GetIRQFlags(void);
nRF24_STATUS_RXFIFO nRF24_GetStatus_RXFIFO(void);
nRF24_STATUS_TXFIFO nRF24_GetStatus_TXFIFO(void);
uint8_t nRF24_GetRXSource(void);
void nRF24_GetRetransmitCounters(uint8_t *pLostPackets, uint8_t *pRetransmittedPackets);

void nRF24_ResetPLOS(void);
void nRF24_FlushTX(void);
void nRF24_FlushRX(void);
void nRF24_ClearIRQFlags(void);
void nRF24_ClearIRQ_RX_DR_Flag(void);

void nRF24_WritePayload(uint8_t *pBuf, uint8_t length);
nRF24_RXResult nRF24_ReadPayload(uint8_t *pBuf, uint8_t *length);

#define nRF24_RX_ON()   nRF24_CE_H();
#define nRF24_RX_OFF()  nRF24_CE_L();

void nRF24_DumpConfiguration();
void nRF24_DumpRegisters();

#endif // __NRF24_H
