/*
 * comm.c
 *
 *  Based on Martin Thomas's version
 *  Created on: 2014-3-6
 *      Author: kiya
 */

/* Martin Thomas 4/2009, 3/2010 */
#include "comm.h"

USART_TypeDef* COM_USART[COMn] = {EVAL_COM1};
GPIO_TypeDef* COM_TX_PORT[COMn] = {EVAL_COM1_TX_GPIO_PORT};
GPIO_TypeDef* COM_RX_PORT[COMn] = {EVAL_COM1_RX_GPIO_PORT};
const uint32_t COM_USART_CLK[COMn] = {EVAL_COM1_CLK};
const uint32_t COM_TX_PORT_CLK[COMn] = {EVAL_COM1_TX_GPIO_CLK};
const uint32_t COM_RX_PORT_CLK[COMn] = {EVAL_COM1_RX_GPIO_CLK};
const uint16_t COM_TX_PIN[COMn] = {EVAL_COM1_TX_PIN};
const uint16_t COM_RX_PIN[COMn] = {EVAL_COM1_RX_PIN};
const uint16_t COM_USART_INT[COMn] = {EVAL_COM1_IRQn};

static void STM_EVAL_COMInit(COM_TypeDef COM, USART_InitTypeDef* USART_InitStruct);

char RxBuffer2[100];
static uint8_t RxCounter2 = 0;
static uint8_t head_of_rxbuf = 0;
static uint8_t avai_of_rxbuf = 0;
const uint8_t NbrOfDataToRead2 = sizeof(RxBuffer2) / sizeof(uint8_t);

int comm_test(void)
{
	return ( USART_GetFlagStatus(USARTx, USART_FLAG_RXNE) == RESET ) ? 0 : 1;
}

char comm_get(void)
{
	while(USART_GetFlagStatus(USARTx, USART_FLAG_RXNE) == RESET) { ; }
	return (char)USART_ReceiveData(USARTx);
}

char comm_getc(void) {
	char c = 0;

	__disable_irq();
	if(avai_of_rxbuf) {
		c = RxBuffer2[head_of_rxbuf++];
		avai_of_rxbuf--;
		if(head_of_rxbuf > NbrOfDataToRead2)
			head_of_rxbuf = 0;
	}
	__enable_irq();

	return c;
}

void comm_put(char d)
{
	while(USART_GetFlagStatus(USARTx, USART_FLAG_TXE) == RESET) { ; }
	USART_SendData(USARTx, (uint16_t)d);
}

void comm_puts(const char* s)
{
	char c;
	while ( ( c = *s++) != '\0' ) {
		comm_put(c);
	}
}

void comm_init (void)
{
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	/* Configure the NVIC Preemption Priority Bits */
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);

	/* Enable the USART Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = COM_USART_INT[COM1];
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	USART_InitStructure.USART_BaudRate = 115200;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

	STM_EVAL_COMInit(COM1, &USART_InitStructure);	// RX in INT mode
}


/**
  * @brief  Configures COM port.
  * @param  COM: Specifies the COM port to be configured.
  *   This parameter can be one of following parameters:
  *     @arg COM1
  *     @arg COM2
  * @param  USART_InitStruct: pointer to a USART_InitTypeDef structure that
  *   contains the configuration information for the specified USART peripheral.
  * @retval None
  */
static void STM_EVAL_COMInit(COM_TypeDef COM, USART_InitTypeDef* USART_InitStruct)
{
  GPIO_InitTypeDef GPIO_InitStructure;

  /* Enable GPIO clock */
  //RCC_APB2PeriphClockCmd(COM_TX_PORT_CLK[COM] | COM_RX_PORT_CLK[COM] | RCC_APB2Periph_AFIO, ENABLE);
  RCC_APB2PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

//  if (COM == COM1)
//  {
//    /* Enable the USART2 Pins Software Remapping */
//    GPIO_PinRemapConfig(GPIO_Remap_USART2, ENABLE);
//    RCC_APB1PeriphClockCmd(COM_USART_CLK[COM], ENABLE);
//  }

  /* Configure USART Tx as alternate function push-pull */
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_Pin = COM_TX_PIN[COM];
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(COM_TX_PORT[COM], &GPIO_InitStructure);

  /* Configure USART Rx as input floating */
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
  GPIO_InitStructure.GPIO_Pin = COM_RX_PIN[COM];
  GPIO_Init(COM_RX_PORT[COM], &GPIO_InitStructure);

  /* USART configuration */
  USART_Init(COM_USART[COM], USART_InitStruct);

	/* Enable USARTy Receive and Transmit interrupts */
	USART_ITConfig(COM_USART[COM], USART_IT_RXNE, ENABLE);
	//USART_ITConfig(COM_USART[COM1], USART_IT_TXE, ENABLE);

  /* Enable USART */
  USART_Cmd(COM_USART[COM], ENABLE);
}


void __uart_putchar(int ch) {
#if 0
	/* Place your implementation of fputc here */
	/* e.g. write a character to the USART */
	USART_SendData(EVAL_COM1, (uint8_t) ch);

	/* Loop until the end of transmission */
	while (USART_GetFlagStatus(EVAL_COM1, USART_FLAG_TC) == RESET)
	{}

#else
	while(USART_GetFlagStatus(EVAL_COM1, USART_FLAG_TC) == RESET) { ; }
	USART_SendData(EVAL_COM1, (uint16_t)ch);
#endif
}

uint8_t GetKey(void)
{
	uint8_t RetVal;

	RetVal = 0;
	if (EVAL_COM1->SR & USART_FLAG_RXNE) {
		RetVal = 0xFF & USART_ReceiveData(EVAL_COM1);
	}

	return(RetVal);
}

/**
  * @brief  This function handles USARTz global interrupt request.
  * @param  None
  * @retval None
  */

void USART2_IRQHandler(void)
{

  if(USART_GetITStatus(COM_USART[COM1], USART_IT_RXNE) != RESET)
  {
    /* Read one byte from the receive data register */
    RxBuffer2[RxCounter2++] = USART_ReceiveData(COM_USART[COM1]);
    avai_of_rxbuf++;
    if(RxCounter2 > NbrOfDataToRead2)
    {
      /* Disable the USARTz Receive interrupt */
      //USART_ITConfig(COM_USART[COM1], USART_IT_RXNE, DISABLE);
    	RxCounter2 = 0;
    }
  }
#if 0
  if(USART_GetITStatus(COM_USART[COM1], USART_IT_TXE) != RESET)
  {
    /* Write one byte to the transmit data register */
    USART_SendData(COM_USART[COM1], TxBuffer2[TxCounter2++]);

    if(TxCounter2 == NbrOfDataToTransfer2)
    {
      /* Disable the USARTz Transmit interrupt */
      USART_ITConfig(COM_USART[COM1], USART_IT_TXE, DISABLE);
    }
  }
#endif
}
