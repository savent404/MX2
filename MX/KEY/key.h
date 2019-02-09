#pragma once

#include "MX_def.h"
#include <stdbool.h>

typedef enum _mx_key_status {
    KEY_PRESS,
    KEY_RELEASE
} keyStatus_t;

typedef enum _mx_key_id {
    KEY_1,
    KEY_2
} keyId_t;

MX_PORT_API bool MX_KEY_Init(void);
MX_PORT_API bool MX_KEY_DeInit(void);
MX_PORT_API keyStatus_t MX_KEY_GetStatus(keyId_t id);
