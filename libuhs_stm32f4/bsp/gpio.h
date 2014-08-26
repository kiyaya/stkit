/*
 * gpio.h
 *
 *  Created on: 2013-12-29
 *      Author: huihuiyao
 */

#ifndef __GPIO_H
#define __GPIO_H

#include <stm32f4xx.h>

#define LED1_ON		GPIO_SetBits(GPIOC, GPIO_Pin_8);
#define LED1_OFF	GPIO_ResetBits(GPIOC, GPIO_Pin_8);
#define LED2_ON		GPIO_SetBits(GPIOC, GPIO_Pin_7);
#define LED2_OFF	GPIO_ResetBits(GPIOC, GPIO_Pin_7);
#define LED3_ON		GPIO_SetBits(GPIOC, GPIO_Pin_6);
#define LED3_OFF	GPIO_ResetBits(GPIOC, GPIO_Pin_6);
#define PWMON       GPIO_SetBits(GPIOC, GPIO_Pin_0);
#define PWMOFF      GPIO_ResetBits(GPIOC, GPIO_Pin_0);

#define LEDn		3

typedef enum 
{  
  BUTTON_WAKEUP = 0,
  BUTTON_KEY = 2,
} Button_TypeDef;

typedef enum 
{  
  BUTTON_MODE_GPIO = 0,
  BUTTON_MODE_EXTI = 1
} ButtonMode_TypeDef;

typedef enum 
{
  LED1 = 0,
  LED2 = 1,
  LED3 = 2,
} Led_TypeDef;

#define LED1_PIN			GPIO_Pin_8
#define LED1_GPIO_PORT		GPIOC
#define LED1_GPIO_CLK		RCC_AHB1Periph_GPIOC
  
#define LED2_PIN			GPIO_Pin_7
#define LED2_GPIO_PORT		GPIOC
#define LED2_GPIO_CLK		RCC_AHB1Periph_GPIOC

#define LED3_PIN			GPIO_Pin_6  
#define LED3_GPIO_PORT		GPIOC
#define LED3_GPIO_CLK		RCC_AHB1Periph_GPIOC

void GPIO_Configuration(void);
void STM_EVAL_LEDInit(Led_TypeDef Led);
void STM_EVAL_LEDOn(Led_TypeDef Led);
void STM_EVAL_LEDOff(Led_TypeDef Led);
void STM_EVAL_LEDToggle(Led_TypeDef Led);
void Button_Init(ButtonMode_TypeDef Button_Mode);
uint32_t Button_GetState(void);
void LEDOn(Led_TypeDef Led);
void LEDOff(Led_TypeDef Led);
void LEDToggle(Led_TypeDef Led);

#endif

/**********************************END OF FILE**********************************/
