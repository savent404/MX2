#pragma once

#include "LED.h"
#include "LED_def.h"
#include "MX_def.h"
#include "cmsis_os.h"
#include <stdbool.h>
#include <string.h>

MX_C_API bool LED_NP_Init(void* arg);
MX_C_API void LED_NP_Handle(PARA_DYNAMIC_t* ptr);
MX_C_API bool LED_NP_Update(PARA_DYNAMIC_t* ptr, bool needBlock);
MX_C_API void LED_NP_updateBG(triggerSets_BG_t);
MX_C_API void LED_NP_updateTG(triggerSets_TG_t);
MX_C_API void LED_NP_updateFT(triggerSets_FT_t);
MX_C_API void LED_NP_applySets(void);
MX_C_API void LED_NP_stashSets(void);

MX_PORT_API bool LED_NP_HW_Init(int num, triggerSets_HW_t);
MX_PORT_API bool LED_NP_HW_Update(const void* arg, int num);
