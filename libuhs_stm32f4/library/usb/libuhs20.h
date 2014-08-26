/*
 * uhs20.h
 *
 *  Created on: 2014Äê7ÔÂ25ÈÕ
 *      Author: kiya
 */

#ifndef UHS20_H_
#define UHS20_H_

#include <stdint.h>

/* IO definitions (access restrictions to peripheral registers) */
#ifdef __cplusplus
  #define     __I     volatile           /*!< defines 'read only' permissions                 */
#else
  #define     __I     volatile const     /*!< defines 'read only' permissions                 */
#endif
#define     __O     volatile             /*!< defines 'write only' permissions                */
#define     __IO    volatile             /*!< defines 'read / write' permissions              */

typedef unsigned char U8;
typedef unsigned short U16;
typedef unsigned long U32;

#ifndef FS_MAX_USB_MS_DRIVERS
#define FS_MAX_USB_MS_DRIVERS (1)
#endif

#define DEBUG_PIN (2)
#define bmSE0       0xF0    	// remove.. disconnect state ?
#define bmSE1       0xF1    	// remove.. reserved / illegal state ?

#define bmHIGHSPEED (0)	//HPRT0_PRTSPD_HIGH_SPEED
#define bmFULLSPEED (1)	//HPRT0_PRTSPD_FULL_SPEED		// full speed
#define bmLOWSPEED	(2)	//HPRT0_PRTSPD_LOW_SPEED		// low speed
#define bmHUBPRE    (4)

#define SE0     (0)
#define SE1     (1)
#define FSHOST  (2)
#define LSHOST  (3)

/* Host error result codes, the 4 LSB's in the HRSL register */
#define hrSUCCESS   (0x00)
#define hrBUSY      (0x01)
#define hrBADREQ    (0x02)
#define hrUNDEF     (0x03)
#define hrNAK       (0x04)
#define hrSTALL     (0x05)
#define hrTOGERR    (0x06)
#define hrWRONGPID  (0x07)
#define hrBADBC     (0x08)
#define hrPIDERR    (0x09)
#define hrPKTERR    (0x0A)
#define hrCRCERR    (0x0B)
#define hrKERR      (0x0C)
#define hrJERR      (0x0D)
#define hrTIMEOUT   (0x0E)
#define hrBABBLE    (0x0F)

/* Host transfer token values for writing the HXFR register (R30)   */
/* OR this bit field with the endpoint number in bits 3:0               */
#define tokSETUP  0x10  // HS=0, ISO=0, OUTNIN=0, SETUP=1
#define tokIN     0x00  // HS=0, ISO=0, OUTNIN=0, SETUP=0
#define tokOUT    0x20  // HS=0, ISO=0, OUTNIN=1, SETUP=0
#define tokINHS   0x80  // HS=1, ISO=0, OUTNIN=0, SETUP=0
#define tokOUTHS  0xA0  // HS=1, ISO=0, OUTNIN=1, SETUP=0
#define tokISOIN  0x40  // HS=0, ISO=1, OUTNIN=0, SETUP=0
#define tokISOOUT 0x60  // HS=0, ISO=1, OUTNIN=1, SETUP=0

/* NAK powers. To save space in endpoint data structure, amount of retries before giving up and returning 0x4 is stored in */
/* bmNakPower as a power of 2. The actual nak_limit is then calculated as nak_limit = ( 2^bmNakPower - 1) */
#define USB_NAK_MAX_POWER		15		//NAK binary order maximum value
#define USB_NAK_DEFAULT			14		//default 32K-1 NAKs before giving up
#define USB_NAK_NOWAIT			1		//Single NAK stops transfer
#define USB_NAK_NONAK			0		//Do not count NAKs, stop retrying after USB Timeout

typedef enum
{
	eUrbIdle = 0,	//URB_IDLE = 0,
	eUrbDone, 		//URB_DONE,
	eUrbNotReady, 	//URB_NOTREADY,
	eUrbError,	//URB_ERROR,
	eUrbStall	//URB_STALL
} libuhs_urb_state;

typedef enum
{
	eEpTypeControl = 0,	// EP_TYPE_CTRL = 0
	eEpTypeIsoc = 1,	// EP_TYPE_ISOC
	eEpTypeBulk = 2,	// EP_TYPE_BULK
	eEpTypeInterrupt = 3,	// EP_TYPE_INTR

	eEpMask	= 3		//EP_TYPE_MSK
} libuhs_ep_type;

/* --------- callback definitions --------- */

#ifdef __cplusplus
extern "C" {
#endif

enum libuhs_errors {
	eLibuhsSuccess = 0,
	eLibuhsErrorInvalidParam = -1,


	eLibuhsError = -99
};

typedef void (*libuhs_delay_micro_second_callback)(U32 micro_second);
typedef void (*libuhs_delay_milli_second_callback)(U32 milli_second);
typedef U32 (*libuhs_get_current_milli_second_callback)(void);
typedef void (*libuhs_printf_callback)(const char* str, ...);
typedef void (*libuhs_debug_pin_toggle_callback)(uint8_t pin);
/*
 * Attached callback
 * extern uint32_t HCD_ResetPort(USB_OTG_CORE_HANDLE *pdev);
 * host.ConnSts
 * host.port_need_reset
*/
typedef int (*libuhs_device_attached_callback)(U8 is_low_speed, U8 nak_limit);
typedef int (*libuhs_device_is_attached_callback)(void);
typedef int (*libuhs_device_is_low_speed_callback)(void);
typedef int (*libuhs_device_need_second_reset_callback)(void);
typedef int (*libuhs_get_device_speed_mode_callback)(void);
/*
 * Added callback
 * extern uint8_t USBH_Alloc_Channel(USB_OTG_CORE_HANDLE *pdev, uint8_t ep_addr);
 * extern uint8_t USBH_Open_Channel(USB_OTG_CORE_HANDLE *pdev, uint8_t hc_num, uint8_t dev_address, uint8_t speed, uint8_t ep_type, uint16_t mps);
 */
typedef int (*libuhs_device_added_callback)(
		U8 dev_addr, U8 is_low_speed, U8 nak_limit,
		U8 out_ep_addr, U8 out_ep_pkt_size, U8 out_ep_type,
		U8 in_ep_addr, U8 in_ep_pkt_size, U8 in_ep_type
);

/*
 * Removed callback
 * extern USBH_Status USBH_Free_Channel(USB_OTG_CORE_HANDLE *pdev, uint8_t idx);
 * extern USB_OTG_STS USB_OTG_HC_Halt(USB_OTG_CORE_HANDLE *pdev, uint8_t hc_num);
 */
typedef int (*libuhs_device_removed_callback)(U8 pipe_num);
/*
 * Reset callback
 * extern void USB_OTG_StopHost(USB_OTG_CORE_HANDLE *pdev);
 */
typedef int (*libuhs_device_reset_callback)(void);
/*
 * Lookup callback
 * extern uint8_t HCD_GetHCState(USB_OTG_CORE_HANDLE *pdev, uint8_t ch_num);
 * extern URB_STATE HCD_GetURB_State(USB_OTG_CORE_HANDLE *pdev, uint8_t ch_num);
 * host.SofHits
 * host.XferCnt
 */
typedef int (*libuhs_get_usb_pipe_status_callback)(U8 pipe_num);
typedef int (*libuhs_get_usb_request_block_status_callback)(U8 pipe_num);
typedef int (*libuhs_get_received_bytes_callback)(U8 pipe_num);
typedef int (*libuhs_get_received_sof_callback)(void);
typedef int (*libuhs_get_nak_count_callback)(uint8_t pipe_num);
/*
 * toggle callback
 * host.hc[num].toggle_in
 * host.hc[num].toggle_out
 */
typedef int (*libuhs_get_toggle_flag_callback)(U8 pipe_num, U8 direction);
typedef int (*libuhs_set_toggle_flag_callback)(U8 pipe_num, U8 direction, U8 new_toggle);
/*
 * Data transfer callback
 * extern USBH_Status USBH_BulkSendData(USB_OTG_CORE_HANDLE *pdev, const uint8_t *buff, uint16_t length, uint8_t hc_num);
 *
 */
typedef int (*libuhs_out_transfer_callback)(const U8 *buf, U16 length, U8 pipe_num, U8 ep, U8 mps);
typedef int (*libuhs_in_transfer_callback)(U8 *buf, U16 length, U8 pipe_num, U8 devAddr, U8 ep, U8 mps);
/*
 * Control data transfer callback
 * extern USBH_Status USBH_CtlSendSetup(USB_OTG_CORE_HANDLE *pdev, uint8_t *buff, uint8_t hc_num);
 */
typedef int (*libuhs_control_out_transfer_callback)(U8 token, const U8 *buf, U8 pipe_num, U8 devAddr, U8 ep, U8 mps);
typedef int (*libuhs_control_in_transfer_callback)(U8 *buf, U16 length, U8 pipe_num, U8 devAddr, U8 ep, U8 mps);
//extern USB_OTG_STS HCD_SubmitRequest(USB_OTG_CORE_HANDLE *pdev , uint8_t hc_num);


typedef struct LibuhsSystemCallbacks
{
	libuhs_delay_micro_second_callback delay_micro_second_cb;
	libuhs_delay_milli_second_callback delay_milli_second_cb;
	libuhs_get_current_milli_second_callback get_milli_second_cb;
	libuhs_printf_callback printf;
	libuhs_debug_pin_toggle_callback debug_pin_toggle_cb;

} LibuhsSystemCallbacks;

typedef struct LibuhsUsbHostCallbacks
{
	libuhs_device_attached_callback device_attached_cb;
	libuhs_device_is_attached_callback is_attached_cb;
	libuhs_device_is_low_speed_callback is_low_speed_cb;
	libuhs_device_need_second_reset_callback need_second_reset_cb;
	libuhs_get_device_speed_mode_callback get_speed_mode_cb;
	libuhs_device_added_callback device_added_cb;
	libuhs_device_removed_callback device_removed_cb;
	libuhs_device_reset_callback reset_device_cb;
	libuhs_get_usb_pipe_status_callback get_pipe_status_cb;
	libuhs_get_usb_request_block_status_callback get_urb_status_cb;
	libuhs_get_received_bytes_callback get_received_bytes_cb;
	libuhs_get_received_sof_callback get_sof_hits_cb;
	libuhs_get_nak_count_callback get_nak_count_cb;
	libuhs_get_toggle_flag_callback get_toggle_cb;
	libuhs_set_toggle_flag_callback set_toggle_cb;
	libuhs_out_transfer_callback out_transfer_cb;
	libuhs_in_transfer_callback in_transfer_cb;
	libuhs_control_out_transfer_callback control_out_transfer_cb;
	libuhs_control_in_transfer_callback control_in_transfer_cb;

} LibuhsUsbHostCallbacks;

typedef struct
{
	struct {
		uint8_t bmLeftButton : 1;
		uint8_t bmRightButton : 1;
		uint8_t bmMiddleButton : 1;
		uint8_t bmDummy : 1;
	};
	int8_t dX;
	int8_t dY;
} MOUSEINFO;

//typedef unsigned char (*libuhs_class_hidboot_key_pressed_callback)(void);
typedef unsigned char (*libuhs_class_hidboot_key_released_callback)(U8 mod, U8 key);
typedef unsigned char (*libuhs_class_hidboot_mouse_activated_callback)(MOUSEINFO *mi);
//typedef unsigned char (*libuhs_class_hidboot_mouse_clicked_callback)(void);

typedef struct LibuhsUsbClassCallbacks
{
	//libuhs_class_hidboot_key_pressed_callback class_key_pressed_cb;
	libuhs_class_hidboot_key_released_callback class_key_released_cb;
	libuhs_class_hidboot_mouse_activated_callback class_mouse_activated_cb;
	//libuhs_class_hidboot_mouse_clicked_callback class_mouse_clicked_cb;

} LibuhsUsbClassCallbacks;

unsigned char register_system_callback(LibuhsSystemCallbacks *callbacks, void *callback_context);
unsigned char register_usb_kernel_callback(LibuhsUsbHostCallbacks *callbacks, void *callback_context);
unsigned char register_usb_class_callback(LibuhsUsbClassCallbacks *callbacks, void *callback_context);

/* --------- callback definitions (end) --------- */

/* --------- library APIs --------- */

// HIDBOOT API
void libuhs_class_hidboot_init(void);
uint8_t libuhs_class_hidboot_is_ready(void);

// CDC API
void libuhs_class_cdc_init(uint32_t baudrate, uint8_t flow_control);
uint8_t libuhs_class_cdc_receive_data(uint16_t *bytes_rcvd, uint8_t *dataptr);
uint8_t libuhs_class_cdc_send_data(uint16_t nbytes, const uint8_t *dataptr);
uint8_t libuhs_class_cdc_is_ready(void);

// Priter API
void libuhs_class_printer_init(void);
uint8_t libuhs_class_printer_is_ready(void);
uint8_t libuhs_class_printer_send_data(uint8_t *buf, uint16_t nbytes);

// Mass storage API
void libuhs_class_msc_init(void);
uint8_t libuhs_class_msc_is_ready(uint8_t drive);
uint8_t libuhs_class_msc_get_max_lun(uint8_t drive);
uint8_t libuhs_class_msc_lun_is_good(uint8_t drive, uint8_t lun);
uint32_t libuhs_class_msc_get_capacity(uint8_t drive, uint8_t lun);
uint16_t libuhs_class_msc_get_sector_size(uint8_t drive, uint8_t lun);
uint8_t libuhs_class_msc_write_protected(uint8_t drive, uint8_t lun);
uint8_t libuhs_class_msc_read_data(uint8_t drive, uint8_t lun, uint32_t addr, uint16_t bsize, uint8_t blocks, uint8_t *buf);
uint8_t libuhs_class_msc_write_data(uint8_t drive, uint8_t lun, uint32_t addr, uint16_t bsize, uint8_t blocks, uint8_t * buf);
void libuhs_class_msc_disable_polling(uint8_t drive);
void libuhs_class_msc_enable_polling(uint8_t drive);

// Library API
int libuhs_kernel_init(void);
void libuhs_class_hub_init(void);
void libuhs_task(void);

/* --------- library APIs (end) --------- */
#ifdef __cplusplus
}
#endif
#endif /* UHS20_H_ */
