#include "Temperature.h"

static const uint16_t NTC_ADC_Table[11] = {
    3424, // 0℃  (电阻最大，分压最高)
    2987, // 10℃
    2467, // 20℃
    1930, // 30℃
    1443, // 40℃
    1040, // 50℃
    731,  // 60℃
    508,  // 70℃
    350,  // 80℃
    240,  // 90℃
    165   // 100℃ (电阻最小，分压最低)
};

void NTC_ADC_Init(void) {
    GPIO_InitTypeDef GPIO_InitStructure;
    ADC_InitTypeDef ADC_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1 | RCC_APB2Periph_GPIOA, ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1; 
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
    ADC_InitStructure.ADC_ScanConvMode = DISABLE; 
    ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
    ADC_InitStructure.ADC_NbrOfChannel = 1;
    ADC_Init(ADC1, &ADC_InitStructure);

    ADC_Cmd(ADC1, ENABLE);

    ADC_ResetCalibration(ADC1);
    while(ADC_GetResetCalibrationStatus(ADC1));

    ADC_StartCalibration(ADC1);
    while(ADC_GetCalibrationStatus(ADC1));

}

static uint16_t NTC_ReadRawADC(uint8_t channel) {
    ADC_RegularChannelConfig(ADC1, channel, 1, ADC_SampleTime_239Cycles5);
    ADC_SoftwareStartConvCmd(ADC1, ENABLE);
    while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC));
    return ADC_GetConversionValue(ADC1);
}

uint16_t NTC_GetFilteredADC(uint8_t channel) {
    uint32_t adc_sum = 0;
    NTC_ReadRawADC(channel); // 第一次读可能不稳定，丢弃
    for (int i = 0; i < 8; i++) {
        adc_sum += NTC_ReadRawADC(channel);
    }
    return (uint16_t)(adc_sum >> 3); // 返回平均值
}

int8_t NTC_ConvertToTemp_LUT(uint16_t adc_value) {

    if (adc_value >= NTC_ADC_Table[0]) return 0;   // 极寒或断路
    if (adc_value <= NTC_ADC_Table[10]) return 100; // 极热或短路

    for (int i = 0; i < 10; i++) {
        // 锁定区间
        if (adc_value <= NTC_ADC_Table[i] && adc_value > NTC_ADC_Table[i + 1]) {
            
            // 明确物理意义：区间上限 ADC (阻值大) 和 区间下限 ADC (阻值小)
            uint16_t adc_upper = NTC_ADC_Table[i];   
            uint16_t adc_lower = NTC_ADC_Table[i + 1];
            
            // 明确物理意义：该区间对应的基础温度 (比如 i=3 时，基础温度是 30℃)
            uint8_t base_temp = i * 10;
            
            // 纯正数运算：当前 ADC 值距离上限 ADC 的偏移量
            uint16_t adc_offset = adc_upper - adc_value; 
            uint16_t adc_span = adc_upper - adc_lower;
            
            // 纯正数推导公式：基础温度 + (ADC 偏移量占比 * 10℃)
            // 先乘 10 再除，保留整数精度，全程无负数、无溢出风险！
            return base_temp + (adc_offset * 10) / adc_span;
        }
    }
    return 0; // 兜底保护
}
