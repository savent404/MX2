#pragma once

#include "MX_def.h"
#include <stdint.h>

#ifndef DAC_FIX_OFFSET
#    define DAC_FIX_OFFSET (0x800)
#endif

MX_C_API void
mux_convert_addToInt(const void* source, int* dest, int size, float* f, int multiFactor);

MX_C_API void
mux_convert_mergeToBuffer(const int* source, void* dest, int size, int shiftBits);

MX_C_API void
mux_resetDmaBuffer(void* in, int size);
