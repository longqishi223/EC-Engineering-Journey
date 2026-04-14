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
#include "key.h"\

/* ================== 全局共享变量 ================== */
// 用于在不同任务之间传递数据（暂时用全局变量，后续可以升级为消息队列）
//volatile uint16_t g_adc_value = 0;
//volatile uint8_t g_temp_c = 0;

QueueHandle_t TempQueue_OLED; // 温度数据队列句柄
QueueHandle_t TempQueue_UART; // 温度数据队列句柄
SemaphoreHandle_t UART_Mutex; // UART串口互斥锁句柄
SemaphoreHandle_t Key_Semaphore; // 按键事件信号量句柄
uint8_t key_A[8] = {0, 0, 0x04, 0, 0, 0, 0, 0}; // 0x04 就是字母 'a'
uint8_t key_Release[8] = {0, 0, 0, 0, 0, 0, 0, 0}; // 全零代表松开按键

typedef struct {
    uint16_t adc_value;
    uint8_t temp_c;
} TempData_t;

/* ==================== 任务句柄 ==================== */
TaskHandle_t ThermalTask_Handler;
TaskHandle_t OLEDTask_Handler;
TaskHandle_t UARTTask_Handler;
TaskHandle_t KeyProcessTask_Handler; // 按键处理任务句柄

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
    uint8_t uart_send_counter = 0; // 用于控制串口发送频率
    while(1) 
    {
        tempData.adc_value = NTC_GetFilteredADC(ADC_Channel_0);
        tempData.temp_c = NTC_ConvertToTemp_LUT(tempData.adc_value);

        // 将温度数据发送到队列，等待OLED任务处理
        xQueueSend(TempQueue_OLED, &tempData, 0);
        if(uart_send_counter++ >= 25) { // 每500ms发送一次串口数据
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
                
                USB_Keyboard_Send(key_A);
                vTaskDelay(pdMS_TO_TICKS(30)); // 模拟人手按下的持续时间
                // 松开按键
                USB_Keyboard_Send(key_Release);

                // 核心逻辑：翻转显示模式 (0变1，1变0)
                g_display_mode = !g_display_mode; 
                
                Safe_Printf("Key Pressed! Mode switched to: %d\r\n", g_display_mode);
                
            }
        }
    }
}

/* ==================== 主 函 数 ==================== */
int main(void)
{
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
    /* 1. 模块初始化 (极简，去掉了你原来的 SysTick_Init) */
    USB_IO_Init();
    OLED_Init(); 
    NTC_ADC_Init();  
    PWM_SG90_Init();  
    UART_Init();      
    Key_EXTI_Init(); // 初始化按键外部中断

    TempQueue_UART = xQueueCreate(5, sizeof(TempData_t)); // 创建一个长度为5的队列，每个元素是TempData_t结构体
    TempQueue_OLED = xQueueCreate(5, sizeof(TempData_t)); // 创建一个长度为5的队列，每个元素是TempData_t结构体
    UART_Mutex = xSemaphoreCreateMutex(); // 创建一个互斥锁用于保护UART资源
    Key_Semaphore = xSemaphoreCreateBinary(); // 创建一个二值信号量用于按键事件

    if(TempQueue_UART == NULL) {
        while(1);// 队列创建失败，处理错误
    }
    if(TempQueue_OLED == NULL) {
        while(1);// 队列创建失败，处理错误
    }
    if(UART_Mutex == NULL) {
        while(1);// 互斥锁创建失败，处理错误
    }
    if(Key_Semaphore == NULL) {
        while(1);// 信号量创建失败，处理错误
    }

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
    xTaskCreate(vTask_OLED,    "OLED",    256, NULL, 2, &OLEDTask_Handler);
    xTaskCreate(vTask_UART,    "UART",    128, NULL, 1, &UARTTask_Handler);
    xTaskCreate(vTask_KeyProcess, "KeyProcess", 128, NULL, 3, &KeyProcessTask_Handler); // 按键处理任务，优先级同OLED

    /* 4. 移交指挥权，启动 RTOS 调度器！ */
    vTaskStartScheduler();

    /* 5. 死亡陷阱 */
    // 一旦启动调度器，CPU 就由 FreeRTOS 接管，正常的程序永远不会跑到这里。
    // 如果跑到了这里，说明 FreeRTOS 的堆内存不足，无法创建任务，系统崩溃。
    while (1) 
    {
    }
}
