///////////////////////////////////////////////////////////////////////////////
// Copyright 2017 J S Bladen.
///////////////////////////////////////////////////////////////////////////////

#ifndef __JSB_ILI9341_H
#define __JSB_ILI9341_H

///////////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
extern "C"
{
#endif

///////////////////////////////////////////////////////////////////////////////

#include <stdint.h>
#include "gfxfont.h"

// Interface to specific hardware:
#include "JSB_ILI9341_hal.h"

///////////////////////////////////////////////////////////////////////////////

#define ILI9341_Width 240
#define ILI9341_Height 320

///////////////////////////////////////////////////////////////////////////////

extern uint8_t UseILI9341_BackBuffer;

///////////////////////////////////////////////////////////////////////////////
// Colors:
// Format: (R5:G6:B5)

#define ILI9341_COLOR_BLACK       0x0000      /*   0,   0,   0 */
#define ILI9341_COLOR_NAVY        0x000F      /*   0,   0, 128 */
#define ILI9341_COLOR_DARKGREEN   0x03E0      /*   0, 128,   0 */
#define ILI9341_COLOR_DARKCYAN    0x03EF      /*   0, 128, 128 */
#define ILI9341_COLOR_MAROON      0x7800      /* 128,   0,   0 */
#define ILI9341_COLOR_PURPLE      0x780F      /* 128,   0, 128 */
#define ILI9341_COLOR_BRIGHTPURPLE 0xB817      /* 192,    0,  192 */ /* JSB */
#define ILI9341_COLOR_DARKPURPLE  0x3807      /* 64,    0,  64 */ /* JSB */
#define ILI9341_COLOR_OLIVE       0x7BE0      /* 128, 128,   0 */
#define ILI9341_COLOR_LIGHTGREY   0xC618      /* 192, 192, 192 */
#define ILI9341_COLOR_DARKGREY    0x7BEF      /* 128, 128, 128 */
#define ILI9341_COLOR_DARKERGREY  0x2104      /*  64,  64,  64 */
#define ILI9341_COLOR_BLUE        0x001F      /*   0,   0, 255 */
#define ILI9341_COLOR_GREEN       0x07E0      /*   0, 255,   0 */
#define ILI9341_COLOR_CYAN        0x07FF      /*   0, 255, 255 */
#define ILI9341_COLOR_RED         0xF800      /* 255,   0,   0 */
#define ILI9341_COLOR_MAGENTA     0xF81F      /* 255,   0, 255 */
#define ILI9341_COLOR_YELLOW      0xFFE0      /* 255, 255,   0 */
#define ILI9341_COLOR_WHITE       0xFFFF      /* 255, 255, 255 */
#define ILI9341_COLOR_ORANGE      0xFD20      /* 255, 165,   0 */
#define ILI9341_COLOR_GREENYELLOW 0xAFE5      /* 173, 255,  47 */
#define ILI9341_COLOR_PINK        0xF81F

///////////////////////////////////////////////////////////////////////////////

typedef enum
{
  tpNone,
  tpLeft,
  tpCentre,
  tpRight
} TextPosition_t;

typedef enum
{
  tdmNone,
  tdmThisCharBar,
  tdmAnyCharBar,
  tdmMergeWithExistingPixels
} TextDrawMode_t;

typedef enum
{
  tvoNone,
  tvoBase, // Line on which chars sit. Some parts of some chars may go below this.
  tvoCentreBetweenBaseAndTop,
} TextVerticalOrigin_t;

// Administration:
void ILI9341_Initialize(SPI_HandleTypeDef *i_phspi, int8_t i_UseBackBuffer);
void ILI9341_SetDefaultState();
uint8_t ILI9341_UsingBackBuffer();
void ILI9341_CopyBackBufferToDisplay();

// Utilities:
uint16_t ILI9341_SwapBytes(uint16_t Value);

// Primitives:
void ILI9341_DrawPixel(int16_t X, int16_t Y, uint16_t Color, int8_t UseCS);
void ILI9341_DrawPixels_MSBFirst(uint16_t X, uint16_t Y, uint16_t Width, uint16_t Height, uint16_t *pPixels);
void ILI9341_DrawBar(uint16_t X, uint16_t Y, uint16_t Width, uint16_t Height, uint16_t Color);
void ILI9341_Clear(uint16_t Color);
void ILI9341_DrawRectangle(uint16_t Left, uint16_t Top, uint16_t Width, uint16_t Height, uint16_t Color);

// Text:
uint16_t ILI9341_SetTextColor(uint16_t Value);
uint16_t ILI9341_SetTextBackgroundColor(uint16_t Value);
TextDrawMode_t ILI9341_SetTextDrawMode(TextDrawMode_t Value);
TextVerticalOrigin_t ILI9341_SetTextVerticalOrigin(TextVerticalOrigin_t Value);
const GFXfont *ILI9341_SetFont(const GFXfont *i_pFont);
uint8_t ILI9341_GetFontYSpacing();
uint16_t ILI9341_GetCharWidth(uint8_t Ch);
uint16_t ILI9341_GetTextWidth(const char *Text);
int8_t ILI9341_GetTextHeightAboveBaseLine();
uint8_t ILI9341_DrawCharAtXY(uint8_t Ch, uint16_t X, uint16_t Y, uint16_t Color);
void ILI9341_DrawTextAtXY(const char *Text, uint16_t X, uint16_t Y, TextPosition_t TextPosition);

///////////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
}
#endif

///////////////////////////////////////////////////////////////////////////////

#endif

///////////////////////////////////////////////////////////////////////////////
