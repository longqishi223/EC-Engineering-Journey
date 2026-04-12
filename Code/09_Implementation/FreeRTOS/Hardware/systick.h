#ifndef __SYSTICK_H
#define __SYSTICK_H

#include "stm32f10x.h"

void SysTick_Init(void);
uint32_t Get_SysTick(void);

#endif /* __SYSTICK_H */
