#include "uart.h"
#include "stm32f10x_it.h"
#include <string.h>

/* UART 接收缓冲区大小 */
#define UART_RX_BUF_SIZE  128

/* UART 接收缓冲区 */
static uint8_t gUartRxBuf[UART_RX_BUF_SIZE];
static volatile uint16_t gUartRxHead = 0;
static volatile uint16_t gUartRxTail = 0;

/* 外部声明: ACPI 命令处理函数 (在 acpi.c 中定义) */
extern void ACPI_HandleUartChar(uint8_t c);

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

    // 5. 使能 USART1 接收中断
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);

    // 6. 配置 NVIC 优先级
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 5;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    // 7. 启动 USART1
    USART_Cmd(USART1, ENABLE);
}

/**
 * UART_SendString - 发送字符串
 */
void UART_SendString(const char *str)
{
    while (*str) {
        USART_SendData(USART1, (uint8_t)*str);
        while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);
        str++;
    }
}

/**
 * UART_ProcessRx - 处理接收缓冲区中的数据
 * 在主循环中调用
 */
void UART_ProcessRx(void)
{
    while (gUartRxHead != gUartRxTail) {
        uint8_t c = gUartRxBuf[gUartRxTail];
        gUartRxTail = (gUartRxTail + 1) % UART_RX_BUF_SIZE;
        ACPI_HandleUartChar(c);
    }
}

/**
 * USART1_IRQHandler - USART1 中断处理函数
 */
void USART1_IRQHandler(void)
{
    if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET) {
        uint8_t c = (uint8_t)USART_ReceiveData(USART1);
        uint16_t next_head = (gUartRxHead + 1) % UART_RX_BUF_SIZE;
        if (next_head != gUartRxTail) {
            gUartRxBuf[gUartRxHead] = c;
            gUartRxHead = next_head;
        }
        USART_ClearITPendingBit(USART1, USART_IT_RXNE);
    }
}

int fputc(int ch, FILE *f)
{
    // 发送一个字符到 USART1
    USART_SendData(USART1, (uint8_t)ch);
    // 等待发送完成
    while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);
    return ch;
}
