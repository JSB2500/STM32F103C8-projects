///////////////////////////////////////////////////////////////////////////////
// Copyright 2017 J S Bladen.
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// History:
// 13/12/2017: Removed Portrait flag and support. Now operates only in portrait mode.
// 13/12/2017: Added XPT2046_Swap_XL_and_XR and XPT2046_Swap_YD_and_YU to support touch panels with wiring errors.
///////////////////////////////////////////////////////////////////////////////

#include "main.h"
#include "JSB_XPT2046.h"

///////////////////////////////////////////////////////////////////////////////

const int XPT2046_Width = 240;
const int XPT2046_Height = 320;

///////////////////////////////////////////////////////////////////////////////
// Calibration:

// Defaults (from 2.8" touchscreen that PJB used for his uni amplifier project):
int XPT2046_RawX_Min = 260;
int XPT2046_RawX_Max = 3760;
int XPT2046_RawY_Min = 260;
int XPT2046_RawY_Max = 3660;

///////////////////////////////////////////////////////////////////////////////

#define ZThreshold 400

///////////////////////////////////////////////////////////////////////////////

static SPI_HandleTypeDef *phspi;

///////////////////////////////////////////////////////////////////////////////

#define XPT2046_CSX_Low() HAL_GPIO_WritePin(XPT2046_CSX_GPIO_Port, XPT2046_CSX_Pin, GPIO_PIN_RESET)
#define XPT2046_CSX_High() HAL_GPIO_WritePin(XPT2046_CSX_GPIO_Port, XPT2046_CSX_Pin, GPIO_PIN_SET)

///////////////////////////////////////////////////////////////////////////////

void XPT2046_Initialize(SPI_HandleTypeDef *i_phspi)
{
  phspi = i_phspi;
}

static int16_t GetBest(int16_t A, int16_t B, int16_t C)
// From Adafruit Arduino library.
// JSB: I think it returns the average of the two closest values.
// JSB: I renamed the identifiers.
{
  int16_t DeltaAB, DeltaCA, DeltaBC;
  int16_t Result = 0;

  if (A > B)
    DeltaAB = A - B;
  else
    DeltaAB = B - A;

  if (A > C)
    DeltaCA = A - C;
  else
    DeltaCA = C - A;

  if (C > B)
    DeltaBC = C - B;
  else
    DeltaBC = B - C;

  if ((DeltaAB <= DeltaCA) && (DeltaAB <= DeltaBC))
    Result = (A + B) >> 1;
  else if ((DeltaCA <= DeltaAB) && (DeltaCA <= DeltaBC))
    Result = (A + C) >> 1;
  else
    Result = (B + C) >> 1;

  return Result;
}

static const uint8_t SampleCommand[] =
{
  // Read: z1, z2, x, x, y, x, y, x, y, END
  0xB1, 0x00, 0xC1, 0x00, 0xD1, 0x00, 0xD1, 0x00, 0x91, 0x00, 0xD1, 0x00, 0x91, 0x00, 0xD1, 0x00, 0x90, 0x00, 0x00
};

inline static uint16_t GetUnsigned12bitValue(uint8_t *pData)
{
  return ((pData[0] << 5) | (pData[1] >> 3)) & 0x0FFF;
}

uint8_t XPT2046_Sample(int16_t *pRawX, int16_t *pRawY, int16_t *pRawZ)
// Returns 1 if touched.
// The origin is bottom left (XL, YD). This is the natural origin of the XPT2046.
// None of the touch screens I've encountered so far are correctly wired. Use compiler defines to reverse the coordinates as required.
{
  int16_t x, y, z;

  *pRawX = 0;
  *pRawY = 0;
  *pRawZ = 0;

  uint8_t RxData[19];
  int16_t X_Positions[3];
  int16_t Y_Positions[3];

  XPT2046_CSX_Low();
  if (HAL_SPI_TransmitReceive(phspi, (uint8_t*) &SampleCommand, (uint8_t*) &RxData, 19, 100) != HAL_OK)
    Error_Handler();
  XPT2046_CSX_High();

  int16_t z1 = GetUnsigned12bitValue(&RxData[1]);
  int16_t z2 = GetUnsigned12bitValue(&RxData[3]);

  // Result from dummy measurement ignored.

  X_Positions[0] = GetUnsigned12bitValue(&RxData[7]);
  X_Positions[1] = GetUnsigned12bitValue(&RxData[11]);
  X_Positions[2] = GetUnsigned12bitValue(&RxData[15]);

  Y_Positions[0] = GetUnsigned12bitValue(&RxData[10]);
  Y_Positions[1] = GetUnsigned12bitValue(&RxData[13]);
  Y_Positions[2] = GetUnsigned12bitValue(&RxData[17]);

  // Neutralize occasional z1 = 4095 values (which are possibly due to comms errors, as x and y values of 4095 often appear with the z value of 4095).
  if (z1 >= 2048)
    z1 = 0;

  z = 4095 + z1 - z2;

  if (z < ZThreshold)
    return 0;

  *pRawZ = z;

  x = GetBest(X_Positions[0], X_Positions[1], X_Positions[2]);
  y = GetBest(Y_Positions[0], Y_Positions[1], Y_Positions[2]);

#if XPT2046_Swap_XL_and_XR
  x = 4095 - x;
#endif

#if XPT2046_Swap_YD_and_YU
  y = 4095 - y;
#endif

  *pRawX = x;
  *pRawY = y;

  return 1;
}

void XPT2046_ConvertRawToScreen(int16_t RawX, int16_t RawY, int16_t *pX, int16_t *pY)
// Assumes portrait mode.
// The origin of the result is top-left.
{
  float K;

  K = ((float)(RawX - XPT2046_RawX_Min) / (float)(XPT2046_RawX_Max - XPT2046_RawX_Min));
  *pX = K * XPT2046_Width;

  K = ((float)(RawY - XPT2046_RawY_Min) / (float)(XPT2046_RawY_Max - XPT2046_RawY_Min));
  *pY = K * XPT2046_Height;
}
