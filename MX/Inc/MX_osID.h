#ifndef _MX_OSID_H_
#define _MX_OSID_H_

#include "cmsis_os.h"

extern osThreadId defaultTaskHandle;
extern osThreadId DACTaskHandle;
extern osThreadId LEDTaskHandle;
extern osThreadId WavTaskHandle;
extern osThreadId SimpleLEDHandle;
extern osMessageQId DAC_BufferHandle;
extern osMessageQId DAC_CMDHandle;
extern osMessageQId LED_CMDHandle;
extern osSemaphoreId DAC_Complete_FlagHandle;

#endif

