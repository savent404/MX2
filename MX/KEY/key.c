#include "key.h"

__MX_WEAK bool MX_KEY_Init(void)
{
    return true;
}

__MX_WEAK bool MX_KEY_DeInit(void)
{
    return true;
}

__MX_WEAK keyStatus_t MX_KEY_GetStatus(keyId_t id)
{
    switch(id)
    {
        case KEY_PWR:
            return KEY_PRESS;
        case KEY_SUB:
            return KEY_RELEASE;
        default:
            return KEY_PRESS;
    }
}
