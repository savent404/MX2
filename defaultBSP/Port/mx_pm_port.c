#include "pm.h"

#include "FreeRTOS.h"
#include "cmsis_os.h"
#include "stm32f1xx.h"
#include "main.h"
#include "adc.h"
#include <stdint.h>
#include "param.h"

static uint16_t getPowerVal(void)
{
    static uint16_t val = 0;
    uint16_t buf = HAL_ADC_GetValue(&hadc1);
    int16_t div = buf - val;
    if ((div > 0 ? div : -div) >= 50)
    {
        val += div / 3;
    }
    else
    {
        val = buf;
    }
    return val;
}

static bool isPluged(void)
{
    return HAL_GPIO_ReadPin(Charge_Check_GPIO_Port,
        Charge_Check_Pin) == GPIO_PIN_SET;
}

bool MX_PM_Init(void)
{
    // init hw
    MX_ADC1_Init();
    // then read ADC till it's stable
    int cnt = 10;
    while (cnt--)
    {
        getPowerVal();
        osDelay(1);
    }
}

void MX_PM_Boot(void)
{
    HAL_GPIO_WritePin(Power_EN_GPIO_Port,
                      Power_EN_Pin,
                      GPIO_PIN_SET);
}

void MX_PM_Shutdown(void)
{
    HAL_GPIO_WritePin(Power_EN_GPIO_Port,
                      Power_EN_Pin,
                      GPIO_PIN_RESET);
}

bool MX_PM_needWarning(void)
{
    return getPowerVal() <= STATIC_USR.vol_warning;
}

bool MX_PM_needPowerOff(void)
{
    return getPowerVal() <= STATIC_USR.vol_poweroff;
}

bool MX_PM_isCharging(void)
{
    return isPluged();
}

bool MX_PM_isCharged(void)
{
    if (isPluged() == false)
        return false;
    return getPowerVal() > STATIC_USR.vol_chargecomplete;
}

bool MX_PM_CurrentCapacity(uint16_t* v)
{
    *v = getPowerVal();
    return true;
}
