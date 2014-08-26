/*
 * main_uhs.c
 *
 *  Created on: 2014Äê7ÔÂ29ÈÕ
 *      Author: kiya
 */

#include "libuhs20.h"
#include <bsp/bsp.h>
#include <bsp/bsp_usb_otg/inc/usb_bsp.h>
#include <bsp/bsp_usb_otg/inc/usbh_hcs.h>

extern void xprintf (const char* str, ...);

USB_OTG_CORE_HANDLE USB_OTG_Core_dev;
static USB_OTG_CORE_HANDLE *mpUsbDevice = &USB_OTG_Core_dev;

int device_attached(uint8_t is_low_speed, uint8_t nak_limit)
{
	// assume:
	// HC0 for control - out
    // HC1 for control - in
    //uint8_t hcnum = USBH_GetFreeChannel(pdev);
    //if(hcnum > 1) {
    //	USBH_Free_Channel(pdev, epInfo.hcNumOut);
    //	USBH_Free_Channel(pdev, epInfo.hcNumIn);
    //}
    //USBH_Free_Channel(pdev, 0);
    //USBH_Free_Channel(pdev, 1);
	//epInfo.hcNumOut = USBH_Alloc_Channel(pdev, 0x00);	// ep_addr = 0
	//epInfo.hcNumIn = USBH_Alloc_Channel(pdev, 0x80);
	//USBH_Open_Channel(pdev, epInfo.hcNumOut, 0x0, (lowspeed)?bmLOWSPEED:bmFULLSPEED, EP_TYPE_CTRL, 0x8);
	//USBH_Open_Channel(pdev, epInfo.hcNumIn,	0x0, (lowspeed)?bmLOWSPEED:bmFULLSPEED, EP_TYPE_CTRL, 0x8);

	uint8_t out_pipe_num, in_pipe_num;

    USBH_Free_Channel(mpUsbDevice, 0);
    USBH_Free_Channel(mpUsbDevice, 1);
	out_pipe_num = USBH_Alloc_Channel(mpUsbDevice, 0x00);	// ep_addr = 0
	in_pipe_num = USBH_Alloc_Channel(mpUsbDevice, 0x80);

    nak_limit = (0x0001UL << ((nak_limit > USB_NAK_MAX_POWER) ? USB_NAK_MAX_POWER : nak_limit));
    nak_limit--;

	USBH_Open_Channel(mpUsbDevice, out_pipe_num, 0x0, (is_low_speed)?bmLOWSPEED:bmFULLSPEED, EP_TYPE_CTRL, 0x8, nak_limit);
	USBH_Open_Channel(mpUsbDevice, in_pipe_num, 0x0, (is_low_speed)?bmLOWSPEED:bmFULLSPEED, EP_TYPE_CTRL, 0x8, nak_limit);

	return (in_pipe_num | (out_pipe_num << 4));
}

int device_added(uint8_t dev_addr, uint8_t is_low_speed, uint8_t nak_limit,
		uint8_t out_ep_addr, uint8_t out_ep_pkt_size, uint8_t out_ep_type,
		uint8_t in_ep_addr, uint8_t in_ep_pkt_size, uint8_t in_ep_type)
{
	uint8_t out_pipe_num = 0, in_pipe_num = 0;

	if(out_ep_addr != 0)
	{
		out_pipe_num = USBH_Alloc_Channel(mpUsbDevice, out_ep_addr);	//Out, addr = 0x01
		USBH_Open_Channel(mpUsbDevice, out_pipe_num, dev_addr,
			(is_low_speed) ? bmLOWSPEED : bmFULLSPEED, out_ep_type, out_ep_pkt_size, nak_limit);
	}
	if(in_ep_addr != 0)
	{
		in_pipe_num = USBH_Alloc_Channel(mpUsbDevice, in_ep_addr);		//In, addr = 0x82
		USBH_Open_Channel(mpUsbDevice, in_pipe_num, dev_addr,
			(is_low_speed) ? bmLOWSPEED : bmFULLSPEED, in_ep_type, in_ep_pkt_size, nak_limit);
	}
	return (in_pipe_num | (out_pipe_num << 4));
}

int device_removed(uint8_t pipe_num)
{
	USB_OTG_HC_Halt(mpUsbDevice, pipe_num);
	USBH_Free_Channel(mpUsbDevice, pipe_num);

	return eLibuhsSuccess;
}

int is_attached(void)
{
	return mpUsbDevice->host.ConnSts;
}

int is_low_speed(void)
{
	return eLibuhsSuccess;
}

/*
 * @brief	: Once a LS device is attached, we may need a 2nd reset.
 */
int need_second_reset(void)
{
	return mpUsbDevice->host.port_need_reset;
}

int get_speed_mode(void)
{
	USB_OTG_HPRT0_TypeDef hprt0;
	hprt0.d32 = USB_OTG_READ_REG32(mpUsbDevice->regs.HPRT0);
	return hprt0.b.prtspd;
}

int reset_device(void)
{
	mpUsbDevice->host.port_need_reset = 0;
	mpUsbDevice->host.SofHits = 0;
	HCD_ResetPort(mpUsbDevice);
	return eLibuhsSuccess;
}

int get_pipe_status(uint8_t pipe_num)
{
	/* not used
					  HC_HALTED,
					  HC_NYET,
					  HC_XACTERR,

	 *
	#define hrBADREQ    0x02
	#define hrUNDEF     0x03
	#define hrWRONGPID  0x07
	#define hrBADBC     0x08
	#define hrPIDERR    0x09
	#define hrCRCERR    0x0B
	#define hrKERR      0x0C
	#define hrJERR      0x0D
	#define hrBUSY
					  */
	uint8_t pipe_status = HCD_GetHCState(mpUsbDevice, pipe_num);
	uint8_t rcode = hrSUCCESS;

	switch(pipe_status) {
		case HC_XFRC:
			rcode = hrSUCCESS;
			break;
		case HC_NAK:
			rcode = hrNAK;
			break;
		case HC_STALL:
			rcode = hrSTALL;
			break;
		case HC_BBLERR:
			rcode = hrBABBLE;
			break;
		case HC_DATATGLERR:
			rcode = hrTOGERR;
			break;
		case HC_HALTED:
		case HC_NYET:
		case HC_IDLE:	//todo: what does this mean?
			rcode = hrPKTERR;	// no matching error, so just use pkterr tempxxly
			break;
		case HC_XACTERR:
			rcode = hrTIMEOUT;
			break;
		default:
			rcode = pipe_status;
			break;
	}

	return rcode;
}

int get_urb_status(uint8_t pipe_num)
{
	return HCD_GetURB_State(mpUsbDevice, pipe_num);
}

int get_received_bytes(uint8_t pipe_num)
{
	return mpUsbDevice->host.XferCnt[pipe_num];
}

int get_sof_hits(void)
{
	return mpUsbDevice->host.SofHits;
}

int get_nak_count(uint8_t pipe_num)
{
	return mpUsbDevice->host.hc[pipe_num].nak_count;
}
int get_toggle(uint8_t pipe_num, uint8_t direction)
{
	if(direction == tokOUT)
		return mpUsbDevice->host.hc[pipe_num].toggle_out;
	else if(direction == tokIN)
		return mpUsbDevice->host.hc[pipe_num].toggle_in;
	else
		return eLibuhsErrorInvalidParam;
}

int set_toggle(uint8_t pipe_num, uint8_t direction, uint8_t new_toggle)
{
	new_toggle &= 0x1;
	if(direction == tokOUT)
		mpUsbDevice->host.hc[pipe_num].toggle_out = new_toggle;
	else if(direction == tokIN)
		mpUsbDevice->host.hc[pipe_num].toggle_in = new_toggle;
	else
		return eLibuhsErrorInvalidParam;
	return eLibuhsSuccess;
}

int out_transfer(const uint8_t *buf, uint16_t length, uint8_t pipe_num, uint8_t ep, uint8_t mps)
{
	return USBH_BulkSendData(mpUsbDevice, buf, length, pipe_num, ep, mps);
}

int in_transfer(uint8_t *buf, uint16_t length, uint8_t pipe_num, uint8_t devAddr, uint8_t ep, uint8_t mps)
{
	return USBH_BulkReceiveData(mpUsbDevice, buf, length, pipe_num, devAddr, ep, mps);
}

int control_out_transfer(uint8_t token, const uint8_t *buf, uint8_t pipe_num, uint8_t devAddr, uint8_t ep, uint8_t mps)
{
	if(token == tokSETUP)
	{
		return USBH_CtlSendSetup(mpUsbDevice, buf, pipe_num, devAddr, ep, mps);
	} else if(token == tokOUTHS) {
		return USBH_CtlSendData(mpUsbDevice, buf, 0, pipe_num, devAddr, ep, mps);
	} else {
		return eLibuhsErrorInvalidParam;
	}
}

int control_in_transfer(uint8_t *buf, uint16_t length, uint8_t pipe_num, uint8_t devAddr, uint8_t ep, uint8_t mps)
{
	return USBH_CtlReceiveData(mpUsbDevice, buf, length, pipe_num, devAddr, ep, mps);
}

LibuhsSystemCallbacks system_callbacks = {
	.delay_micro_second_cb = delay_us,
	.delay_milli_second_cb = delay_ms,
	.get_milli_second_cb = millis,	//_raw_tick,
	.printf = xprintf,
	.debug_pin_toggle_cb = STM_EVAL_LEDToggle
};

LibuhsUsbHostCallbacks usb_kernel_callbacks = {
	.device_attached_cb = device_attached,
	.is_attached_cb = is_attached,
	.is_low_speed_cb = is_low_speed,
	.need_second_reset_cb = need_second_reset,
	.get_speed_mode_cb = get_speed_mode,
	.device_added_cb = device_added,
	.device_removed_cb = device_removed,
	.reset_device_cb = reset_device,
	.get_pipe_status_cb = get_pipe_status,
	.get_urb_status_cb = get_urb_status,
	.get_received_bytes_cb = get_received_bytes,
	.get_sof_hits_cb = get_sof_hits,
	.get_nak_count_cb = get_nak_count,
	.get_toggle_cb = get_toggle,
	.set_toggle_cb = set_toggle,
	.out_transfer_cb = out_transfer,
	.in_transfer_cb = in_transfer,
	.control_out_transfer_cb = control_out_transfer,
	.control_in_transfer_cb = control_in_transfer
};

#if 1	// todo : add a buffer to cache these inputs for PEG polling then.
//PegKeyboardDevice *mpKeyDev = new PegKeyboardDevice(PegUSBKeyTable, PEG_USB_KEYTABLE_LEN);
unsigned char key_released(U8 mod, U8 key)
{
    //if(mpKeyDev != NULL) {
    //	if(OemToAscii(mod, key) || isLockingKey(key))
    //		mpKeyDev->SendUSBKeyMessage(key, mod);
    //};
}
#ifdef PEG_MOUSE_SUPPORT
IntegrityMouse *mpMouseDev = new IntegrityMouse;
#endif
unsigned char mouse_activated(MOUSEINFO *mi)
{
	#ifdef PEG_MOUSE_SUPPORT
		unsigned int button = 0;
		if(mpMouseDev != NULL) {
			if(mi->bmLeftButton)	button |= 1;
			if(mi->bmRightButton)	button |= 2;

			mpMouseDev->UpdateState(button, mi->dX, mi->dY);
			mpMouseDev->Run();
		}
	#endif
}
#endif

LibuhsUsbClassCallbacks usb_class_callbacks = {
	.class_key_released_cb = key_released,
	.class_mouse_activated_cb = mouse_activated

};
void register_libuhs_callback(void)
{
	register_system_callback(&system_callbacks, 0);
	register_usb_kernel_callback(&usb_kernel_callbacks, 0);
	register_usb_class_callback(&usb_class_callbacks, 0);
}
