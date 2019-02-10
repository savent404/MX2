#ifndef _LED_H_
#define _LED_H_

#include "MX_def.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "FreeRTOS.h"
#include "cmsis_os.h"

#include "LED_def.h"

extern LED_IF_t ledIf;

MX_C_API osEvent LED_GetMessage(uint32_t timeout);
MX_C_API void LED_Start_Trigger(LED_Message_t message);
MX_C_API void LED_Bank_Update(PARA_DYNAMIC_t*);
MX_C_API void MX_LED_Init(void);

#ifndef USE_NP
#define USE_NP 0
#endif

#endif