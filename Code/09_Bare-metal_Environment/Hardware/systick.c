#include "systick.h"
static volatile uint32_t g_SystemTick = 0; // 全局系统滴答计数器

void SysTick_Init(void)
{
    // 配置 SysTick 定时器，使用系统时钟，1ms 中断一次
    if(SysTick_Config(SystemCoreClock / 1000) != 0)
    {
        // 配置失败，进入死循环
        while(1);
    }
}

uint32_t Get_SysTick(void)
{
    return g_SystemTick;
}

void SysTick_Handler(void)
{
    g_SystemTick++; // 每次 SysTick 中断时递增计数器
}

