/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// usbh_sip10.h - USB host SIP10 class implementation
//
// Author: 185275258 (QQ Group)
//
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/


#ifndef __USBH_SIP10_H
#define __USBH_SIP10_H

#ifdef __cplusplus
 extern "C" {
#endif

#include "usbh_core.h"

extern USBH_ClassTypeDef    USBH_SIP10_Class;
#define USBH_SIP10_CLASS    &USBH_SIP10_Class

/** PID & VID definition */
#define USB_SIP10_VID   (0x1670)
#define USB_SIP10_PID   (0x00CA)

/* Class-specific Requests for Printers */

/* SIP-10 Interface Class Code : 0xFF (Vendor-Specific) */

/* SIP-10 Interface Class SubClass Codes : 0xFF (Vendor-Specific) */

/* SIP-10 Interface Class Protocol Codes : 0xFF (Vendor-Specific) */


typedef enum
{
    SIP10_INIT_STATE = 0,
    SIP10_IDLE_STATE,
    //SIP10_GET_PORT_STATUS_STATE,
    SIP10_SOFT_RESET,
    SIP10_READ_STATE,
    SIP10_WRITE_STATE,

    SIP10_ERROR_STATE,
    //MSC_PERIODIC_CHECK,

} SIP10_StateTypeDef;


typedef enum
{
    SIP10_BOT_INIT = 1,
    //BOT_SEND_CBW_WAIT,
    SIP10_BOT_DATA_IN,
    SIP10_BOT_DATA_IN_WAIT,
    SIP10_BOT_DATA_OUT,
    SIP10_BOT_DATA_OUT_WAIT,
    //BOT_RECEIVE_CSW,
    //BOT_RECEIVE_CSW_WAIT,
    SIP10_BOT_ERROR_IN,
    //BOT_ERROR_OUT,
    //BOT_UNRECOVERED_ERROR

} SIP10_BOT_StateTypeDef;

typedef enum
{
    SIP10_REQ_IDLE = 0,
    SIP10_REQ_RESET,
    //SIP10_REQ_GET_DEVICE_ID,
    SIP10_REQ_ERROR,

} SIP10_ReqStateTypeDef;

typedef enum
{
    SIP10_OK,
    SIP10_ERR,

} SIP10_ErrorTypeDef;

typedef struct
{
    //uint32_t                   data[16];
    SIP10_BOT_StateTypeDef        state;
    //BOT_StateTypeDef           prev_state;
    //BOT_CMDStateTypeDef        cmd_state;
    //BOT_CBWTypeDef             cbw;
    //uint8_t                    Reserved1;
    //BOT_CSWTypeDef             csw;
    //uint8_t                    Reserved2[3];
    //uint8_t                    *pbuf;
} SIP10_BOT_HandleTypeDef;

typedef struct _SIP10_Process
{
    uint8_t                 OutPipe;
    uint8_t                 InPipe;
    uint8_t                 OutEp;
    uint8_t                 InEp;

    uint16_t                OutEpSize;
    uint16_t                InEpSize;

    SIP10_StateTypeDef        state;
    SIP10_ErrorTypeDef        error;
    SIP10_ReqStateTypeDef     req_state;
    SIP10_ReqStateTypeDef     prev_req_state;
    SIP10_BOT_HandleTypeDef   hbot;

    uint32_t                timer;

    //uint8_t                 DeviceID[USB_PRINTER_DEVICE_ID_MAX_SIZE+3]; /** +3 fix alignment issue */

} SIP10_HandleTypeDef;


USBH_StatusTypeDef USBH_SIP10_TransmitReceive(USBH_HandleTypeDef *phost,
        const char *p_cmd, int cmd_len, char *p_result, int result_len);


#ifdef __cplusplus
}
#endif

#endif /* __USBH_SIP10_H */




// End of file
