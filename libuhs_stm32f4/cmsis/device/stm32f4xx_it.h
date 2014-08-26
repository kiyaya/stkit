/**
  ******************************************************************************
  * @file    Project/STM32F4xx_StdPeriph_Templates/stm32f4xx_it.h 
  * @author  MCD Application Team
  * @version V1.2.0
  * @date    19-September-2013
  * @brief   This file contains the headers of the interrupt handlers.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2013 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __STM32F4xx_IT_H
#define __STM32F4xx_IT_H

#ifdef __cplusplus
 extern "C" {
#endif 

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"

 /* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/

 /* The lower the value, the greater the priority of the corresponding
 * interrupt.
 * __NVIC_PRIO_BITS = 4, so the lowest priority is 15.
 * */
enum
{
	ISR_PRIORITY_USART 		= 15,
	ISR_PRIORITY_I2C1_ER 	= 2,
	ISR_PRIORITY_I2C1_EV 	= 2,
	ISR_PRIORITY_I2C2_ER 	= 2,
	ISR_PRIORITY_I2C2_EV 	= 2,
	ISR_PRIORITY_USB_FS 	= 1,
	ISR_PRIORITY_USB_HS 	= 1,
	ISR_PRIORITY_DMA2D		= 1
};

/*  we use group-4, so no sub priority needed. See prvSetupHardware(). */
enum
{
	ISR_SUB_PRIORITY_USART 		= 0,
	ISR_SUB_PRIORITY_I2C1_ER 	= 0,
	ISR_SUB_PRIORITY_I2C1_EV 	= 0,
	ISR_SUB_PRIORITY_I2C2_ER	= 0,
	ISR_SUB_PRIORITY_I2C2_EV 	= 0,
	ISR_SUB_PRIORITY_USB_FS 	= 0,
	ISR_SUB_PRIORITY_USB_HS	 	= 0,
	ISR_SUB_PRIORITY_DMA2D		= 0

};

/* Exported macro ------------------------------------------------------------*/
#define DisableInterrupt() do{__disable_irq();  }while(0)
#define EnableInterrupt() do{__enable_irq(); }while(0)

/* Exported functions ------------------------------------------------------- */
#if 1	// currently we use Default_Handler_c to manage these exceptions
void Default_Handler_c(unsigned int * hardfault_args);
#else
void NMI_Handler(void);
void HardFault_Handler(void);
void MemManage_Handler(void);
void BusFault_Handler(void);
void UsageFault_Handler(void);
void SVC_Handler(void);
void DebugMon_Handler(void);
void PendSV_Handler(void);
void SysTick_Handler(void);
#endif
void I2C1_EV_IRQHandler(void);
void I2C1_ER_IRQHandler(void);
void I2C2_EV_IRQHandler(void);
void I2C2_ER_IRQHandler(void);

#endif /* __STM32F4xx_IT_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
