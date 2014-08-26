/*
 * bsp.c
 *
 *  Created on: 2014-3-6
 *      Author: kiya
 */

#include "bsp.h"

void BSP_init(void)
{
	/* Configure the system ready to run the demo.  The clock configuration
	can be done here if it was not done before main() was called. */

	GPIO_Configuration();

#ifdef USE_SDCARD
	comm_init();
	rtc_init();
#endif
}

void __cxa_pure_virtual(void) { while (1); }
