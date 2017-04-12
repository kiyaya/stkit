/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// usbh_printer.c - USB host printer class implementation
//
// Author: 185275258 (QQ Group)
//
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/

/* Includes ------------------------------------------------------------------*/
#include "usbh_printer.h"


static USBH_StatusTypeDef USBH_PRT_InterfaceInit(USBH_HandleTypeDef *phost);

static USBH_StatusTypeDef USBH_PRT_InterfaceDeInit(USBH_HandleTypeDef *phost);

static USBH_StatusTypeDef USBH_PRT_ClassRequest(USBH_HandleTypeDef *phost);

static USBH_StatusTypeDef USBH_PRT_Process(USBH_HandleTypeDef *phost);

static USBH_StatusTypeDef USBH_PRT_SOFProcess(USBH_HandleTypeDef *phost);

static USBH_StatusTypeDef USBH_PRT_BOT_GetDeviceID(USBH_HandleTypeDef *phost,
        uint8_t *o_buf, size_t wbytes);

static USBH_StatusTypeDef USBH_PRT_BOT_GetPortStatus(USBH_HandleTypeDef *phost,
        uint8_t *o_buf, size_t wbytes);

static USBH_StatusTypeDef USBH_PRT_BOT_RdWrProcess(USBH_HandleTypeDef *phost,
        uint8_t *pbuf, const size_t bsize);

USBH_ClassTypeDef USBH_PRINTER_Class =
{
    "PRT",
    USB_PRINTER_INTF_CLASS,
    USBH_PRT_InterfaceInit,
    USBH_PRT_InterfaceDeInit,
    USBH_PRT_ClassRequest,
    USBH_PRT_Process,
    USBH_PRT_SOFProcess,
    NULL
};

/**
  * @brief  USBH_TEMPLATE_InterfaceInit 
  *         The function init the TEMPLATE class.
  * @param  phost: Host handle
  * @retval USBH Status
  */
static USBH_StatusTypeDef USBH_PRT_InterfaceInit (USBH_HandleTypeDef *phost)
{	
    uint8_t interface = 0;
    USBH_StatusTypeDef status = USBH_FAIL ;
    PRT_HandleTypeDef *PRT_Handle;

    interface = USBH_FindInterface(phost, phost->pActiveClass->ClassCode,
            USB_PRINTER_INTF_SUBCLASS, USB_PRINTER_INTF_PROTOCOL_BIDIR);

    if(interface == 0xFF) /* Not Valid Interface */
    {
      USBH_DbgLog ("Cannot Find the interface for %s class.", phost->pActiveClass->Name);
      status = USBH_FAIL;
    }
    else
    {
      USBH_SelectInterface (phost, interface);

      phost->pActiveClass->pData = (PRT_HandleTypeDef *)USBH_malloc (sizeof(PRT_HandleTypeDef));
      PRT_Handle =  (PRT_HandleTypeDef *) phost->pActiveClass->pData;

      if(phost->device.CfgDesc.Itf_Desc[phost->device.current_interface].Ep_Desc[0].bEndpointAddress & 0x80)
      {
        PRT_Handle->InEp = (phost->device.CfgDesc.Itf_Desc[phost->device.current_interface].Ep_Desc[0].bEndpointAddress);
        PRT_Handle->InEpSize  = phost->device.CfgDesc.Itf_Desc[phost->device.current_interface].Ep_Desc[0].wMaxPacketSize;
      }
      else
      {
        PRT_Handle->OutEp = (phost->device.CfgDesc.Itf_Desc[phost->device.current_interface].Ep_Desc[0].bEndpointAddress);
        PRT_Handle->OutEpSize  = phost->device.CfgDesc.Itf_Desc[phost->device.current_interface].Ep_Desc[0].wMaxPacketSize;
      }

      if(phost->device.CfgDesc.Itf_Desc[phost->device.current_interface].Ep_Desc[1].bEndpointAddress & 0x80)
      {
        PRT_Handle->InEp = (phost->device.CfgDesc.Itf_Desc[phost->device.current_interface].Ep_Desc[1].bEndpointAddress);
        PRT_Handle->InEpSize  = phost->device.CfgDesc.Itf_Desc[phost->device.current_interface].Ep_Desc[1].wMaxPacketSize;
      }
      else
      {
        PRT_Handle->OutEp = (phost->device.CfgDesc.Itf_Desc[phost->device.current_interface].Ep_Desc[1].bEndpointAddress);
        PRT_Handle->OutEpSize  = phost->device.CfgDesc.Itf_Desc[phost->device.current_interface].Ep_Desc[1].wMaxPacketSize;
      }

      //PRT_Handle->current_lun = 0;
      //PRT_Handle->rw_lun = 0;
      PRT_Handle->state = PRT_INIT_STATE;
      PRT_Handle->error = PRT_OK;
      PRT_Handle->req_state = PRT_REQ_IDLE;
      PRT_Handle->OutPipe = USBH_AllocPipe(phost, PRT_Handle->OutEp);
      PRT_Handle->InPipe = USBH_AllocPipe(phost, PRT_Handle->InEp);

      //USBH_MSC_BOT_Init(phost);

      /* De-Initialize LUNs information */
      //USBH_memset(PRT_Handle->unit, 0, sizeof(PRT_Handle->unit));

      /* Open the new channels */
      USBH_OpenPipe  (phost,
                      PRT_Handle->OutPipe,
                      PRT_Handle->OutEp,
                      phost->device.address,
                      phost->device.speed,
                      USB_EP_TYPE_BULK,
                      PRT_Handle->OutEpSize);

      USBH_OpenPipe  (phost,
                      PRT_Handle->InPipe,
                      PRT_Handle->InEp,
                      phost->device.address,
                      phost->device.speed,
                      USB_EP_TYPE_BULK,
                      PRT_Handle->InEpSize);

      USBH_LL_SetToggle  (phost, PRT_Handle->InPipe,0);
      USBH_LL_SetToggle  (phost, PRT_Handle->OutPipe,0);

      status = USBH_OK;

    }

    return status;
}



/**
  * @brief  USBH_TEMPLATE_InterfaceDeInit 
  *         The function DeInit the Pipes used for the TEMPLATE class.
  * @param  phost: Host handle
  * @retval USBH Status
  */
static USBH_StatusTypeDef USBH_PRT_InterfaceDeInit (USBH_HandleTypeDef *phost)
{
    PRT_HandleTypeDef *PRT_Handle = (PRT_HandleTypeDef *) phost->pActiveClass->pData;

    if (PRT_Handle->OutPipe)
    {
        USBH_ClosePipe(phost, PRT_Handle->OutPipe);
        USBH_FreePipe  (phost, PRT_Handle->OutPipe);
        PRT_Handle->OutPipe = 0;     /* Reset the Channel as Free */
    }

    if (PRT_Handle->InPipe)
    {
        USBH_ClosePipe(phost, PRT_Handle->InPipe);
        USBH_FreePipe  (phost, PRT_Handle->InPipe);
        PRT_Handle->InPipe = 0;     /* Reset the Channel as Free */
    }

    if(phost->pActiveClass->pData)
    {
        USBH_free (phost->pActiveClass->pData);
        phost->pActiveClass->pData = 0;
    }

    return USBH_OK;

}

/**
  * @brief  USBH_TEMPLATE_ClassRequest 
  *         The function is responsible for handling Standard requests
  *         for TEMPLATE class.
  * @param  phost: Host handle
  * @retval USBH Status
  */
static USBH_StatusTypeDef USBH_PRT_ClassRequest (USBH_HandleTypeDef *phost)
{   
#if 0

    /** Drive USB library to the next state */
#if (USBH_USE_OS == 1)
    osMessagePut ( phost->os_event, USBH_CONTROL_EVENT, 0);
#endif

    return USBH_OK;

#else

    PRT_HandleTypeDef *PRT_Handle = (PRT_HandleTypeDef *) phost->pActiveClass->pData;
    USBH_StatusTypeDef status = USBH_BUSY;

    switch (PRT_Handle->req_state)
    {
    case PRT_REQ_IDLE:
    case PRT_REQ_GET_DEVICE_ID:
        status = USBH_PRT_BOT_GetDeviceID(phost, PRT_Handle->DeviceID, USB_PRINTER_DEVICE_ID_MAX_SIZE);

        if (status == USBH_NOT_SUPPORTED)
        {
            PRT_Handle->DeviceID[0] = 0;
            status = USBH_OK;
        }

        break;

    case PRT_REQ_RESET:
    case PRT_REQ_ERROR:
    default:
        break;
    }

    if (status == USBH_OK)
        phost->pUser(phost, HOST_USER_CLASS_ACTIVE);

    return status;

#endif
}


/**
  * @brief  USBH_TEMPLATE_Process 
  *         The function is for managing state machine for TEMPLATE data transfers 
  * @param  phost: Host handle
  * @retval USBH Status
  */
static USBH_StatusTypeDef USBH_PRT_Process(USBH_HandleTypeDef *phost)
{
    PRT_HandleTypeDef *PRT_Handle = (PRT_HandleTypeDef *) phost->pActiveClass->pData;
    USBH_StatusTypeDef status = USBH_BUSY;
    //USBH_StatusTypeDef req_status = USBH_OK;

    switch (PRT_Handle->state)
    {
    case PRT_INIT_STATE:
        PRT_Handle->state = PRT_IDLE_STATE;
    case PRT_IDLE_STATE:
        status = USBH_OK;
        break;
/*
    case PRT_GET_PORT_STATUS_STATE:
        req_status = USBH_PRT_BOT_GetPortStatus(phost, &PRT_Handle->port_status, 1);

        if (req_status == USBH_OK)
        {
            PRT_Handle->state = PRT_IDLE;
        }
        else if (req_status != USBH_BUSY)
        {
            PRT_Handle->state = PRT_ERROR_STATE;
        }
        break;
*/
    case PRT_SOFT_RESET:
    case PRT_READ_STATE:
    case PRT_WRITE_STATE:
        status = USBH_OK;
        break;

    case PRT_ERROR_STATE:
        // TODO : Do something if Printer returns error.
        PRT_Handle->state = PRT_IDLE_STATE;
        break;

    }

    return status;  //USBH_OK;
}


/**
  * @brief  USBH_PRT_SOFProcess
  *         The function is for SOF state
  * @param  phost: Host handle
  * @retval USBH Status
  */
static USBH_StatusTypeDef USBH_PRT_SOFProcess(USBH_HandleTypeDef *phost)
{

    return USBH_OK;
}


#if 0
/**
  * @brief  USBH_TEMPLATE_Init 
  *         The function Initialize the TEMPLATE function
  * @param  phost: Host handle
  * @retval USBH Status
  */
USBH_StatusTypeDef USBH_TEMPLATE_Init (USBH_HandleTypeDef *phost)
{
  USBH_StatusTypeDef Status = USBH_BUSY;
#if (USBH_USE_OS == 1)
  osEvent event;
  
  event = osMessageGet( phost->class_ready_event, osWaitForever );
  
  if( event.status == osEventMessage )      
  {
    if(event.value.v == USBH_CLASS_EVENT)
    {
#else 
      
  while ((Status == USBH_BUSY) || (Status == USBH_FAIL))
  {
    /* Host background process */
    USBH_Process(phost);
    if(phost->gState == HOST_CLASS)
    {
#endif        
      Status = USBH_OK;
    }
  }
  return Status;   
}

/**
  * @brief  USBH_TEMPLATE_IOProcess 
  *         TEMPLATE TEMPLATE process
  * @param  phost: Host handle
  * @retval USBH Status
  */
USBH_StatusTypeDef USBH_TEMPLATE_IOProcess (USBH_HandleTypeDef *phost)
{
  if (phost->device.is_connected == 1)
  {
    if(phost->gState == HOST_CLASS)
    {
      USBH_PRT_Process(phost);
    }
  }
  
  return USBH_OK;

}

#endif



static USBH_StatusTypeDef USBH_PRT_BOT_GetDeviceID(USBH_HandleTypeDef *phost,
        uint8_t *o_buf, size_t wbytes)
{

    phost->Control.setup.b.bmRequestType = USB_REQ_PRINTER_IN;
    phost->Control.setup.b.bRequest = USB_PRINTER_GET_DEVICE_ID;
    phost->Control.setup.b.wValue.w = 0;
    phost->Control.setup.b.wIndex.w = 0;
    phost->Control.setup.b.wLength.w = USB_PRINTER_DEVICE_ID_MAX_SIZE;

    return USBH_CtlReq(phost, o_buf, USB_PRINTER_DEVICE_ID_MAX_SIZE);
}

static uint8_t USBH_PRT_BOT_GetPortStatus(USBH_HandleTypeDef *phost,
        uint8_t *o_buf, size_t wbytes)
{

    phost->Control.setup.b.bmRequestType = USB_REQ_PRINTER_IN;
    phost->Control.setup.b.bRequest = USB_PRINTER_GET_PORT_STATUS;
    phost->Control.setup.b.wValue.w = 0;
    phost->Control.setup.b.wIndex.w = 0;
    phost->Control.setup.b.wLength.w = 1;

    return USBH_CtlReq(phost, o_buf, 1);
}

USBH_StatusTypeDef USBH_PRT_SoftReset(USBH_HandleTypeDef *phost)
{

    phost->Control.setup.b.bmRequestType = USB_REQ_PRINTER_OUT;
    phost->Control.setup.b.bRequest = USB_PRINTER_SOFT_RESET;
    phost->Control.setup.b.wValue.w = 0;
    phost->Control.setup.b.wIndex.w = 0;
    phost->Control.setup.b.wLength.w = 0;

    return USBH_CtlReq(phost, NULL, 0);
}

static USBH_StatusTypeDef USBH_PRT_BOT_RdWrProcess(USBH_HandleTypeDef *phost,
        uint8_t *pbuf, const size_t bsize)
{
    PRT_HandleTypeDef *PRT_Handle = (PRT_HandleTypeDef *) phost->pActiveClass->pData;
    USBH_URBStateTypeDef URB_Status = USBH_URB_IDLE;
    USBH_StatusTypeDef status = USBH_BUSY;

    switch (PRT_Handle->hbot.state)
    {
    case PRT_BOT_DATA_OUT:
        PRT_Handle->hbot.state = PRT_BOT_DATA_OUT_WAIT;
        USBH_BulkSendData(phost, pbuf, bsize, PRT_Handle->OutPipe, 0);
        break;

    case PRT_BOT_DATA_OUT_WAIT:
        URB_Status = USBH_LL_GetURBState(phost, PRT_Handle->OutPipe);

        if (URB_Status == USBH_URB_DONE)
        {
            PRT_Handle->hbot.state = PRT_BOT_INIT;
            status = USBH_OK;
        #if (USBH_USE_OS == 1)
            osMessagePut ( phost->os_event, USBH_URB_EVENT, 0);
        #endif
        }
        else if (URB_Status == USBH_URB_NOTREADY)   /** Re-send data */
        {
            PRT_Handle->hbot.state = PRT_BOT_DATA_OUT;
        #if (USBH_USE_OS == 1)
            osMessagePut ( phost->os_event, USBH_URB_EVENT, 0);
        #endif
        }
        else if (URB_Status == USBH_URB_STALL)
        {
            while(1) {} ; // TODO: Abort transfer by ??, and reset toggle.
        }
        break;

    case PRT_BOT_INIT:
    case PRT_BOT_DATA_IN:
    case PRT_BOT_DATA_IN_WAIT:
    default:
        status = USBH_OK;   // TODO: Need support reading data from printer?
    }

    return status;
}

USBH_StatusTypeDef USBH_PRT_ReadData(USBH_HandleTypeDef *phost,
        const uint8_t *pcmd, size_t cmdlen, uint8_t *o_buf, size_t rbytes)
{
    //PRT_HandleTypeDef *PRT_Handle = (PRT_HandleTypeDef *) phost->pActiveClass->pData;
    //return USBH_BulkSendData(phost, pcmd, cmdlen, PRT_Handle->OutPipe, 0);
    return USBH_OK;
}

USBH_StatusTypeDef USBH_PRT_SendData(USBH_HandleTypeDef *phost,
        const uint8_t *i_buf, size_t wb)
{
    PRT_HandleTypeDef *PRT_Handle = (PRT_HandleTypeDef *) phost->pActiveClass->pData;
    uint32_t timeout;

    if (PRT_Handle->state == PRT_IDLE_STATE && phost->gState == HOST_CLASS &&
            phost->device.is_connected)
    {
        PRT_Handle->state = PRT_WRITE_STATE;
        PRT_Handle->hbot.state = PRT_BOT_DATA_OUT;

        timeout = phost->Timer;

        while (USBH_PRT_BOT_RdWrProcess(phost, (uint8_t *) i_buf, wb) == USBH_BUSY)
        {
            if ((phost->Timer - timeout) > 1000 * wb || phost->device.is_connected == 0)
            {
                PRT_Handle->state = PRT_IDLE_STATE;
                return USBH_FAIL;
            }
        }

        PRT_Handle->state = PRT_IDLE_STATE;
        return USBH_OK;
    }

    return USBH_FAIL;
}

size_t USBH_PRT_GetDeviceID(USBH_HandleTypeDef *phost, uint8_t *o_buf, size_t bsize)
{
    PRT_HandleTypeDef *PRT_Handle = (PRT_HandleTypeDef *) phost->pActiveClass->pData;

    if (bsize < USB_PRINTER_DEVICE_ID_MAX_SIZE)
        bsize = USB_PRINTER_DEVICE_ID_MAX_SIZE;

    memcpy(o_buf, PRT_Handle->DeviceID, bsize);

    return bsize;
}

USBH_StatusTypeDef USBH_PRT_GetPortStatus(USBH_HandleTypeDef *phost,
        uint8_t *o_buf, size_t wbytes)
{
    PRT_HandleTypeDef *PRT_Handle = (PRT_HandleTypeDef *) phost->pActiveClass->pData;
    uint32_t timeout;

    if (PRT_Handle->state == PRT_IDLE_STATE)
    {
        timeout = phost->Timer;

        while (USBH_PRT_BOT_GetPortStatus(phost, o_buf, 1) == USBH_BUSY)
        {
            if ((phost->Timer - timeout) > 10000 || (phost->device.is_connected == 0))
            {
                return USBH_FAIL;
            }
        }
    }

    return USBH_OK;
}



// End of file
