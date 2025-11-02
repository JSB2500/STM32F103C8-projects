#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "JSB_General.h"

///////////////////////////////////////////////////////////////////////////////
// Uncategorized:

uint16_t SwapBytes(uint16_t Value)
{
  return (Value >> 8) | ((Value & 0xFF) << 8);
}

///////////////////////////////////////////////////////////////////////////////
// Memory allocation:

void IfNotNullFreeAndNull(void **ppMemory)
{
  if (*ppMemory)
  {
    free(*ppMemory);
    *ppMemory = NULL;
  }
}

///////////////////////////////////////////////////////////////////////////////
// Min/Max:

uint32_t Min_uint32(uint32_t A, uint32_t B)
{
    return A < B ? A : B;
}

int32_t Min_int32(int32_t A, int32_t B)
{
    return A < B ? A : B;
}

uint32_t Max_uint32(uint32_t A, uint32_t B)
{
    return A > B ? A : B;
}

int32_t Max_int32(int32_t A, int32_t B)
{
    return A > B ? A : B;
}

float Min_float(float A, float B)
{
  return A < B ? A : B;
}

float Max_float(float A, float B)
{
  return A > B ? A : B;
}

///////////////////////////////////////////////////////////////////////////////
// Clamp:

uint16_t Clamp_uint16(uint16_t Value, uint16_t MinValue , uint16_t MaxValue)
{
  if (Value < MinValue)
    Value = MinValue;
  if (Value > MaxValue)
    Value = MaxValue;
  return Value;
}

uint32_t Clamp_uint32(uint32_t Value, uint32_t MinValue, uint32_t MaxValue)
{
  if (Value < MinValue)
    Value = MinValue;
  else if (Value > MaxValue)
    Value = MaxValue;

  return Value;
}

int32_t Clamp_int32(int32_t Value, int32_t MinValue, int32_t MaxValue)
{
  if (Value < MinValue)
    Value = MinValue;
  else if (Value > MaxValue)
    Value = MaxValue;

  return Value;
}

float Clamp_float(float Value, float MinValue, float MaxValue)
{
  if (Value < MinValue)
    Value = MinValue;
  else if (Value > MaxValue)
    Value = MaxValue;

  return Value;
}

///////////////////////////////////////////////////////////////////////////////
// Wrap:

int Wrap_int(int Value, int Size)
{
  if (Value < 0)
    Value += Size;
  else if (Value >= Size)
    Value -= Size;

  return Value;
}

///////////////////////////////////////////////////////////////////////////////
// Maths:

double Sqr(double Value)
{
  return Value * Value;
}

double Cube(double Value)
{
  return Value * Value * Value;
}

double LinearInterpolate(double Value0, double Value1, double InterpolationParameter)
{
  return (1.0 - InterpolationParameter) * Value0 + InterpolationParameter * Value1;
}

double CubicInterpolate(double Value0, double Value1, double Value2, double Value3, double InterpolationParameter)
{
  double C0, C1, C2, C3;
  double InterpolationCoefficient;

  C3 = (((Value3 - Value0) - 3.0 * (Value1 - Value0)) - 3.0 * ((Value2 - Value0) - 2.0 * (Value1-Value0))) / 6.0;
  C2 = ((Value2 - Value0) - 2.0 * (Value1 - Value0) - 6.0 * C3) / 2.0;
  C1 = Value1 - Value0 - C3 - C2;
  C0 = Value0;

  InterpolationCoefficient = 1 + InterpolationParameter; // Interpolate the second of the three segments.

  return C3 * Cube(InterpolationCoefficient) + C2 * Sqr(InterpolationCoefficient) + C1 * InterpolationCoefficient + C0;
}

///////////////////////////////////////////////////////////////////////////////
// Strings:

void TrimRight(char *pString, char UnwantedChar)
{
  char *pChar = pString + strlen(pString); // Point to terminator.

  while(1)
  {
    if (pChar == pString)
      return;

    --pChar;

    if (*pChar != UnwantedChar)
      return;

    *pChar = 0;
  }
}

///////////////////////////////////////////////////////////////////////////////
// Conversion to strings:

static const char *String_Off = "Off";
static const char *String_On = "On";
static const char *String_No = "No";
static const char *String_Yes = "Yes";

const char *BooleanToOffOn(uint8_t Value)
{
  return Value ? String_On : String_Off;
}

const char *BooleanToNoYes(uint8_t Value)
{
  return Value ? String_Yes : String_No;
}

///////////////////////////////////////////////////////////////////////////////
// GUIDs:

uint8_t StringToGUID(char *Value, GUID_t *o_pGUID)
{
  GUID_t Result;
  unsigned int X[11];
  int NumFields, NumChars;

  // STM32 sscanf bug (20181012): %08X (rather than %08x) assumes result is signed. Hence 0x9000000 => 0x7FFFFFF!!!
  NumFields = sscanf(Value, "{%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x}%n",
      &X[0], &X[1], &X[2], &X[3], &X[4], &X[5], &X[6], &X[7], &X[8], &X[9], &X[10], &NumChars);

  if ((NumFields != 11) || (NumChars != 38))
    return 0;

  Result.Field0 = X[0];
  Result.Field1 = X[1];
  Result.Field2 = X[2];
  Result.Field3[0] = X[3];
  Result.Field3[1] = X[4];
  Result.Field3[2] = X[5];
  Result.Field3[3] = X[6];
  Result.Field3[4] = X[7];
  Result.Field3[5] = X[8];
  Result.Field3[6] = X[9];
  Result.Field3[7] = X[10];

  *o_pGUID = Result;

  return 1;
}

void GUIDToString(GUID_t *i_Value, char *o_Value)
{
  sprintf(o_Value, "{%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}",
    (unsigned int)i_Value->Field0,
    i_Value->Field1,
    i_Value->Field2,
    i_Value->Field3[0], i_Value->Field3[1], i_Value->Field3[2], i_Value->Field3[3], i_Value->Field3[4], i_Value->Field3[5], i_Value->Field3[6], i_Value->Field3[7]);
}

uint8_t GUID_IsEqual(GUID_t *pA, GUID_t *pB)
{
  return memcmp(pA, pB, sizeof(GUID_t)) == 0 ? 1 : 0;
}

///////////////////////////////////////////////////////////////////////////////
