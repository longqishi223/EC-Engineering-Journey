/*******************************************************************************
* File Name          : acpi.h
* Author             : EC Simulation Framework
* Version            : V1.1.0
* Date               : 2026/04/16
* Description        : ACPI (Advanced Configuration and Power Interface) implementation
*                     for STM32F103C8T6 using UART communication
*
*                     使用 UART 作为 ACPI 通信通道:
*                     - 波特率: 115200
*                     - 数据位: 8
*                     - 停止位: 1
*                     - 无校验
*
*                     协议格式 (ASCII 明文，类似 AT 命令):
*                     Host → EC:   $ACPI,CMD,PARAM1,PARAM2,...\r\n
*                     EC → Host:   $ACK,RET,DATA1,DATA2,...\r\n
*                     EC → Host:   $EVT,EVENT_CODE,DATA\r\n  (异步事件通知)
*
*                     常用命令:
*                     $ACPI,VER            → 获取 EC 版本
*                     $ACPI,STATE          → 获取电源状态
*                     $ACPI,BAT            → 获取电池信息
*                     $ACPI,TEMP           → 获取温度
*                     $ACPI,LID            → 获取 LID 状态
*                     $ACPI,FAN            → 获取风扇状态
*                     $ACPI,EVENT          → 获取待处理事件
*                     $ACPI,SSTAT,S        → 设置系统状态 (S0-S5)
*                     $ACPI,SFAN,S          → 设置风扇速度 (0-100)
*                     $ACPI,SCI,E          → 触发 SCI 中断 (测试用)
*                     $ACPI,RESET          → 复位 EC
*
*                     事件通知 (EC → Host):
*                     $EVT,PWRBTN          → 电源按钮按下
*                     $EVT,LID_OPEN        → LID 打开
*                     $EVT,LID_CLOSE       → LID 关闭
*                     $EVT,BAT_LOW         → 电池低电量
*                     $EVT,BAT_CRIT        → 电池严重不足
*                     $EVT,THERM_WARN      → 温度警告
*                     $EVT,THERM_CRIT      → 温度严重超标
*                     $EVT,AC_IN           → AC 电源连接
*                     $EVT,AC_OUT          → AC 电源断开
*                     $EVT,WAKE            → 系统唤醒
*
********************************************************************************
* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONOND IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/

#ifndef __ACPI_H
#define __ACPI_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"

/* Private typedef -----------------------------------------------------------*/

/**
 * ACPI 命令码 (数字代号，便于解析)
 */
typedef enum {
    ACPI_CMD_VER            = 0x01,  /**< 获取固件版本 */
    ACPI_CMD_STATE          = 0x02,  /**< 获取电源状态 */
    ACPI_CMD_BAT            = 0x03,  /**< 获取电池信息 */
    ACPI_CMD_TEMP           = 0x04,  /**< 获取温度 */
    ACPI_CMD_LID            = 0x05,  /**< 获取 LID 状态 */
    ACPI_CMD_FAN            = 0x06,  /**< 获取风扇状态 */
    ACPI_CMD_EVENT          = 0x07,  /**< 获取待处理事件 */
    ACPI_CMD_SSTAT          = 0x10,  /**< 设置系统状态 */
    ACPI_CMD_SFAN           = 0x11,  /**< 设置风扇速度 */
    ACPI_CMD_SCI            = 0x12,  /**< 触发 SCI (测试) */
    ACPI_CMD_RESET          = 0xFF   /**< 复位 EC */
} ACPI_COMMAND_CODE;

/**
 * ACPI 系统状态 (S0-S5)
 */
typedef enum {
    ACPI_STATE_S0 = 0,   /**< 工作 */
    ACPI_STATE_S1 = 1,   /**< 浅睡眠 */
    ACPI_STATE_S3 = 3,   /**< 深度睡眠 (STR) */
    ACPI_STATE_S4 = 4,   /**< 冬眠 (STD) */
    ACPI_STATE_S5 = 5    /**< 软关机 */
} ACPI_SYSTEM_STATE;

/**
 * ACPI 事件码
 */
typedef enum {
    ACPI_EVT_NONE         = 0x00,
    ACPI_EVT_PWRBTN       = 0x01,  /**< 电源按钮 */
    ACPI_EVT_LID_OPEN     = 0x03,  /**< LID 打开 */
    ACPI_EVT_LID_CLOSE    = 0x04,  /**< LID 关闭 */
    ACPI_EVT_BAT_LOW      = 0x05,  /**< 电池低电量 */
    ACPI_EVT_BAT_CRIT     = 0x06,  /**< 电池严重不足 */
    ACPI_EVT_THERM_WARN   = 0x07,  /**< 温度警告 */
    ACPI_EVT_THERM_CRIT   = 0x08,  /**< 温度严重 */
    ACPI_EVT_AC_IN        = 0x09,  /**< AC 连接 */
    ACPI_EVT_AC_OUT       = 0x0A,  /**< AC 断开 */
    ACPI_EVT_WAKE         = 0x0B   /**< 唤醒 */
} ACPI_EVENT_CODE;

/**
 * ACPI 返回码
 */
typedef enum {
    ACPI_OK               = 0,    /**< 成功 */
    ACPI_ERR              = 1,    /**< 错误 */
    ACPI_ERR_NOSUP        = 2,    /**< 不支持 */
    ACPI_ERR_PARAM        = 3,    /**< 参数错误 */
    ACPI_ERR_BUSY         = 4     /**< 忙 */
} ACPI_RETURN_CODE;

/**
 * ACPI 设备信息
 */
typedef struct {
    uint8_t  ec_version[8];      /**< EC 版本字符串 */
    uint8_t  system_state;       /**< 系统状态 S0-S5 */
    uint8_t  device_state;       /**< 设备状态 D0-D3 */
    uint8_t  sci_pending;        /**< SCI 待处理标志 */
    uint8_t  sci_event;          /**< SCI 事件码 */
    uint16_t temperature;        /**< 温度 (0.1°C, 300=30.0°C) */
    uint8_t  battery_level;      /**< 电池电量 0-100% */
    uint8_t  battery_status;     /**< 电池状态 */
    uint8_t  lid_state;          /**< LID 状态 0=合上 1=打开 */
    uint8_t  fan_speed;          /**< 风扇速度 0-100% */
    uint8_t  ac_connected;       /**< AC 连接 0=断开 1=连接 */
    uint32_t uptime;            /**< 运行时间秒 */
} ACPI_DEVICE_INFO;

/* Private define ------------------------------------------------------------*/

#define ACPI_UART_TX_BUF_SIZE  128   /**< UART 发送缓冲区大小 */
#define ACPI_UART_RX_BUF_SIZE  128   /**< UART 接收缓冲区大小 */
#define ACPI_EVENT_QUEUE_SIZE   16    /**< 事件队列大小 */

/* Private variables ---------------------------------------------------------*/

extern ACPI_DEVICE_INFO gAcpiInfo;

/* Extern function prototypes -----------------------------------------------*/

/**
 * ACPI_Init - 初始化 ACPI 子系统
 */
void ACPI_Init(void);

/**
 * ACPI_Process - ACPI 主循环处理
 * 在主循环或定时器中调用
 */
void ACPI_Process(void);

/**
 * ACPI_UART_Init - 初始化 ACPI UART 通信
 * 使用现有的 UART_Init
 */
void ACPI_UART_Init(void);

/**
 * ACPI_HandleUartChar - 处理接收到的 UART 字符
 * @param c: 接收的字符
 * @return: 1 表示收到完整命令，0 表示还在接收中
 */
uint8_t ACPI_HandleUartChar(uint8_t c);

/**
 * ACPI_SendResponse - 通过 UART 发送响应
 * @param ret: 返回码
 * @param data: 数据指针
 * @param len: 数据长度
 */
void ACPI_SendResponse(uint8_t ret, uint8_t *data, uint8_t len);

/**
 * ACPI_SendEvent - 发送事件通知到 Host
 * @param event: 事件码
 * @param data: 事件数据 (可为 NULL)
 */
void ACPI_SendEvent(uint8_t event, uint8_t *data);

/**
 * ACPI_GetPendingEvent - 获取待处理事件
 * @return: 事件码，0 表示无事件
 */
uint8_t ACPI_GetPendingEvent(void);

/**
 * ACPI_IncrementUptime - 增加运行时间 (每秒调用)
 */
void ACPI_IncrementUptime(void);

/* 模拟传感器接口 -----------------------------------------------*/

/**
 * ACPI_SetTemperature - 设置温度值 (模拟)
 */
void ACPI_SetTemperature(uint16_t temp);

/**
 * ACPI_SetBattery - 设置电池状态
 */
void ACPI_SetBattery(uint8_t level, uint8_t status);

/**
 * ACPI_SetLid - 设置 LID 状态
 */
void ACPI_SetLid(uint8_t state);

/**
 * ACPI_SetAcConnected - 设置 AC 连接状态
 */
void ACPI_SetAcConnected(uint8_t connected);

#endif /* __ACPI_H */

/******************* (C) COPYRIGHT 2026 EC Simulation Framework *****END OF FILE****/
