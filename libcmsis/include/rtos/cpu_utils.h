/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// cpu_utils.h - CPU utilities for runtime statistics.
//
// Author: 185275258 (QQ Group)
//
// Notes:
//   Partly from STMicroelectronics.
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/


/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef _CPU_UTILS_H__
#define _CPU_UTILS_H__

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/

/* Exported functions ------------------------------------------------------- */
unsigned int osGetCPUUsage (void);

void os_heap_dump(char *buf);

void FPS_Calculate(void);

unsigned int FPS_GetAndClearAccumulatedFrames(void);

void vConfigureTimerForRunTimeStats( void );

unsigned int vGetTimerCounterForRunTimeStats( void );

//void vResetTimerCounterForRunTimeStats( void );

#ifdef __cplusplus
}
#endif

#endif /* _CPU_UTILS_H__ */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
