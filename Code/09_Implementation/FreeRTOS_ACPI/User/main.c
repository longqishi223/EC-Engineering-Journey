#include "stm32f10x.h"
#include "FreeRTOS.h"
#include "task.h"

// 引入你的硬件驱动头文件
#include "OLED.h"
#include "Temperature.h"
#include "pwm.h"
#include "uart.h"
#include "queue.h"
#include "semphr.h"
#include <stdarg.h>
#include "key.h"
#include "acpi.h"

/* ================== 全局共享变量 ================== */
// 用于在不同任务之间传递数据（暂时用全局变量，后续可以升级为消息队列）
//volatile uint16_t g_adc_value = 0;
//volatile uint8_t g_temp_c = 0;

QueueHandle_t TempQueue_OLED; // 温度数据队列句柄
QueueHandle_t TempQueue_UART; // 温度数据队列句柄
SemaphoreHandle_t UART_Mutex; // UART串口互斥锁句柄
SemaphoreHandle_t Key_Semaphore; // 按键事件信号量句柄

typedef struct {
    uint16_t adc_value;
    uint8_t temp_c;
} TempData_t;

/* ==================== 任务句柄 ==================== */
TaskHandle_t ThermalTask_Handler;
TaskHandle_t OLEDTask_Handler;
TaskHandle_t UARTTask_Handler;
TaskHandle_t KeyProcessTask_Handler; // 按键处理任务句柄
TaskHandle_t ACPITask_Handler;       // ACPI 任务句柄

// 0: 默认显示温度界面
// 1: 显示设置/调试界面
volatile uint8_t g_display_mode = 0;


/* ================= 任务1：热管理控制 =================
 * 优先级：最高 (3)
 * 周期：严格 20ms
 * ================================================= */
void vTask_Thermal(void *pvParameters)
{
    TempData_t tempData;
    uint8_t uart_send_counter = 0;
    while(1)
    {
        tempData.adc_value = NTC_GetFilteredADC(ADC_Channel_0);
        tempData.temp_c = NTC_ConvertToTemp_LUT(tempData.adc_value);

        /* 将温度同步到 ACPI (用于串口查询) */
        ACPI_SetTemperature((uint16_t)tempData.temp_c * 10);  /* 转为 0.1°C 单位 */

        /* 将温度数据发送到队列 */
        xQueueSend(TempQueue_OLED, &tempData, 0);
        if(uart_send_counter++ >= 25) {
            uart_send_counter = 0;
            xQueueSend(TempQueue_UART, &tempData, 0);
        }
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}

/* ================= 任务2：OLED 刷新 =================
 * 优先级：中等 (2)
 * 周期：大约 100ms
 * ================================================= */
void vTask_OLED(void *pvParameters) 
{
    TempData_t receivedData;
    // 定义一个“影子变量”来记录上一次的模式。
    // 设为 255 是为了让第一次开机时，必定触发一次完整的静态UI绘制。
    uint8_t last_mode = 255; 

    while(1) 
    {
        if(xQueueReceive(TempQueue_OLED, &receivedData, portMAX_DELAY) == pdTRUE) {
            
            /* ===================================================
             * 第一步：边缘触发 (Edge Trigger) - 只在模式切换的瞬间执行 1 次！
             * =================================================== */
            if(g_display_mode != last_mode) {
                OLED_Clear(); // 先暴力清空整个屏幕的旧数据
                
                if(g_display_mode == 0) {
                    // 把模式 0 的“底图”画上去（全生命周期只画这一次）
                    OLED_ShowString(1, 1, "=== RTOS Thermal ===");
                    OLED_ShowString(2, 1, "ADC Raw : ");
                    OLED_ShowString(3, 1, "CPU Temp: ");
                    OLED_ShowString(3, 13, "C"); 
                    OLED_ShowString(4, 1, "Fan Angle: ");
                    OLED_ShowString(4, 13, "deg");
                } 
                else {
                    // 把模式 1 的“底图”画上去（全生命周期只画这一次）
                    OLED_ShowString(1, 1, "=== DEBUG MODE ===");
                    OLED_ShowString(2, 1, "System State: "); 
                    OLED_ShowString(3, 1, "Test Value  : "); 
                }
                
                // 极其关键：更新影子变量！这样下次循环就不会再进这个大括号了
                last_mode = g_display_mode; 
            }

            /* ===================================================
             * 第二步：高频刷新 (Level Update) - 每 20ms 执行一次，只刷数字！
             * =================================================== */
            if(g_display_mode == 0) {
                // 因为底部的字母已经有了，我们只管填数字的坑
                OLED_ShowNum(2, 11, receivedData.adc_value, 4);
                OLED_ShowNum(3, 11, receivedData.temp_c, 4);
                uint8_t display_angle = (receivedData.temp_c > 32) ? 180 : (receivedData.temp_c > 29) ? 90 : 0;
                OLED_ShowNum(4, 11, display_angle, 3); 
                PWM_SG90_SetAngle(display_angle); 
            } 
            else {
                // 模式 1 里的动态刷新
                OLED_ShowString(2, 15, "OK"); 
                OLED_ShowNum(3, 15, 6, 1); 
            }
            vTaskDelay(pdMS_TO_TICKS(200));
        }
    }
}

/* ================= 任务3：串口打印 =================
 * 优先级：最低 (1)
 * 周期：大约 500ms
 * ================================================= */

void Safe_Printf(const char *format, ...) {
    if(xSemaphoreTake(UART_Mutex, portMAX_DELAY) == pdTRUE) {
        va_list args;
        va_start(args, format); 
        vprintf(format, args);
        va_end(args);
        xSemaphoreGive(UART_Mutex);
    }
}

void vTask_UART(void *pvParameters) 
{
    while(1) 
    {
        TempData_t receivedData;
        if(xQueueReceive(TempQueue_UART, &receivedData, portMAX_DELAY) == pdTRUE) {
            // 这里直接使用 FreeRTOS 的串口打印函数，假设它已经被正确实现了线程安全
            Safe_Printf("RTOS Heartbeat | ADC Raw: %u, CPU Temp: %u C\r\n", receivedData.adc_value, receivedData.temp_c);
        }
    }
}

void EXTI4_IRQHandler(void) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    if(EXTI_GetITStatus(EXTI_Line4) != RESET) {
        xSemaphoreGiveFromISR(Key_Semaphore, &xHigherPriorityTaskWoken); // 触发按键事件信号量
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken); // 如果需要切换任务，立即切换
        EXTI_ClearITPendingBit(EXTI_Line4); // 清除中断标志
    }
}

void vTask_KeyProcess(void *pvParameters)
{
    TickType_t last_wake_time = 0;
    while(1)
    {
        if(xSemaphoreTake(Key_Semaphore, portMAX_DELAY) == pdTRUE) {
            if(xTaskGetTickCount() - last_wake_time > pdMS_TO_TICKS(200)){
                last_wake_time = xTaskGetTickCount();

                /* 按键触发 LID 状态变化 (模拟开合盖) */
                static uint8_t lid_state = 1;
                lid_state = !lid_state;
                ACPI_SetLid(lid_state);

                /* 翻转显示模式 */
                g_display_mode = !g_display_mode;

                Safe_Printf("Key Pressed! LID=%d, Mode=%d\r\n", lid_state, g_display_mode);
            }
        }
    }
}

/* ================= 任务5：ACPI 处理 =================
 * 优先级：较低 (1)
 * 周期：实时处理 UART 命令
 * ================================================= */
void vTask_ACPI(void *pvParameters)
{
    while(1) {
        /* 处理 UART 接收缓冲区的 ACPI 命令 */
        UART_ProcessRx();

        /* ACPI 主循环处理 (事件检测) */
        ACPI_Process();

        vTaskDelay(pdMS_TO_TICKS(10));  /* 10ms 周期 */
    }
}

/* ==================== 主 函 数 ==================== */
int main(void)
{
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
    /* 1. 模块初始化 */
    UART_Init();          /* UART 必须在 ACPI 之前初始化 */
    OLED_Init();
    NTC_ADC_Init();
    PWM_SG90_Init();
    Key_EXTI_Init();      /* 初始化按键外部中断 */

    /* 2. ACPI 初始化 */
    ACPI_Init();          /* 初始化 ACPI 子系统 */

    /* 3. 创建队列和信号量 */
    TempQueue_UART = xQueueCreate(5, sizeof(TempData_t));
    TempQueue_OLED = xQueueCreate(5, sizeof(TempData_t));
    UART_Mutex = xSemaphoreCreateMutex();
    Key_Semaphore = xSemaphoreCreateBinary();

    if(TempQueue_UART == NULL || TempQueue_OLED == NULL ||
       UART_Mutex == NULL || Key_Semaphore == NULL) {
        while(1);  /* 资源创建失败 */
    }

    /* 4. 显示静态界面 */
    OLED_ShowString(1, 1, "=== RTOS Thermal ===");
    OLED_ShowString(2, 1, "ADC Raw : ");
    OLED_ShowString(3, 1, "CPU Temp: ");
    OLED_ShowString(3, 13, "C");
    OLED_ShowString(4, 1, "Fan Angle: ");
    OLED_ShowString(4, 13, "deg");

    /* 5. 创建多任务生态 */
    xTaskCreate(vTask_Thermal,    "Thermal",    128, NULL, 3, &ThermalTask_Handler);
    xTaskCreate(vTask_OLED,       "OLED",       256, NULL, 2, &OLEDTask_Handler);
    xTaskCreate(vTask_UART,       "UART",       256, NULL, 1, &UARTTask_Handler);
    xTaskCreate(vTask_KeyProcess, "KeyProcess", 128, NULL, 3, &KeyProcessTask_Handler);
    xTaskCreate(vTask_ACPI,       "ACPI",       256, NULL, 1, &ACPITask_Handler);  /* ACPI 任务 */

    /* 6. 启动 RTOS 调度器 */
    vTaskStartScheduler();

    /* 7. 死亡陷阱 */
    while(1) {
    }
}
