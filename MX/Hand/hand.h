#pragma once

#include "MX_def.h"
#include <stdbool.h>
#include <stdint.h>

typedef union {
    uint32_t hex;
    struct {
        unsigned lvSpin : 4;
        bool isStab : 1;
        bool isSwing : 1;
        bool isClash : 1;
    } unio;
} HAND_TriggerId_t;

MX_PORT_API bool MX_HAND_Init(void);
MX_PORT_API bool MX_HAND_DeInit(void);
MX_PORT_API HAND_TriggerId_t MX_HAND_GetTrigger(void);
