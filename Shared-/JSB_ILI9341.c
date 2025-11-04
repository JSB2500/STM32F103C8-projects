// Copyright 2017 J S Bladen.

///////////////////////////////////////////////////////////////////////////////
// Acknowledgements:
//
// => ILI9341 driver based on Adafruit ILI9341 driver.
// => Font technology based on Adafruit graphics / font libraries.
///////////////////////////////////////////////////////////////////////////////

#include <string.h>
#include <stdlib.h>

#include "JSB_General.h"
#include "JSB_ILI9341.h"

static SPI_HandleTypeDef *phspi;

#define TextColor_Default (ILI9341_COLOR_WHITE)
#define TextBackgroundColor_Default (ILI9341_COLOR_BLACK)
#define TextDrawMode_Default (tdmThisCharBar)
#define TextVerticalOrigin_Default (tvoBase)

static uint16_t TextColor = TextColor_Default;
static uint16_t TextBackgroundColor = TextBackgroundColor_Default;
static TextDrawMode_t TextDrawMode = TextDrawMode_Default;
static TextVerticalOrigin_t TextVerticalOrigin = TextVerticalOrigin_Default;

static const GFXfont *pFont = NULL;

static uint8_t UseBackBuffer = 0;
static uint16_t *pBackBuffer = NULL;

///////////////////////////////////////////////////////////////////////////////
// ILI9341 commands, mostly from Adafruit IPI9341 library:

#define ILI9341_NOP                 0x00
#define ILI9341_SWRESET             0x01
#define ILI9341_RDDID               0x04
#define ILI9341_RDDST               0x09

#define ILI9341_RDMODE              0x0A
#define ILI9341_RDMADCTL            0x0B
#define ILI9341_RDPIXFMT            0x0C
#define ILI9341_RDIMGFMT            0x0D
#define ILI9341_RDDSM               0x0E
#define ILI9341_RDSELFDIAG          0x0F

#define ILI9341_SLPIN               0x10
#define ILI9341_SLPOUT              0x11
#define ILI9341_PTLON               0x12
#define ILI9341_NORON               0x13

#define ILI9341_INVOFF              0x20
#define ILI9341_INVON               0x21
#define ILI9341_GAMMASET            0x26
#define ILI9341_DISPOFF             0x28
#define ILI9341_DISPON              0x29

#define ILI9341_CASET               0x2A
#define ILI9341_PASET               0x2B
#define ILI9341_RAMWR               0x2C
#define ILI9341_RGBSET              0x2D
#define ILI9341_RAMRD               0x2E

#define ILI9341_PTLAR               0x30
#define ILI9341_VSCRDEF             0x33
#define ILI9341_TEOFF               0x34
#define ILI9341_TEON                0x35
#define ILI9341_MADCTL              0x36
#define ILI9341_VSCRSADD            0x37
#define ILI9341_IDMOFF              0x38
#define ILI9341_IDMON               0x39
#define ILI9341_PIXFMT              0x3A
#define ILI9341_WRITE_MEM_CONTINUE  0x3C
#define ILI9341_READ_MEM_CONTINUE   0x3E

#define ILI9341_SET_TEAR_SCANLINE   0x44
#define ILI9341_GET_SCANLINE        0x45

#define ILI9341_WDB                 0x51
#define ILI9341_RDDISBV             0x52
#define ILI9341_WCD                 0x53
#define ILI9341_RDCTRLD             0x54
#define ILI9341_WRCABC              0x55
#define ILI9341_RDCABC              0x56
#define ILI9341_WRITE_CABC          0x5E
#define ILI9341_READ_CABC           0x5F

#define ILI9341_RGB_INTERFACE       0xB0
#define ILI9341_FRMCTR1             0xB1
#define ILI9341_FRMCTR2             0xB2
#define ILI9341_FRMCTR3             0xB3
#define ILI9341_INVCTR              0xB4
#define ILI9341_BPC                 0xB5
#define ILI9341_DFUNCTR             0xB6
#define ILI9341_ETMOD               0xB7
#define ILI9341_BACKLIGHT1          0xB8
#define ILI9341_BACKLIGHT2          0xB9
#define ILI9341_BACKLIGHT3          0xBA
#define ILI9341_BACKLIGHT4          0xBB
#define ILI9341_BACKLIGHT5          0xBC
#define ILI9341_BACKLIGHT7          0xBE
#define ILI9341_BACKLIGHT8          0xBF

#define ILI9341_PWCTR1              0xC0
#define ILI9341_PWCTR2              0xC1
#define ILI9341_PWCTR3              0xC2
#define ILI9341_PWCTR4              0xC3
#define ILI9341_PWCTR5              0xC4
#define ILI9341_VMCTR1              0xC5
#define ILI9341_VMCTR2              0xC7

#define LCD_NVMWR                   0xD0   /* NV Memory Write */
#define LCD_NVMPKEY                 0xD1   /* NV Memory Protection Key */
#define LCD_RDNVM                   0xD2   /* NV Memory Status Read */
#define LCD_READ_ID4                0xD3   /* Read ID4 */

#define ILI9341_RDID1               0xDA
#define ILI9341_RDID2               0xDB
#define ILI9341_RDID3               0xDC
#define ILI9341_RDID4               0xDD

#define ILI9341_GMCTRP1             0xE0
#define ILI9341_GMCTRN1             0xE1
#define LCD_DGAMCTRL1               0xE2
#define LCD_DGAMCTRL2               0xE3
#define LCD_INTERFACE               0xF6

/* Extend register commands */
#define LCD_POWERA                  0xCB
#define LCD_POWERB                  0xCF
#define LCD_DTCA                    0xE8
#define LCD_DTCB                    0xEA
#define LCD_POWER_SEQ               0xED
#define LCD_3GAMMA_EN               0xF2
#define LCD_PRC                     0xF7
///////////////////////////////////////////////////////////////////////////////

#define ILI9341_CSX_Low() HAL_GPIO_WritePin(ILI9341_CSX_GPIO_Port, ILI9341_CSX_Pin, GPIO_PIN_RESET)
#define ILI9341_CSX_High() HAL_GPIO_WritePin(ILI9341_CSX_GPIO_Port, ILI9341_CSX_Pin, GPIO_PIN_SET)
#define ILI9341_DC_Low() HAL_GPIO_WritePin(ILI9341_D_CX_GPIO_Port, ILI9341_D_CX_Pin, GPIO_PIN_RESET)
#define ILI9341_DC_High() HAL_GPIO_WritePin(ILI9341_D_CX_GPIO_Port, ILI9341_D_CX_Pin, GPIO_PIN_SET)

///////////////////////////////////////////////////////////////////////////////

//static uint32_t ILI9341_Read(uint8_t ReadSize)
//{
//  HAL_StatusTypeDef status = HAL_OK;
//  uint32_t readvalue;
//
//  status = HAL_SPI_Receive(phspi, (uint8_t*) &readvalue, ReadSize, 100);
//
//  if (status != HAL_OK)
//    Error_Handler();
//
//  return readvalue;
//}

static void ILI9341_Write8(uint8_t Value)
{
  uint8_t ReadData;

  if (HAL_SPI_TransmitReceive(phspi, (uint8_t*) &Value, &ReadData, 1, 100) != HAL_OK)
    Error_Handler();
}

//static void ILI9341_Write16(uint16_t Value)
//{
//  uint16_t WriteData, ReadData;
//
//  WriteData = SwapBytes(Value);
//
//  phspi->Init.DataSize = SPI_DATASIZE_16BIT;
//  phspi->Instance->CR1 |= SPI_DATASIZE_16BIT; //!!!Added for F103
//
//  if (HAL_SPI_TransmitReceive(phspi, (uint8_t*)&WriteData, (uint8_t*)&ReadData, 1, 100) != HAL_OK)
//    Error_Handler();
//
//  phspi->Instance->CR1 &= ~SPI_DATASIZE_16BIT; //!!!Added for F103
//  phspi->Init.DataSize = SPI_DATASIZE_8BIT;
//}

static void ILI9341_Write16(uint16_t Value)
// MSB first.
{
  ILI9341_Write8(Value >> 8);
  ILI9341_Write8(Value);
}

static void ILI9341_Write32(uint32_t Value)
// MSW first.
{
  ILI9341_Write16(Value >> 16);
  ILI9341_Write16(Value);
}

static void ILI9341_WriteCommand(int8_t Value)
{
  ILI9341_DC_Low();
  ILI9341_Write8(Value);
  ILI9341_DC_High();
}

void ILI9341_SetDefaultState()
{
  TextColor = TextColor_Default;
  TextBackgroundColor = TextBackgroundColor_Default;
  TextDrawMode = TextDrawMode_Default;
}

void ILI9341_Initialize(SPI_HandleTypeDef *i_phspi, int8_t i_UseBackBuffer)
///////////////////////////////////////////////////////////////////////////////
// Minimum configuration:
// ILI9341_WriteCommand(ILI9341_MADCTL); // Memory Access Control
// ILI9341_Write8(0x48);
//
// ILI9341_WriteCommand(ILI9341_PIXFMT);
// ILI9341_Write8(0x55); // DPI=[6:4] DBI=[2:0]
//
// ILI9341_WriteCommand(ILI9341_SLPOUT); // Exit sleep
// HAL_Delay(120);
// ILI9341_WriteCommand(ILI9341_DISPON); // Display on
///////////////////////////////////////////////////////////////////////////////
{
  phspi = i_phspi;
  UseBackBuffer = i_UseBackBuffer;

  if (UseBackBuffer)
  {
    pBackBuffer = calloc(1, ILI9341_Width * ILI9341_Height * sizeof(uint16_t));
    if (!pBackBuffer)
      Error_Handler();
  }

  ILI9341_SetDefaultState();

  HAL_GPIO_WritePin(ILI9341_RESX_GPIO_Port, ILI9341_RESX_Pin, GPIO_PIN_RESET);
  HAL_Delay(1);
  HAL_GPIO_WritePin(ILI9341_RESX_GPIO_Port, ILI9341_RESX_Pin, GPIO_PIN_SET);
  HAL_Delay(5);

  ILI9341_CSX_Low();

  ILI9341_WriteCommand(0xEF);
  ILI9341_Write8(0x03);
  ILI9341_Write8(0x80);
  ILI9341_Write8(0x02);

  ILI9341_WriteCommand(LCD_POWERB);
  ILI9341_Write8(0x00);
  ILI9341_Write8(0XC1);
  ILI9341_Write8(0X30);

  ILI9341_WriteCommand(LCD_POWER_SEQ);
  ILI9341_Write8(0x64);
  ILI9341_Write8(0x03);
  ILI9341_Write8(0X12);
  ILI9341_Write8(0X81);

  ILI9341_WriteCommand(LCD_DTCA);
  ILI9341_Write8(0x85);
  ILI9341_Write8(0x00);
  ILI9341_Write8(0x78);

  ILI9341_WriteCommand(LCD_POWERA);
  ILI9341_Write8(0x39);
  ILI9341_Write8(0x2C);
  ILI9341_Write8(0x00);
  ILI9341_Write8(0x34);
  ILI9341_Write8(0x02);

  ILI9341_WriteCommand(LCD_PRC);
  ILI9341_Write8(0x20);

  ILI9341_WriteCommand(LCD_DTCB);
  ILI9341_Write8(0x00);
  ILI9341_Write8(0x00);

  ILI9341_WriteCommand(ILI9341_PWCTR1); // Power control
  ILI9341_Write8(0x23); // VRH=[5:0]

  ILI9341_WriteCommand(ILI9341_PWCTR2); // Power control
  ILI9341_Write8(0x10); // BT=[3:0]

  ILI9341_WriteCommand(ILI9341_VMCTR1); // VCOM control
  ILI9341_Write8(0x3e); // VMH=[6:0]
  ILI9341_Write8(0x28); // VML=[6:0]

  ILI9341_WriteCommand(ILI9341_VMCTR2); // VCOM control 2
  ILI9341_Write8(0x86); // VMF=[6:0]

  ILI9341_WriteCommand(ILI9341_MADCTL); // Memory Access Control
  ILI9341_Write8(0x48);

  ILI9341_WriteCommand(ILI9341_VSCRSADD); // Vertical scroll
  ILI9341_Write16(0); // Zero

  ILI9341_WriteCommand(ILI9341_PIXFMT);
  ILI9341_Write8(0x55); // DPI=[6:4] DBI=[2:0]

  ILI9341_WriteCommand(ILI9341_FRMCTR1);
  ILI9341_Write8(0x00);
  ILI9341_Write8(0x18);

  ILI9341_WriteCommand(ILI9341_DFUNCTR); // Display Function Control
  ILI9341_Write8(0x08);
  ILI9341_Write8(0x82);
  ILI9341_Write8(0x27);

//  ILI9341_WriteCommand(ILI9341_WCD); // JSB
//  ILI9341_Write8(0xFF);

  ILI9341_WriteCommand(0xF2); // 3Gamma
  ILI9341_Write8(0x00); // Disable

  ILI9341_WriteCommand(ILI9341_GAMMASET); // Gamma curve selected
  ILI9341_Write8(0x01);
  //
  ILI9341_WriteCommand(ILI9341_GMCTRP1); // Set Gamma
  ILI9341_Write8(0x0F);
  ILI9341_Write8(0x31);
  ILI9341_Write8(0x2B);
  ILI9341_Write8(0x0C);
  ILI9341_Write8(0x0E);
  ILI9341_Write8(0x08);
  ILI9341_Write8(0x4E);
  ILI9341_Write8(0xF1);
  ILI9341_Write8(0x37);
  ILI9341_Write8(0x07);
  ILI9341_Write8(0x10);
  ILI9341_Write8(0x03);
  ILI9341_Write8(0x0E);
  ILI9341_Write8(0x09);
  ILI9341_Write8(0x00);
  //
  ILI9341_WriteCommand(ILI9341_GMCTRN1); // Set Gamma
  ILI9341_Write8(0x00);
  ILI9341_Write8(0x0E);
  ILI9341_Write8(0x14);
  ILI9341_Write8(0x03);
  ILI9341_Write8(0x11);
  ILI9341_Write8(0x07);
  ILI9341_Write8(0x31);
  ILI9341_Write8(0xC1);
  ILI9341_Write8(0x48);
  ILI9341_Write8(0x08);
  ILI9341_Write8(0x0F);
  ILI9341_Write8(0x0C);
  ILI9341_Write8(0x31);
  ILI9341_Write8(0x36);
  ILI9341_Write8(0x0F);

  ILI9341_WriteCommand(ILI9341_SLPOUT); // Exit sleep
  HAL_Delay(120);
  ILI9341_WriteCommand(ILI9341_DISPON); // Display on

  ILI9341_CSX_High();
}

void ILI9341_SetAddrWindow(uint16_t X, uint16_t Y, uint16_t Width, uint16_t Height, int8_t UseCS)
{
  if (UseCS)
    ILI9341_CSX_Low();

  uint32_t xa = ((uint32_t) X << 16) | (X + Width - 1);
  uint32_t ya = ((uint32_t) Y << 16) | (Y + Height - 1);

  ILI9341_WriteCommand(ILI9341_CASET); // Column addr set
  ILI9341_Write32(xa);

  ILI9341_WriteCommand(ILI9341_PASET); // Row addr set
  ILI9341_Write32(ya);

  ILI9341_WriteCommand(ILI9341_RAMWR); // Write to RAM

  if (UseCS)
    ILI9341_CSX_High();
}

static void ILI9341_DrawPixel_ToDisplay(int16_t X, int16_t Y, uint16_t Color, int8_t UseCS)
{
  if (UseCS)
    ILI9341_CSX_Low();

  if ((X < 0) || (X >= ILI9341_Width) || (Y < 0) || (Y >= ILI9341_Height))
    return;

  ILI9341_SetAddrWindow(X, Y, 1, 1, 0);
  ILI9341_Write16(Color);

  if (UseCS)
    ILI9341_CSX_Low();
}

static void ILI9341_DrawPixel_ToBackBuffer(int16_t X, int16_t Y, uint16_t Color)
{
  if ((X < 0) || (X >= ILI9341_Width) || (Y < 0) || (Y >= ILI9341_Height))
    return;

  pBackBuffer[X + Y * ILI9341_Width] = Color;
}

void ILI9341_DrawPixel(int16_t X, int16_t Y, uint16_t Color, int8_t UseCS)
{
  if (UseBackBuffer)
    ILI9341_DrawPixel_ToBackBuffer(X, Y, Color);
  else
    ILI9341_DrawPixel_ToDisplay(X, Y, Color, UseCS);
}

static void ILI9341_DrawPixels_MSBFirst_ToDisplay(uint16_t X, uint16_t Y, uint16_t Width, uint16_t Height, uint16_t *pPixels)
// Supplied pixel data must be byte swapped i.e. MSB first.
{
  if ((Width == 0) || (Height == 0))
    return;

  ILI9341_CSX_Low();

  ILI9341_WriteCommand(0x2A); // Set start and end columns.
  ILI9341_Write16(X);
  ILI9341_Write16(X + Width - 1);

  ILI9341_WriteCommand(0x2B); // Set start and end pages.
  ILI9341_Write16(Y);
  ILI9341_Write16(Y + Height - 1);

  ILI9341_WriteCommand(0x2C); // Memory write.
#if __CORTEX_M >= 7
  uint32_t NumBytes = Width * Height * sizeof(uint16_t);
  SCB_CleanDCache_by_Addr((uint32_t *)pPixels, NumBytes);
#endif
  for (int16_t RowIndex = 0; RowIndex < Height; ++RowIndex)
  {
    if (HAL_SPI_Transmit_DMA(phspi, (uint8_t*)pPixels, Width * sizeof(uint16_t)) != HAL_OK)
      Error_Handler();
    // Alternative:   if (HAL_SPI_Transmit(phspi, (uint8_t*) pPixels, Width * sizeof(uint16_t)), 1000) != HAL_OK)
    pPixels += Width;
    do {} while (phspi->hdmatx->State == HAL_DMA_STATE_BUSY);
  }

  ILI9341_CSX_High();
}

static void ILI9341_DrawPixels_MSBFirst_ToBackBuffer(uint16_t i_X, uint16_t i_Y, uint16_t i_Width, uint16_t i_Height, uint16_t *i_pPixels)
// Supplied pixel data must be byte swapped i.e. MSB first.
{
  uint16_t *pLine, *pPixel;

  if ((i_Width == 0) || (i_Height == 0))
    return;

  pPixel = i_pPixels;

  for (uint32_t Y = Clamp_uint16(i_Y, 0 , ILI9341_Height); Y < Clamp_uint16(i_Y + i_Height, 0 , ILI9341_Height); ++Y)
  {
    pLine = &pBackBuffer[Y * ILI9341_Width];
    for (uint32_t X = Clamp_uint16(i_X, 0, ILI9341_Width); X < Clamp_uint16(i_X + i_Width, 0, ILI9341_Width); ++X)
    {
      pLine[X] = *pPixel++;
    }
  }
}

void ILI9341_DrawPixels_MSBFirst(uint16_t X, uint16_t Y, uint16_t Width, uint16_t Height, uint16_t *pPixels)
// Supplied pixel data must be byte swapped i.e. MSB first.
{
  if (UseBackBuffer)
    ILI9341_DrawPixels_MSBFirst_ToBackBuffer(X, Y, Width, Height, pPixels);
  else
    ILI9341_DrawPixels_MSBFirst_ToDisplay(X, Y, Width, Height, pPixels);
}

uint8_t ILI9341_UsingBackBuffer()
{
  return UseBackBuffer;
}

void ILI9341_CopyBackBufferToDisplay()
{
  ILI9341_DrawPixels_MSBFirst_ToDisplay(0, 0, ILI9341_Width, ILI9341_Height, pBackBuffer);
}

static uint16_t ColumnColors[320]; //  // Num elements should be max(Width,Height) to support both orientations. Global so that F767 DMA can access it. (Not required with F103).

static void ILI9341_DrawBar_ToDisplay(uint16_t X, uint16_t Y, uint16_t Width, uint16_t Height, uint16_t Color)
{
  if ((Width == 0) || (Height == 0))
    return;

  ILI9341_CSX_Low();

  ILI9341_WriteCommand(0x2A); // Set start and end columns.
  ILI9341_Write16(X);
  ILI9341_Write16(X + Width - 1);

  ILI9341_WriteCommand(0x2B); // Set start and end pages.
  ILI9341_Write16(Y);
  ILI9341_Write16(Y + Height - 1);

//  // Slow but safe version.
//  ILI9341_WriteCommand(0x2C); // Memory write.
//  for (int16_t RowIndex = 0; RowIndex < Height; ++RowIndex)
//  {
//    for (int16_t ColumnIndex = 0; ColumnIndex < Width; ++ColumnIndex)
//    {
//      ILI9341_Write16(Color);
//    }
//  }

//  // Doesn't work with F103, even with phspi->Instance->CR1 != SPI_DATASIZE_16BIT etc.
//  uint16_t ColumnColors[320];
//  uint16_t ReadData[320];
//  uint16_t Color_MSBFirst = SwapBytes(Color);
//  for (int16_t ColumnIndex = 0; ColumnIndex < Width; ++ColumnIndex)
//    ColumnColors[ColumnIndex] = Color_MSBFirst;
//  ILI9341_WriteCommand(0x2C); // Memory write.
//  phspi->Init.DataSize = SPI_DATASIZE_16BIT;
//  for (int16_t RowIndex = 0; RowIndex < Height; ++RowIndex)
//  {
//    if (HAL_SPI_TransmitReceive(phspi, (uint8_t*)&ColumnColors, (uint8_t*)&ReadData, Width, 100) != HAL_OK)
//      Error_Handler();
//  }
//  phspi->Init.DataSize = SPI_DATASIZE_8BIT;

  uint16_t Color_MSBFirst = SwapBytes(Color);
  for (int16_t ColumnIndex = 0; ColumnIndex < Width; ++ColumnIndex)
    ColumnColors[ColumnIndex] = Color_MSBFirst;
  ILI9341_WriteCommand(0x2C); // Memory write.
  for (int16_t RowIndex = 0; RowIndex < Height; ++RowIndex)
  {
//    if (HAL_SPI_Transmit(phspi, (uint8_t*) &ColumnColors, NumBytes, 100) != HAL_OK)
//      Error_Handler();

    // This is about twice as fast as HAL_SPI_Transmit() on STM32F103 with 72MHz clock and 18MHz SPI clock.
    uint32_t NumBytes = Width * 2;
#if __CORTEX_M >= 7
    SCB_CleanDCache_by_Addr((uint32_t *)ColumnColors, NumBytes);
#endif
    if (HAL_SPI_Transmit_DMA(phspi, (uint8_t*) ColumnColors, NumBytes) != HAL_OK)
      Error_Handler();
    do {} while (phspi->hdmatx->State == HAL_DMA_STATE_BUSY);
  }

  ILI9341_CSX_High();
}

void ILI9341_DrawBar_ToBackBuffer(uint16_t i_X, uint16_t i_Y, uint16_t i_Width, uint16_t i_Height, uint16_t i_Color)
{
  uint16_t *pLine;
  uint16_t Color_MSBFirst;

  Color_MSBFirst = SwapBytes(i_Color);

  for (uint32_t Y = Clamp_uint16(i_Y, 0 , ILI9341_Height); Y < Clamp_uint16(i_Y + i_Height, 0 , ILI9341_Height); ++Y)
  {
    pLine = &pBackBuffer[Y * ILI9341_Width];
    for (uint32_t X = Clamp_uint16(i_X, 0, ILI9341_Width); X < Clamp_uint16(i_X + i_Width, 0, ILI9341_Width); ++X)
    {
      pLine[X] = Color_MSBFirst;
    }
  }
}

void ILI9341_DrawBar(uint16_t X, uint16_t Y, uint16_t Width, uint16_t Height, uint16_t Color)
{
  if (UseBackBuffer)
    ILI9341_DrawBar_ToBackBuffer(X, Y, Width, Height, Color);
  else
    ILI9341_DrawBar_ToDisplay(X, Y, Width, Height, Color);
}

void ILI9341_Clear(uint16_t Color)
{
  ILI9341_DrawBar(0, 0, ILI9341_Width, ILI9341_Height, Color);
}

void ILI9341_DrawRectangle(uint16_t Left, uint16_t Top, uint16_t Width, uint16_t Height, uint16_t Color)
{
  if (!UseBackBuffer)
    ILI9341_CSX_Low();

  for (uint32_t X = Left; X < Left + Width; ++X)
  {
    ILI9341_DrawPixel(X, Top, Color, 0);
    ILI9341_DrawPixel(X, Top + Height - 1, Color, 0);
  }

  for (uint32_t Y = Top; Y < Top + Height; ++Y)
  {
    ILI9341_DrawPixel(Left, Y, Color, 0);
    ILI9341_DrawPixel(Left + Width - 1, Y, Color, 0);
  }

  if (!UseBackBuffer)
    ILI9341_CSX_High();
}

const GFXfont *ILI9341_SetFont(const GFXfont *i_pFont)
{
  const GFXfont *Result;

  Result = pFont;
  pFont = i_pFont;
  return Result;
}

uint8_t ILI9341_GetFontYSpacing()
{
  return pFont->yAdvance;
}

static uint8_t IsNonPrintingChar(uint8_t Ch)
{
  return ((Ch < pFont->first) || (Ch > pFont->last));
}

uint16_t GetCharWidth(uint8_t Ch)
{
  if (IsNonPrintingChar(Ch))
    return 0;

  Ch -= pFont->first;
  GFXglyph *pGlyph = &pFont->pGlyph[Ch];
  uint8_t w = pGlyph->width;
  int8_t xo = pGlyph->xOffset;

  if (w == 0)
    return pGlyph->xAdvance;

  return xo + w;
}

uint16_t ILI9341_GetTextWidth(const char *Text)
{
  uint16_t TotalWidth, NumChars;
  const char *pText;

  NumChars = strlen(Text);

  pText = Text;

  TotalWidth=0;

  for (uint16_t CharIndex = 0; CharIndex < NumChars; ++CharIndex)
    TotalWidth += GetCharWidth(*pText++);

  return TotalWidth;
}

uint8_t ILI9341_DrawCharAtXY(uint8_t Ch, uint16_t X, uint16_t Y, uint16_t Color)
// X: X position of left edge of char.
// Y: Y position of line on which the char sits. The char may go below this line (e.g. g j p q y).
// Returns required X advance.
// Based on Adafruit_GFX.cpp.
{
  uint16_t bo;
  uint8_t w, h, xx, yy;
  int8_t xo, yo, yo_min, yo_max;
  uint8_t bits;
  uint8_t bit;
  GFXglyph *pGlyph;
  uint8_t *pBitmap;
  uint16_t Color_MSBFirst, TextBackgroundColor_MSBFirst;
  uint16_t *pMemChar, *pMemCharPixel;
  uint8_t CharWidth, CharHeight;

  if (!pFont)
    return 0;
  if (IsNonPrintingChar(Ch))
    return 0;

  Ch -= pFont->first;
  if (TextVerticalOrigin == tvoCentreBetweenBaseAndTop)
    Y -= (pFont->yOffsetMin / 2);

  pGlyph = &pFont->pGlyph[Ch];
  pBitmap = pFont->pBitmap;

  bo = pGlyph->bitmapOffset;
  w = pGlyph->width;
  h = pGlyph->height;
  xo = pGlyph->xOffset;
  yo = pGlyph->yOffset;
  bits = 0;
  bit = 0;
  yo_min = pFont->yOffsetMin;
  yo_max = pFont->yOffsetMax;

  switch(TextDrawMode)
  {
    case tdmNone:
      break;

    case tdmThisCharBar:
      Color_MSBFirst = SwapBytes(Color);
      TextBackgroundColor_MSBFirst = SwapBytes(TextBackgroundColor);
      pMemChar = (uint16_t *)malloc(w * h * 2);
      pMemCharPixel = pMemChar;

      for (yy = 0; yy < h; ++yy)
      {
        for (xx = 0; xx < w; ++xx)
        {
          if (!(bit++ & 7))
            bits = pBitmap[bo++];
          *pMemCharPixel = bits & 0x80 ? Color_MSBFirst : TextBackgroundColor_MSBFirst;
          ++pMemCharPixel;
          bits <<= 1;
        }
      }
      ILI9341_DrawPixels_MSBFirst(X + xo, Y + yo, w, h, pMemChar);
      free(pMemChar);
      break;

    case tdmAnyCharBar:
      Color_MSBFirst = SwapBytes(Color);
      TextBackgroundColor_MSBFirst = SwapBytes(TextBackgroundColor);
      CharWidth = pGlyph->xAdvance;
      CharHeight = yo_max - yo_min + 1;
      pMemChar = (uint16_t *)malloc(CharWidth * CharHeight * sizeof(uint16_t));

      // Slow?
      pMemCharPixel = pMemChar;
      for (uint16_t PixelIndex = 0; PixelIndex < CharWidth * CharHeight; ++PixelIndex)
        *pMemCharPixel++ = TextBackgroundColor_MSBFirst;

      for (yy = 0; yy < h; ++yy)
      {
        pMemCharPixel = &pMemChar[(- yo_min + yo + yy) * CharWidth + xo];

        for (xx = 0; xx < w; ++xx)
        {
          if (!(bit++ & 7))
            bits = pBitmap[bo++];
          if (bits & 0x80)
            *pMemCharPixel = Color_MSBFirst;
          ++pMemCharPixel;
          bits <<= 1;
        }
      }
      ILI9341_DrawPixels_MSBFirst(X, Y + yo_min, CharWidth, CharHeight, pMemChar);
      free(pMemChar);
      break;

    case tdmMergeWithExistingPixels:
      ILI9341_CSX_Low();
      for (yy = 0; yy < h; ++yy)
      {
        for (xx = 0; xx < w; ++xx)
        {
          if (!(bit++ & 7))
            bits = pBitmap[bo++];
          if (bits & 0x80)
            ILI9341_DrawPixel(X + xo + xx, Y + yo + yy, Color, 0);
          bits <<= 1;
        }
      }
      ILI9341_CSX_High();
      break;
  }

  if (w == 0)
    return pGlyph->xAdvance;
  return xo + w;
}

uint16_t ILI9341_SetTextColor(uint16_t Value)
{
  uint16_t Result;

  Result = TextColor;
  TextColor = Value;
  return Result;
}

uint16_t ILI9341_SetTextBackgroundColor(uint16_t Value)
{
  uint16_t Result;

  Result = TextBackgroundColor;
  TextBackgroundColor = Value;
  return Result;
}

TextDrawMode_t ILI9341_SetTextDrawMode(TextDrawMode_t Value)
{
  TextDrawMode_t Result;

  Result = TextDrawMode;
  TextDrawMode = Value;
  return Result;
}

TextVerticalOrigin_t ILI9341_SetTextVerticalOrigin(TextVerticalOrigin_t Value)
{
  TextVerticalOrigin_t Result;

  Result = TextVerticalOrigin;
  TextVerticalOrigin = Value;
  return Result;
}

void ILI9341_DrawTextAtXY(const char *Text, uint16_t X, uint16_t Y, TextPosition_t TextPosition)
{
  uint8_t *pText;
  uint8_t Ch;
  uint16_t NumChars;
  uint8_t DX;
  
  pText = (uint8_t *) Text;

  if (!pText)
    return;

  NumChars = strlen(Text);

  switch (TextPosition)
  {
    case tpCentre:
      X -= ILI9341_GetTextWidth(Text) / 2;
      break;
    case tpRight:
      X -= ILI9341_GetTextWidth(Text);
      break;
    default:
      break;
  }

  for (uint16_t CharIndex = 0; CharIndex < NumChars; ++CharIndex)
  {
    Ch = *pText;
    DX = ILI9341_DrawCharAtXY(Ch, X, Y, TextColor);
    ++pText;
    X += DX;
  }
}

void ILI9341_Test_DrawGrid()
{
  int Index, X, Y;

  for (Index = 0; Index < 10; ++Index)
  {
    Y = 35 * Index;
    ILI9341_DrawBar(0, Y, 234, 1, ILI9341_COLOR_WHITE);
  }

  for (Index = 0; Index < 10; ++Index)
  {
    X = 26 * Index;
    ILI9341_DrawBar(X, 0, 1, 315, ILI9341_COLOR_WHITE);
  }
}
