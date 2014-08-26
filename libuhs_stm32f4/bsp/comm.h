#ifndef COMM_H_
#define COMM_H_

#include <stm32f4xx.h>


#define USARTx USART1
typedef enum
{
  COM1 = 0,
  COM2 = 1
} COM_TypeDef;


/** @addtogroup STM3210C_EVAL_LOW_LEVEL_COM
  * @{
  */
#define COMn 1

/**
 * @brief Definition for COM port1, connected to USART2 (USART2 pins remapped on GPIOD)
 */
#define EVAL_COM1                        USART2
#define EVAL_COM1_CLK                    RCC_APB1Periph_USART2
#define EVAL_COM1_TX_PIN                 GPIO_Pin_2
#define EVAL_COM1_TX_GPIO_PORT           GPIOA
#define EVAL_COM1_TX_GPIO_CLK            RCC_AHB1Periph_GPIOA
#define EVAL_COM1_RX_PIN                 GPIO_Pin_3
#define EVAL_COM1_RX_GPIO_PORT           GPIOA
#define EVAL_COM1_RX_GPIO_CLK            RCC_AHB1Periph_GPIOA
#define EVAL_COM1_IRQn                   USART2_IRQn

/**
  * @}
  */

void comm_init(void);
int  comm_test(void);
void comm_put(char);
void comm_puts(const char*);
char comm_getc(void);

void __uart_putchar(int ch);
//uint8_t GetKey(void);

#endif

