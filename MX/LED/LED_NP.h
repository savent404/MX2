#pragma once

#include "LED_def.h"
#include <string.h>
#include "FreeRTOS.h"
#include "cmsis_os.h"
#include "LED_NP_HAL.h"
#include "LED.h"

#ifdef __cplusplus
extern "C" {
#endif

bool LED_NP_Init(void * arg);
void LED_NP_Handle(PARA_DYNAMIC_t* ptr);
bool LED_NP_Update(PARA_DYNAMIC_t* ptr);

#ifdef __cplusplus
}
#endif
