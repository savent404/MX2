#include "SimpleLED.h"
#include "cmsis_os.h"

static osThreadId self;
static SimpleLED_Acction_t* pacction;
static int32_t timer = 0;
static int32_t cnt = 0;
static bool loopFlag = 0;

static int interval = 10;

void SimpleLED_Init(void)
{
    SimpleLED_HW_Init();
    pacction = SimpleLED_GetAction(SIMPLELED_STATUS_SLEEP);
    osThreadDef(self, SimpleLED_Callback, osPriorityLow, 0, 512);
    self = osThreadCreate(osThread(self), NULL);
}

void SimpleLED_DeInit(void)
{
    osThreadSuspend(self);
    SimpleLED_HW_DeInit();
    osThreadTerminate(self);
}

void SimpleLED_ChangeStatus(SimpleLED_Status_t status)
{
    pacction = SimpleLED_GetAction(status);
}

void SimpleLED_Callback(void const * arg)
{
    for(;;)
    {
        osDelay(interval);

        if ((timer -= interval) > 0)
            continue;

        if (pacction == NULL)
        {
            SimpleLED_Opra(0);
            timer = 100;
            continue;
        }

        /** Still working [0~pacction->Num] */
        if (loopFlag == false &&
            cnt < pacction->Num)
        {
            SimpleLED_Opra(*(pacction->Action + cnt++));
        }
        else if (loopFlag == false)
        {
            pacction = SimpleLED_GetAction(SIMPLELED_STATUS_ON);
        }
        else  /* loopFlag == true */
        {
            SimpleLED_Opra(*(pacction->Action + (cnt++ % pacction->Num)));
        }
        timer = pacction->Delay;
    }
}

SimpleLED_Acction_t* SimpleLED_GetAction(SimpleLED_Status_t status)
{
    SimpleLED_Acction_t *ans;
    switch( status)
    {
        case SIMPLELED_STATUS_SLEEP:
            ans = NULL;
            break;
        case SIMPLELED_STATUS_STANDBY:
            ans = (USR.accent + USR.bank_now)->Standby;
            loopFlag = true;
            break;
        case SIMPLELED_STATUS_ON:
            ans = (USR.accent + USR.bank_now)->Clash;
            loopFlag = false;
            break;
        case SIMPLELED_STATUS_LOCKUP:
            ans = (USR.accent + USR.bank_now)->Lockup;
            loopFlag = true;
            break;
        default:
            ans = NULL;
            loopFlag = false;
    }
    cnt = 0;
    timer = 0;
    return ans;
}

void SimpleLED_Opra(uint32_t led)
{
    uint32_t masked = led & USR.config->SimpleLED_MASK;

    SimpleLED_HW_Opra(masked);
}

__MX_WEAK void SimpleLED_HW_Init(void)
{

}
__MX_WEAK void SimpleLED_HW_DeInit(void)
{
    
}
__MX_WEAK void SimpleLED_HW_Opra(uint32_t mask)
{

}
