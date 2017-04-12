/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// libcmsis_conf.h - Configuration file for library CMSIS_RESKIN
//
// Author: 185275258 (QQ Group)
//
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/

#ifndef LIBCMSIS_CONF_H_
#define LIBCMSIS_CONF_H_

#ifndef USE_STDPERIPH_DRIVER
  #define USE_STDPERIPH_DRIVER
#endif

#ifndef USE_HAL_DRIVER
  #define USE_HAL_DRIVER
#endif

#ifndef STM32F429xx
#define STM32F429xx
#endif

#ifndef STM32F429_439xx
  #define STM32F429_439xx
#endif

#ifndef ARM_MATH_CM4
  #define ARM_MATH_CM4
#endif

#ifndef ARM_MATH_MATRIX_CHECK
  #define ARM_MATH_MATRIX_CHECK
#endif

#ifndef USE_USB_FS
  #define USE_USB_FS
#endif

#ifndef USE_USB_HS
  #define USE_USB_HS
  #ifndef USE_USB_HS_IN_FS
    #define USE_USB_HS_IN_FS
  #endif
#endif

#ifndef HSE_VALUE
  #define HSE_VALUE (8000000)
#endif


#include <stm32f4xx.h>

#endif /* LIBCMSIS_CONF_H_ */
