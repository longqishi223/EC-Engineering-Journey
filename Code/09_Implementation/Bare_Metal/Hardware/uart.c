#include "uart.h"

void UART_Init(void)
{
    // 1. 使能 GPIOA 和 USART1 时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_USART1, ENABLE);

    // 2. 配置 PA9 (TX) 为复用推挽输出
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; // 复用推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // 3. 配置 PA10 (RX) 为浮空输入
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; // 浮空输入
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // 4. 配置 USART1 参数
    USART_InitTypeDef USART_InitStructure;
    USART_InitStructure.USART_BaudRate = 115200; // 波特率
    USART_InitStructure.USART_WordLength = USART_WordLength_8b; // 数据位长度
    USART_InitStructure.USART_StopBits = USART_StopBits_1; // 停止位长度
    USART_InitStructure.USART_Parity = USART_Parity_No; // 无奇偶校验
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None; // 无硬件流控制
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx; // 同时启用接收和发送
    USART_Init(USART1, &USART_InitStructure);

    // 5. 启动 USART1
    USART_Cmd(USART1, ENABLE);
}

int fputc(int ch, FILE *f)
{
    // 发送一个字符到 USART1
    USART_SendData(USART1, (uint8_t)ch);
    // 等待发送完成
    while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);
    return ch;
}
