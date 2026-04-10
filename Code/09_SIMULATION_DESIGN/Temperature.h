#ifndef NTC_TEMPERATURE_H
#define NTC_TEMPERATURE_H

#include "stm32f10x.h"

/* ==========================================
 * 模块：NTC 温度传感器底层驱动
 * 硬件：STM32F103 ADC1, 10K NTC (B=3950)
 * ========================================== */

// 1. 初始化温度 ADC 硬件引脚
void NTC_ADC_Init(void);

// 2. 获取经过滑动平均滤波的 ADC 值 (极其重要，防风扇抽搐)
uint16_t NTC_GetFilteredADC(uint8_t channel);

// 3. 极速查表法计算温度 (全整数运算，杜绝 float)
int8_t NTC_ConvertToTemp_LUT(uint16_t adc_raw);

#endif /* NTC_TEMPERATURE_H */