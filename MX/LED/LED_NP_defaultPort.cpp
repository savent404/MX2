#include "LED_NP.h"
#include "color.hpp"

__MX_WEAK bool LED_NP_HW_Init(int num)
{
    // init hw
    return true;
}

__MX_WEAK bool LED_NP_HW_Update(const void* arg, int num)
{
    const RGB* ptrColor = static_cast<const RGB*> (arg);

    /** wait last dma transfer end */
    // etc. osSemaphoreWait(....)

    for (int i = 0; i < num; i++)
    {
        uint8_t rgb[3] = {
            ptrColor->wR(),
            ptrColor->wG(),
            ptrColor->wB(),
        };
        // fill into buffer

        ptrColor++;
    }
    /** Start dma transfer */
    return true;
}