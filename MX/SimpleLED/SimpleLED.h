#pragma once

#include "MX_def.h"
#include "debug.h"
#include "param.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

typedef enum {
    SIMPLELED_STATUS_SLEEP,
    SIMPLELED_STATUS_STANDBY,
    SIMPLELED_STATUS_ON,
    SIMPLELED_STATUS_LOCKUP,
    SIMPLELED_STATUS_CLASH
} SimpleLED_Status_t;

MX_C_API void SimpleLED_Init(void);
MX_C_API void SimpleLED_DeInit(void);
MX_C_API void SimpleLED_ChangeStatus(SimpleLED_Status_t status);

MX_INTERNAL_API void SimpleLED_Callback(void const* arg);
MX_INTERNAL_API SimpleLED_Acction_t* SimpleLED_GetAction(SimpleLED_Status_t);
MX_INTERNAL_API void                 SimpleLED_Opra(uint32_t led);

MX_PORT_API void SimpleLED_HW_Init(void);
MX_PORT_API void SimpleLED_HW_DeInit(void);
MX_PORT_API void SimpleLED_HW_Opra(uint32_t led);
