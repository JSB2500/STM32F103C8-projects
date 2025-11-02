///////////////////////////////////////////////////////////////////////////////
// Copyright 2017 J S Bladen.
///////////////////////////////////////////////////////////////////////////////

#ifndef __JSB_XPT2046_H
#define __JSB_XPT2046_H

#include "JSB_XPT2046_hal.h"

extern const int XPT2046_Width;
extern const int XPT2046_Height;

extern int XPT2046_RawX_Min;
extern int XPT2046_RawX_Max;
extern int XPT2046_RawY_Min;
extern int XPT2046_RawY_Max;

void XPT2046_Initialize(SPI_HandleTypeDef *i_phspi);
uint8_t XPT2046_Sample(int16_t *pRawX, int16_t *pRawY, int16_t *pRawZ);
void XPT2046_ConvertRawToScreen(int16_t RawX, int16_t RawY, int16_t *pX, int16_t *pY);

#endif
