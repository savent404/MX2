#pragma once

#include "MX_def.h"
#include "LED_def.h"
#include <string.h>
#include <stdbool.h>
#include "cmsis_os.h"
#include "LED.h"

MX_C_API bool LED_NP_Init(void * arg);
MX_C_API void LED_NP_Handle(PARA_DYNAMIC_t* ptr);
MX_C_API bool LED_NP_Update(PARA_DYNAMIC_t* ptr);
MX_C_API void LED_NP_updateBG(triggerSets_BG_t);
MX_C_API void LED_NP_updateTG(triggerSets_TG_t);
MX_C_API void LED_NP_updateFT(triggerSets_FT_t);

MX_PORT_API bool LED_NP_HW_Init(int num);
MX_PORT_API bool LED_NP_HW_Update(const void* arg, int num);
