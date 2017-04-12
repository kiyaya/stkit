/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// cpu_utils.cpp - CPU utilities for runtime statistics.
//
// Author: 185275258 (QQ Group)
//
// Notes:
//   Partly from STMicroelectronics, to use this module, the following steps
// should be followed :
//
//	 1- in the _OS_Config.h file (ex. FreeRTOSConfig.h) enable the following macros :
//	     - #define configUSE_IDLE_HOOK        1
//		 - #define configUSE_TICK_HOOK        1
//
//	 2- in the _OS_Config.h define the following macros :
//		 - #define traceTASK_SWITCHED_IN()  extern void StartIdleMonitor(void); \
//											 StartIdleMonitor()
//		 - #define traceTASK_SWITCHED_OUT() extern void EndIdleMonitor(void); \
//											 EndIdleMonitor()
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/

/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include <stm32f4xx.h>
#include <cmsis_os.h>

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define CALCULATION_PERIOD    1000

/* Private macro -------------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

xTaskHandle xIdleHandle = NULL;
__IO uint32_t osCPU_Usage = 0;
uint32_t osCPU_IdleStartTime = 0;
uint32_t osCPU_IdleSpentTime = 0;
uint32_t osCPU_TotalIdleTime = 0;
uint32_t FPS_accumulated_frames = 0;

/* Private functions ---------------------------------------------------------*/
/**
  * @brief  Application Idle Hook
  * @param  None 
  * @retval None
  */
void vApplicationIdleHook_cpuload(void)
{
    if( xIdleHandle == NULL )
    {
        /* Store the handle to the idle task. */
        xIdleHandle = osThreadGetId();  //xTaskGetCurrentTaskHandle();
    }
}

/**
  * @brief  Application Idle Hook
  * @param  None 
  * @retval None
  */
void vApplicationTickHook_cpuload(void)
{
	static int tick = 0;
  
	if(tick ++ > CALCULATION_PERIOD)
	{
		tick = 0;

		if(osCPU_TotalIdleTime > 1000)
		{
			osCPU_TotalIdleTime = 1000;
		}
		osCPU_Usage = (100 - (osCPU_TotalIdleTime * 100) / CALCULATION_PERIOD);
		osCPU_TotalIdleTime = 0;
	}
}

#define CFG_ENABLE_PRINTTASKRUNTIME  0
#if CFG_ENABLE_PRINTTASKRUNTIME == 1
typedef struct PrintTaskRunTimeObj_tag {
    uint32_t lastSwichInTime;
    xTaskHandle lastSwichInTask;
} PrintTaskRunTimeObj_t;

static PrintTaskRunTimeObj_t sPrintTaskRunTimeObj = {0};
static void PintTaskRunTime()
{
    
    xTaskHandle switchOutTask = xTaskGetCurrentTaskHandle();
    SWO_PrintString("\nts,");
    if (sPrintTaskRunTimeObj.lastSwichInTask == switchOutTask)
    {
        const signed char *taskName = pcTaskGetTaskName(switchOutTask);
        //uint32_t curTime = xTaskGetTickCount();
        uint32_t curTime = vGetTimerCounterForRunTimeStats();
        
        char buf[16];
        snprintf(buf, sizeof(buf), "%s,%u\n", taskName, curTime - sPrintTaskRunTimeObj.lastSwichInTime);
        SWO_PrintString(buf);
        
    }
    else
    {
        SWO_PrintString("e!\n");
    }
}
#endif
/**
  * @brief  Start Idle monitor
  * @param  None 
  * @retval None
  */
void StartIdleMonitor (void)
{
	if( osThreadGetId() == xIdleHandle )
	{
		osCPU_IdleStartTime = osKernelSysTick();    //xTaskGetTickCount();
	}

#if CFG_ENABLE_PRINTTASKRUNTIME == 1	
	//sPrintTaskRunTimeObj.lastSwichInTime = xTaskGetTickCount();
	sPrintTaskRunTimeObj.lastSwichInTime = vGetTimerCounterForRunTimeStats();
	
	sPrintTaskRunTimeObj.lastSwichInTask = xTaskGetCurrentTaskHandle();
#endif	
}

/**
  * @brief  Stop Idle monitor
  * @param  None 
  * @retval None
  */
void EndIdleMonitor (void)
{
	if( osThreadGetId() == xIdleHandle )
	{
		/* Store the handle to the idle task. */
		osCPU_IdleSpentTime = osKernelSysTick() - osCPU_IdleStartTime;
		osCPU_TotalIdleTime += osCPU_IdleSpentTime;
	}

#if CFG_ENABLE_PRINTTASKRUNTIME == 1	
	PintTaskRunTime();
#endif	
}

/**
  * @brief  Stop Idle monitor
  * @param  None 
  * @retval None
  */
unsigned int osGetCPUUsage(void)
{
	return osCPU_Usage;
}

static TIM_HandleTypeDef sTimerHandler;

void vConfigureTimerForRunTimeStats( void )
{
    //TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;

    /* Set TIMx instance */
    sTimerHandler.Instance = TIM2;

    /* Initialize TIM3 peripheral as follows:
         + Period = 1000000 / 1000 - 1 (1MHz/1kHz, 1MHz is clock, 1kHz is the output)
         + Pre-scaler = ((SystemCoreClock/2)/1000000) - 1
         + ClockDivision = 0
         + Counter direction = Up
    */
    sTimerHandler.Init.Period = (1000000 / 1000) - 1;
    sTimerHandler.Init.Prescaler = (SystemCoreClock / 2) / 1000000 - 1;
    sTimerHandler.Init.ClockDivision = 0;
    sTimerHandler.Init.CounterMode = TIM_COUNTERMODE_UP;
    if(HAL_TIM_Base_Init(&sTimerHandler) != HAL_OK)
    {
        return;
    }

}

unsigned int vGetTimerCounterForRunTimeStats( void )
{
	return __HAL_TIM_GET_COUNTER(&sTimerHandler); //TIM_GetCounter(TIM2);
}

#if 0   // if shell command showes wrong RTOS task statistic data, you may need
// to add this function to FreeRTOS callback.
void vResetTimerCounterForRunTimeStats( void )
{
	TIM_PrescalerConfig(TIM2, TIM_GetPrescaler(TIM2), TIM_PSCReloadMode_Immediate);
}
#endif
void FPS_Calculate(void)
{
	FPS_accumulated_frames++;
}

unsigned int FPS_GetAndClearAccumulatedFrames(void)
{
	unsigned int fps;
	fps = FPS_accumulated_frames;
	FPS_accumulated_frames = 0;
	return fps;
}

// End of file
