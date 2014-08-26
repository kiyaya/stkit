/*
 * delay.h
 *
 *  Created on: 2013-12-29
 *      Author: huihuiyao
 */

#ifndef __DELAY_H
#define __DELAY_H

#include <stm32f4xx.h>


/* --- Delay definition --- */

uint32_t millis(void);
void delay_ms(uint32_t count);
void delay_us(uint32_t count);
#define delay delay_ms

//void delay(__IO uint32_t nCount);
//void delay_ms(__IO uint32_t nTime);
void delay_tick_calc(void);
#endif

/**********************************END OF FILE**********************************/
