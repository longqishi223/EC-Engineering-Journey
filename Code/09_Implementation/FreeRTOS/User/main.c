#include "stm32f10x.h"
#include "FreeRTOS.h"
#include "task.h"

// 引入你的硬件驱动头文件
#include "OLED.h"
#include "Temperature.h"
#include "pwm.h"
#include "uart.h"
// ⚠️ 注意：这里彻底删除了 systick.h 及其头文件，我们再也不需要手搓滴答定时器了！

/* ================== 全局共享变量 ================== */
// 用于在不同任务之间传递数据（暂时用全局变量，后续可以升级为消息队列）
volatile uint16_t g_adc_value = 0;
volatile uint8_t g_temp_c = 0;

/* ==================== 任务句柄 ==================== */
TaskHandle_t ThermalTask_Handler;
TaskHandle_t OLEDTask_Handler;
TaskHandle_t UARTTask_Handler;

/* ================= 任务1：热管理控制 =================
 * 优先级：最高 (3)
 * 周期：严格 20ms
 * ================================================= */
void vTask_Thermal(void *pvParameters) 
{
    // 记录上次唤醒的时间（供 vTaskDelayUntil 使用）
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(20); // 转换为操作系统的 Tick 数

    while(1) 
    {
        // 1. 采样与计算
        g_adc_value = NTC_GetFilteredADC(ADC_Channel_0);
        g_temp_c = NTC_ConvertToTemp_LUT(g_adc_value);

        // 2. 决策与执行 (阶梯式控制)
        uint8_t pwm_value = (g_temp_c > 32) ? 180 : (g_temp_c > 29) ? 90 : 0;
        PWM_SG90_SetAngle(pwm_value);

        // 3. 绝对延时：保证严格的 20ms 闭环周期，不受上面代码执行耗时的影响！
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}

/* ================= 任务2：OLED 刷新 =================
 * 优先级：中等 (2)
 * 周期：大约 100ms
 * ================================================= */
void vTask_OLED(void *pvParameters) 
{
    while(1) 
    {
        OLED_ShowNum(2, 11, g_adc_value, 4);
        OLED_ShowNum(3, 11, g_temp_c, 4);
        
        // 为了 UI 显示同步，再算一次角度
        uint8_t display_angle = (g_temp_c > 32) ? 180 : (g_temp_c > 29) ? 90 : 0;
        OLED_ShowNum(4, 11, display_angle, 3); 

        // 相对延时：UI 刷新不需要那么严苛的周期，普通的 vTaskDelay 就够了
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

/* ================= 任务3：串口打印 =================
 * 优先级：最低 (1)
 * 周期：大约 500ms
 * ================================================= */
void vTask_UART(void *pvParameters) 
{
    while(1) 
    {
        printf("RTOS Heartbeat | ADC Raw: %u, CPU Temp: %u C\r\n", g_adc_value, g_temp_c);
        
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

/* ==================== 主 函 数 ==================== */
int main(void)
{
    /* 1. 模块初始化 (极简，去掉了你原来的 SysTick_Init) */
    OLED_Init(); 
    NTC_ADC_Init();  
    PWM_SG90_Init();  
    UART_Init();      

    /* 2. 显示静态界面 */
    OLED_ShowString(1, 1, "=== RTOS Thermal ===");
    OLED_ShowString(2, 1, "ADC Raw : ");
    OLED_ShowString(3, 1, "CPU Temp: ");
    OLED_ShowString(3, 13, "C"); 
    OLED_ShowString(4, 1, "Fan Angle: ");
    OLED_ShowString(4, 13, "deg");

    /* 3. 创建多任务生态 */
    // 参数含义：任务函数, 名字, 堆栈大小(字), 传参, 优先级(越大越高), 句柄
    xTaskCreate(vTask_Thermal, "Thermal", 128, NULL, 3, &ThermalTask_Handler);
    xTaskCreate(vTask_OLED,    "OLED",    128, NULL, 2, &OLEDTask_Handler);
    xTaskCreate(vTask_UART,    "UART",    128, NULL, 1, &UARTTask_Handler);

    /* 4. 移交指挥权，启动 RTOS 调度器！ */
    vTaskStartScheduler();

    /* 5. 死亡陷阱 */
    // 一旦启动调度器，CPU 就由 FreeRTOS 接管，正常的程序永远不会跑到这里。
    // 如果跑到了这里，说明 FreeRTOS 的堆内存不足，无法创建任务，系统崩溃。
    while (1) 
    {
    }
}
