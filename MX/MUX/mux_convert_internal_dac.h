#pragma once

#include "MX_def.h"
#include <stdint.h>

#ifndef DAC_FIX_OFFSET
#define DAC_FIX_OFFSET (0x800)
#endif

MX_C_API static inline void
mux_convert_addToInt(const void* source, int* dest, int size, float* f)
{
    static const float factor = 64;
    static const float div = 2;
    float _f = *f;
    int buf;
    int16_t* _s = (int16_t*)source;
    int* _d = dest;
    for (int i = 0; i < size; i++) {
        buf = *_d + (*_s++);

        if (buf > INT16_MAX) {
            _f = INT16_MAX / div / buf;
            buf = INT16_MAX;
        }
        buf *= _f;
        if (buf < INT16_MIN) {
            _f = INT16_MIN / div / buf;
            buf = INT16_MIN;
        }
        if (_f < 1.0f) {
            _f += (1.0f - _f) / factor;
        }
        *_d++ = buf;
    }
    *f = _f;
}

MX_C_API static inline void
mux_convert_mergeToBuffer(const int* source, void* dest, int size, int vol)
{
    uint16_t* _d = (uint16_t*)dest;
    const int* _s = source;

    // 16bit->12bit
    // 2^2 = 4 level vol
    static const int offset = (4 + 2);
    static const int zeroOffset = DAC_FIX_OFFSET;
    uint16_t buf;

    for (int i = 0; i < size; i++) {
        buf = (uint16_t)(((*_s++ * vol) >> offset) + zeroOffset);
        *_d++ = buf;
    }
}

MX_C_API static inline void
mux_resetDmaBuffer(void* in, int size)
{
    uint16_t* ptr = (uint16_t*)in;
    for (int i = 0; i < size; i++) {
        *ptr++ = DAC_FIX_OFFSET;
    }
}
