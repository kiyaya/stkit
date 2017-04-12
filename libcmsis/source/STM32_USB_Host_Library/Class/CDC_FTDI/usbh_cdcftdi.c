/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// usbh_cdcftdi.c - USB host CDC class implementation for FTDI chip
//
// Author: 185275258 (QQ Group)
//
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/

#include "usbh_cdcftdi.h"

#define USBH_CDC_BUFFER_SIZE                 1024

static USBH_StatusTypeDef USBH_CDC_InterfaceInit(USBH_HandleTypeDef *phost);

static USBH_StatusTypeDef USBH_CDC_InterfaceDeInit(USBH_HandleTypeDef *phost);

static USBH_StatusTypeDef USBH_CDC_Process(USBH_HandleTypeDef *phost);

static USBH_StatusTypeDef USBH_CDC_SOFProcess(USBH_HandleTypeDef *phost);

static USBH_StatusTypeDef USBH_CDC_ClassRequest(USBH_HandleTypeDef *phost);

static USBH_StatusTypeDef GetLineCoding(USBH_HandleTypeDef *phost,
        CDC_LineCodingTypeDef *linecoding);

static USBH_StatusTypeDef SetLineCoding(USBH_HandleTypeDef *phost,
        CDC_LineCodingTypeDef *linecoding);

static void CDC_ProcessTransmission(USBH_HandleTypeDef *phost);

static void CDC_ProcessReception(USBH_HandleTypeDef *phost);

#if 1   // FTDI chip
static USBH_StatusTypeDef ftdi_set_baudrate(USBH_HandleTypeDef *phost,
        uint32_t ftdi_chip, uint32_t baud);

static USBH_StatusTypeDef ftdi_set_modem_control(USBH_HandleTypeDef *phost,
        uint16_t signal);

static USBH_StatusTypeDef ftdi_set_flow_control(USBH_HandleTypeDef *phost,
        uint8_t protocol, uint8_t xon, uint8_t xoff);

static USBH_StatusTypeDef ftdi_set_data(USBH_HandleTypeDef *phost,
        uint16_t databm);

#endif

USBH_ClassTypeDef CDC_Class =
{
  "CDC",
  USB_CDC_CLASS,
  USBH_CDC_InterfaceInit,
  USBH_CDC_InterfaceDeInit,
  USBH_CDC_ClassRequest,
  USBH_CDC_Process,
  USBH_CDC_SOFProcess,
  NULL
};

/**
 * @brief  USBH_CDC_InterfaceInit
 *         The function init the CDC class.
 * @param  phost: Host handle
 * @retval USBH Status
 */
static USBH_StatusTypeDef USBH_CDC_InterfaceInit(USBH_HandleTypeDef *phost)
{

    USBH_StatusTypeDef status = USBH_FAIL;
    uint8_t interface;
    CDC_HandleTypeDef *CDC_Handle;

    //interface = USBH_FindInterface(phost,
    //                               COMMUNICATION_INTERFACE_CLASS_CODE,
    //                               ABSTRACT_CONTROL_MODEL,
    //                               COMMON_AT_COMMAND);
    interface = USBH_FindInterface(phost, VENDOR_SPECIFIC, VENDOR_SPECIFIC,
            VENDOR_SPECIFIC);

    if (interface == 0xFF) /* No Valid Interface */
    {
        USBH_DbgLog(
                "Cannot Find the interface for Communication Interface Class.",
                phost->pActiveClass->Name);
    }
    else
    {
        USBH_SelectInterface(phost, interface);
        phost->pActiveClass->pData = (CDC_HandleTypeDef *) USBH_malloc(
                sizeof(CDC_HandleTypeDef));
        CDC_Handle = (CDC_HandleTypeDef*) phost->pActiveClass->pData;

        /*Collect the notification endpoint address and length*/
        if (phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[0].bEndpointAddress
                & 0x80)
        {
            CDC_Handle->CommItf.NotifEp =
                    phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[0].bEndpointAddress;
            CDC_Handle->CommItf.NotifEpSize =
                    phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[0].wMaxPacketSize;
        }

        /*Allocate the length for host channel number in*/
        CDC_Handle->CommItf.NotifPipe = USBH_AllocPipe(phost,
                CDC_Handle->CommItf.NotifEp);

        /* Open pipe for Notification endpoint */
        USBH_OpenPipe(phost, CDC_Handle->CommItf.NotifPipe,
                CDC_Handle->CommItf.NotifEp, phost->device.address,
                phost->device.speed,
                USB_EP_TYPE_INTR, CDC_Handle->CommItf.NotifEpSize);

        USBH_LL_SetToggle(phost, CDC_Handle->CommItf.NotifPipe, 0);

        //interface = USBH_FindInterface(phost,
        //                               DATA_INTERFACE_CLASS_CODE,
        //                               RESERVED,
        //                               NO_CLASS_SPECIFIC_PROTOCOL_CODE);
        interface = USBH_FindInterface(phost, VENDOR_SPECIFIC, VENDOR_SPECIFIC,
                VENDOR_SPECIFIC);

        if (interface == 0xFF) /* No Valid Interface */
        {
            USBH_DbgLog("Cannot Find the interface for Data Interface Class.",
                    phost->pActiveClass->Name);
        }
        else
        {
            /*Collect the class specific endpoint address and length*/
            if (phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[0].bEndpointAddress
                    & 0x80)
            {
                CDC_Handle->DataItf.InEp =
                        phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[0].bEndpointAddress;
                CDC_Handle->DataItf.InEpSize =
                        phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[0].wMaxPacketSize;
            }
            else
            {
                CDC_Handle->DataItf.OutEp =
                        phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[0].bEndpointAddress;
                CDC_Handle->DataItf.OutEpSize =
                        phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[0].wMaxPacketSize;
            }

            if (phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[1].bEndpointAddress
                    & 0x80)
            {
                CDC_Handle->DataItf.InEp =
                        phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[1].bEndpointAddress;
                CDC_Handle->DataItf.InEpSize =
                        phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[1].wMaxPacketSize;
            }
            else
            {
                CDC_Handle->DataItf.OutEp =
                        phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[1].bEndpointAddress;
                CDC_Handle->DataItf.OutEpSize =
                        phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[1].wMaxPacketSize;
            }

            /*Allocate the length for host channel number out*/
            CDC_Handle->DataItf.OutPipe = USBH_AllocPipe(phost,
                    CDC_Handle->DataItf.OutEp);

            /*Allocate the length for host channel number in*/
            CDC_Handle->DataItf.InPipe = USBH_AllocPipe(phost,
                    CDC_Handle->DataItf.InEp);

            /* Open channel for OUT endpoint */
            USBH_OpenPipe(phost, CDC_Handle->DataItf.OutPipe,
                    CDC_Handle->DataItf.OutEp, phost->device.address,
                    phost->device.speed,
                    USB_EP_TYPE_BULK, CDC_Handle->DataItf.OutEpSize);
            /* Open channel for IN endpoint */
            USBH_OpenPipe(phost, CDC_Handle->DataItf.InPipe,
                    CDC_Handle->DataItf.InEp, phost->device.address,
                    phost->device.speed,
                    USB_EP_TYPE_BULK, CDC_Handle->DataItf.InEpSize);

            CDC_Handle->state = CDC_IDLE_STATE;

            USBH_LL_SetToggle(phost, CDC_Handle->DataItf.OutPipe, 0);
            USBH_LL_SetToggle(phost, CDC_Handle->DataItf.InPipe, 0);
            status = USBH_OK;
        }
    }
    return status;
}

/**
 * @brief  USBH_CDC_InterfaceDeInit
 *         The function DeInit the Pipes used for the CDC class.
 * @param  phost: Host handle
 * @retval USBH Status
 */
USBH_StatusTypeDef USBH_CDC_InterfaceDeInit(USBH_HandleTypeDef *phost)
{
    CDC_HandleTypeDef *CDC_Handle =
            (CDC_HandleTypeDef*) phost->pActiveClass->pData;

    if (CDC_Handle->CommItf.NotifPipe)
    {
        USBH_ClosePipe(phost, CDC_Handle->CommItf.NotifPipe);
        USBH_FreePipe(phost, CDC_Handle->CommItf.NotifPipe);
        CDC_Handle->CommItf.NotifPipe = 0; /* Reset the Channel as Free */
    }

    if (CDC_Handle->DataItf.InPipe)
    {
        USBH_ClosePipe(phost, CDC_Handle->DataItf.InPipe);
        USBH_FreePipe(phost, CDC_Handle->DataItf.InPipe);
        CDC_Handle->DataItf.InPipe = 0; /* Reset the Channel as Free */
    }

    if (CDC_Handle->DataItf.OutPipe)
    {
        USBH_ClosePipe(phost, CDC_Handle->DataItf.OutPipe);
        USBH_FreePipe(phost, CDC_Handle->DataItf.OutPipe);
        CDC_Handle->DataItf.OutPipe = 0; /* Reset the Channel as Free */
    }

    if (phost->pActiveClass->pData)
    {
        USBH_free(phost->pActiveClass->pData);
        phost->pActiveClass->pData = 0;
    }

    return USBH_OK;
}

/**
 * @brief  USBH_CDC_ClassRequest
 *         The function is responsible for handling Standard requests
 *         for CDC class.
 * @param  phost: Host handle
 * @retval USBH Status
 */
static USBH_StatusTypeDef USBH_CDC_ClassRequest(USBH_HandleTypeDef *phost)
{
    USBH_StatusTypeDef status = USBH_FAIL;
    CDC_HandleTypeDef *CDC_Handle = (CDC_HandleTypeDef*) phost->pActiveClass->pData;
    
#if 1
    status = ftdi_set_baudrate(phost, CDC_Handle->CDC_Desc.CDC_HeaderFuncDesc.bcdCDC, 115200);
    if (status == USBH_OK) {
        status = ftdi_set_flow_control(phost, 0x0, 0x11, 0x13);
        if (status == USBH_OK) {
            phost->pUser(phost, HOST_USER_CLASS_ACTIVE);
        }
    }    
#else
    /*Issue the get line coding request*/
    status = GetLineCoding(phost, &CDC_Handle->LineCoding);
    if (status == USBH_OK) {
    phost->pUser(phost, HOST_USER_CLASS_ACTIVE);
    }
#endif
    return status;
}

/**
 * @brief  USBH_CDC_Process
 *         The function is for managing state machine for CDC data transfers
 * @param  phost: Host handle
 * @retval USBH Status
 */
static USBH_StatusTypeDef USBH_CDC_Process(USBH_HandleTypeDef *phost)
{
    USBH_StatusTypeDef status = USBH_BUSY;
    USBH_StatusTypeDef req_status = USBH_OK;
    CDC_HandleTypeDef *CDC_Handle =
            (CDC_HandleTypeDef*) phost->pActiveClass->pData;

    switch (CDC_Handle->state)
    {

    case CDC_IDLE_STATE:
        status = USBH_OK;
        break;

    case CDC_SET_LINE_CODING_STATE:
        req_status = SetLineCoding(phost, CDC_Handle->pUserLineCoding);

        if (req_status == USBH_OK)
        {
            CDC_Handle->state = CDC_GET_LAST_LINE_CODING_STATE;
        }

        else if (req_status != USBH_BUSY)
        {
            CDC_Handle->state = CDC_ERROR_STATE;
        }
        break;

    case CDC_GET_LAST_LINE_CODING_STATE:
        req_status = GetLineCoding(phost, &(CDC_Handle->LineCoding));

        if (req_status == USBH_OK)
        {
            CDC_Handle->state = CDC_IDLE_STATE;

            if ((CDC_Handle->LineCoding.b.bCharFormat
                    == CDC_Handle->pUserLineCoding->b.bCharFormat)
                    && (CDC_Handle->LineCoding.b.bDataBits
                            == CDC_Handle->pUserLineCoding->b.bDataBits)
                    && (CDC_Handle->LineCoding.b.bParityType
                            == CDC_Handle->pUserLineCoding->b.bParityType)
                    && (CDC_Handle->LineCoding.b.dwDTERate
                            == CDC_Handle->pUserLineCoding->b.dwDTERate))
            {
                USBH_CDC_LineCodingChanged(phost);
            }
        }

        else if (req_status != USBH_BUSY)
        {
            CDC_Handle->state = CDC_ERROR_STATE;
        }

        break;

    case CDC_TRANSFER_DATA:
        CDC_ProcessTransmission(phost);
        CDC_ProcessReception(phost);
        break;

    case CDC_ERROR_STATE:
        req_status = USBH_ClrFeature(phost, 0x00);

        if (req_status == USBH_OK)
        {
            /*Change the state to waiting*/
            CDC_Handle->state = CDC_IDLE_STATE;
        }
        break;

    default:
        break;

    }

    return status;
}

/**
 * @brief  USBH_CDC_SOFProcess
 *         The function is for managing SOF callback
 * @param  phost: Host handle
 * @retval USBH Status
 */
static USBH_StatusTypeDef USBH_CDC_SOFProcess(USBH_HandleTypeDef *phost)
{
    return USBH_OK;
}

/**
 * @brief  USBH_CDC_Stop
 *         Stop current CDC Transmission
 * @param  phost: Host handle
 * @retval USBH Status
 */
USBH_StatusTypeDef USBH_CDC_Stop(USBH_HandleTypeDef *phost)
{
    CDC_HandleTypeDef *CDC_Handle =
            (CDC_HandleTypeDef*) phost->pActiveClass->pData;

    if (phost->gState == HOST_CLASS)
    {
        CDC_Handle->state = CDC_IDLE_STATE;

        USBH_ClosePipe(phost, CDC_Handle->CommItf.NotifPipe);
        USBH_ClosePipe(phost, CDC_Handle->DataItf.InPipe);
        USBH_ClosePipe(phost, CDC_Handle->DataItf.OutPipe);
    }
    return USBH_OK;
}
/**
 * @brief  This request allows the host to find out the currently
 *         configured line coding.
 * @param  pdev: Selected device
 * @retval USBH_StatusTypeDef : USB ctl xfer status
 */
static USBH_StatusTypeDef GetLineCoding(USBH_HandleTypeDef *phost,
        CDC_LineCodingTypeDef *linecoding)
{

    phost->Control.setup.b.bmRequestType = USB_D2H | USB_REQ_TYPE_CLASS |
    USB_REQ_RECIPIENT_INTERFACE;

    phost->Control.setup.b.bRequest = CDC_GET_LINE_CODING;
    phost->Control.setup.b.wValue.w = 0;
    phost->Control.setup.b.wIndex.w = 0;
    phost->Control.setup.b.wLength.w = LINE_CODING_STRUCTURE_SIZE;

    return USBH_CtlReq(phost, linecoding->Array, LINE_CODING_STRUCTURE_SIZE);
}

/**
 * @brief  This request allows the host to specify typical asynchronous
 * line-character formatting properties
 * This request applies to asynchronous byte stream data class interfaces
 * and endpoints
 * @param  pdev: Selected device
 * @retval USBH_StatusTypeDef : USB ctl xfer status
 */
static USBH_StatusTypeDef SetLineCoding(USBH_HandleTypeDef *phost,
        CDC_LineCodingTypeDef *linecodin)
{
    phost->Control.setup.b.bmRequestType = USB_H2D | USB_REQ_TYPE_CLASS |
    USB_REQ_RECIPIENT_INTERFACE;

    phost->Control.setup.b.bRequest = CDC_SET_LINE_CODING;
    phost->Control.setup.b.wValue.w = 0;

    phost->Control.setup.b.wIndex.w = 0;

    phost->Control.setup.b.wLength.w = LINE_CODING_STRUCTURE_SIZE;

    return USBH_CtlReq(phost, linecodin->Array, LINE_CODING_STRUCTURE_SIZE);
}

/**
 * @brief  This function prepares the state before issuing the class specific commands
 * @param  None
 * @retval None
 */
USBH_StatusTypeDef USBH_CDC_SetLineCoding(USBH_HandleTypeDef *phost,
        CDC_LineCodingTypeDef *linecodin)
{
    CDC_HandleTypeDef *CDC_Handle =
            (CDC_HandleTypeDef*) phost->pActiveClass->pData;
    if (phost->gState == HOST_CLASS)
    {
        CDC_Handle->state = CDC_SET_LINE_CODING_STATE;
        CDC_Handle->pUserLineCoding = linecodin;

#if (USBH_USE_OS == 1)
        osMessagePut(phost->os_event, USBH_CLASS_EVENT, 0);
#endif  
    }
    return USBH_OK;
}

/**
 * @brief  This function prepares the state before issuing the class specific commands
 * @param  None
 * @retval None
 */
USBH_StatusTypeDef USBH_CDC_GetLineCoding(USBH_HandleTypeDef *phost,
        CDC_LineCodingTypeDef *linecodin)
{
    CDC_HandleTypeDef *CDC_Handle =
            (CDC_HandleTypeDef*) phost->pActiveClass->pData;

    if ((phost->gState == HOST_CLASS) || (phost->gState == HOST_CLASS_REQUEST))
    {
        *linecodin = CDC_Handle->LineCoding;
        return USBH_OK;
    }
    else
    {
        return USBH_FAIL;
    }
}

/**
 * @brief  This function return last received data size
 * @param  None
 * @retval None
 */
uint16_t USBH_CDC_GetLastReceivedDataSize(USBH_HandleTypeDef *phost)
{
    CDC_HandleTypeDef *CDC_Handle =
            (CDC_HandleTypeDef*) phost->pActiveClass->pData;

    if (phost->gState == HOST_CLASS)
    {
        return USBH_LL_GetLastXferSize(phost, CDC_Handle->DataItf.InPipe);;
    }
    else
    {
        return 0;
    }
}

USBH_StatusTypeDef USBH_CDC_TransmitBlocking(USBH_HandleTypeDef *phost,
        const uint8_t *pbuff, uint32_t length)
{
    CDC_HandleTypeDef *CDC_Handle =
            (CDC_HandleTypeDef*) phost->pActiveClass->pData;
    uint32_t timeout;

    if (length > 64) length = 64;
    if (USBH_OK == USBH_CDC_Transmit(phost, pbuff, length))
    {
        timeout = phost->Timer;
        while (CDC_IDLE != CDC_Handle->data_tx_state)
        {
            if ((phost->Timer - timeout) > 1000 * length ||
                    phost->device.is_connected == 0) {
                USBH_CDC_Stop(phost);
                return USBH_FAIL;
            }
        }
        return USBH_OK;
    }

    return USBH_FAIL;
}

/**
 * @brief  This function prepares the state before issuing the class specific commands
 * @param  None
 * @retval None
 */
USBH_StatusTypeDef USBH_CDC_Transmit(USBH_HandleTypeDef *phost, const uint8_t *pbuff,
        uint32_t length)
{
    USBH_StatusTypeDef Status = USBH_BUSY;
    CDC_HandleTypeDef *CDC_Handle =
            (CDC_HandleTypeDef*) phost->pActiveClass->pData;

    if ((CDC_Handle->state == CDC_IDLE_STATE)
            || (CDC_Handle->state == CDC_TRANSFER_DATA))
    {
        CDC_Handle->pTxData = (uint8_t *) pbuff;
        CDC_Handle->TxDataLength = length;
        CDC_Handle->state = CDC_TRANSFER_DATA;
        CDC_Handle->data_tx_state = CDC_SEND_DATA;
        Status = USBH_OK;
#if (USBH_USE_OS == 1)
        osMessagePut(phost->os_event, USBH_CLASS_EVENT, 0);
#endif      
    }
    return Status;
}

size_t USBH_CDC_ReceiveBlocking(USBH_HandleTypeDef *phost,
                                     uint8_t *pbuff,
                                     uint32_t length)
{
    CDC_HandleTypeDef *CDC_Handle =
            (CDC_HandleTypeDef*) phost->pActiveClass->pData;
    uint32_t timeout;

    if (USBH_OK == USBH_CDC_Receive(phost, pbuff, length))
    {
        timeout = phost->Timer;
        while (CDC_IDLE != CDC_Handle->data_rx_state)
        {
            if ((phost->Timer - timeout) > 1000 * length
                    || phost->device.is_connected == 0) {
                USBH_CDC_Stop(phost);
                return 0;
            }
        }
        return (USBH_LL_GetLastXferSize(phost, CDC_Handle->DataItf.InPipe));
    }

    return 0;
}

/**
 * @brief  This function prepares the state before issuing the class specific commands
 * @param  None
 * @retval None
 */
USBH_StatusTypeDef USBH_CDC_Receive(USBH_HandleTypeDef *phost, uint8_t *pbuff,
        uint32_t length)
{
    USBH_StatusTypeDef Status = USBH_BUSY;
    CDC_HandleTypeDef *CDC_Handle =
            (CDC_HandleTypeDef*) phost->pActiveClass->pData;

    if ((CDC_Handle->state == CDC_IDLE_STATE)
            || (CDC_Handle->state == CDC_TRANSFER_DATA))
    {
        CDC_Handle->pRxData = pbuff;
        CDC_Handle->RxDataLength = length;
        CDC_Handle->state = CDC_TRANSFER_DATA;
        CDC_Handle->data_rx_state = CDC_RECEIVE_DATA;
        Status = USBH_OK;
#if (USBH_USE_OS == 1)
        osMessagePut(phost->os_event, USBH_CLASS_EVENT, 0);
#endif        
    }
    return Status;
}

/**
 * @brief  The function is responsible for sending data to the device
 *  @param  pdev: Selected device
 * @retval None
 */
static void CDC_ProcessTransmission(USBH_HandleTypeDef *phost)
{
    CDC_HandleTypeDef *CDC_Handle =
            (CDC_HandleTypeDef*) phost->pActiveClass->pData;
    USBH_URBStateTypeDef URB_Status = USBH_URB_IDLE;

    switch (CDC_Handle->data_tx_state)
    {

    case CDC_SEND_DATA:
        if (CDC_Handle->TxDataLength > CDC_Handle->DataItf.OutEpSize)
        {
            USBH_BulkSendData(phost, CDC_Handle->pTxData,
                    CDC_Handle->DataItf.OutEpSize, CDC_Handle->DataItf.OutPipe,
                    1);
        }
        else
        {
            USBH_BulkSendData(phost, CDC_Handle->pTxData,
                    CDC_Handle->TxDataLength, CDC_Handle->DataItf.OutPipe, 1);
        }

        CDC_Handle->data_tx_state = CDC_SEND_DATA_WAIT;

        break;

    case CDC_SEND_DATA_WAIT:

        URB_Status = USBH_LL_GetURBState(phost, CDC_Handle->DataItf.OutPipe);

        /*Check the status done for transmission*/
        if (URB_Status == USBH_URB_DONE)
        {
            if (CDC_Handle->TxDataLength > CDC_Handle->DataItf.OutEpSize)
            {
                CDC_Handle->TxDataLength -= CDC_Handle->DataItf.OutEpSize;
                CDC_Handle->pTxData += CDC_Handle->DataItf.OutEpSize;
            }
            else
            {
                CDC_Handle->TxDataLength = 0;
            }

            if (CDC_Handle->TxDataLength > 0)
            {
                CDC_Handle->data_tx_state = CDC_SEND_DATA;
            }
            else
            {
                CDC_Handle->data_tx_state = CDC_IDLE;
                USBH_CDC_TransmitCallback(phost);
            }
#if (USBH_USE_OS == 1)
            osMessagePut(phost->os_event, USBH_CLASS_EVENT, 0);
#endif    
        }
        else if (URB_Status == USBH_URB_NOTREADY)
        {
            CDC_Handle->data_tx_state = CDC_SEND_DATA;
#if (USBH_USE_OS == 1)
            osMessagePut(phost->os_event, USBH_CLASS_EVENT, 0);
#endif          
        }
        break;
    default:
        break;
    }
}
/**
 * @brief  This function responsible for reception of data from the device
 *  @param  pdev: Selected device
 * @retval None
 */

static void CDC_ProcessReception(USBH_HandleTypeDef *phost)
{
    CDC_HandleTypeDef *CDC_Handle =
            (CDC_HandleTypeDef*) phost->pActiveClass->pData;
    USBH_URBStateTypeDef URB_Status = USBH_URB_IDLE;
    uint16_t length;

    switch (CDC_Handle->data_rx_state)
    {

    case CDC_RECEIVE_DATA:

        USBH_BulkReceiveData(phost, CDC_Handle->pRxData,
                CDC_Handle->DataItf.InEpSize, CDC_Handle->DataItf.InPipe);

        CDC_Handle->data_rx_state = CDC_RECEIVE_DATA_WAIT;

        break;

    case CDC_RECEIVE_DATA_WAIT:

        URB_Status = USBH_LL_GetURBState(phost, CDC_Handle->DataItf.InPipe);

        /*Check the status done for reception*/
        if (URB_Status == USBH_URB_DONE)
        {
            length = USBH_LL_GetLastXferSize(phost, CDC_Handle->DataItf.InPipe);

            if (((CDC_Handle->RxDataLength - length) > 0)
                    && (length > CDC_Handle->DataItf.InEpSize))
            {
                CDC_Handle->RxDataLength -= length;
                CDC_Handle->pRxData += length;
                CDC_Handle->data_rx_state = CDC_RECEIVE_DATA;
            }
            else
            {
                CDC_Handle->data_rx_state = CDC_IDLE;
                USBH_CDC_ReceiveCallback(phost);
            }
#if (USBH_USE_OS == 1)
            osMessagePut(phost->os_event, USBH_CLASS_EVENT, 0);
#endif          
        }
        break;

    default:
        break;
    }
}

/**
 * @brief  The function informs user that data have been received
 *  @param  pdev: Selected device
 * @retval None
 */
__weak void USBH_CDC_TransmitCallback(USBH_HandleTypeDef *phost)
{

}

/**
 * @brief  The function informs user that data have been sent
 *  @param  pdev: Selected device
 * @retval None
 */
__weak void USBH_CDC_ReceiveCallback(USBH_HandleTypeDef *phost)
{

}

/**
 * @brief  The function informs user that Settings have been changed
 *  @param  pdev: Selected device
 * @retval None
 */
__weak void USBH_CDC_LineCodingChanged(USBH_HandleTypeDef *phost)
{

}

#if 1   // FTDI CHIP
#define bmREQ_FTDI_OUT  0x40
#define bmREQ_FTDI_IN   0xc0

//#define bmREQ_FTDI_OUT        USB_SETUP_HOST_TO_DEVICE|USB_SETUP_TYPE_CLASS|USB_SETUP_RECIPIENT_INTERFACE
//#define bmREQ_FTDI_IN     USB_SETUP_DEVICE_TO_HOST|USB_SETUP_TYPE_CLASS|USB_SETUP_RECIPIENT_INTERFACE

#define FTDI_VID            0x0403  // FTDI VID
#define FTDI_PID            0x6001  // FTDI PID
#define FT232AM             0x0200
#define FT232BM             0x0400
#define FT2232              0x0500
#define FT232R              0x0600

// Commands
#define FTDI_SIO_RESET                      0 /* Reset the port */
#define FTDI_SIO_MODEM_CTRL                 1 /* Set the modem control register */
#define FTDI_SIO_SET_FLOW_CTRL              2 /* Set flow control register */
#define FTDI_SIO_SET_BAUD_RATE              3 /* Set baud rate */
#define FTDI_SIO_SET_DATA                   4 /* Set the data characteristics of the port */
#define FTDI_SIO_GET_MODEM_STATUS           5 /* Retrieve current value of modem status register */
#define FTDI_SIO_SET_EVENT_CHAR             6 /* Set the event character */
#define FTDI_SIO_SET_ERROR_CHAR             7 /* Set the error character */

#define FTDI_SIO_RESET_SIO                  0
#define FTDI_SIO_RESET_PURGE_RX             1
#define FTDI_SIO_RESET_PURGE_TX             2

#define FTDI_SIO_SET_DATA_PARITY_NONE       (0x0 << 8 )
#define FTDI_SIO_SET_DATA_PARITY_ODD        (0x1 << 8 )
#define FTDI_SIO_SET_DATA_PARITY_EVEN       (0x2 << 8 )
#define FTDI_SIO_SET_DATA_PARITY_MARK       (0x3 << 8 )
#define FTDI_SIO_SET_DATA_PARITY_SPACE      (0x4 << 8 )
#define FTDI_SIO_SET_DATA_STOP_BITS_1       (0x0 << 11)
#define FTDI_SIO_SET_DATA_STOP_BITS_15      (0x1 << 11)
#define FTDI_SIO_SET_DATA_STOP_BITS_2       (0x2 << 11)
#define FTDI_SIO_SET_BREAK                  (0x1 << 14)

#define FTDI_SIO_SET_DTR_MASK               0x1
#define FTDI_SIO_SET_DTR_HIGH               ( 1 | ( FTDI_SIO_SET_DTR_MASK  << 8))
#define FTDI_SIO_SET_DTR_LOW                ( 0 | ( FTDI_SIO_SET_DTR_MASK  << 8))
#define FTDI_SIO_SET_RTS_MASK               0x2
#define FTDI_SIO_SET_RTS_HIGH               ( 2 | ( FTDI_SIO_SET_RTS_MASK << 8 ))
#define FTDI_SIO_SET_RTS_LOW                ( 0 | ( FTDI_SIO_SET_RTS_MASK << 8 ))

#define FTDI_SIO_DISABLE_FLOW_CTRL          0x0
#define FTDI_SIO_RTS_CTS_HS                 (0x1 << 8)
#define FTDI_SIO_DTR_DSR_HS                 (0x2 << 8)
#define FTDI_SIO_XON_XOFF_HS                (0x4 << 8)

#define FTDI_SIO_CTS_MASK                   0x10
#define FTDI_SIO_DSR_MASK                   0x20
#define FTDI_SIO_RI_MASK                    0x40
#define FTDI_SIO_RLSD_MASK                  0x80

static USBH_StatusTypeDef ftdi_set_baudrate(USBH_HandleTypeDef *phost,
        uint32_t ftdi_chip, uint32_t baud)
{
    uint16_t baud_value, baud_index = 0;
    uint32_t divisor3;
    USBH_StatusTypeDef status;
    uint32_t timeout;

    divisor3 = 48000000 / 2 / baud; // divisor shifted 3 bits to the left

    if (ftdi_chip == FT232AM)
    {
        if ((divisor3 & 0x7) == 7)
            divisor3++; // round x.7/8 up to x+1

        baud_value = divisor3 >> 3;
        divisor3 &= 0x7;

        if (divisor3 == 1)
            baud_value |= 0xc000;
        else // 0.125
        if (divisor3 >= 4)
            baud_value |= 0x4000;
        else // 0.5
        if (divisor3 != 0)
            baud_value |= 0x8000; // 0.25
        if (baud_value == 1)
            baud_value = 0; /* special case for maximum baud rate */
    }
    else
    {
        static const unsigned char divfrac[8] =
        { 0, 3, 2, 0, 1, 1, 2, 3 };
        static const unsigned char divindex[8] =
        { 0, 0, 0, 1, 0, 1, 1, 1 };

        baud_value = divisor3 >> 3;
        baud_value |= divfrac[divisor3 & 0x7] << 14;
        baud_index = divindex[divisor3 & 0x7];

        /* Deal with special cases for highest baud rates. */
        if (baud_value == 1)
            baud_value = 0;
        else // 1.0
        if (baud_value == 0x4001)
            baud_value = 1; // 1.5
    }

    USBH_UsrLog("baud_value:%xh", baud_value);
    USBH_UsrLog("baud_index:%xh", baud_index);

    phost->Control.setup.b.bmRequestType = bmREQ_FTDI_OUT;
    phost->Control.setup.b.bRequest = FTDI_SIO_SET_BAUD_RATE;
    phost->Control.setup.b.wValue.w = baud_value;
    phost->Control.setup.b.wIndex.w = baud_index;
    phost->Control.setup.b.wLength.w = 0;

    status = USBH_CtlReq(phost, 0, 0);
    timeout = phost->Timer;
    while(status != USBH_OK)
    {
        if ((phost->Timer - timeout) > 10000 || (phost->device.is_connected == 0))
        {
            break;
        }
        status = USBH_CtlReq(phost, 0, 0);
    }
    return status;
}

static USBH_StatusTypeDef ftdi_set_modem_control(USBH_HandleTypeDef *phost,
        uint16_t signal)
{
    phost->Control.setup.b.bmRequestType = bmREQ_FTDI_OUT;
    phost->Control.setup.b.bRequest = FTDI_SIO_MODEM_CTRL;
    phost->Control.setup.b.wValue.w = signal;
    phost->Control.setup.b.wIndex.w = 0;
    phost->Control.setup.b.wLength.w = 0;
    return USBH_CtlReq(phost, 0, 0);
}

static USBH_StatusTypeDef ftdi_set_flow_control(USBH_HandleTypeDef *phost,
        uint8_t protocol, uint8_t xon, uint8_t xoff)
{
    USBH_StatusTypeDef status;
    uint32_t timeout;

    phost->Control.setup.b.bmRequestType = bmREQ_FTDI_OUT;
    phost->Control.setup.b.bRequest = FTDI_SIO_SET_FLOW_CTRL;
    phost->Control.setup.b.wValue.bw.lsb = xon;
    phost->Control.setup.b.wValue.bw.msb = xoff;
    phost->Control.setup.b.wIndex.w = protocol << 8;
    phost->Control.setup.b.wLength.w = 0;

    status = USBH_CtlReq(phost, 0, 0);
    timeout = phost->Timer;
    while(status != USBH_OK)
    {
        if ((phost->Timer - timeout) > 10000 || (phost->device.is_connected == 0))
        {
            break;
        }
        status = USBH_CtlReq(phost, 0, 0);
    }

    return status;
}

static USBH_StatusTypeDef ftdi_set_data(USBH_HandleTypeDef *phost,
        uint16_t databm)
{
    phost->Control.setup.b.bmRequestType = bmREQ_FTDI_OUT;
    phost->Control.setup.b.bRequest = FTDI_SIO_SET_DATA;
    phost->Control.setup.b.wValue.w = databm;
    phost->Control.setup.b.wIndex.w = 0;
    phost->Control.setup.b.wLength.w = 0;
    return USBH_CtlReq(phost, 0, 0);
}

#endif  // end of ftdi chip
// End of file
