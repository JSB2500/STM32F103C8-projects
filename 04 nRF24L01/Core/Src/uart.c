#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "uart.h"

UART_HandleTypeDef UART_Handle;

void UART_SendChar(char ch)
{
  HAL_UART_Transmit(&UART_Handle, (uint8_t *) &ch, 1, 0xFFFF);
}

void UART_SendStr(char *p)
{
  HAL_UART_Transmit(&UART_Handle, (uint8_t *) p, strlen(p), 0xFFFF);
}

void UART_printf(const char * format, ...)
{
  #define MaxNumChars 256

  char buffer[MaxNumChars];

  va_list args;
  va_start(args, format);
  vsnprintf(buffer, MaxNumChars, format, args);
  UART_SendStr(buffer);
  va_end(args);
}

void UART_SendInt(int32_t num)
{
  char str[10]; // 10 chars max for INT32_MAX
  int i = 0;
  if (num < 0)
  {
    UART_SendChar('-');
    num *= -1;
  }
  do
    str[i++] = num % 10 + '0';
  while ((num /= 10) > 0);
  for (i--; i >= 0; i--)
    UART_SendChar(str[i]);
}

void UART_SendInt0(int32_t num)
{
  char str[10]; // 10 chars max for INT32_MAX
  int i = 0;
  if (num < 0)
  {
    UART_SendChar('-');
    num *= -1;
  }
  if ((num < 10) && (num >= 0))
    UART_SendChar('0');
  do
    str[i++] = num % 10 + '0';
  while ((num /= 10) > 0);
  for (i--; i >= 0; i--)
    UART_SendChar(str[i]);
}

#define HEX_CHARS "0123456789ABCDEF"

void UART_SendHex8(uint16_t num)
{
  UART_SendChar(HEX_CHARS[(num >> 4) % 0x10]);
  UART_SendChar(HEX_CHARS[(num & 0x0f) % 0x10]);
}

void UART_SendHex16(uint16_t num)
{
  uint8_t i;
  for (i = 12; i > 0; i -= 4)
    UART_SendChar(HEX_CHARS[(num >> i) % 0x10]);
  UART_SendChar(HEX_CHARS[(num & 0x0f) % 0x10]);
}

void UART_SendHex32(uint32_t num)
{
  uint8_t i;
  for (i = 28; i > 0; i -= 4)
    UART_SendChar(HEX_CHARS[(num >> i) % 0x10]);
  UART_SendChar(HEX_CHARS[(num & 0x0f) % 0x10]);
}

void UART_SendBufHex(uint8_t *buf, uint16_t bufsize)
{
  uint16_t i;
  char ch;
  for (i = 0; i < bufsize; i++)
  {
    ch = *buf++;
    UART_SendChar(HEX_CHARS[(ch >> 4) % 0x10]);
    UART_SendChar(HEX_CHARS[(ch & 0x0f) % 0x10]);
  }
}
