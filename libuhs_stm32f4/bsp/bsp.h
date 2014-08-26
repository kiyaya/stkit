/*
 * bsp.c
 *
 *  Created on: 2014-3-6
 *      Author: kiya
 */

#ifndef _bsp_h_
#define _bsp_h_
#ifdef __cplusplus
 extern "C" {
#endif

 /* --- includes. --- */
#include <stdbool.h>
#include <stddef.h>

#include "misc.h" /* High level functions for NVIC and SysTick (add-on to CMSIS functions) */
#include "comm.h"
#include "delay.h"
#include "gpio.h"
#include "rtc.h"
#include "swo_printf.h"
//#include "dcc_stdio.h"
#include "tiny_printf.h"

 /* External function prototypes (defined in syscalls.c) */
extern int printf(const char *fmt, ...);

/* --- Function declaration --- */
void BSP_init(void);


/* --- Compiler related --- */
void __cxa_pure_virtual(void);
//#define __init      __attribute__ ((__section__ (".init.text")))


#ifdef __cplusplus
}
#endif
#endif
