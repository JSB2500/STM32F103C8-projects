#ifndef UART_H_
#define UART_H_

#include "stdint.h"
#include "stm32f1xx_hal.h"

extern UART_HandleTypeDef UART_Handle;

void UART_SendChar(char ch);
void UART_SendStr(char *p);
void UART_printf(const char * format, ...);
void UART_SendInt(int32_t num);
void UART_SendInt0(int32_t num);
void UART_SendHex8(uint16_t num);
void UART_SendHex16(uint16_t num);
void UART_SendHex32(uint32_t num);
void UART_SendBufHex(uint8_t *buf, uint16_t bufsize);

#endif
