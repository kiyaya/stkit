/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// usbh_printer.h - USB host printer class implementation
//
// Author: 185275258 (QQ Group)
//
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/


#ifndef __USBH_PRINTER_H
#define __USBH_PRINTER_H

#ifdef __cplusplus
 extern "C" {
#endif

#include "usbh_core.h"

extern USBH_ClassTypeDef    USBH_PRINTER_Class;
#define USBH_PRINTER_CLASS  &USBH_PRINTER_Class


/* Class-specific Requests for Printers */
#define  USB_PRINTER_GET_DEVICE_ID          (0)
#define  USB_PRINTER_GET_PORT_STATUS        (1)
#define  USB_PRINTER_SOFT_RESET             (2)

/* Printer Interface Class Code */
#define USB_PRINTER_INTF_CLASS              (7)

/* Printer Interface Class SubClass Codes */
#define USB_PRINTER_INTF_SUBCLASS           (1)

/* Printer Interface Class Protocol Codes */
#define USB_PRINTER_INTF_PROTOCOL_NONE      (0)
#define USB_PRINTER_INTF_PROTOCOL_UNDIR     (1)
#define USB_PRINTER_INTF_PROTOCOL_BIDIR     (2)
#define USB_PRINTER_INTF_PROTOCOL_IEEE1284  (3)

/* Non-class Requests for Printers */
#define USB_PRINTER_SEND_BULK               (3)
#define USB_PRINTER_RECV_BULK               (4)


/* Commands to printer */
#define USB_PRINTER_DEVICE_ID_MAX_SIZE      (305)
#define USB_REQ_PRINTER_OUT (USB_H2D|USB_REQ_TYPE_CLASS|USB_REQ_RECIPIENT_INTERFACE)
#define USB_REQ_PRINTER_IN  (USB_D2H|USB_REQ_TYPE_CLASS|USB_REQ_RECIPIENT_INTERFACE)

/* Printer Port-Status Bits */
#define  USB_PRINTER_PAPER_OUT              (0x20) /* 0 = Not empty */
                                                   /* 1 = Paper empty */
#define  USB_PRINTER_SELECTED               (0x10) /* 0 = Not selected */
                                                   /* 1 = Selected */
#define  USB_PRINTER_NO_ERROR               (0x08) /* 0 = Error */
                                                   /* 1 = No error!!! */

/* Bulk In/Out End point-found tags */
#define  USB_PRINTER_IN_TAG                 (0x01)
#define  USB_PRINTER_OUT_TAG                (0x02)
#define  USB_PRINTER_BI_TAG                 (USB_PRINTER_IN_TAG | USB_PRINTER_OUT_TAG)

/* Error codes */
#define  USB_PRINTER_UNSUPPORTED_SUB_CLASS  (0x80)
#define  USB_PRINTER_UNSUPPORTED_PROTOCOL   (0x81)
#define  USB_PRINTER_BAD_COMMAND            (0x82)



typedef enum
{
    PRT_INIT_STATE = 0,
    PRT_IDLE_STATE,
    PRT_GET_PORT_STATUS_STATE,
    PRT_SOFT_RESET,
    PRT_READ_STATE,
    PRT_WRITE_STATE,

    PRT_ERROR_STATE,
    //MSC_PERIODIC_CHECK,

} PRT_StateTypeDef;

typedef enum
{
    PRT_BOT_INIT = 1,
    //BOT_SEND_CBW_WAIT,
    PRT_BOT_DATA_IN,
    PRT_BOT_DATA_IN_WAIT,
    PRT_BOT_DATA_OUT,
    PRT_BOT_DATA_OUT_WAIT,
    //BOT_RECEIVE_CSW,
    //BOT_RECEIVE_CSW_WAIT,
    //BOT_ERROR_IN,
    //BOT_ERROR_OUT,
    //BOT_UNRECOVERED_ERROR

} PRT_BOT_StateTypeDef;

typedef enum
{
    PRT_OK,
    PRT_ERR,
} PRT_ErrorTypeDef;

typedef enum
{
    PRT_REQ_IDLE = 0,
    PRT_REQ_RESET,
    PRT_REQ_GET_DEVICE_ID,
    PRT_REQ_ERROR,

} PRT_ReqStateTypeDef;

/* Structure for PRINTER process */


typedef struct
{
    //uint32_t                   data[16];
    PRT_BOT_StateTypeDef        state;
    //BOT_StateTypeDef           prev_state;
    //BOT_CMDStateTypeDef        cmd_state;
    //BOT_CBWTypeDef             cbw;
    //uint8_t                    Reserved1;
    //BOT_CSWTypeDef             csw;
    //uint8_t                    Reserved2[3];
    //uint8_t                    *pbuf;
} PRT_BOT_HandleTypeDef;

typedef struct _PRINTER_Process
{
    uint8_t                 OutPipe;
    uint8_t                 InPipe;
    uint8_t                 OutEp;
    uint8_t                 InEp;

    uint16_t                OutEpSize;
    uint16_t                InEpSize;

    PRT_StateTypeDef        state;
    PRT_ErrorTypeDef        error;
    PRT_ReqStateTypeDef     req_state;
    PRT_ReqStateTypeDef     prev_req_state;
    PRT_BOT_HandleTypeDef   hbot;
    uint32_t                timer;

    uint8_t             DeviceID[USB_PRINTER_DEVICE_ID_MAX_SIZE+3]; /** +3 fix alignment issue */

} PRT_HandleTypeDef;

size_t USBH_PRT_GetDeviceID(USBH_HandleTypeDef *phost, uint8_t *o_buf, size_t bsize);

USBH_StatusTypeDef USBH_PRT_GetPortStatus(USBH_HandleTypeDef *phost,
        uint8_t *o_buf, size_t wbytes);

USBH_StatusTypeDef USBH_PRT_SoftReset(USBH_HandleTypeDef *phost);

USBH_StatusTypeDef USBH_PRT_ReadData(USBH_HandleTypeDef *phost,
        const uint8_t *pcmd, size_t cmdlen, uint8_t *o_buf, size_t rbytes);

USBH_StatusTypeDef USBH_PRT_SendData(USBH_HandleTypeDef *phost,
        const uint8_t *i_buf, size_t wbytes);

#if 0

USBH_StatusTypeDef USBH_TEMPLATE_IOProcess (USBH_HandleTypeDef *phost);
USBH_StatusTypeDef USBH_TEMPLATE_Init (USBH_HandleTypeDef *phost);

#endif


#ifdef __cplusplus
}
#endif

#endif /* __USBH_PRINTER_H */




// End of file
