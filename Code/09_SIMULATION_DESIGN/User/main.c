#include "stm32f10x.h"
#include "Delay.h"
#include "OLED.h"
#include "Timer.h" // 假设你有这个头文件，如果没有，见代码末尾注释
#include "AD.h"    // 假设你有这个头文件


// 引用在 AD.c 中定义的全局变量
// 这个变量会被 DMA 自动更新，不需要我们在 main 里手动写值
extern volatile uint16_t AD_Value;

int main(void)
{
    /* 1. 模块初始化 */
    OLED_Init(); // 初始化屏幕
    AD_Init();   // 初始化ADC和DMA
    
    // 初始化定时器 TIM3
    // 目的：产生更新事件(Update)来触发ADC转换
    // 公式：频率 = 72M / (PSC+1) / (ARR+1)
    // 这里设置 PSC=71, ARR=999 -> 频率 = 1kHz (即每1ms采集一次数据)
    Timer_Init(1000 - 1, 72 - 1); 

    /* 2. 显示静态界面（不变化的文字只刷一次，提高效率） */
    OLED_ShowString(1, 1, "AD Value:");
    OLED_ShowString(2, 1, "Volage:0.00V");

    /* 3. 主循环 */
    while (1)
    {
        // --- 显示原始 AD 值 (0 ~ 4095) ---
        // 参数：行, 列, 变量, 长度
        OLED_ShowNum(1, 10, AD_Value, 4);

        // --- 计算并显示电压值 (0V ~ 3.3V) ---
        // 算法：AD值 / 4095 * 3.3V
        float Voltage = (float)AD_Value / 4095 * 3.3;
        
        // 显示整数部分 (比如 1.23V 中的 1)
        OLED_ShowNum(2, 8, (uint8_t)Voltage, 1);
        
        // 显示小数部分 (比如 1.23V 中的 23)
        // 技巧：(Voltage * 100) % 100 可以取出后两位小数
        OLED_ShowNum(2, 10, (uint16_t)(Voltage * 100) % 100, 2);

        // --- 延时 ---
        // 这里的延时只是为了人眼能看清数字变化
        // ADC 的采样频率是由 Timer_Init 决定的，与这里的 Delay 无关
        Delay_ms(100);
    }
}
