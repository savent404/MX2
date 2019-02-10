#include "hand.h"

__MX_WEAK bool MX_HAND_Init(void)
{
    return true;
}

__MX_WEAK bool MX_HAND_DeInit(void)
{
    return true;
}

__MX_WEAK HAND_TriggerId_t MX_HAND_GetTrigger(void)
{
    return HAND_NULL;
}
