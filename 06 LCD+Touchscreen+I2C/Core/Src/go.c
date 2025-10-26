#include <math.h>
#include <stdlib.h>

#include "adc.h"
#include "tim.h"
#include "main.h"
#include "stm32f1xx_hal.h"
#include "JSB_ILI9341.h"
#include "JSB_XPT2046.h"
#include "JSB_MCP444X.h"
#include "gfxfont.h"
#include "FreeSans9pt7b.h"
#include "FreeSans12pt7b.h"
#include "go.h"

typedef enum
{
  PotentiometerDevice_Left = 0,
  PotentiometerDevice_Right = 1
} PotentiometerDevice_t;

#define NumSamples 128 // Default: 128 /* Consider slowing the sampling frequency to enable this to be reduced without missing low frequencies */
#define UseWindow 0

#define MinVolume 0
#define MaxVolume 128
#define MinTone (-64)
#define MaxTone (+64)

#define StepSize 4

#define DefaultVolume 100
#define DefaultBass 0
#define DefaultTreble 0

#define Bar_X 60
#define Bar_MaxLength 180
#define Bar_Height 28
#define Bar_Min_dB -60

#define Channel_YSeparation 4

#define Button_Down_Left 0
#define Button_Up_Left 140
#define Button_Volume_Top 200
#define Button_Bass_Top 240
#define Button_Treble_Top 280
#define Button_VolumeAndTone_Width 100
#define Button_VolumeAndTone_Height 35

#define Button_Settings_Left 175
#define Button_Settings_Width 65
#define Button_Settings_Height 35
#define Button_Settings_Default_Top 80
#define Button_Settings_Save_Top 120

static uint8_t Volume = 0;
static int8_t Treble = 0, Bass = 0;
static uint8_t PotentiometerDevice_IsPresent_Left = 0, PotentiometerDevice_IsPresent_Right = 0;
static uint8_t Volume_Left_Read = 0, Volume_Right_Read = 0;

int16_t min(int16_t A, int16_t B)
{
  return A < B ? A : B;
}

int16_t max(int16_t A, int16_t B)
{
  return A > B ? A : B;
}

int16_t SignExtend9Bitsto16Bits(int16_t Value)
{
  if (Value & 0x100)
    Value |= 0xFF00;

  return Value;
}

void DrawChannel(char *ChannelName, uint16_t Y, float Level_Linear)
{
  char S[128];
  uint16_t BarLength, BarEnd, TextY;
  float Level_dB;

  if (Level_Linear > 0)
    Level_dB = 20.0f * log10(Level_Linear);
  else
    Level_dB = -200.0f;

  const GFXfont *pFont = ILI9341_SetFont(&FreeSans12pt7b);

  TextY = Y + ILI9341_GetFontYSpacing() - 8;

  sprintf(S, "%s:", ChannelName);
  ILI9341_DrawTextAtXY(S, 0, TextY, tpLeft);

  BarLength = round((1.0f - (Level_dB / Bar_Min_dB)) * Bar_MaxLength);
  BarEnd = Bar_X + BarLength;
  ILI9341_DrawBar(Bar_X, Y, BarLength, Bar_Height, ILI9341_COLOR_DARKGREEN);
  ILI9341_DrawBar(BarEnd, Y, Bar_MaxLength - BarLength, Bar_Height, ILI9341_COLOR_DARKERGREY);

  ILI9341_SetFont(&FreeSans9pt7b);
  sprintf(S, "%0.1f dB", Level_dB);
  TextDrawMode_t TextDrawMode = ILI9341_SetTextDrawMode(tdmMergeWithExistingPixels);
  ILI9341_DrawTextAtXY(S, (Bar_X + Bar_MaxLength) / 2, TextY, tpCentre);
  ILI9341_SetTextDrawMode(TextDrawMode);

  ILI9341_SetFont(pFont);
}

void DrawButton(uint16_t Left, uint16_t Top, uint16_t Width, uint16_t Height, uint16_t Color, char *pText)
{
  const GFXfont *pFont = ILI9341_SetFont(&FreeSans9pt7b);
  ILI9341_DrawBar(Left, Top, Width, Height, Color);
  uint16_t TextBackgroundColor = ILI9341_SetTextBackgroundColor(Color);
  ILI9341_DrawTextAtXY(pText, Left + Width / 2, Top + 24, tpCentre);
  ILI9341_SetTextBackgroundColor(TextBackgroundColor);
  ILI9341_SetFont(pFont);
}

uint8_t IsPointInButton(uint16_t X, uint16_t Y, uint16_t Left, uint16_t Top, uint16_t Width, uint16_t Height)
{
  if ((X < Left) || (X >= Left + Width))
    return 0;
  if ((Y < Top) || (Y >= Top + Height))
    return 0;

  return 1;
}

float sqr(float x)
{
  return x * x;
}

typedef struct
{
  uint16_t Left;
  uint16_t Right;
} ADCSample_t;

ADCSample_t ADCSamples[NumSamples];
float Window[NumSamples];

uint8_t ADCDone = 0;
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
  ADCDone = 1;
}

void StartSampling()
{
  ADCDone = 0;

  // HAL_ADC_Start_DMA(&hadc1, (uint32_t *) ADCSamples, NumSamples);
  HAL_ADC_Start(&hadc2); // I don't know why this is needed.
  HAL_ADCEx_MultiModeStart_DMA(&hadc1, (uint32_t *) ADCSamples, NumSamples);
}

void Window_Init()
{
  for (int16_t SampleIndex = 0; SampleIndex < NumSamples; ++SampleIndex)
    Window[SampleIndex] = 0.5f * (1.0f - cos(2.0f * M_PI * ((float) SampleIndex / (NumSamples - 1))));
}

typedef enum
{
  chNone,
  chLeft,
  chRight
} Channel_t;

float CalculateLevel(Channel_t Channel)
{
  float Sum, SampleValue, Mean, SumOfSquares, SampleValueSquared;
  ADCSample_t *pADCSample;
  float ADCSample;

  Sum = 0;
  for (uint16_t SampleIndex = 0; SampleIndex < NumSamples; ++SampleIndex)
  {
    pADCSample = &ADCSamples[SampleIndex];

    switch (Channel)
    {
      case chLeft:
        ADCSample = pADCSample->Left;
        break;
      case chRight:
        ADCSample = pADCSample->Right;
        break;
      default:
        ADCSample = 0.0f; // Actually an error condition.
    }

    SampleValue = (ADCSample - 2048.0f) / 2048.0f;
    Sum += SampleValue;
  }
  Mean = Sum / NumSamples;

  SumOfSquares = 0;
  Sum = 0;
  for (uint16_t SampleIndex = 0; SampleIndex < NumSamples; ++SampleIndex)
  {
    pADCSample = &ADCSamples[SampleIndex];

    switch (Channel)
    {
      case chLeft:
        ADCSample = pADCSample->Left;
        break;
      case chRight:
        ADCSample = pADCSample->Right;
        break;
      default:
        ADCSample = 0.0f; // Actually an error condition.
    }

    SampleValue = (ADCSample - 2048.0f) / 2048.0f;

    SampleValue -= Mean;

    if (UseWindow)
      SampleValue *= Window[SampleIndex];

    Sum += SampleValue;
    SampleValueSquared = SampleValue * SampleValue;
    SumOfSquares += SampleValueSquared;
  }

// Can't use this method when using a Window:   Level_Left = sqrt((SumOfSquares / NumSamples) - sqr(Sum / NumSamples)); // Could combine sqrt with dB log but that would restrict analysis methods.
  return sqrt(SumOfSquares / NumSamples);
}

void ClearScreenAndDrawButtons()
{
  ILI9341_Clear(0x0000);

  DrawButton(Button_Settings_Left, Button_Settings_Default_Top, Button_Settings_Width, Button_Settings_Height, ILI9341_COLOR_PURPLE, "Default");
  DrawButton(Button_Settings_Left, Button_Settings_Save_Top, Button_Settings_Width, Button_Settings_Height, ILI9341_COLOR_PURPLE, "Save");

  DrawButton(Button_Down_Left, Button_Volume_Top, Button_VolumeAndTone_Width, Button_VolumeAndTone_Height, ILI9341_COLOR_MAROON, "Vol-");
  DrawButton(Button_Up_Left, Button_Volume_Top, Button_VolumeAndTone_Width, Button_VolumeAndTone_Height, ILI9341_COLOR_MAROON, "Vol+");

  DrawButton(Button_Down_Left, Button_Bass_Top, Button_VolumeAndTone_Width, Button_VolumeAndTone_Height, ILI9341_COLOR_MAROON, "Bass-");
  DrawButton(Button_Up_Left, Button_Bass_Top, Button_VolumeAndTone_Width, Button_VolumeAndTone_Height, ILI9341_COLOR_MAROON, "Bass+");

  DrawButton(Button_Down_Left, Button_Treble_Top, Button_VolumeAndTone_Width, Button_VolumeAndTone_Height, ILI9341_COLOR_MAROON, "Treble-");
  DrawButton(Button_Up_Left, Button_Treble_Top, Button_VolumeAndTone_Width, Button_VolumeAndTone_Height, ILI9341_COLOR_MAROON, "Treble+");
}

///////////////////////////////////////////////////////////////////////////////
// Settings:

#define SettingsDevice PotentiometerDevice_Left /* The settings are stored in the EEPROM of the left channel potentiometer chip */
#define SettingsDevice_IsPresent PotentiometerDevice_IsPresent_Left

// EEPROM address map:
#define SettingsDeviceAddress_StartupVolume 0x00
#define SettingsDeviceAddress_StartupBass 0x01
#define SettingsDeviceAddress_StartupTreble 0x02

void Settings_SetDefault()
{
  Volume = DefaultVolume;
  Bass = DefaultBass;
  Treble = DefaultTreble;
}

void Settings_SetStartupVolume(uint8_t Value)
{
  if (SettingsDevice_IsPresent)
    MCP444X_WriteNonvolatileMemoryValue(SettingsDevice, SettingsDeviceAddress_StartupVolume, Value);
}

uint8_t Settings_GetStartupVolume()
{
  if (SettingsDevice_IsPresent)
    return MCP444X_ReadNonvolatileMemoryValue(SettingsDevice, SettingsDeviceAddress_StartupVolume);

  return DefaultVolume;
}

void Settings_SetStartupBass(int8_t Value)
// -64 to 64
{
  if (SettingsDevice_IsPresent)
    MCP444X_WriteNonvolatileMemoryValue(SettingsDevice, SettingsDeviceAddress_StartupBass, Value);
}

int8_t Settings_GetStartupBass()
// -64 to 64
{
  if (SettingsDevice_IsPresent)
    return SignExtend9Bitsto16Bits(MCP444X_ReadNonvolatileMemoryValue(SettingsDevice, SettingsDeviceAddress_StartupBass));

  return DefaultBass;
}

void Settings_SetStartupTreble(int8_t Value)
// -64 to 64
{
  if (SettingsDevice_IsPresent)
    MCP444X_WriteNonvolatileMemoryValue(SettingsDevice, SettingsDeviceAddress_StartupTreble, Value);
}

int8_t Settings_GetStartupTreble()
// -64 to 64
{
  if (SettingsDevice_IsPresent)
    return SignExtend9Bitsto16Bits(MCP444X_ReadNonvolatileMemoryValue(SettingsDevice, SettingsDeviceAddress_StartupTreble));

  return DefaultTreble;
}

void SetVolume_Left(uint8_t Value)
{
  if (PotentiometerDevice_IsPresent_Left)
  {
    MCP444X_SetPotentiometerPosition_Volatile(PotentiometerDevice_Left, Potentiometer_2, Value);
    Volume_Left_Read = MCP444X_GetPotentiometerPosition_Volatile(PotentiometerDevice_Left, Potentiometer_2); // For ensuring chip is working.
  }
}

void SetVolume_Right(uint8_t Value)
{
  if (PotentiometerDevice_IsPresent_Right)
  {
    MCP444X_SetPotentiometerPosition_Volatile(PotentiometerDevice_Right, Potentiometer_2, Value);
    Volume_Right_Read = MCP444X_GetPotentiometerPosition_Volatile(PotentiometerDevice_Right, Potentiometer_2); // For ensuring chip is working.
  }
}

void SetBass(uint8_t Value)
{
  Value += 64;

  if (PotentiometerDevice_IsPresent_Left)
    MCP444X_SetPotentiometerPosition_Volatile(PotentiometerDevice_Left, Potentiometer_0, Value);

  if (PotentiometerDevice_IsPresent_Right)
    MCP444X_SetPotentiometerPosition_Volatile(PotentiometerDevice_Right, Potentiometer_0, Value);
}

void SetTreble(uint8_t Value)
{
  Value += 64;

  if (PotentiometerDevice_IsPresent_Left)
    MCP444X_SetPotentiometerPosition_Volatile(PotentiometerDevice_Left, Potentiometer_1, Value);

  if (PotentiometerDevice_IsPresent_Right)
    MCP444X_SetPotentiometerPosition_Volatile(PotentiometerDevice_Right, Potentiometer_1, Value);
}

void SetVolumeAndTone()
{
  SetVolume_Left(Volume);
  SetVolume_Right(Volume);
  SetBass(Bass);
  SetTreble(Treble);
}

void Settings_Save()
{
  Settings_SetStartupBass(Bass);
  Settings_SetStartupTreble(Treble);
  Settings_SetStartupVolume(Volume);

  // Also set potentiometer positions so that the level is correct on startup.
  if (PotentiometerDevice_IsPresent_Left)
  {
    MCP444X_SetPotentiometerPosition_NonVolatile(PotentiometerDevice_Left, Potentiometer_0, Bass);
    MCP444X_SetPotentiometerPosition_NonVolatile(PotentiometerDevice_Left, Potentiometer_1, Treble);
    MCP444X_SetPotentiometerPosition_NonVolatile(PotentiometerDevice_Left, Potentiometer_2, Volume);
  }
  //
  if (PotentiometerDevice_IsPresent_Right)
  {
    MCP444X_SetPotentiometerPosition_NonVolatile(PotentiometerDevice_Right, Potentiometer_0, Bass);
    MCP444X_SetPotentiometerPosition_NonVolatile(PotentiometerDevice_Right, Potentiometer_1, Treble);
    MCP444X_SetPotentiometerPosition_NonVolatile(PotentiometerDevice_Right, Potentiometer_2, Volume);
  }
}

///////////////////////////////////////////////////////////////////////////////

#define TextYInc 20

void Go()
{
  float Level_Left, Level_Right;
  int16_t Touch_RawX, Touch_RawY, Touch_RawZ;
  int16_t Touch_X, Touch_Y;
  int16_t TextY;
  uint16_t Channel_YInc;
  uint16_t TextWidth;
  char S[128];

  PotentiometerDevice_IsPresent_Left = MCP444X_IsDevicePresent(PotentiometerDevice_Left);
  PotentiometerDevice_IsPresent_Right = MCP444X_IsDevicePresent(PotentiometerDevice_Right);

  Window_Init();

  ILI9341_SetTextColor(ILI9341_COLOR_WHITE);
  ILI9341_SetTextBackgroundColor(ILI9341_COLOR_BLACK);
  ILI9341_SetTextDrawMode(tdmAnyCharBar); // Slower but enables flicker free update.

  ILI9341_SetFont(&FreeSans12pt7b);
  Channel_YInc = ILI9341_GetFontYSpacing() + Channel_YSeparation;

  HAL_ADCEx_Calibration_Start(&hadc1);
  HAL_ADCEx_Calibration_Start(&hadc2);

  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2); // !!! Temp - For frequency source for testing.

  // **************************************************************************

  ILI9341_Clear(0x0000);

  ILI9341_SetFont(&FreeSans9pt7b);

  TextY = 20;

  ILI9341_DrawTextAtXY("*** JSB ***", 120, TextY, tpCentre);
  TextY += TextYInc;

  sprintf(S, "Left channel present: %s", PotentiometerDevice_IsPresent_Left? "Yes" : "No");
  ILI9341_DrawTextAtXY(S, 0, TextY, tpLeft);
  TextY += TextYInc;
  //
  sprintf(S, "Right channel present: %s", PotentiometerDevice_IsPresent_Right? "Yes" : "No");
  ILI9341_DrawTextAtXY(S, 0, TextY, tpLeft);
  TextY += TextYInc;

  TextY += TextYInc;

  // Settings_SetStartupVolume(100); // To set.
  sprintf(S, "Getting volume ... ");
  ILI9341_DrawTextAtXY(S, 0, TextY, tpLeft);
  Volume = Settings_GetStartupVolume();
  TextWidth = ILI9341_GetTextWidth(S);
  ILI9341_DrawTextAtXY("Done", TextWidth, TextY, tpLeft);
  TextY += TextYInc;
  sprintf(S, "Volume: %d", Volume);
  ILI9341_DrawTextAtXY(S, 0, TextY, tpLeft);
  TextY += TextYInc;

  // Settings_SetStartupTreble(0); // To set.
  sprintf(S, "Getting bass ... ");
  ILI9341_DrawTextAtXY(S, 0, TextY, tpLeft);
  Bass = Settings_GetStartupBass();
  TextWidth = ILI9341_GetTextWidth(S);
  ILI9341_DrawTextAtXY("Done", TextWidth, TextY, tpLeft);
  TextY += TextYInc;
  sprintf(S, "Bass: %d", Bass);
  ILI9341_DrawTextAtXY(S, 0, TextY, tpLeft);
  TextY += TextYInc;

  // Settings_SetStartupTreble(0); // To set.
  sprintf(S, "Getting treble ... ");
  ILI9341_DrawTextAtXY(S, 0, TextY, tpLeft);
  Treble = Settings_GetStartupTreble();
  TextWidth = ILI9341_GetTextWidth(S);
  ILI9341_DrawTextAtXY("Done", TextWidth, TextY, tpLeft);
  TextY += TextYInc;
  sprintf(S, "treble: %d", Treble);
  ILI9341_DrawTextAtXY(S, 0, TextY, tpLeft);
  TextY += TextYInc;

  SetVolumeAndTone();

  TextY += TextYInc;

  sprintf(S, "Num samples: %d", NumSamples);
  ILI9341_DrawTextAtXY(S, 0, TextY, tpLeft);
  TextY += TextYInc;
  //
  sprintf(S, "Using window: %s", UseWindow ? "Yes" : "No");
  ILI9341_DrawTextAtXY(S, 0, TextY, tpLeft);

  HAL_Delay(5000);

  // **************************************************************************

  ClearScreenAndDrawButtons();

  StartSampling();

  while (1)
  {
    do
    {
    } while (!ADCDone);

    Level_Left = CalculateLevel(chLeft);
    Level_Right = CalculateLevel(chRight);

    StartSampling();

    uint8_t Touched = XPT2046_Sample(&Touch_RawX, &Touch_RawY, &Touch_RawZ);

    XPT2046_ConvertRawToScreen(Touch_RawX, Touch_RawY, &Touch_X, &Touch_Y);

    DrawChannel("Left", 0, Level_Left);
    DrawChannel("Right", Channel_YInc, Level_Right);

    ILI9341_SetFont(&FreeSans9pt7b);
    //
    TextY = 100;
    //
    sprintf(S, "Volume: %d (%d, %d)         ", Volume, Volume_Left_Read, Volume_Right_Read);
    ILI9341_DrawTextAtXY(S, 0, TextY, tpLeft);
    TextY += TextYInc;
    //
    sprintf(S, "Bass: %d            ", Bass);
    ILI9341_DrawTextAtXY(S, 0, TextY, tpLeft);
    TextY += TextYInc;
    //
    sprintf(S, "Treble: %d            ", Treble);
    ILI9341_DrawTextAtXY(S, 0, TextY, tpLeft);
    TextY += TextYInc;
    //
    TextY += TextYInc;

// #define ShowTouchInfo

#ifdef ShowTouchInfo
    sprintf(S, "Touched: %s   ", Touched ? "Yes" : "No");
    ILI9341_DrawTextAtXY(S, 0, TextY, tpLeft);
    TextY += TextYInc;
    //
    sprintf(S, "XYZ: %d %d %d                               ", Touch_RawX, Touch_RawY, Touch_RawZ);
    ILI9341_DrawTextAtXY(S, 0, TextY, tpLeft);
    TextY += TextYInc;
#endif

    if (Touched)
    {
      if (IsPointInButton(Touch_X, Touch_Y, Button_Settings_Left, Button_Settings_Default_Top, Button_Settings_Width, Button_Settings_Height))
        Settings_SetDefault();
      if (IsPointInButton(Touch_X, Touch_Y, Button_Settings_Left, Button_Settings_Save_Top, Button_Settings_Width, Button_Settings_Height))
        Settings_Save();

      if (IsPointInButton(Touch_X, Touch_Y, Button_Down_Left, Button_Volume_Top, Button_VolumeAndTone_Width, Button_VolumeAndTone_Height))
        Volume = max((uint16_t)Volume - StepSize, MinVolume);
      if (IsPointInButton(Touch_X, Touch_Y, Button_Up_Left, Button_Volume_Top, Button_VolumeAndTone_Width, Button_VolumeAndTone_Height))
        Volume = min((uint16_t)Volume + StepSize, MaxVolume);

      if (IsPointInButton(Touch_X, Touch_Y, Button_Down_Left, Button_Bass_Top, Button_VolumeAndTone_Width, Button_VolumeAndTone_Height))
        Bass = max((uint16_t)Bass - StepSize, MinTone);
      if (IsPointInButton(Touch_X, Touch_Y, Button_Up_Left, Button_Bass_Top, Button_VolumeAndTone_Width, Button_VolumeAndTone_Height))
        Bass = min((uint16_t)Bass + StepSize, MaxTone);

      if (IsPointInButton(Touch_X, Touch_Y, Button_Down_Left, Button_Treble_Top, Button_VolumeAndTone_Width, Button_VolumeAndTone_Height))
        Treble = max((uint16_t)Treble - StepSize, MinTone);
      if (IsPointInButton(Touch_X, Touch_Y, Button_Up_Left, Button_Treble_Top, Button_VolumeAndTone_Width, Button_VolumeAndTone_Height))
        Treble = min((uint16_t)Treble + StepSize, MaxTone);
    }

    SetVolumeAndTone();
  }
}
