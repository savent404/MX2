#include "mux_convert_internal.h"

MX_C_API void
mux_convert_addToInt(const void* source, int* dest, int size, const float absolutMulti, float* f, int multiFactor)
{
    static const float factor = 64;
    static const float div    = 2;
    static const int   top    = INT16_MAX * (1 << MX_MUX_WAV_VOL_LEVEL);
    static const int   button = INT16_MIN * (1 << MX_MUX_WAV_VOL_LEVEL);
    float              _f     = *f * absolutMulti;
    int                buf;
    int16_t*           _s = (int16_t*)source;
    int*               _d = dest;
    for (int i = 0; i < size; i++, _s++) {
        buf = *_d + (*_s) * multiFactor;

        if (buf > top) {
            _f = top / div / buf;
        } else if (buf < button) {
            _f = button / div / buf;
        }
        if (_f < 1.0f) {
            _f += 0.1f / factor;
        }
        *_d++ = *_d + (*_s) * multiFactor * _f;
    }
    *f = _f / absolutMulti;
}

MX_C_API void
mux_convert_mergeToBuffer(const int* source, void* dest, int size, int shitfBits)
{
    uint16_t*  _d = (uint16_t*)dest;
    const int* _s = source;

    // 16bit->12bit
    // 2^2 = 4 level vol
    int      offset     = (4 + shitfBits);
    int      zeroOffset = DAC_FIX_OFFSET;
    uint16_t buf;

    for (int i = 0; i < size; i++) {
        buf   = (uint16_t)(((*_s++) >> offset) + zeroOffset);
        *_d++ = buf;
    }
}

MX_C_API void
mux_resetDmaBuffer(void* in, int size)
{
    uint16_t* ptr = (uint16_t*)in;
    for (int i = 0; i < size; i++) {
        *ptr++ = DAC_FIX_OFFSET;
    }
}
