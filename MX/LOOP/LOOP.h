#pragma once

#include "MX.h"
#include "MX_def.h"
#include <stdbool.h>
#include <stdint.h>

MX_C_API bool MX_LOOP_Init(void);
MX_C_API bool MX_LOOP_DeInit(void);

MX_INTERNAL_API void MX_LOOP_Handle(void const *);