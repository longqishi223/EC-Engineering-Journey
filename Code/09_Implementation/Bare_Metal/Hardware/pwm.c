#include "pwm.h"

void PWM_SG90_Init(void)
{
    // 1. 使能 GPIOA 和 TIM3 时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

    // 2. 配置 PA6 为复用推挽输出 (TIM3_CH1)
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; // 复用推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // 3. 配置 TIM3 基本参数
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_TimeBaseStructure.TIM_Period = 20000 - 1; // 20ms周期 (50Hz)
    TIM_TimeBaseStructure.TIM_Prescaler = 72 - 1; // 72MHz / 72 = 1MHz计数频率
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);

    // 4. 配置 TIM3 的 PWM 模式
    TIM_OCInitTypeDef TIM_OCInitStructure;
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1; // PWM模式1
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; // 输出使能
    TIM_OCInitStructure.TIM_Pulse = 0; // 初始占空比 (1.5ms)
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High; // 高电平有效
    TIM_OC1Init(TIM3, &TIM_OCInitStructure); // 配置通道1

    // 5. 启动定时器
    TIM_Cmd(TIM3, ENABLE);
}

void PWM_SG90_SetAngle(uint8_t angle)
{
    if (angle > 180) angle = 180; // 限制角度范围
    uint16_t pulse_length = 1000 + (angle * 1000) / 180; // 1ms ~ 2ms 对应 0° ~ 180°
    TIM_SetCompare1(TIM3, pulse_length); // 更新占空比
}