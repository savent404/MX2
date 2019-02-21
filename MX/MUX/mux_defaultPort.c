#include "mux.h"
#include "param.h"
#include <stdint.h>

__MX_WEAK void MX_MUX_HW_Init(MUX_Track_Id_t tid)
{
    switch (tid) {
    case 0:
        // init tim, dac
        break;
    case 1:
        // init etc. I2S
        break;
    default:
        break;
    }
}

__MX_WEAK void MX_MUX_HW_DeInit(MUX_Track_Id_t tid)
{
    switch (tid) {
    case 0:
        break;
    case 1:
        break;
    default:
        break;
    }
}

__MX_WEAK void MX_MUX_HW_Start(MUX_Track_Id_t tid, void* source, int size)
{
    /*
    switch (tid):
    .......
    */
}

__MX_WEAK void MX_MUX_HW_Stop(MUX_Track_Id_t tid)
{
    /*
    switch (tid):
    .......
    */
}