#pragma once

#include "MX_def.h"
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>

MX_PORT_API void MX_Console_Print(uint8_t* string, uint16_t size);

MX_C_API int MX_Console_Printf(const char*, ...);