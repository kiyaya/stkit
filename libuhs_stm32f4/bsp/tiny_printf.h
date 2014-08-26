/*
 * tiny_printf.h
 *
 *  Created on: 2014Äê3ÔÂ28ÈÕ
 *      Author: kiya
 */

#ifndef TINY_PRINTF_H_
#define TINY_PRINTF_H_
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stm32f4xx.h>
#include <stdarg.h>

void ts_itoa(char **buf, unsigned int d, int base);
int ts_formatstring(char *buf, const char *fmt, va_list va);
int ts_formatlength(const char *fmt, va_list va);

void put_dump (const uint8_t *buff, uint32_t ofs, int cnt);
int xatoi (char **str, long *res);
extern void xprintf (const char* str, ...);
void xputs(const char* str);

#ifdef __cplusplus
}
#endif
#endif /* TINY_PRINTF_H_ */
