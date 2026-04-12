#include "stm32f10x.h"
#include "Delay.h"
#include "OLED.h"
#include "Temperature.h"
#include "pwm.h"
#include "uart.h"
#include "systick.h"

// 引用在 AD.c 中定义的全局变量
// 这个变量会被 DMA 自动更新，不需要我们在 main 里手动写值
//extern volatile uint16_t AD_Value;

int main(void)
{
    /* 1. 模块初始化 */
    SysTick_Init(); // 初始化系统滴答定时器
    OLED_Init(); // 初始化屏幕
    NTC_ADC_Init();   // 初始化ADC和DMA
    PWM_SG90_Init();  // 初始化PWM
    UART_Init();     // 初始化串口

    /* 2. 显示静态界面 */
    OLED_ShowString(1, 1, "=== EC Thermal ===");
    OLED_ShowString(2, 1, "ADC Raw : ");
    OLED_ShowString(3, 1, "CPU Temp: ");
    OLED_ShowString(3, 13, "C"); // 摄氏度单位符号
    OLED_ShowString(4, 1, "Fan Angle: ");
    OLED_ShowString(4, 13, "deg"); // 角度单位符号

    uint32_t timer_thermal = 0; // 用于定时执行热管理逻辑的计时器
    uint32_t timer_uart = 0; // 用于定时发送 UART 数据的计时器
    uint32_t timer_ui = 0; // 用于定时更新 UI 的计时器
    //uint32_t timer_pwm = 0; // 用于定时更新 PWM 的计时器

    uint16_t adc_value = 0;
    uint8_t temp_c = 0;

    /* 3. 主循环 */
    while (1)
    {
        uint32_t current_tick = Get_SysTick(); // 获取当前系统滴答计数

        if(current_tick - timer_thermal >= 20) // 每隔 20ms 执行一次
        {
            timer_thermal = current_tick; // 更新上次执行的时间
            adc_value = NTC_GetFilteredADC(ADC_Channel_0); // 从 NTC ADC 获取滤波后的值
            temp_c = NTC_ConvertToTemp_LUT(adc_value); // 将 ADC 值转换为温度

            uint8_t pwm_value = (temp_c > 32) ? 180 : (temp_c > 29) ? 90 : 0; // 根据温度计算 PWM 占空比
            PWM_SG90_SetAngle(pwm_value); // 设置风扇角度 
        }

        if(current_tick - timer_ui >= 100) // 每隔 100ms 更新一次 UI
        {
            timer_ui = current_tick; // 更新上次执行的时间
            OLED_ShowNum(3, 11, temp_c, 4);
            OLED_ShowNum(4, 11, (temp_c > 32) ? 180 : (temp_c > 29) ? 90 : 0, 3); // 显示风扇角度
        }

        if(current_tick - timer_uart >= 500) // 每隔 1s 发送一次 UART 数据
        {
            timer_uart = current_tick; // 更新上次执行的时间
            printf("ADC Raw: %u, CPU Temp: %u C\r\n", adc_value, temp_c); // 通过 UART 输出 ADC 原始值和温度
        }
    }
}
