/**
 * @file    main.c
 * @brief   Test bench for the EC Power Sequence State Machine
 */

#include <stdio.h>
#include "power_sequence.h"

// 声明外部变量，这样我们就能在 main 函数里模拟硬件信号的改变了
extern HardwareSignals_t hw_signals; 

int main() {
    printf("=========================================\n");
    printf("   EC Power Sequence Simulator Started   \n");
    printf("=========================================\n\n");

    // 1. 初始化 EC 状态机
    PowerSeq_Init();

    // 2. 模拟插上电源适配器 (从 G3 完全断电 -> S5 待机)
    printf("\n---> [USER ACTION] Plugging in AC Adapter...\n");
    hw_signals.ac_present = true;
    PowerSeq_Task(); // 运行一次状态机
    
    // 3. 模拟按下电源键开机 (S5 -> TRANSITION 过渡态)
    printf("\n---> [USER ACTION] Pressing Power Button...\n");
    hw_signals.pwr_btn_pressed = true;
    PowerSeq_Task(); // 运行状态机，EC 检测到按键并唤醒 PCH
    
    // 松开电源键 (机械按键不会一直按着)
    hw_signals.pwr_btn_pressed = false; 

    // 4. 继续运行状态机，让主板跑完上电时序 (TRANSITION -> S0)
    printf("\n---> [SYSTEM] Continuing power sequence...\n");
    PowerSeq_Task(); // PCH 发出 SLP 信号，EC 开启各路电源
    
    // 检查是否成功进入 S0 (正常工作状态)
    if (Get_Current_System_State() == SYS_STATE_S0) {
        printf("\n*** SUCCESS: System is fully ON and running Windows/Linux (S0 State)! ***\n");
    }

    // ---------------- 以下为进阶测试 ---------------- //

    // 5. 模拟合上盖子或点击睡眠 (S0 -> S3)
    printf("\n---> [USER ACTION] OS commands Sleep (S3)...\n");
    hw_signals.slp_s3_n = false; // 模拟 PCH 拉低 SLP_S3# 信号
    PowerSeq_Task(); // EC 侦测到信号，切断 CPU 供电但保持 RAM 供电

    if (Get_Current_System_State() == SYS_STATE_S3) {
        printf("\n*** SUCCESS: System is sleeping (S3). RAM is still powered. Zzz... ***\n");
    }

    // 6. 模拟从睡眠中唤醒 (S3 -> TRANSITION -> S0)
    printf("\n---> [USER ACTION] Pressing Power Button to Wake...\n");
    hw_signals.pwr_btn_pressed = true;
    PowerSeq_Task(); // EC 触发唤醒事件
    
    hw_signals.pwr_btn_pressed = false; // 松开按键
    PowerSeq_Task(); // 走完唤醒流程

    if (Get_Current_System_State() == SYS_STATE_S0) {
        printf("\n*** SUCCESS: System woke up and is back to S0! ***\n");
    }

    printf("\n=========================================\n");
    printf("   Simulation Complete. Exiting...       \n");
    printf("=========================================\n");

    return 0;
}
