#ifndef __JSB_GENERAL_H
#define __JSB_GENERAL_H

#include "stdint.h"

///////////////////////////////////////////////////////////////////////////////
// Uncategorized:

uint16_t SwapBytes(uint16_t Value);

///////////////////////////////////////////////////////////////////////////////
// Memory allocation:

void IfNotNullFreeAndNull(void **ppMemory);

///////////////////////////////////////////////////////////////////////////////
// Min/Max:

uint32_t Min_uint32(uint32_t A, uint32_t B);
int32_t Min_int32(int32_t A, int32_t B);
uint32_t Max_uint32(uint32_t A, uint32_t B);
int32_t Max_int32(int32_t A, int32_t B);
//
float Min_float(float A, float B);
float Max_float(float A, float B);

///////////////////////////////////////////////////////////////////////////////
// Clamp:

uint16_t Clamp_uint16(uint16_t Value, uint16_t MinValue , uint16_t MaxValue);
uint32_t Clamp_uint32(uint32_t Value, uint32_t Min, uint32_t Max);
int32_t Clamp_int32(int32_t Value, int32_t Min, int32_t Max);
float Clamp_float(float Value, float Min, float Max);

///////////////////////////////////////////////////////////////////////////////
// Wrap:

int Wrap_int(int Value, int Size);

///////////////////////////////////////////////////////////////////////////////
// Maths:

double Sqr(double Value);
double Cube(double Value);
double LinearInterpolate(double Value0, double Value1, double InterpolationParameter);
double CubicInterpolate(double Value0, double Value1, double Value2, double Value3, double InterpolationParameter);

///////////////////////////////////////////////////////////////////////////////
// Strings:

void TrimRight(char *pString, char UnwantedChar);

///////////////////////////////////////////////////////////////////////////////
// Conversion to strings:

const char *BooleanToOffOn(uint8_t Value);
const char *BooleanToNoYes(uint8_t Value);

///////////////////////////////////////////////////////////////////////////////
// GUIDs:

typedef struct
{
  uint32_t Field0;
  uint16_t Field1;
  uint16_t Field2;
  uint8_t Field3[8];
} GUID_t;

uint8_t StringToGUID(char *Value, GUID_t *o_pGUID);
void GUIDToString(GUID_t *i_Value, char *o_Value);
uint8_t GUID_IsEqual(GUID_t *pA, GUID_t *pB);

///////////////////////////////////////////////////////////////////////////////
// Audio:

typedef struct
{
  int16_t Left, Right;
} AudioSample16_t;

typedef struct
{
  int32_t Left, Right;
} AudioSample24_t;

typedef enum
{
  abhNone,
  abhFirstHalf,
  abhSecondHalf
} AudioBufferHalf_t;

///////////////////////////////////////////////////////////////////////////////

#endif
