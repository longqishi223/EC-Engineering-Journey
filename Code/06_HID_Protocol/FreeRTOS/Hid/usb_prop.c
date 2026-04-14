/******************** (C) COPYRIGHT 2010 STMicroelectronics ********************
* File Name          : usb_prop.c
* Author             : MCD Application Team
* Version            : V3.2.1
* Date               : 07/05/2010
* Description        : All processings related to CustomHID Mouse Demo
********************************************************************************
* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "usb_lib.h"
#include "usb_conf.h"
#include "usb_prop.h"
#include "usb_desc.h"
#include "usb_pwr.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
uint32_t ProtocolValue;

/* -------------------------------------------------------------------------- */
/*  Structures initializations */
/* -------------------------------------------------------------------------- */

DEVICE Device_Table =
{
    EP_NUM,
    1
};

DEVICE_PROP Device_Property =
{
    CustomHID_init,
    CustomHID_Reset,
    CustomHID_Status_In,
    CustomHID_Status_Out,
    CustomHID_Data_Setup,
    CustomHID_NoData_Setup,
    CustomHID_Get_Interface_Setting,
    CustomHID_GetDeviceDescriptor,
    CustomHID_GetConfigDescriptor,
    CustomHID_GetStringDescriptor,
    0,
    0x40 /*MAX PACKET SIZE*/
};
USER_STANDARD_REQUESTS User_Standard_Requests =
{
    CustomHID_GetConfiguration,
    CustomHID_SetConfiguration,
    CustomHID_GetInterface,
    CustomHID_SetInterface,
    CustomHID_GetStatus,
    CustomHID_ClearFeature,
    CustomHID_SetEndPointFeature,
    CustomHID_SetDeviceFeature,
    CustomHID_SetDeviceAddress
};

ONE_DESCRIPTOR Device_Descriptor =
{
    (uint8_t *)CustomHID_DeviceDescriptor,
    CUSTOMHID_SIZ_DEVICE_DESC
};

ONE_DESCRIPTOR Config_Descriptor =
{
    (uint8_t *)CustomHID_ConfigDescriptor,
    CUSTOMHID_SIZ_CONFIG_DESC
};

ONE_DESCRIPTOR CustomHID_Report_Descriptor =
{
    (uint8_t *)CustomHID_ReportDescriptor,
    CUSTOMHID_SIZ_REPORT_DESC
};

ONE_DESCRIPTOR CustomHID_Descriptor =
{
    (uint8_t *)CustomHID_ConfigDescriptor + CUSTOMHID_OFF_HID_DESC,
    CUSTOMHID_SIZ_HID_DESC
};

ONE_DESCRIPTOR String_Descriptor[4] =
{
    {(uint8_t *)CustomHID_StringLangID, CUSTOMHID_SIZ_STRING_LANGID},
    {(uint8_t *)CustomHID_StringVendor, CUSTOMHID_SIZ_STRING_VENDOR},
    {(uint8_t *)CustomHID_StringProduct, CUSTOMHID_SIZ_STRING_PRODUCT},
    {(uint8_t *)CustomHID_StringSerial, CUSTOMHID_SIZ_STRING_SERIAL}
};

/* Extern variables ----------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Extern function prototypes ------------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/*******************************************************************************
* Function Name  : HexToChar.
* Description    : Convert Hex 32Bits value into char.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
static void IntToUnicode (uint32_t value, uint8_t *pbuf, uint8_t len)
{
    uint8_t idx = 0;

    for( idx = 0 ; idx < len ; idx ++)
    {
        if( ((value >> 28)) < 0xA )
        {
            pbuf[ 2 * idx] = (value >> 28) + '0';
        }
        else
        {
            pbuf[2 * idx] = (value >> 28) + 'A' - 10;
        }

        value = value << 4;

        pbuf[ 2 * idx + 1] = 0;
    }
}
/*******************************************************************************
* Function Name  : Get_SerialNum.
* Description    : Create the serial number string descriptor.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void Get_SerialNum(void)
{
    uint32_t Device_Serial0, Device_Serial1, Device_Serial2;

    Device_Serial0 = *(__IO uint32_t *)(0x1FFFF7E8);
    Device_Serial1 = *(__IO uint32_t *)(0x1FFFF7EC);
    Device_Serial2 = *(__IO uint32_t *)(0x1FFFF7F0);

    Device_Serial0 += Device_Serial2;

    if (Device_Serial0 != 0)
    {
        IntToUnicode (Device_Serial0, &CustomHID_StringSerial[2], 8);
        IntToUnicode (Device_Serial1, &CustomHID_StringSerial[18], 4);
    }
}
/*******************************************************************************
* Function Name  : CustomHID_init.
* Description    : CustomHID Mouse init routine.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void CustomHID_init(void)
{

    /* Update the serial number string descriptor with the data from the unique
    ID*/
    Get_SerialNum();

    pInformation->Current_Configuration = 0;
    /* Connect the device */
    PowerOn();

    /* Perform basic device initialization operations */
    USB_SIL_Init();

    bDeviceState = UNCONNECTED;
}

/*******************************************************************************
* Function Name  : CustomHID_Reset.
* Description    : CustomHID Mouse reset routine.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void CustomHID_Reset(void)
{
    /* Set CustomHID_DEVICE as not configured */
    pInformation->Current_Configuration = 0;
    pInformation->Current_Interface = 0;/*the default Interface*/
    pInformation->Current_Feature = CustomHID_ConfigDescriptor[7];

    SetBTABLE(BTABLE_ADDRESS);

    /* Initialize Endpoint 0 (默认控制端点，不动它) */
    SetEPType(ENDP0, EP_CONTROL);
    SetEPTxStatus(ENDP0, EP_TX_STALL);
    SetEPRxAddr(ENDP0, ENDP0_RXADDR);
    SetEPTxAddr(ENDP0, ENDP0_TXADDR);
    Clear_Status_Out(ENDP0);
    SetEPRxCount(ENDP0, Device_Property.MaxPacketSize);
    SetEPRxValid(ENDP0);

    /* ====== 修改这里！初始化 Endpoint 1 作为键盘发送端点 ====== */
    SetEPType(ENDP1, EP_INTERRUPT);    // 中断端点
    SetEPTxAddr(ENDP1, ENDP1_TXADDR);  // 设置发送缓冲区地址 (注意这里变成了 Tx)
    SetEPTxCount(ENDP1, 8);            // 键盘报文规定是 8 个字节
    SetEPRxStatus(ENDP1, EP_RX_DIS);   // 禁用接收 (键盘不需要一直收数据)
    SetEPTxStatus(ENDP1, EP_TX_NAK);   // 默认状态设为 NAK (有数据时再激活)
    /* ========================================================= */

    // 把原来关于 ENDP2 的初始化全部删掉！键盘用不到 EP2

    bDeviceState = ATTACHED;
    /* Set this device to response on default address */
    SetDeviceAddress(0);
}

/*******************************************************************************
* Function Name  : CustomHID_SetConfiguration.
* Description    : Udpade the device state to configured.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void CustomHID_SetConfiguration(void)
{
    DEVICE_INFO *pInfo = &Device_Info;

    if (pInfo->Current_Configuration != 0)
    {
        /* Device configured */
        bDeviceState = CONFIGURED;
    }
}
/*******************************************************************************
* Function Name  : CustomHID_SetConfiguration.
* Description    : Udpade the device state to addressed.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void CustomHID_SetDeviceAddress (void)
{
    bDeviceState = ADDRESSED;
}
/*******************************************************************************
* Function Name  : CustomHID_Status_In.
* Description    : CustomHID status IN routine.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void CustomHID_Status_In(void)
{}

/*******************************************************************************
* Function Name  : CustomHID_Status_Out
* Description    : CustomHID status OUT routine.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void CustomHID_Status_Out(void)
{}

/*******************************************************************************
* Function Name  : CustomHID_Data_Setup
* Description    : Handle the data class specific requests.
* Input          : Request Nb.
* Output         : None.
* Return         : USB_UNSUPPORT or USB_SUCCESS.
*******************************************************************************/
RESULT CustomHID_Data_Setup(uint8_t RequestNo)
{
    uint8_t *(*CopyRoutine)(uint16_t);

    CopyRoutine = NULL;
    if ((RequestNo == GET_DESCRIPTOR)
            && (Type_Recipient == (STANDARD_REQUEST | INTERFACE_RECIPIENT))
            && (pInformation->USBwIndex0 == 0))
    {

        if (pInformation->USBwValue1 == REPORT_DESCRIPTOR)
        {
            CopyRoutine = CustomHID_GetReportDescriptor;
        }
        else if (pInformation->USBwValue1 == HID_DESCRIPTOR_TYPE)
        {
            CopyRoutine = CustomHID_GetHIDDescriptor;
        }

    } /* End of GET_DESCRIPTOR */

    /*** GET_PROTOCOL ***/
    else if ((Type_Recipient == (CLASS_REQUEST | INTERFACE_RECIPIENT))
             && RequestNo == GET_PROTOCOL)
    {
        CopyRoutine = CustomHID_GetProtocolValue;
    }


    if (CopyRoutine == NULL)
    {
        return USB_UNSUPPORT;
    }

    pInformation->Ctrl_Info.CopyData = CopyRoutine;
    pInformation->Ctrl_Info.Usb_wOffset = 0;
    (*CopyRoutine)(0);
    return USB_SUCCESS;
}

/*******************************************************************************
* Function Name  : CustomHID_NoData_Setup
* Description    : handle the no data class specific requests
* Input          : Request Nb.
* Output         : None.
* Return         : USB_UNSUPPORT or USB_SUCCESS.
*******************************************************************************/
RESULT CustomHID_NoData_Setup(uint8_t RequestNo)
{
    if ((Type_Recipient == (CLASS_REQUEST | INTERFACE_RECIPIENT))
            && (RequestNo == SET_PROTOCOL))
    {
        return CustomHID_SetProtocol();
    }

    else
    {
        return USB_UNSUPPORT;
    }
}

/*******************************************************************************
* Function Name  : CustomHID_GetDeviceDescriptor.
* Description    : Gets the device descriptor.
* Input          : Length
* Output         : None.
* Return         : The address of the device descriptor.
*******************************************************************************/
uint8_t *CustomHID_GetDeviceDescriptor(uint16_t Length)
{
    return Standard_GetDescriptorData(Length, &Device_Descriptor);
}

/*******************************************************************************
* Function Name  : CustomHID_GetConfigDescriptor.
* Description    : Gets the configuration descriptor.
* Input          : Length
* Output         : None.
* Return         : The address of the configuration descriptor.
*******************************************************************************/
uint8_t *CustomHID_GetConfigDescriptor(uint16_t Length)
{
    return Standard_GetDescriptorData(Length, &Config_Descriptor);
}

/*******************************************************************************
* Function Name  : CustomHID_GetStringDescriptor
* Description    : Gets the string descriptors according to the needed index
* Input          : Length
* Output         : None.
* Return         : The address of the string descriptors.
*******************************************************************************/
uint8_t *CustomHID_GetStringDescriptor(uint16_t Length)
{
    uint8_t wValue0 = pInformation->USBwValue0;
    if (wValue0 > 4)
    {
        return NULL;
    }
    else
    {
        return Standard_GetDescriptorData(Length, &String_Descriptor[wValue0]);
    }
}

/*******************************************************************************
* Function Name  : CustomHID_GetReportDescriptor.
* Description    : Gets the HID report descriptor.
* Input          : Length
* Output         : None.
* Return         : The address of the configuration descriptor.
*******************************************************************************/
uint8_t *CustomHID_GetReportDescriptor(uint16_t Length)
{
    return Standard_GetDescriptorData(Length, &CustomHID_Report_Descriptor);
}

/*******************************************************************************
* Function Name  : CustomHID_GetHIDDescriptor.
* Description    : Gets the HID descriptor.
* Input          : Length
* Output         : None.
* Return         : The address of the configuration descriptor.
*******************************************************************************/
uint8_t *CustomHID_GetHIDDescriptor(uint16_t Length)
{
    return Standard_GetDescriptorData(Length, &CustomHID_Descriptor);
}

/*******************************************************************************
* Function Name  : CustomHID_Get_Interface_Setting.
* Description    : tests the interface and the alternate setting according to the
*                  supported one.
* Input          : - Interface : interface number.
*                  - AlternateSetting : Alternate Setting number.
* Output         : None.
* Return         : USB_SUCCESS or USB_UNSUPPORT.
*******************************************************************************/
RESULT CustomHID_Get_Interface_Setting(uint8_t Interface, uint8_t AlternateSetting)
{
    if (AlternateSetting > 0)
    {
        return USB_UNSUPPORT;
    }
    else if (Interface > 0)
    {
        return USB_UNSUPPORT;
    }
    return USB_SUCCESS;
}

/*******************************************************************************
* Function Name  : CustomHID_SetProtocol
* Description    : CustomHID Set Protocol request routine.
* Input          : None.
* Output         : None.
* Return         : USB SUCCESS.
*******************************************************************************/
RESULT CustomHID_SetProtocol(void)
{
    uint8_t wValue0 = pInformation->USBwValue0;
    ProtocolValue = wValue0;
    return USB_SUCCESS;
}

/*******************************************************************************
* Function Name  : CustomHID_GetProtocolValue
* Description    : get the protocol value
* Input          : Length.
* Output         : None.
* Return         : address of the protcol value.
*******************************************************************************/
uint8_t *CustomHID_GetProtocolValue(uint16_t Length)
{
    if (Length == 0)
    {
        pInformation->Ctrl_Info.Usb_wLength = 1;
        return NULL;
    }
    else
    {
        return (uint8_t *)(&ProtocolValue);
    }
}

/******************* (C) COPYRIGHT 2010 STMicroelectronics *****END OF FILE****/
