/*******************************************************************************
* File Name          : acpi.c
* Author             : EC Simulation Framework
* Version            : V1.1.0
* Date               : 2026/04/16
* Description        : ACPI implementation using UART communication
*
*                     本模块实现完整的 ACPI 协议模拟:
*                     1. UART 命令-响应通信
*                     2. 电源状态管理 (S0-S5)
*                     3. SCI 事件通知机制
*                     4. 电池/温度/LID 状态监控
*                     5. 边缘检测事件触发
*
*                     UART 通信协议:
*                     - 波特率: 115200 8N1
*                     - 命令格式: $ACPI,CMD,PARAM...\r\n
*                     - 响应格式: $ACK,RET,DATA...\r\n
*                     - 事件格式: $EVT,EVENT,DATA...\r\n
*
*                     真实 EC 中的 ACPI 实现要点:
*                     1. EC 通过 eSPI/SMBus/LPC 与 Host 通信
*                     2. EC 维护 ACPI 寄存器/状态区
*                     3. 硬件事件触发 SCI 中断
*                     4. Host 调用 _Qxx 方法读取事件
*                     5. EC 返回事件数据给 Host
*
*                     本实现使用 UART 模拟上述过程，便于学习理解
*
********************************************************************************
* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "acpi.h"
#include "uart.h"
#include <string.h>
#include <stdio.h>

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/

#define ACPI_CMD_PREFIX      '$'     /**< 命令前缀 */
#define ACPI_CMD_DELIM      ','     /**< 参数分隔符 */
#define ACPI_CMD_TERMINATE  '\n'   /**< 命令结束符 */
#define ACPI_MAX_PARAM       8      /**< 最大参数数量 */

/* 温度阈值 */
#define TEMP_WARN_THRESH    600     /**< 60°C = 温度警告 */
#define TEMP_CRIT_THRESH    750     /**< 75°C = 温度严重 */

/* 电池阈值 */
#define BAT_LOW_THRESH       20      /**< 20% = 低电量 */
#define BAT_CRIT_THRESH      5      /**< 5% = 严重低电量 */

/* Private macro -------------------------------------------------------------*/
#define IS_DIGIT(c)          ((c) >= '0' && (c) <= '9')
#define IS_LETTER(c)         (((c) >= 'A' && (c) <= 'Z') || ((c) >= 'a' && (c) <= 'z'))

/* Private variables ---------------------------------------------------------*/

/**
 * ACPI 全局设备信息
 */
ACPI_DEVICE_INFO gAcpiInfo = {
    .ec_version      = "1.00",
    .system_state    = ACPI_STATE_S0,
    .device_state    = 0,
    .sci_pending     = 0,
    .sci_event       = 0,
    .temperature     = 300,     /**< 30.0°C */
    .battery_level   = 100,
    .battery_status = 0,
    .lid_state       = 1,       /**< 默认 LID 打开 */
    .fan_speed       = 50,
    .ac_connected    = 1,       /**< 默认 AC 连接 */
    .uptime          = 0
};

/**
 * UART 接收状态机
 */
typedef enum {
    ACPI_UART_IDLE,         /**< 等待 '$' */
    ACPI_UART_READ_CMD,     /**< 读取命令 */
    ACPI_UART_READ_PARAMS   /**< 读取参数 */
} ACPI_UART_STATE;

static ACPI_UART_STATE gUartState = ACPI_UART_IDLE;
static uint8_t gUartCmd[16];           /**< 命令缓冲区 */
static uint8_t gUartParam[ACPI_MAX_PARAM][16];  /**< 参数缓冲区 */
static uint8_t gUartCmdLen = 0;
static uint8_t gUartParamIdx = 0;
static uint8_t gUartParamLen = 0;
static uint8_t gUartSkipPrefix = 0;   /**< 是否跳过 ACPI 前缀 */
static uint8_t gUartCommand = 0;       /**< 解析出的命令码 */

/**
 * ACPI 事件队列
 */
static uint8_t gEventQueue[ACPI_EVENT_QUEUE_SIZE];
static uint8_t gEventHead = 0;
static uint8_t gEventTail = 0;

/**
 * 上次状态 (用于边缘检测)
 */
static uint8_t gLastTempWarn = 0;
static uint8_t gLastTempCrit = 0;
static uint8_t gLastBatLow = 0;
static uint8_t gLastBatCrit = 0;
static uint8_t gLastLid = 1;
static uint8_t gLastAc = 1;

/**
 * UART 发送缓冲区
 */
static char gTxBuf[ACPI_UART_TX_BUF_SIZE];

/* Private function prototypes -----------------------------------------------*/
static void ACPI_ParseCommand(void);
static void ACPI_ProcessGetVersion(void);
static void ACPI_ProcessGetState(void);
static void ACPI_ProcessGetBattery(void);
static void ACPI_ProcessGetTemp(void);
static void ACPI_ProcessGetLid(void);
static void ACPI_ProcessGetFan(void);
static void ACPI_ProcessGetEvent(void);
static void ACPI_ProcessSetState(uint8_t state);
static void ACPI_ProcessSetFan(uint8_t speed);
static void ACPI_ProcessTriggerSci(uint8_t event);
static void ACPI_ProcessReset(void);
static void ACPI_EnqueueEvent(uint8_t event);
static void ACPI_CheckAllEvents(void);

/* Private functions ---------------------------------------------------------*/

/*******************************************************************************
* Function Name  : ACPI_ParseCommand
* Description    : 解析并执行 UART 命令
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
static void ACPI_ParseCommand(void)
{
    /* 命令码在 gUartCommand 中 (有参数时) 或 gUartCmd[0] 中 (无参数时) */
    uint8_t cmd;
    if (gUartCommand != 0) {
        cmd = gUartCommand;  /* 有参数的命令 */
    } else {
        cmd = gUartCmd[0];        /* 无参数的命令 */
    }

    /* 调试输出收到的命令 */
    gTxBuf[0] = '\0';

    /* 根据命令码处理 */
    switch (cmd) {
        /* === 获取类命令 === */
        case 'V':  /* VER */
            ACPI_ProcessGetVersion();
            break;

        case 'S':  /* STATE */
            ACPI_ProcessGetState();
            break;

        case 'B':  /* BAT */
            ACPI_ProcessGetBattery();
            break;

        case 'T':  /* TEMP */
            ACPI_ProcessGetTemp();
            break;

        case 'L':  /* LID */
            ACPI_ProcessGetLid();
            break;

        case 'F':  /* FAN */
            ACPI_ProcessGetFan();
            break;

        case 'E':  /* EVENT */
            ACPI_ProcessGetEvent();
            break;

        /* === 设置类命令 === */
        case 'W':  /* SSTAT - 写入状态 */
            if (gUartParamLen > 0) {
                ACPI_ProcessSetState(gUartParam[0][0] - '0');
            } else {
                ACPI_SendResponse(ACPI_ERR_PARAM, NULL, 0);
            }
            break;

        case 'D':  /* SFAN - 设置风扇 */
            if (gUartParamLen > 0) {
                uint8_t speed = 0;
                /* 解析速度值 (可能是多位数字) */
                for (uint8_t i = 0; i < strlen((char*)gUartParam[0]); i++) {
                    speed = speed * 10 + (gUartParam[0][i] - '0');
                }
                ACPI_ProcessSetFan(speed);
            } else {
                ACPI_SendResponse(ACPI_ERR_PARAM, NULL, 0);
            }
            break;

        case 'I':  /* SCI - 触发 SCI */
            if (gUartParamLen > 0) {
                ACPI_ProcessTriggerSci(gUartParam[0][0] - '0');
            } else {
                ACPI_SendResponse(ACPI_ERR_PARAM, NULL, 0);
            }
            break;

        case 'R':  /* RESET */
            ACPI_ProcessReset();
            break;

        default:
            ACPI_SendResponse(ACPI_ERR_NOSUP, NULL, 0);
            break;
    }
}

/*******************************************************************************
* Function Name  : ACPI_ProcessGetVersion
* Description    : 处理 VER 命令 - 获取固件版本
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
static void ACPI_ProcessGetVersion(void)
{
    uint8_t data[16];
    uint8_t len = strlen((char*)gAcpiInfo.ec_version);
    memcpy(data, gAcpiInfo.ec_version, len);
    ACPI_SendResponse(ACPI_OK, data, len);
}

/*******************************************************************************
* Function Name  : ACPI_ProcessGetState
* Description    : 处理 STATE 命令 - 获取电源状态
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
static void ACPI_ProcessGetState(void)
{
    uint8_t data[4];
    data[0] = gAcpiInfo.system_state;    /* S0-S5 */
    data[1] = gAcpiInfo.device_state;     /* D0-D3 */
    data[2] = gAcpiInfo.sci_pending;     /* SCI 标志 */
    data[3] = gAcpiInfo.sci_event;       /* 事件码 */
    ACPI_SendResponse(ACPI_OK, data, 4);
}

/*******************************************************************************
* Function Name  : ACPI_ProcessGetBattery
* Description    : 处理 BAT 命令 - 获取电池信息
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
static void ACPI_ProcessGetBattery(void)
{
    uint8_t data[4];
    data[0] = gAcpiInfo.battery_level;    /* 电量 0-100% */
    data[1] = gAcpiInfo.battery_status;  /* 状态 */
    data[2] = gAcpiInfo.ac_connected;    /* AC 连接状态 */
    data[3] = (gAcpiInfo.battery_level <= BAT_LOW_THRESH) ? 1 : 0;  /* 是否低电量 */
    ACPI_SendResponse(ACPI_OK, data, 4);
}

/*******************************************************************************
* Function Name  : ACPI_ProcessGetTemp
* Description    : 处理 TEMP 命令 - 获取温度
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
static void ACPI_ProcessGetTemp(void)
{
    /* 温度单位是 0.1°C，300 = 30.0°C */
    uint16_t temp = gAcpiInfo.temperature;
    uint8_t data[8];
    /* 格式化为字符串：如 "300" 表示 30.0°C */
    uint8_t len = sprintf((char*)data, "%u.%u", temp / 10, temp % 10);
    ACPI_SendResponse(ACPI_OK, data, len);
}

/*******************************************************************************
* Function Name  : ACPI_ProcessGetLid
* Description    : 处理 LID 命令 - 获取 LID 状态
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
static void ACPI_ProcessGetLid(void)
{
    uint8_t data[1];
    data[0] = gAcpiInfo.lid_state;  /* 0=合上, 1=打开 */
    ACPI_SendResponse(ACPI_OK, data, 1);
}

/*******************************************************************************
* Function Name  : ACPI_ProcessGetFan
* Description    : 处理 FAN 命令 - 获取风扇状态
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
static void ACPI_ProcessGetFan(void)
{
    uint8_t data[1];
    data[0] = gAcpiInfo.fan_speed;  /* 0-100% */
    ACPI_SendResponse(ACPI_OK, data, 1);
}

/*******************************************************************************
* Function Name  : ACPI_ProcessGetEvent
* Description    : 处理 EVENT 命令 - 获取待处理事件
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
static void ACPI_ProcessGetEvent(void)
{
    uint8_t event = ACPI_GetPendingEvent();
    uint8_t data[2];
    data[0] = gAcpiInfo.sci_pending;  /* 是否有事件 */
    data[1] = event;                  /* 事件码 */
    ACPI_SendResponse(ACPI_OK, data, 2);
}

/*******************************************************************************
* Function Name  : ACPI_ProcessSetState
* Description    : 处理 SSTAT 命令 - 设置系统状态
* Input          : state - 目标状态 (S0=0, S3=3, S5=5)
* Output         : None
* Return         : None
*******************************************************************************/
static void ACPI_ProcessSetState(uint8_t state)
{
    if (state > ACPI_STATE_S5) {
        ACPI_SendResponse(ACPI_ERR_PARAM, NULL, 0);
        return;
    }

    gAcpiInfo.system_state = state;

    /* 如果进入 S3/S4/S5，发送 Wake 事件 */
    if (state >= ACPI_STATE_S3) {
        /* 模拟进入睡眠 */
        sprintf(gTxBuf, "System entering S%d\r\n", state);
        UART_SendString(gTxBuf);
    }

    ACPI_SendResponse(ACPI_OK, NULL, 0);
}

/*******************************************************************************
* Function Name  : ACPI_ProcessSetFan
* Description    : 处理 SFAN 命令 - 设置风扇速度
* Input          : speed - 速度 0-100
* Output         : None
* Return         : None
*******************************************************************************/
static void ACPI_ProcessSetFan(uint8_t speed)
{
    if (speed > 100) {
        ACPI_SendResponse(ACPI_ERR_PARAM, NULL, 0);
        return;
    }

    gAcpiInfo.fan_speed = speed;
    ACPI_SendResponse(ACPI_OK, NULL, 0);
}

/*******************************************************************************
* Function Name  : ACPI_ProcessTriggerSci
* Description    : 处理 SCI 命令 - 手动触发 SCI (测试用)
* Input          : event - 事件码
* Output         : None
* Return         : None
*******************************************************************************/
static void ACPI_ProcessTriggerSci(uint8_t event)
{
    ACPI_SendEvent(event, NULL);
    ACPI_SendResponse(ACPI_OK, NULL, 0);
}

/*******************************************************************************
* Function Name  : ACPI_ProcessReset
* Description    : 处理 RESET 命令 - 复位 EC
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
static void ACPI_ProcessReset(void)
{
    /* 重置状态 */
    gAcpiInfo.system_state = ACPI_STATE_S0;
    gAcpiInfo.device_state = 0;
    gAcpiInfo.sci_pending = 0;
    gAcpiInfo.sci_event = 0;
    gAcpiInfo.fan_speed = 50;
    gAcpiInfo.battery_level = 100;
    gAcpiInfo.battery_status = 0;

    /* 清空事件队列 */
    gEventHead = 0;
    gEventTail = 0;

    ACPI_SendResponse(ACPI_OK, NULL, 0);

    /* 发送就绪事件 */
    ACPI_SendEvent(ACPI_EVT_PWRBTN, NULL);
}

/*******************************************************************************
* Function Name  : ACPI_EnqueueEvent
* Description    : 将事件加入队列
* Input          : event - 事件码
* Output         : None
* Return         : None
*******************************************************************************/
static void ACPI_EnqueueEvent(uint8_t event)
{
    uint8_t next = (gEventHead + 1) % ACPI_EVENT_QUEUE_SIZE;
    if (next != gEventTail) {
        gEventQueue[gEventHead] = event;
        gEventHead = next;
        gAcpiInfo.sci_pending = 1;
        gAcpiInfo.sci_event = event;
    }
}

/*******************************************************************************
* Function Name  : ACPI_CheckAllEvents
* Description    : 检查所有传感器事件 (边缘检测)
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
static void ACPI_CheckAllEvents(void)
{
    /* === 温度事件检测 === */
    uint8_t now_warn = (gAcpiInfo.temperature >= TEMP_WARN_THRESH) ? 1 : 0;
    uint8_t now_crit = (gAcpiInfo.temperature >= TEMP_CRIT_THRESH) ? 1 : 0;

    if (now_warn && !gLastTempWarn) {
        ACPI_SendEvent(ACPI_EVT_THERM_WARN, NULL);
    }
    if (now_crit && !gLastTempCrit) {
        ACPI_SendEvent(ACPI_EVT_THERM_CRIT, NULL);
    }
    gLastTempWarn = now_warn;
    gLastTempCrit = now_crit;

    /* === 电池事件检测 === */
    uint8_t now_low = (gAcpiInfo.battery_level <= BAT_LOW_THRESH) ? 1 : 0;
    uint8_t now_crit_bat = (gAcpiInfo.battery_level <= BAT_CRIT_THRESH) ? 1 : 0;

    if (now_low && !gLastBatLow) {
        ACPI_SendEvent(ACPI_EVT_BAT_LOW, NULL);
    }
    if (now_crit_bat && !gLastBatCrit) {
        ACPI_SendEvent(ACPI_EVT_BAT_CRIT, NULL);
    }
    gLastBatLow = now_low;
    gLastBatCrit = now_crit_bat;

    /* === LID 事件检测 === */
    if (gAcpiInfo.lid_state != gLastLid) {
        if (gAcpiInfo.lid_state == 0) {
            ACPI_SendEvent(ACPI_EVT_LID_CLOSE, NULL);
        } else {
            ACPI_SendEvent(ACPI_EVT_LID_OPEN, NULL);
        }
        gLastLid = gAcpiInfo.lid_state;
    }

    /* === AC 连接事件检测 === */
    if (gAcpiInfo.ac_connected != gLastAc) {
        if (gAcpiInfo.ac_connected) {
            ACPI_SendEvent(ACPI_EVT_AC_IN, NULL);
        } else {
            ACPI_SendEvent(ACPI_EVT_AC_OUT, NULL);
        }
        gLastAc = gAcpiInfo.ac_connected;
    }
}

/* Extern function prototypes ------------------------------------------------*/

/*******************************************************************************
* Function Name  : ACPI_Init
* Description    : ACPI 子系统初始化
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void ACPI_Init(void)
{
    /* 清空事件队列 */
    gEventHead = 0;
    gEventTail = 0;

    /* 重置 SCI 状态 */
    gAcpiInfo.sci_pending = 0;
    gAcpiInfo.sci_event = 0;

    /* 重置边缘检测状态 */
    gLastTempWarn = 0;
    gLastTempCrit = 0;
    gLastBatLow = 0;
    gLastBatCrit = 0;
    gLastLid = gAcpiInfo.lid_state;
    gLastAc = gAcpiInfo.ac_connected;

    /* 重置 UART 状态机 */
    gUartState = ACPI_UART_IDLE;
    gUartCmdLen = 0;
    gUartParamIdx = 0;
    gUartParamLen = 0;

    /* 发送就绪通知 */
    sprintf(gTxBuf, "\r\nACPI initialized, EC v%s ready\r\n", gAcpiInfo.ec_version);
    UART_SendString(gTxBuf);
}

/*******************************************************************************
* Function Name  : ACPI_Process
* Description    : ACPI 主循环处理
*                  在主循环或定时器中定期调用
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void ACPI_Process(void)
{
    /* 检查各传感器事件 */
    ACPI_CheckAllEvents();
}

/*******************************************************************************
* Function Name  : ACPI_UART_Init
* Description    : ACPI UART 初始化
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void ACPI_UART_Init(void)
{
    ACPI_Init();
}

/*******************************************************************************
* Function Name  : ACPI_HandleUartChar
* Description    : 处理接收到的 UART 字符
*                  实现命令解析状态机
*
*                  协议格式:
*                  $ACPI,CMD,PARAM1,PARAM2...\r\n
*
* Input          : c - 接收的字符
* Output         : None
* Return         : 1 表示收到完整命令并处理，0 表示还在接收中
*******************************************************************************/
uint8_t ACPI_HandleUartChar(uint8_t c)
{
    static uint8_t param_char_idx = 0;

    switch (gUartState) {
        case ACPI_UART_IDLE:
            if (c == ACPI_CMD_PREFIX) {
                /* 收到命令前缀，开始接收命令 */
                gUartState = ACPI_UART_READ_CMD;
                gUartCmdLen = 0;
                gUartParamIdx = 0;
                gUartParamLen = 0;
                gUartSkipPrefix = 0;
                gUartCommand = 0;
                param_char_idx = 0;
            }
            break;

        case ACPI_UART_READ_CMD:
            if (c == ACPI_CMD_DELIM) {
                if (!gUartSkipPrefix) {
                    /* 第一次遇到逗号 - 跳过 $ACPI, 前缀 */
                    gUartSkipPrefix = 1;
                    gUartCmdLen = 0;
                    /* 继续保持在 READ_CMD 状态，接收实际命令 */
                } else {
                    /* 第二次遇到逗号 - 命令结束，开始接收参数 */
                    gUartCmd[gUartCmdLen] = '\0';
                    gUartCommand = gUartCmd[0];  /* 保存命令码 */
                    gUartState = ACPI_UART_READ_PARAMS;
                    gUartParamIdx = 0;
                    gUartParamLen = 0;
                    param_char_idx = 0;
                }
            } else if (c == ACPI_CMD_TERMINATE || c == '\r') {
                /* 命令立即结束 (无参数) - 先保存命令码，再清缓冲区 */
                gUartCommand = gUartCmd[0];  /* 先保存命令字符 */
                gUartCmd[gUartCmdLen] = '\0';
                gUartParamLen = 0;
                gUartState = ACPI_UART_IDLE;
                ACPI_ParseCommand();
                return 1;
            } else {
                /* 继续接收命令字符 */
                if (gUartCmdLen < 15) {
                    gUartCmd[gUartCmdLen++] = c;
                }
            }
            break;

        case ACPI_UART_READ_PARAMS:
            if (c == ACPI_CMD_DELIM) {
                /* 参数结束，存储参数，开始下一个参数 */
                gUartParam[gUartParamIdx][param_char_idx] = '\0';
                gUartParamIdx++;
                param_char_idx = 0;
                if (gUartParamIdx >= ACPI_MAX_PARAM) {
                    gUartParamIdx = ACPI_MAX_PARAM - 1;
                }
            } else if (c == ACPI_CMD_TERMINATE || c == '\r') {
                /* 命令结束 */
                gUartParam[gUartParamIdx][param_char_idx] = '\0';
                gUartParamLen = gUartParamIdx + 1;
                gUartState = ACPI_UART_IDLE;
                ACPI_ParseCommand();
                return 1;
            } else {
                /* 继续接收参数字符 */
                if (param_char_idx < 15) {
                    gUartParam[gUartParamIdx][param_char_idx++] = c;
                }
            }
            break;
    }

    return 0;
}

/*******************************************************************************
* Function Name  : ACPI_SendResponse
* Description    : 发送响应到 UART
*                  格式: $ACK,RET,DATA...\r\n
* Input          : ret - 返回码
*                  data - 数据
*                  len - 数据长度
* Output         : None
* Return         : None
*******************************************************************************/
void ACPI_SendResponse(uint8_t ret, uint8_t *data, uint8_t len)
{
    uint8_t i, pos = 0;

    /* $ACK, */
    gTxBuf[pos++] = '$';
    gTxBuf[pos++] = 'A';
    gTxBuf[pos++] = 'C';
    gTxBuf[pos++] = 'K';
    gTxBuf[pos++] = ',';
    gTxBuf[pos++] = '0' + ret;  /* 返回码转为字符 */
    gTxBuf[pos++] = ',';

    /* 数据 - 字符串直接文本输出，二进制数据用十六进制 */
    /* 判断是否为可打印字符串（用于版本等字符串数据） */
    uint8_t is_printable_string = 1;
    for (i = 0; i < len; i++) {
        if (data[i] < 0x20 || data[i] > 0x7E) {
            is_printable_string = 0;
            break;
        }
    }

    if (is_printable_string && len > 1) {
        /* 字符串：直接连续输出，不加分隔符 */
        for (i = 0; i < len; i++) {
            gTxBuf[pos++] = data[i];
        }
    } else {
        /* 二进制数值：每个字节用逗号分隔 */
        for (i = 0; i < len; i++) {
            if (data[i] >= 0x20 && data[i] <= 0x7E) {
                gTxBuf[pos++] = data[i];
            } else {
                /* 非可打印字符用十六进制表示 */
                uint8_t nibble_h = (data[i] >> 4) & 0x0F;
                uint8_t nibble_l = data[i] & 0x0F;
                gTxBuf[pos++] = (nibble_h < 10) ? ('0' + nibble_h) : ('A' + nibble_h - 10);
                gTxBuf[pos++] = (nibble_l < 10) ? ('0' + nibble_l) : ('A' + nibble_l - 10);
            }
            if (i < len - 1) {
                gTxBuf[pos++] = ',';
            }
        }
    }

    /* \r\n */
    gTxBuf[pos++] = '\r';
    gTxBuf[pos++] = '\n';
    gTxBuf[pos++] = '\0';  /* \0放最后 */

    UART_SendString(gTxBuf);
}

/*******************************************************************************
* Function Name  : ACPI_SendEvent
* Description    : 发送事件通知到 Host
*                  格式: $EVT,EVENT,DATA...\r\n
* Input          : event - 事件码
*                  data - 事件数据 (可为 NULL)
* Output         : None
* Return         : None
*******************************************************************************/
void ACPI_SendEvent(uint8_t event, uint8_t *data)
{
    uint8_t i, pos = 0;
    const char *evt_name = "UNK";

    /* 事件名称映射 */
    switch (event) {
        case ACPI_EVT_PWRBTN:   evt_name = "PWRBTN"; break;
        case ACPI_EVT_LID_OPEN: evt_name = "LID_OPEN"; break;
        case ACPI_EVT_LID_CLOSE: evt_name = "LID_CLOSE"; break;
        case ACPI_EVT_BAT_LOW:  evt_name = "BAT_LOW"; break;
        case ACPI_EVT_BAT_CRIT: evt_name = "BAT_CRIT"; break;
        case ACPI_EVT_THERM_WARN: evt_name = "THERM_WARN"; break;
        case ACPI_EVT_THERM_CRIT: evt_name = "THERM_CRIT"; break;
        case ACPI_EVT_AC_IN:    evt_name = "AC_IN"; break;
        case ACPI_EVT_AC_OUT:   evt_name = "AC_OUT"; break;
        case ACPI_EVT_WAKE:     evt_name = "WAKE"; break;
    }

    /* $EVT, */
    gTxBuf[pos++] = '$';
    gTxBuf[pos++] = 'E';
    gTxBuf[pos++] = 'V';
    gTxBuf[pos++] = 'T';
    gTxBuf[pos++] = ',';
    gTxBuf[pos++] = event;  /* 事件码作为字符 */
    gTxBuf[pos++] = ',';
    gTxBuf[pos++] = '(';
    gTxBuf[pos++] = '0' + event;  /* 十六进制表示 */
    gTxBuf[pos++] = ')';
    gTxBuf[pos++] = ',';
    gTxBuf[pos++] = '"';
    for (i = 0; evt_name[i]; i++) {
        gTxBuf[pos++] = evt_name[i];
    }
    gTxBuf[pos++] = '"';

    if (data != NULL) {
        gTxBuf[pos++] = ',';
        gTxBuf[pos++] = '0';
        gTxBuf[pos++] = 'x';
        /* 数据十六进制 */
        gTxBuf[pos++] = '0' + (data[0] >> 4);
        gTxBuf[pos++] = '0' + (data[0] & 0x0F);
    }

    /* \r\n */
    gTxBuf[pos++] = '\r';
    gTxBuf[pos++] = '\n';
    gTxBuf[pos++] = '\0';

    UART_SendString(gTxBuf);

    /* 加入事件队列 */
    ACPI_EnqueueEvent(event);
}

/*******************************************************************************
* Function Name  : ACPI_GetPendingEvent
* Description    : 获取待处理事件 (从队列取出)
* Input          : None
* Output         : None
* Return         : 事件码，0 表示无事件
*******************************************************************************/
uint8_t ACPI_GetPendingEvent(void)
{
    if (gEventHead == gEventTail) {
        return 0;  /* 队列空 */
    }

    uint8_t event = gEventQueue[gEventTail];
    gEventTail = (gEventTail + 1) % ACPI_EVENT_QUEUE_SIZE;

    if (gEventHead == gEventTail) {
        gAcpiInfo.sci_pending = 0;
        gAcpiInfo.sci_event = 0;
    }

    return event;
}

/*******************************************************************************
* Function Name  : ACPI_IncrementUptime
* Description    : 增加运行时间计数
*                  应由定时器每秒调用一次
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void ACPI_IncrementUptime(void)
{
    gAcpiInfo.uptime++;
}

/* 模拟传感器接口 -----------------------------------------------*/

/**
 * ACPI_SetTemperature - 设置温度值
 * 用于模拟测试，正常由 NTC 传感器读取
 */
void ACPI_SetTemperature(uint16_t temp)
{
    gAcpiInfo.temperature = temp;
}

/**
 * ACPI_SetBattery - 设置电池状态
 */
void ACPI_SetBattery(uint8_t level, uint8_t status)
{
    gAcpiInfo.battery_level = level;
    gAcpiInfo.battery_status = status;
}

/**
 * ACPI_SetLid - 设置 LID 状态
 */
void ACPI_SetLid(uint8_t state)
{
    gAcpiInfo.lid_state = state ? 1 : 0;
}

/**
 * ACPI_SetAcConnected - 设置 AC 连接状态
 */
void ACPI_SetAcConnected(uint8_t connected)
{
    gAcpiInfo.ac_connected = connected ? 1 : 0;
}

/******************* (C) COPYRIGHT 2026 EC Simulation Framework *****END OF FILE****/
