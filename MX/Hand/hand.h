#pragma once

#include "MX_def.h"
#include <stdbool.h>

typedef enum {
    HAND_NULL = 0,
    HAND_WAVE = 1,
    HAND_CLIK = 2,
} HAND_TriggerId_t;

MX_PORT_API bool MX_HAND_Init(void);
MX_PORT_API bool MX_HAND_DeInit(void);
MX_PORT_API HAND_TriggerId_t MX_HAND_GetTrigger(void);
