#include "hand.h"

__MX_WEAK void MX_HAND_HW_Init(void)
{
}

__MX_WEAK void MX_HAND_HW_DeInit(void)
{
}

__MX_WEAK bool MX_HAND_HW_getData(float acc[3], float gyro[3])
{
    for (int i = 0; i < 3; i++) {
        acc[i] = i;
        gyro[i] = i + 1;
    }
    return true;
}

__MX_WEAK bool MX_HAND_HW_isSwing(void)
{
    return false;
}

__MX_WEAK bool MX_HAND_HW_isClash(void)
{
    return false;
}