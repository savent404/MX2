#pragma once

#include "MX_def.h"
#include "PARAM_def.h"
#include "USR_CONFIG.h"
#include "triggerPath.h"
#include <stdbool.h>

MX_C_API bool           MX_PARAM_Init(void);
MX_PORT_API const char* MX_PARAM_GetPrefix(void);
