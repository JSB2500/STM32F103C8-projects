// Functions to manage the nRF24L01+ transceiver

#include <stdio.h>
#include "nrf24.h"
#include "uart.h"

///////////////////////////////////////////////////////////////////////////////
// nRF24L01+ chip:

// Instructions:
#define nRF24_CMD_R_REGISTER       (uint8_t)0x00 // Register read
#define nRF24_CMD_W_REGISTER       (uint8_t)0x20 // Register write
#define nRF24_CMD_R_RX_PAYLOAD     (uint8_t)0x61 // Read RX payload
#define nRF24_CMD_W_TX_PAYLOAD     (uint8_t)0xA0 // Write TX payload
#define nRF24_CMD_FLUSH_TX         (uint8_t)0xE1 // Flush TX FIFO
#define nRF24_CMD_FLUSH_RX         (uint8_t)0xE2 // Flush RX FIFO
#define nRF24_CMD_REUSE_TX_PL      (uint8_t)0xE3 // Reuse TX payload
#define nRF24_CMD_LOCK_UNLOCK      (uint8_t)0x50 // Lock/unlock exclusive features
#define nRF24_CMD_NOP              (uint8_t)0xFF // No operation (used for reading status register)

// Registers:
#define nRF24_REG_CONFIG           (uint8_t)0x00 // Configuration register
#define nRF24_REG_EN_AA            (uint8_t)0x01 // Enable "Auto acknowledgment"
#define nRF24_REG_EN_RXADDR        (uint8_t)0x02 // Enable RX addresses
#define nRF24_REG_SETUP_AW         (uint8_t)0x03 // Setup of address widths
#define nRF24_REG_SETUP_RETR       (uint8_t)0x04 // Setup of automatic retransmit
#define nRF24_REG_RF_CH            (uint8_t)0x05 // RF channel
#define nRF24_REG_RF_SETUP         (uint8_t)0x06 // RF setup register
#define nRF24_REG_STATUS           (uint8_t)0x07 // Status register
#define nRF24_REG_OBSERVE_TX       (uint8_t)0x08 // Transmit observe register
#define nRF24_REG_RPD              (uint8_t)0x09 // Received power detector
#define nRF24_REG_RX_ADDR_P0       (uint8_t)0x0A // Receive address data pipe 0
#define nRF24_REG_RX_ADDR_P1       (uint8_t)0x0B // Receive address data pipe 1
#define nRF24_REG_RX_ADDR_P2       (uint8_t)0x0C // Receive address data pipe 2
#define nRF24_REG_RX_ADDR_P3       (uint8_t)0x0D // Receive address data pipe 3
#define nRF24_REG_RX_ADDR_P4       (uint8_t)0x0E // Receive address data pipe 4
#define nRF24_REG_RX_ADDR_P5       (uint8_t)0x0F // Receive address data pipe 5
#define nRF24_REG_TX_ADDR          (uint8_t)0x10 // Transmit address
#define nRF24_REG_RX_PW_P0         (uint8_t)0x11 // Number of bytes in RX payload in data pipe 0
#define nRF24_REG_RX_PW_P1         (uint8_t)0x12 // Number of bytes in RX payload in data pipe 1
#define nRF24_REG_RX_PW_P2         (uint8_t)0x13 // Number of bytes in RX payload in data pipe 2
#define nRF24_REG_RX_PW_P3         (uint8_t)0x14 // Number of bytes in RX payload in data pipe 3
#define nRF24_REG_RX_PW_P4         (uint8_t)0x15 // Number of bytes in RX payload in data pipe 4
#define nRF24_REG_RX_PW_P5         (uint8_t)0x16 // Number of bytes in RX payload in data pipe 5
#define nRF24_REG_FIFO_STATUS      (uint8_t)0x17 // FIFO status register
#define nRF24_REG_DYNPD            (uint8_t)0x1C // Enable dynamic payload length
#define nRF24_REG_FEATURE          (uint8_t)0x1D // Feature register

// Addresses of the RX_PW_P# registers:
static const uint8_t nRF24_RX_PW_PIPE[6] =
{
  nRF24_REG_RX_PW_P0,
  nRF24_REG_RX_PW_P1,
  nRF24_REG_RX_PW_P2,
  nRF24_REG_RX_PW_P3,
  nRF24_REG_RX_PW_P4,
  nRF24_REG_RX_PW_P5
};

// Addresses of the RX/TX address registers:
static const uint8_t nRF24_ADDR_REGS[7] =
{
  nRF24_REG_RX_ADDR_P0,
  nRF24_REG_RX_ADDR_P1,
  nRF24_REG_RX_ADDR_P2,
  nRF24_REG_RX_ADDR_P3,
  nRF24_REG_RX_ADDR_P4,
  nRF24_REG_RX_ADDR_P5,
  nRF24_REG_TX_ADDR
};

// Register bits:
//
// CONFIG:
#define nRF24_CONFIG_PRIM_RX       (uint8_t)0x01 // PRIM_RX bit in CONFIG register
#define nRF24_CONFIG_PWR_UP        (uint8_t)0x02 // PWR_UP bit in CONFIG register
#define nRF24_CONFIG_RX_DR         (uint8_t)0x40 // RX_DR bit (data ready RX FIFO interrupt)
#define nRF24_CONFIG_TX_DS         (uint8_t)0x20 // TX_DS bit (data sent TX FIFO interrupt)
#define nRF24_CONFIG_MAX_RT        (uint8_t)0x10 // MAX_RT bit (maximum number of TX retransmits interrupt)

// Masks:
#define nRF24_MASK_REG_MAP         (uint8_t)0x1F // Mask bits[4:0] for CMD_RREG and CMD_WREG commands
#define nRF24_MASK_CRC             (uint8_t)0x0C // Mask for CRC bits [3:2] in CONFIG register
#define nRF24_MASK_STATUS_IRQ      (uint8_t)0x70 // Mask for all IRQ bits in STATUS register
#define nRF24_MASK_TX_PWR          (uint8_t)0x06 // Mask RF_PWR[2:1] bits in RF_SETUP register
#define nRF24_MASK_RX_P_NO         (uint8_t)0x0E // Mask RX_P_NO[3:1] bits in STATUS register
#define nRF24_MASK_DATARATE        (uint8_t)0x28 // Mask RD_DR_[5,3] bits in RF_SETUP register
#define nRF24_MASK_EN_RX           (uint8_t)0x3F // Mask ERX_P[5:0] bits in EN_RXADDR register
#define nRF24_MASK_RX_PW           (uint8_t)0x3F // Mask [5:0] bits in RX_PW_Px register
#define nRF24_MASK_RETR_ARD        (uint8_t)0xF0 // Mask for ARD[7:4] bits in SETUP_RETR register
#define nRF24_MASK_RETR_ARC        (uint8_t)0x0F // Mask for ARC[3:0] bits in SETUP_RETR register
#define nRF24_MASK_RXFIFO          (uint8_t)0x03 // Mask for RX FIFO status bits [1:0] in FIFO_STATUS register
#define nRF24_MASK_TXFIFO          (uint8_t)0x30 // Mask for TX FIFO status bits [5:4] in FIFO_STATUS register
#define nRF24_MASK_PLOS_CNT        (uint8_t)0xF0 // Mask for PLOS_CNT[7:4] bits in OBSERVE_TX register
#define nRF24_MASK_ARC_CNT         (uint8_t)0x0F // Mask for ARC_CNT[3:0] bits in OBSERVE_TX register
///////////////////////////////////////////////////////////////////////////////

static uint8_t nRF24_ReadReg(uint8_t reg)
{
  uint8_t TxData[2], RxData[2];
  uint8_t Result;

  TxData[0] = nRF24_CMD_R_REGISTER | (reg & nRF24_MASK_REG_MAP );
  TxData[1] = nRF24_CMD_NOP;

  nRF24_CSN_L();
  do
  {
  } while (nRF24_LL_RW(TxData, RxData, 2) != HAL_OK); // Will hang on error, by design.
  nRF24_CSN_H();

  Result = RxData[1];

  return Result;
}

static void nRF24_WriteReg(uint8_t reg, uint8_t value)
{
  uint8_t TxData[2], RxData[2];

  nRF24_CSN_L();

  if ((reg == nRF24_CMD_FLUSH_TX ) || (reg == nRF24_CMD_FLUSH_RX ) || (reg == nRF24_CMD_REUSE_TX_PL ) || (reg == nRF24_CMD_NOP ))
  {
    do
    {
    } while (nRF24_LL_RW(&reg, RxData, 1) != HAL_OK); // Will hang on error, by design.
  }
  else
  {
    // If register...
    if ((reg & 0xC0) == 0x00)
      reg = nRF24_CMD_W_REGISTER | (reg & nRF24_MASK_REG_MAP );

    TxData[0] = reg;
    TxData[1] = value;
    do
    {
    } while (nRF24_LL_RW(TxData, RxData, 2) != HAL_OK); // Will hang on error, by design.
  }

  nRF24_CSN_H();
}

static void nRF24_ReadMBReg(uint8_t reg, uint8_t *pBuf, uint8_t NumBytes)
{
  uint8_t i;
  uint8_t TxData[32];
  uint8_t RxData;

  // assert_param(NumBytes<=32); // JSB: I can't use this because enabling assert_param results in a PLLR value error. CubeMX problem?
  if (NumBytes > 32)
    return; // !!! Error condition.

  for (i = 0; i < NumBytes; ++i)
    TxData[i] = nRF24_CMD_NOP;

  nRF24_CSN_L();
  do
  {
  } while (nRF24_LL_RW(&reg, &RxData, 1) != HAL_OK); // Will hang on error, by design.
  do
  {
  } while (nRF24_LL_RW(TxData, pBuf, NumBytes) != HAL_OK); // Will hang on error, by design.
  nRF24_CSN_H();
}

static void nRF24_WriteMBReg(uint8_t reg, uint8_t *pBuf, uint8_t NumBytes)
{
  uint8_t RxData[32];

  // assert_param(NumBytes<=32); // JSB: I can't use this because enabling assert_param results in a PLLR value error. CubeMX problem?
  if (NumBytes > 32)
    return; // !!! Error condition.

  reg |= nRF24_CMD_W_REGISTER; // JSB: This converts register values to write commands and doesn't affect any other multi-byte write commands.

  nRF24_CSN_L();
  do
  {
  } while (nRF24_LL_RW(&reg, RxData, 1) != HAL_OK); // Will hang on error, by design.
  do
  {
  } while (nRF24_LL_RW(pBuf, RxData, NumBytes) != HAL_OK); // Will hang on error, by design.
  nRF24_CSN_H();
}

// Set transceiver to it's initial state.
// Note: RX/TX addresses remains unchanged.
void nRF24_Init(void)
{
  // Write to registers their initial values
  nRF24_WriteReg(nRF24_REG_CONFIG, 0x78);
  nRF24_WriteReg(nRF24_REG_EN_AA, 0x3F);
  nRF24_WriteReg(nRF24_REG_EN_RXADDR, 0x00); //!!!JSB Was 0x03
  nRF24_WriteReg(nRF24_REG_SETUP_AW, 0x03);
  nRF24_WriteReg(nRF24_REG_SETUP_RETR, 0x03);
  nRF24_WriteReg(nRF24_REG_RF_CH, 0x02);
  nRF24_WriteReg(nRF24_REG_RF_SETUP, 0x0E);
  nRF24_WriteReg(nRF24_REG_STATUS, 0x00);
  nRF24_WriteReg(nRF24_REG_RX_PW_P0, 0x00);
  nRF24_WriteReg(nRF24_REG_RX_PW_P1, 0x00);
  nRF24_WriteReg(nRF24_REG_RX_PW_P2, 0x00);
  nRF24_WriteReg(nRF24_REG_RX_PW_P3, 0x00);
  nRF24_WriteReg(nRF24_REG_RX_PW_P4, 0x00);
  nRF24_WriteReg(nRF24_REG_RX_PW_P5, 0x00);
  nRF24_WriteReg(nRF24_REG_DYNPD, 0x00);
  nRF24_WriteReg(nRF24_REG_FEATURE, 0x00);

  // Clear the FIFO's
  nRF24_FlushRX();
  nRF24_FlushTX();

  // Clear any pending interrupt flags
  nRF24_ClearIRQFlags();

  // Deassert CSN pin (chip release)
  nRF24_CSN_H();
}

uint8_t nRF24_Check(void)
// Return:
//   0: Fail
//   1: Success
{
  uint8_t rxbuf[5];
  uint8_t i;
  uint8_t *ptr = (uint8_t *) "nRF24";

  // Write test TX address and read TX_ADDR register
  nRF24_WriteMBReg(nRF24_REG_TX_ADDR, ptr, 5);
  nRF24_ReadMBReg(nRF24_REG_TX_ADDR, rxbuf, 5);

  // Compare buffers, return error on first mismatch
  for (i = 0; i < 5; i++)
  {
    if (rxbuf[i] != *ptr++)
      return 0;
  }

  return 1;
}

void nRF24_EnableInterrupts(uint8_t RX_DR, uint8_t TX_DS, uint8_t MAX_RT)
// JSB: Added.
{
  uint8_t Value;

  Value = nRF24_ReadReg(nRF24_REG_CONFIG);

  if (RX_DR)
    Value &= ~nRF24_CONFIG_RX_DR;
  else
    Value |= nRF24_CONFIG_RX_DR;

  if (TX_DS)
    Value &= ~nRF24_CONFIG_TX_DS;
  else
    Value |= nRF24_CONFIG_TX_DS;

  if (MAX_RT)
    Value &= ~nRF24_CONFIG_MAX_RT;
  else
    Value |= nRF24_CONFIG_MAX_RT;

  nRF24_WriteReg(nRF24_REG_CONFIG, Value);
}

int8_t nRF24_ConfigRegisterIsZero()
// JSB: Added. To identify that chip has crashed / got corrupted e.g. on STM32F769 board.
{
  uint8_t reg;

  reg = nRF24_ReadReg(nRF24_REG_CONFIG);

  return reg == 0x00;
}

// Control transceiver power mode
// input:
//   mode - new state of power mode, one of nRF24_PWR_xx values
void nRF24_SetPowerMode(nRF24_PWR value)
{
  uint8_t reg;

  reg = nRF24_ReadReg(nRF24_REG_CONFIG);
  if (value == nRF24_PWR_UP)
  {
    // Power up the transceiver.
    // It goes into Standby-I mode with current consumption of about 26uA.
    reg |= nRF24_CONFIG_PWR_UP;
  }
  else
  {
    // Power down the transceiver.
    // Current consumption is about 900nA.
    reg &= ~nRF24_CONFIG_PWR_UP;
  }
  nRF24_WriteReg(nRF24_REG_CONFIG, reg);
}

// Set transceiver operational mode
// input:
//   mode - operational mode, one of nRF24_MODE_xx values
void nRF24_SetOperationalMode(nRF24_MODE value)
{
  uint8_t reg;

  // Configure PRIM_RX bit of the CONFIG register
  reg = nRF24_ReadReg(nRF24_REG_CONFIG);
  reg &= ~nRF24_CONFIG_PRIM_RX;
  reg |= (value & nRF24_CONFIG_PRIM_RX );
  nRF24_WriteReg(nRF24_REG_CONFIG, reg);
}

void nRF24_SetCRCScheme(nRF24_CRC value)
// NB: Transceiver will forcibly turn on the CRC if auto acknowledgement is enabled for at least one RX pipe.
{
  uint8_t reg;

  // Configure EN_CRC[3] and CRCO[2] bits of the CONFIG register
  reg = nRF24_ReadReg(nRF24_REG_CONFIG);
  reg &= ~nRF24_MASK_CRC;
  reg |= (value | nRF24_MASK_CRC );
  nRF24_WriteReg(nRF24_REG_CONFIG, reg);
}

// Set frequency channel
// input:
//   channel - radio frequency channel, value from 0 to 127
// note: frequency will be (2400 + channel)MHz
// note: PLOS_CNT[7:4] bits of the OBSERVER_TX register will be reset
void nRF24_SetRFChannel(uint8_t channel)
{
  nRF24_WriteReg(nRF24_REG_RF_CH, channel);
}

void nRF24_GetAutoRetransmission(nRF24_ARD *ard, uint8_t *arc)
// JSB: Added
{
  uint8_t Value;

  Value = nRF24_ReadReg(nRF24_REG_SETUP_RETR);

  *ard = (Value >> 4);
  *arc = (Value & 0x0F);
}

// Set automatic retransmission parameters
// input:
//   ard - auto retransmit delay. One of nRF24_ARD_xx values.
//   arc - Number of auto retransmits. 0 to 15.
void nRF24_SetAutoRetransmission(nRF24_ARD ard, uint8_t arc)
{
  nRF24_WriteReg(nRF24_REG_SETUP_RETR, (uint8_t) ((ard << 4) | (arc & nRF24_MASK_RETR_ARC )));
}

uint8_t nRF24_GetAddressWidth()
// JSB: Added.
{
  int AddressWidth;

  AddressWidth = nRF24_ReadReg(nRF24_REG_SETUP_AW) + 2;

  if ((AddressWidth < 3) || (AddressWidth > 5))
    AddressWidth = 0;

  return AddressWidth;
}

// Set of address widths
// input:
//   addr_width - RX/TX address field width, value from 3 to 5
// note: this setting is common for all pipes
void nRF24_SetAddressWidth(uint8_t addr_width)
{
  nRF24_WriteReg(nRF24_REG_SETUP_AW, addr_width - 2);
}

uint64_t nRF24_GetAddress(nRF24_PIPE pipe)
// JSB: Added.
{
  uint8_t RXAddressBytes[5];
  uint8_t RXAddressByte;
  uint64_t Result = 0x00;

  if ((pipe >= nRF24_PIPE2) && (pipe <= nRF24_PIPE5))
  {
    Result = nRF24_GetAddress(nRF24_PIPE1);
    RXAddressByte = nRF24_ReadReg(nRF24_ADDR_REGS[pipe]);
    Result |= RXAddressByte;
  }
  else
  {
    nRF24_ReadMBReg(nRF24_ADDR_REGS[pipe], RXAddressBytes, nRF24_GetAddressWidth());

    for (int8_t ByteIndex = nRF24_GetAddressWidth() - 1; ByteIndex >= 0; --ByteIndex) // JSB: Index must be signed.
    {
      RXAddressByte = RXAddressBytes[ByteIndex];
      Result = Result << 8;
      Result |= RXAddressByte;
    }
  }
  return Result;
}

void nRF24_SetAddress(nRF24_PIPE pipe, const uint64_t address)
// JSB: Changed to use uint64_t address to end confusion about byte ordering, which is particularly a problem when trying to get this working with transceivers that use other libraries.
{
  uint8_t AddressWidth;

  switch(pipe)
  {
    case nRF24_PIPETX:
    case nRF24_PIPE0:
    case nRF24_PIPE1:
      AddressWidth = nRF24_GetAddressWidth();
      nRF24_WriteMBReg(nRF24_ADDR_REGS[pipe], (uint8_t *) &address, AddressWidth); // Assumes that "address" is stored little-endian.
      break;

    case nRF24_PIPE2:
    case nRF24_PIPE3:
    case nRF24_PIPE4:
    case nRF24_PIPE5:
      nRF24_WriteReg(nRF24_ADDR_REGS[pipe], address & 0xFF); // Write LSBbyte of address.
      break;

    default:
      // Incorrect pipe number -> do nothing
      break;
  }
}

nRF24_TXPWR nRF24_GetTXPower()
// JSB: Added.
{
  uint8_t RegisterValue = nRF24_ReadReg(nRF24_REG_RF_SETUP);

  return (RegisterValue & nRF24_MASK_TX_PWR );
}

void nRF24_GetTXPowerAsString(char *S)
// JSB: Added.
{
  switch(nRF24_GetTXPower())
  {
    case nRF24_TXPWR_Minus18dBm:
      sprintf(S, "-18dBm");
      break;
    case nRF24_TXPWR_Minus12dBm:
      sprintf(S, "-12dBm");
      break;
    case nRF24_TXPWR_Minus6dBm:
      sprintf(S, "-6dBm");
      break;
    case nRF24_TXPWR_0dBm:
      sprintf(S, "0dBm");
      break;
    default:
      sprintf(S, "<Error: Unhandled case>");
      UART_printf("Error: Unhandled case");
      break;
  }
}

void nRF24_SetTXPower(nRF24_TXPWR value)
{
  uint8_t reg;

  // Configure RF_PWR[2:1] bits of the RF_SETUP register
  reg = nRF24_ReadReg(nRF24_REG_RF_SETUP);
  reg &= ~nRF24_MASK_TX_PWR;
  reg |= value;
  nRF24_WriteReg(nRF24_REG_RF_SETUP, reg);
}

nRF24_DataRate nRF24_GetDataRate()
// JSB: Added.
{
  uint8_t RegisterValue = nRF24_ReadReg(nRF24_REG_RF_SETUP);

  return (RegisterValue & nRF24_MASK_DATARATE );
}

void nRF24_GetDataRateAsString(char *S)
// JSB: Added.
{
  switch(nRF24_GetDataRate())
  {
    case nRF24_DataRate_1Mbps:
      sprintf(S, "1Mbps");
      break;
    case nRF24_DataRate_2Mbps:
      sprintf(S, "2Mbps");
      break;
    case nRF24_DataRate_250kbps:
      sprintf(S, "250kbps");
      break;
    default:
      sprintf(S, "<Error: Unhandled case>");
      UART_printf("Error: Unhandled case");
      break;
  }
}

// Configure transceiver data rate
// input:
//   data_rate - data rate, one of nRF24_DR_xx values
void nRF24_SetDataRate(nRF24_DataRate data_rate)
{
  uint8_t reg;

  // Configure RF_DR_LOW[5] and RF_DR_HIGH[3] bits of the RF_SETUP register
  reg = nRF24_ReadReg(nRF24_REG_RF_SETUP);
  reg &= ~nRF24_MASK_DATARATE;
  reg |= data_rate;
  nRF24_WriteReg(nRF24_REG_RF_SETUP, reg);
}

// Configure a specified RX pipe
// input:
//   pipe - number of the RX pipe, value from 0 to 5
//   aa_state - state of auto acknowledgment, one of nRF24_AA_xx values
//   payload_len - payload length in bytes
void nRF24_SetRXPipe(nRF24_PIPE pipe, nRF24_AA aa_state, uint8_t payload_len)
{
  uint8_t reg;

  // Enable the specified pipe (EN_RXADDR register)
  reg = (nRF24_ReadReg(nRF24_REG_EN_RXADDR) | (1 << pipe)) & nRF24_MASK_EN_RX;
  nRF24_WriteReg(nRF24_REG_EN_RXADDR, reg);

  // Set RX payload length (RX_PW_Px register)
  nRF24_WriteReg(nRF24_RX_PW_PIPE[pipe], payload_len & nRF24_MASK_RX_PW);

  // Set auto acknowledgment for a specified pipe (EN_AA register)
  reg = nRF24_ReadReg(nRF24_REG_EN_AA);
  if (aa_state == nRF24_AA_ON)
  {
    reg |= (1 << pipe);
  }
  else
  {
    reg &= ~(1 << pipe);
  }
  nRF24_WriteReg(nRF24_REG_EN_AA, reg);
}

// Disable specified RX pipe
// input:
//   PIPE - number of RX pipe, value from 0 to 5
void nRF24_ClosePipe(nRF24_PIPE pipe)
{
  uint8_t reg;

  reg = nRF24_ReadReg(nRF24_REG_EN_RXADDR);
  reg &= ~(1 << pipe);
  reg &= nRF24_MASK_EN_RX;
  nRF24_WriteReg(nRF24_REG_EN_RXADDR, reg);
}

// Enable the auto retransmit (a.k.a. enhanced ShockBurst) for the specified RX pipe
// input:
//   pipe - number of the RX pipe, value from 0 to 5
void nRF24_EnableAA(nRF24_PIPE pipe)
{
  uint8_t reg;

  // Set bit in EN_AA register
  reg = nRF24_ReadReg(nRF24_REG_EN_AA);
  reg |= (1 << pipe);
  nRF24_WriteReg(nRF24_REG_EN_AA, reg);
}

// Disable the auto retransmit (a.k.a. enhanced ShockBurst) for one or all RX pipes
// input:
//   pipe - number of the RX pipe, value from 0 to 5, any other value will disable AA for all RX pipes
void nRF24_DisableAA(nRF24_PIPE pipe)
{
  uint8_t reg;

  if (pipe > 5)
  {
    // Disable Auto-ACK for ALL pipes
    nRF24_WriteReg(nRF24_REG_EN_AA, 0x00);
  }
  else
  {
    // Clear bit in the EN_AA register
    reg = nRF24_ReadReg(nRF24_REG_EN_AA);
    reg &= ~(1 << pipe);
    nRF24_WriteReg(nRF24_REG_EN_AA, reg);
  }
}

uint8_t nRF24_GetStatus(void)
{
  return nRF24_ReadReg(nRF24_REG_STATUS);
}

// Get pending IRQ flags
// return: current status of RX_DR, TX_DS and MAX_RT bits of the STATUS register
uint8_t nRF24_GetIRQFlags(void)
{
  return (nRF24_ReadReg(nRF24_REG_STATUS) & nRF24_MASK_STATUS_IRQ );
}

// Get status of the RX FIFO
nRF24_STATUS_RXFIFO nRF24_GetStatus_RXFIFO(void)
{
  return (nRF24_ReadReg(nRF24_REG_FIFO_STATUS) & nRF24_MASK_RXFIFO );
}

// Get status of the TX FIFO
// note: the TX_REUSE bit ignored
nRF24_STATUS_TXFIFO nRF24_GetStatus_TXFIFO(void)
{
  return ((nRF24_ReadReg(nRF24_REG_FIFO_STATUS) & nRF24_MASK_TXFIFO ) >> 4);
}

// Get pipe number for the payload available for reading from RX FIFO
// return: pipe number or 0x07 if the RX FIFO is empty
uint8_t nRF24_GetRXSource(void)
{
  return ((nRF24_ReadReg(nRF24_REG_STATUS) & nRF24_MASK_RX_P_NO ) >> 1);
}

void nRF24_GetRetransmitCounters(uint8_t *pLostPackets, uint8_t *pRetransmittedPackets)
{
  uint8_t reg;

  reg = nRF24_ReadReg(nRF24_REG_OBSERVE_TX);

  *pLostPackets = (reg & nRF24_MASK_PLOS_CNT ) >> 4;
  *pRetransmittedPackets = (reg & nRF24_MASK_ARC_CNT);
}

// Reset packet lost counter (PLOS_CNT bits in OBSERVER_TX register)
void nRF24_ResetPLOS(void)
{
  uint8_t reg;

  // The PLOS counter is reset after write to RF_CH register
  reg = nRF24_ReadReg(nRF24_REG_RF_CH);
  nRF24_WriteReg(nRF24_REG_RF_CH, reg);
}

// Flush the TX FIFO
void nRF24_FlushTX(void)
{
  nRF24_WriteReg(nRF24_CMD_FLUSH_TX, nRF24_CMD_NOP);
}

// Flush the RX FIFO
void nRF24_FlushRX(void)
{
  nRF24_WriteReg(nRF24_CMD_FLUSH_RX, nRF24_CMD_NOP);
}

// Clear any pending IRQ flags
void nRF24_ClearIRQFlags(void)
{
  uint8_t reg;

  // Clear RX_DR, TX_DS and MAX_RT bits of the STATUS register
  reg = nRF24_ReadReg(nRF24_REG_STATUS);
  reg |= nRF24_MASK_STATUS_IRQ;
  nRF24_WriteReg(nRF24_REG_STATUS, reg);
}

void nRF24_ClearIRQ_RX_DR_Flag(void)
// Added by JSB.
{
  uint8_t reg;

  reg = nRF24_ReadReg(nRF24_REG_STATUS);
  reg |= nRF24_STATUS_RX_DR;
  nRF24_WriteReg(nRF24_REG_STATUS, reg);
}

// Write TX payload
// input:
//   pBuf - pointer to the buffer with payload data
//   length - payload length in bytes
void nRF24_WritePayload(uint8_t *pBuf, uint8_t length)
{
  nRF24_WriteMBReg(nRF24_CMD_W_TX_PAYLOAD, pBuf, length);
}

// Read top level payload available in the RX FIFO
// input:
//   pBuf - pointer to the buffer to store a payload data
//   length - pointer to variable to store a payload length
// return: one of nRF24_RX_xx values
//   nRF24_RX_PIPEX - packet has been received from the pipe number X
//   nRF24_RX_EMPTY - the RX FIFO is empty
nRF24_RXResult nRF24_ReadPayload(uint8_t *pBuf, uint8_t *length)
{
  uint8_t pipe;

  // Extract a payload pipe number from the STATUS register
  pipe = (nRF24_ReadReg(nRF24_REG_STATUS) & nRF24_MASK_RX_P_NO ) >> 1;

  // RX FIFO empty?
  if (pipe < 6)
  {
    // Get payload length
    *length = nRF24_ReadReg(nRF24_RX_PW_PIPE[pipe]);

    // Read a payload from the RX FIFO
    if (*length)
    {
      nRF24_ReadMBReg(nRF24_CMD_R_RX_PAYLOAD, pBuf, *length);
    }

    return ((nRF24_RXResult) pipe);
  }

  // The RX FIFO is empty
  *length = 0;

  return nRF24_RX_EMPTY;
}

void nRF24_DumpConfiguration(void)
// Print nRF24L01+ current configuration (for debug purposes)
{
  uint8_t RegisterValue, j, PipeIndex;
  uint8_t AddressWidth;
  uint64_t RXTXAddress;
  char S[32];

  UART_printf("nRF24L01/nRF24L01+ configuration:\r\n");
  UART_printf("Begin\r\n");

  // CONFIG
  RegisterValue = nRF24_ReadReg(nRF24_REG_CONFIG);
  UART_printf("[0x%02X] 0x%02X CONFIG: Interrupt mask=%1X CRC enabled=%s CRC=%s Power=%s Mode=%s\r\n", nRF24_REG_CONFIG, RegisterValue, RegisterValue >> 4, (RegisterValue & 0x08) ? "Yes" : "No", (RegisterValue & 0x04) ? "2 bytes" : "1 byte", (RegisterValue & 0x02) ? "On" : "Off", (RegisterValue & 0x01) ? "RX" : "TX");

  // EN_AA
  RegisterValue = nRF24_ReadReg(nRF24_REG_EN_AA);
  UART_printf("[0x%02X] 0x%02X EN_AA: Enable auto-acknowledgement on pipe: ", nRF24_REG_EN_AA, RegisterValue);
  for (j = 0; j < 6; j++)
  {
    UART_printf("[P%1u%s]%s", j, (RegisterValue & (1 << j)) ? "+" : "-", (j == 5) ? "\r\n" : " ");
  }

  // EN_RXADDR
  RegisterValue = nRF24_ReadReg(nRF24_REG_EN_RXADDR);
  UART_printf("[0x%02X] 0x%02X EN_RXADDR: Enable RX pipe: ", nRF24_REG_EN_RXADDR, RegisterValue);
  for (j = 0; j < 6; j++)
  {
    UART_printf("[P%1u%s]%s", j, (RegisterValue & (1 << j)) ? "+" : "-", (j == 5) ? "\r\n" : " ");
  }

  // SETUP_AW
  RegisterValue = nRF24_ReadReg(nRF24_REG_SETUP_AW);
  AddressWidth = (RegisterValue & 0x03) + 2;
  UART_printf("[0x%02X] 0x%02X SETUP_AW: Address width=%u\r\n", nRF24_REG_SETUP_AW, RegisterValue, AddressWidth);

  // SETUP_RETR
  RegisterValue = nRF24_ReadReg(nRF24_REG_SETUP_RETR);
  uint8_t ard, arc;
  nRF24_GetAutoRetransmission(&ard, &arc);
  UART_printf("[0x%02X] 0x%02X SETUP_RETR ARD=%d (%uus) ARC=%d\r\n", nRF24_REG_SETUP_RETR, RegisterValue, ard, (ard * 250) + 250, arc);

  // RF_CH
  RegisterValue = nRF24_ReadReg(nRF24_REG_RF_CH);
  UART_printf("[0x%02X] 0x%02X RF_CH: Channel=%d (0x%02X) Frequency=%.3uGHz\r\n", nRF24_REG_RF_CH, RegisterValue, RegisterValue, RegisterValue, 2400 + RegisterValue);

  // RF_SETUP
  RegisterValue = nRF24_ReadReg(nRF24_REG_RF_SETUP);
  UART_printf("[0x%02X] 0x%02X RF_SETUP: CONT_WAVE=%s PLL_LOCK=%s", nRF24_REG_RF_SETUP, RegisterValue, (RegisterValue & 0x80) ? "On" : "Off", (RegisterValue & 0x80) ? "On" : "Off");
  nRF24_GetDataRateAsString(S);
  UART_printf(" DataRate=%s", S);
  nRF24_GetTXPowerAsString(S);
  UART_printf(" RF power=%s\r\n", S);

  // STATUS
  RegisterValue = nRF24_ReadReg(nRF24_REG_STATUS);
  UART_printf("[0x%02X] 0x%02X STATUS: IRQ=%01X RX_PIPE=%u TX_FULL=%s\r\n", nRF24_REG_STATUS, RegisterValue, (RegisterValue & 0x70) >> 4, (RegisterValue & 0x0E) >> 1, (RegisterValue & 0x01) ? "Yes" : "No");

  // OBSERVE_TX
  RegisterValue = nRF24_ReadReg(nRF24_REG_OBSERVE_TX);
  UART_printf("[0x%02X] 0x%02X PLOS_CNT=%u ARC_CNT=%u\r\n", nRF24_REG_OBSERVE_TX, RegisterValue, RegisterValue >> 4, RegisterValue & 0x0F);

  // RPD
  RegisterValue = nRF24_ReadReg(nRF24_REG_RPD);
  UART_printf("[0x%02X] 0x%02X RPD=%s\r\n", nRF24_REG_RPD, RegisterValue, (RegisterValue & 0x01) ? "Yes" : "No");

  // RX_ADDR_P0 to RX_ADDR_P5
  for (PipeIndex = 0; PipeIndex < 6; ++PipeIndex)
  {
    RXTXAddress = nRF24_GetAddress(nRF24_PIPE0 + PipeIndex);
    UART_printf("[0x%02X] RX_ADDR_P%d=0x%01X%08X\r\n", nRF24_REG_RX_ADDR_P0 + PipeIndex, PipeIndex, (uint32_t) (RXTXAddress >> 32), (uint32_t) RXTXAddress); // JSB: I couldn't get PRIx64 to work. Perhaps this printf does not support 64 bit integers.
  }

  // TX_ADDR
  RXTXAddress = nRF24_GetAddress(nRF24_PIPETX);
  UART_printf("[0x%02X] TX_ADDR=0x%01X%08X\r\n", nRF24_REG_TX_ADDR, (uint32_t) (RXTXAddress >> 32), (uint32_t) RXTXAddress); // JSB: I couldn't get PRIx64 to work. Perhaps this printf does not support 64 bit integers.

  for (PipeIndex = 0; PipeIndex < 6; ++PipeIndex)
  {
    RegisterValue = nRF24_ReadReg(nRF24_REG_RX_PW_P0 + PipeIndex);
    UART_printf("[0x%02X] RX_PW_P%d=%u\r\n", nRF24_REG_RX_PW_P0 + PipeIndex, PipeIndex, RegisterValue);
  }

  UART_printf("End\r\n\r\n");
}

void nRF24_DumpRegisters(void)
// JSB: Added.
{
  uint8_t RegisterAddress, RegisterValue, AddressWidth;
  uint8_t RXAddressBytes[5];

  UART_printf("nRF24L01/nRF24L01+ registers:\r\n");
  UART_printf("Begin\r\n");

  for (RegisterAddress = 0x00; RegisterAddress <= 0x1D; ++RegisterAddress)
  {
    UART_printf("0x%02X: ", RegisterAddress);

    switch(RegisterAddress)
    {
      case nRF24_REG_RX_ADDR_P0 :
      case nRF24_REG_RX_ADDR_P1 :
      case nRF24_REG_TX_ADDR :
        AddressWidth = nRF24_GetAddressWidth();
        nRF24_ReadMBReg(RegisterAddress, RXAddressBytes, AddressWidth);
        for (int ByteIndex = 0; ByteIndex < AddressWidth; ++ByteIndex)
          UART_printf("0x%02X ", RXAddressBytes[ByteIndex]);
        UART_printf("[LSB to MSB]");
        break;

      case 0x18:
      case 0x19:
      case 0x1A:
      case 0x1B:
        // Payload registers.
        break;

      default:
        RegisterValue = nRF24_ReadReg(RegisterAddress);
        UART_printf("0x%02X", RegisterValue);
    }

    UART_printf("\r\n");
  }
  UART_printf("End\r\n\r\n");
}
