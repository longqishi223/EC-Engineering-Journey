#include "stm32f10x.h"
#include "usb_lib.h"
#include "usb_pwr.h"
#include "usb_io.h"

// 只保留最核心的时钟和中断初始化
void USB_IO_Init(void)
{
    NVIC_InitTypeDef NVIC_InitStructure;

    // 1. USB 时钟配置 (非常关键：系统时钟72MHz / 1.5 = 48MHz)
    RCC_USBCLKConfig(RCC_USBCLKSource_PLLCLK_1Div5);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USB, ENABLE);

    // 2. 配置 USB 中断优先级 (配合你的 FreeRTOS)
    // NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
    NVIC_InitStructure.NVIC_IRQChannel = USB_LP_CAN1_RX0_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 5; // 优先级低于系统内核
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    
    // 3. 启动 USB 核心初始化
    USB_Init();
}

// 封装一个极其简单的键盘发送函数，供你的 RTOS 任务调用！
void USB_Keyboard_Send(uint8_t *report)
{
    // 如果 USB 还没有被电脑识别配置好，就不发数据
    if(bDeviceState != CONFIGURED) return; 

    uint32_t timeout = 0xFFFFF;
    while((GetEPTxStatus(ENDP1) != EP_TX_NAK) && timeout--);
    
    if (timeout == 0) return; // 超时，主机未响应，放弃发送

    // 把 8 字节的键盘报文拷贝到 USB 硬件的 PMA 内存中
    UserToPMABufferCopy(report, ENDP1_TXADDR, 8);
    // 告诉 USB 硬件：数据准备好了，下一次电脑来要数据时直接发走！
    SetEPTxValid(ENDP1); 
}
