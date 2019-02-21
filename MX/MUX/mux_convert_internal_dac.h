#pragma once

#include "MX_def.h"
#include <stdint.h>

MX_C_API static inline void
mux_convert_addToInt(const void* source, int* dest, int size, float* f)
{
    static const float factor = 32.0;
    float _f = *f;
    int buf;
    int16_t* _s = (int16_t*)source;
    int* _d = dest;
    for (int i = 0; i < size; i++) {
        buf = *_d + (*_s++ / MX_MUX_MAXIUM_TRACKID);

        if (buf > INT16_MAX) {
            _f = INT16_MAX / 2.0f / buf;
            buf = INT16_MAX;
        }
        buf *= _f;
        if (buf < INT16_MIN) {
            _f = INT16_MAX / 2.0f / buf;
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
    static const int zeroOffset = 0x1000 / 2;
    uint16_t buf;

    for (int i = 0; i < size; i++)
    {
        buf = (uint16_t)(((*_s++ * vol) >> offset) + zeroOffset);
        *_d++ = buf;
    }
}

MX_C_API static inline void
mux_resetDmaBuffer(void* in, int size)
{
    uint16_t* ptr = (uint16_t*)in;
    for (int i = 0; i < size; i++)
    {
        *ptr = 0x1000/2;
    }
}