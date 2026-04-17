#ifndef __UART_H
#define __UART_H

#include "stm32f10x.h"
#include <stdio.h>

void UART_Init(void);
void UART_SendString(const char *str);
void UART_ProcessRx(void);

#endif /* __UART_H */
