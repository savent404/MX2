#ifndef _LED_H_
#define _LED_H_

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "FreeRTOS.h"
#include "cmsis_os.h"

#include "LED_def.h"

#ifdef __cplusplus
extern "C" {
#endif

extern LED_IF_t ledIf;

osEvent LED_GetMessage(uint32_t timeout);
void LED_Start_Trigger(LED_Message_t message);
void LED_Bank_Update(PARA_DYNAMIC_t*);

#ifdef __cplusplus
}
#endif

#ifndef USE_NP
#define USE_NP 0
#endif

#endif