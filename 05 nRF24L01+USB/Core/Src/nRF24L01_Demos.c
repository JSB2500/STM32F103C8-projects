#include <Options.h>
#include "string.h"
#include "nRF24L01_Demos.h"
#include "uart.h"
#include "hd44780.h"
#include "nrf24.h"

#ifdef HasSmallLCD
extern LCD_PCF8574_HandleTypeDef LCD; // JSB
#endif

// Timeout counter (depends on the CPU speed)
// Used for not stuck waiting for IRQ
#define nRF24_WAIT_TIMEOUT         (uint32_t)0x000FFFFF

// Result of packet transmission
typedef enum
{
  nRF24_TX_ERROR = (uint8_t) 0x00, // Unknown error
  nRF24_TX_SUCCESS,                // Packet has been transmitted successfully
  nRF24_TX_TIMEOUT,                // It was timeout during packet transmit
  nRF24_TX_MAXRT                   // Transmit failed with maximum auto retransmit count
} nRF24_TXResult;

nRF24_TXResult nRF24_TransmitPacket(uint8_t *pBuf, uint8_t length)
{
  volatile uint32_t wait = nRF24_WAIT_TIMEOUT;
  uint8_t status;

  // Deassert the CE pin (in case if it still high)
  nRF24_CE_L();

  // Transfer a data from the specified buffer to the TX FIFO
  nRF24_WritePayload(pBuf, length);

  // Start a transmission by asserting CE pin (must be held at least 10us)
  nRF24_CE_H();

  // Poll the transceiver status register until one of the following flags will be set:
  //   TX_DS  - means the packet has been transmitted
  //   MAX_RT - means the maximum number of TX retransmits happened
  // Note: this solution is far from perfect, better to use IRQ instead of polling the status
  do
  {
    status = nRF24_GetStatus();
    if (status & (nRF24_STATUS_TX_DS | nRF24_STATUS_MAX_RT))
      break;
  } while (wait--);

  // Deassert the CE pin (Standby-II --> Standby-I)
  nRF24_CE_L();

  if (!wait)
  {
    // Timeout
    return nRF24_TX_TIMEOUT;
  }

  // Check the flags in STATUS register
  UART_SendStr("[");
  UART_SendHex8(status);
  UART_SendStr("] ");

  // Clear pending IRQ flags
  nRF24_ClearIRQFlags(); // JSB: ???

  if (status & nRF24_STATUS_MAX_RT)
  {
    // Auto retransmit counter exceeds the programmed maximum limit (FIFO is not removed)
    return nRF24_TX_MAXRT;
  }

  if (status & nRF24_STATUS_TX_DS)
  {
    // Successful transmission
    return nRF24_TX_SUCCESS;
  }

  // Some banana happens, a payload remains in the TX FIFO, flush it
  nRF24_FlushTX();

  return nRF24_TX_ERROR;
}

#define HEX_CHARS "0123456789ABCDEF"

void strcat_HexBytes(char *S, uint8_t *pHexBytes, uint16_t NumBytes)
{
  uint16_t i;
  uint8_t Byte;
  char s[3];

  for (i = 0; i < NumBytes; i++)
  {
    Byte = *pHexBytes++;
    snprintf(s, 3, "%02X", Byte);
    strcat(S, s);
  }
}

void PrintPayload(uint32_t Pipe, uint8_t *pPayload, uint16_t PayloadLength)
// Print a payload contents to UART etc.
{
  UART_SendStr("Pipe:");
  UART_SendInt(Pipe);
  UART_SendStr(" Payload:");
  UART_SendBufHex(pPayload, PayloadLength);
  UART_SendStr("\r\n");

#ifdef HasSmallLCD
  char S[128] = "Payload: ";
  strcat_HexBytes(S, pPayload, PayloadLength);
  LCD_SetLocation(&LCD, 0, 0);
  LCD_WriteString(&LCD, S);
#endif
}

void nRF24L01_Init()
{
  // Disable RX/TX:
  nRF24_CE_L();

  // Disable SPI interface:
  nRF24_CSN_H();

//  //!!!JSB Begin temp:
  // This seems to help nRF24_Check pass. I don't know why. It's not always necessary. Hmm...

  uint8_t TxData0[] =
      { 0x10, 0x11, 0x22, 0x33, 0x44, 0x55 };
  uint8_t TxData1[] =
      { 0x10, 0x00, 0x00, 0x00, 0x00, 0x00 };
  uint8_t RxData[16];

  nRF24_CSN_L();
  HAL_SPI_TransmitReceive(&nRF24L01_SPI_Handle, TxData0, RxData, 0, 100);
  nRF24_CSN_H();

  nRF24_CSN_L();
  HAL_SPI_TransmitReceive(&nRF24L01_SPI_Handle, TxData1, RxData, 6, 100);
  nRF24_CSN_H();
//!!!JSB End temp

// Check it is connected etc:
  UART_SendStr("nRF24L01 check: ");
  if (!nRF24_Check())
  {
    UART_SendStr("Failed!\r\n");

#ifdef HasSmallLCD
    LCD_SetLocation(&LCD, 0, 0);
    LCD_WriteString(&LCD, "nRF24L01: Failed!");
#endif

    Error_Handler();
  }
  UART_SendStr("Passed\r\n");

#ifdef HasSmallLCD
  LCD_SetLocation(&LCD, 0, 0);
  LCD_WriteString(&LCD, "nRF24L01: Passed");
  HAL_Delay(1000); // Enable message to be read.
#endif

  // Initialize:
  nRF24_Init();
}

uint8_t nRF24_payload[32]; // Buffer to store a payload of maximum width
nRF24_RXResult pipe; // Pipe number
uint8_t payload_length; // Length of received payload
nRF24_TXResult tx_res;

void RxSingle()
{
  nRF24_DisableAA(0xFF); // Disable ShockBurst for all RX pipes.
  nRF24_SetRFChannel(115);
  nRF24_SetDataRate(nRF24_DataRate_250kbps); // !!!JSB Try 1Mbps?
  nRF24_SetCRCScheme(nRF24_CRC_2byte);
  nRF24_SetAddressWidth(5);
  nRF24_SetAddress(nRF24_PIPE0, 0xB418E71CE3);
  nRF24_SetRXPipe(nRF24_PIPE0, nRF24_AA_OFF, 5); // Auto-ACK disabled. Payload length 5 bytes
  nRF24_SetOperationalMode(nRF24_MODE_RX);
  nRF24_SetPowerMode(nRF24_PWR_UP);

  nRF24_DumpConfiguration(); // !!!JSB
  nRF24_DumpRegisters(); // !!!JSB

  // Enable transceiver
  nRF24_CE_H();

  // The main loop
//  while (1)
//  {
//    // Handle interrupts.
//  }

  while (1)
  {
    if (nRF24_GetStatus() & nRF24_STATUS_RX_DR)
    {
      while (nRF24_GetStatus_RXFIFO() != nRF24_STATUS_RXFIFO_EMPTY)
      {
        pipe = nRF24_ReadPayload(nRF24_payload, &payload_length);
        if (pipe == 255)
        {
          UART_SendStr("No payload!!!\r\n");
        }
        else
        {
          ToggleLEDState();
          PrintPayload(pipe, nRF24_payload, payload_length);
        }
      }
    }
  }
}

void TxSingle()
// Transmits to one address with a 5 byte payload, 2 byte CRC, and no Enhanced ShockBurst.
{
  const int payload_length = 5;
  uint32_t PayloadItemIndex, NumPayloadItemValue;

  nRF24_DisableAA(0xFF); // Disable ShockBurst for all RX pipes
  nRF24_SetRFChannel(115);
  nRF24_SetDataRate(nRF24_DataRate_250kbps);
  nRF24_SetCRCScheme(nRF24_CRC_2byte);
  nRF24_SetAddressWidth(5);
  nRF24_SetAddress(nRF24_PIPETX, 0xB418E71CE3);
  nRF24_SetTXPower(nRF24_TXPWR_0dBm);
  nRF24_SetAutoRetransmission(nRF24_ARD_2500us, 10);
  nRF24_SetOperationalMode(nRF24_MODE_TX);
  nRF24_ClearIRQFlags();

  nRF24_DumpConfiguration(); // !!!JSB
  nRF24_DumpRegisters(); // !!!JSB

  // Wake the transceiver
  nRF24_SetPowerMode(nRF24_PWR_UP);

  // The main loop
  NumPayloadItemValue = 0;
  while (1)
  {
    // Prepare data packet
    for (PayloadItemIndex = 0; PayloadItemIndex < payload_length; ++PayloadItemIndex)
    {
      nRF24_payload[PayloadItemIndex] = NumPayloadItemValue++;
      if (NumPayloadItemValue > 0x000000FF)
        NumPayloadItemValue = 0;
    }

    // Print payload contents to UART:
    UART_SendStr("Payload:");
    UART_SendBufHex(nRF24_payload, payload_length);
    UART_SendStr(" ... TX: ");

#ifdef HasSmallLCD
    char S[128] = "\0";
    char T[8] = "\0";
    sprintf(S, "Payload: ");
    for (PayloadItemIndex = 0; PayloadItemIndex < payload_length; ++PayloadItemIndex)
    {
      sprintf(T, "%02X", nRF24_payload[PayloadItemIndex]);
      strcat(S, T);
    }
    LCD_SetLocation(&LCD, 0, 0);
    LCD_WriteString(&LCD, S);
#endif

    // Transmit packet
#ifdef HasSmallLCD
    strcpy(S, "Status: ");
#endif
    tx_res = nRF24_TransmitPacket(nRF24_payload, payload_length);
    switch(tx_res)
    {
      case nRF24_TX_SUCCESS:
        UART_SendStr("OK");
#ifdef HasSmallLCD
        strcat(S, "OK");
#endif
        break;
      case nRF24_TX_TIMEOUT:
        UART_SendStr("TIMEOUT");
#ifdef HasSmallLCD
        strcat(S, "TIMEOUT");
#endif
        break;
      case nRF24_TX_MAXRT:
        UART_SendStr("MAX RETRANSMIT");
#ifdef HasSmallLCD
        strcat(S, "RETRANSMIT");
#endif
        break;
      default:
        UART_SendStr("ERROR");
#ifdef HasSmallLCD
        strcat(S, "ERROR");
#endif
        break;
    }
    UART_SendStr("\r\n");

#ifdef HasSmallLCD
    LCD_SetLocation(&LCD, 0, 1);
    LCD_WriteString(&LCD, S);
#endif

    HAL_Delay(500);
  }
}

void RxSingleESB()
// This is simple receiver with Enhanced ShockBurst:
//   - RX address: 'ESB'
//   - payload: 10 bytes
//   - RF channel: 40 (2440MHz)
//   - data rate: 2Mbps
//   - CRC scheme: 2 byte
//
// The transmitter sends a 10-byte packets to the address 'ESB' with Auto-ACK (ShockBurst enabled)
{
  // Set RF channel
  nRF24_SetRFChannel(40);

  // Set data rate
  nRF24_SetDataRate(nRF24_DataRate_2Mbps);

  // Set CRC scheme
  nRF24_SetCRCScheme(nRF24_CRC_2byte);

  // Set address width
  nRF24_SetAddressWidth(3);

  // Configure RX PIPE
  nRF24_SetAddress(nRF24_PIPE1, 0xB418E71CE3);
  nRF24_SetRXPipe(nRF24_PIPE1, nRF24_AA_ON, 10); // Auto-ACK: enabled, payload length: 10 bytes

  // Set TX power for Auto-ACK (maximum, to ensure that transmitter will hear ACK reply)
  nRF24_SetTXPower(nRF24_TXPWR_0dBm);

  // Set operational mode (PRX == receiver)
  nRF24_SetOperationalMode(nRF24_MODE_RX);

  // Clear any pending IRQ flags
  nRF24_ClearIRQFlags();

  // Wake the transceiver
  nRF24_SetPowerMode(nRF24_PWR_UP);

  // Put the transceiver to the RX mode
  nRF24_CE_H();

  // The main loop
  while (1)
  {
    //
    // Constantly poll the status of the RX FIFO and get a payload if FIFO is not empty
    //
    // This is far from best solution, but it's ok for testing purposes
    // More smart way is to use the IRQ pin :)
    //
    if (nRF24_GetStatus_RXFIFO() != nRF24_STATUS_RXFIFO_EMPTY)
    {
      // Get a payload from the transceiver
      pipe = nRF24_ReadPayload(nRF24_payload, &payload_length);

      // Clear all pending IRQ flags
      nRF24_ClearIRQFlags();

      // Print a payload contents to UART
      UART_SendStr("Receive pipe:");
      UART_SendInt(pipe);
      UART_SendStr(" Payload:");
      UART_SendBufHex(nRF24_payload, payload_length);
      UART_SendStr("\r\n");
    }
  }
}

void TxSingleESB()
// This is simple transmitter with Enhanced ShockBurst (to one logic address):
//   - TX address: 'ESB'
//   - payload: 10 bytes
//   - RF channel: 40 (2440MHz)
//   - data rate: 2Mbps
//   - CRC scheme: 2 byte
//
// The transmitter sends a 10-byte packets to the address 'ESB' with Auto-ACK (ShockBurst enabled)
{
  uint32_t i, j;

  // Set RF channel
  nRF24_SetRFChannel(40);

  // Set data rate
  nRF24_SetDataRate(nRF24_DataRate_2Mbps);

  // Set CRC scheme
  nRF24_SetCRCScheme(nRF24_CRC_2byte);

  // Set address width
  nRF24_SetAddressWidth(3);

  // Configure TX PIPE
  static const uint64_t Address = 0xB418E71CE3; // TX address.
  nRF24_SetAddress(nRF24_PIPETX, Address); // RX pipe 0 address. Must be the same as TX for auto-ACK.
  nRF24_SetAddress(nRF24_PIPE0, Address);

  // Set TX power (maximum)
  nRF24_SetTXPower(nRF24_TXPWR_0dBm);

  // Configure auto retransmit: 10 retransmissions with pause of 2500s in between
  nRF24_SetAutoRetransmission(nRF24_ARD_2500us, 10);

  // Enable Auto-ACK for pipe#0 (for ACK packets)
  nRF24_EnableAA(nRF24_PIPE0);

  // Set operational mode (PTX == transmitter)
  nRF24_SetOperationalMode(nRF24_MODE_TX);

  // Clear any pending IRQ flags
  nRF24_ClearIRQFlags();

  // Wake the transceiver
  nRF24_SetPowerMode(nRF24_PWR_UP);

  // Some variables
  uint32_t packets_lost = 0; // global counter of lost packets
  uint8_t NumLostPackets;
  uint8_t NumRetransmittedPackets;

  // The main loop
  payload_length = 10;
  j = 0;
  while (1)
  {
    // Prepare data packet
    for (i = 0; i < payload_length; i++)
    {
      nRF24_payload[i] = j++;
      if (j > 0x000000FF)
        j = 0;
    }

    // Print a payload
    UART_SendStr("Payload:");
    UART_SendBufHex(nRF24_payload, payload_length);
    UART_SendStr(" ... TX: ");

    // Transmit packet
    tx_res = nRF24_TransmitPacket(nRF24_payload, payload_length);
    nRF24_GetRetransmitCounters(&NumLostPackets, &NumRetransmittedPackets);
    switch(tx_res)
    {
      case nRF24_TX_SUCCESS:
        UART_SendStr("OK");
        break;
      case nRF24_TX_TIMEOUT:
        UART_SendStr("TIMEOUT");
        break;
      case nRF24_TX_MAXRT:
        UART_SendStr("MAX RETRANSMIT");
        packets_lost += NumLostPackets;
        nRF24_ResetPLOS();
        break;
      default:
        UART_SendStr("ERROR");
        break;
    }
    UART_SendStr("   ARC=");
    UART_SendInt(NumRetransmittedPackets);
    UART_SendStr(" LOST=");
    UART_SendInt(packets_lost);
    UART_SendStr("\r\n");

    HAL_Delay(500);
  }
}
