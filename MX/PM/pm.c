#include "pm.h"
#include "adc.h"

__MX_WEAK bool MX_PM_Init(void)
{
    MX_ADC1_Init();
    return true;
}

__MX_WEAK bool MX_PM_DeInit(void)
{
    return true;
}

__MX_WEAK void MX_PM_Boot(void)
{
    // enable continues power
}

__MX_WEAK void MX_PM_Shutdown(void)
{
    // disable continues power
}

__MX_WEAK bool MX_PM_needWarning(void)
{
    return false;
}

__MX_WEAK bool MX_PM_needPowerOff(void)
{
    return false;
}

__MX_WEAK bool MX_PM_isCharging(void)
{
    return false;
}

__MX_WEAK bool MX_PM_isCharged(void)
{
    return false;
}

__MX_WEAK bool MX_PM_CurrentCapacity(uint16_t *permillage)
{
    *permillage=0x0000;
    return false;
}
