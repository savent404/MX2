#include "LOOP.h"
#include "FreeRTOS.h"
#include "cmsis_os.h"


#include "triggerSets.h"
static osThreadId loopThreadId;

// [0]: auto 'in'
// [1]: auto 'powerOff'
static int autoTimeout[2] = {0, 0};
// [0]: trigger B timer
// [1]: trigger C timer
// [2]: trigger D timer
static int frozenTimer[3] = {0, 0, 0};

#define KEY_PWR_PRESS     (0x01)
#define KEY_SUB_PRESS     (0x02)
#define KEY_PWR_RELEASE   (0x04)
#define KEY_SUB_RELEASE   (0x08)

static uint8_t scanKey(void);
static bool askTrigger(uint8_t);
static void handleReady(void);
static void handleRunning(void);
static void handleCharged(void);
static void handleCharging(void);

static bool canBoot(void);

#define update_param(pos, tgName)\
    do {                                                                                      \
        const char* wavName = MX_TriggerPath_GetName(USR.trigger##tgName, pos);               \
        triggerSets_BG_t bg =                                                                 \
            triggerSets_readBG(_MX_TriggerPath_getOtherPath(USR.triggerBG##tgName, wavName)); \
        MX_LED_updateBG(bg);                                                                  \
        triggerSets_freeBG(bg);                                                               \
        triggerSets_TG_t tg =                                                                 \
            triggerSets_readTG(_MX_TriggerPath_getOtherPath(USR.triggerTG##tgName, wavName)); \
        MX_LED_updateTG(tg);                                                                  \
        triggerSets_freeTG(tg);                                                               \
        triggerSets_FT_t ft =                                                                 \
            triggerSets_readFT(_MX_TriggerPath_getOtherPath(USR.triggerFT##tgName, wavName)); \
        MX_LED_updateFT(ft);                                                                  \
        triggerSets_freeFT(ft);                                                               \
    } while (0)

bool MX_LOOP_Init(void)
{
    osThreadDef(loop, MX_LOOP_Handle, osPriorityNormal, 0, 1024);
    loopThreadId = osThreadCreate(osThread(loop), NULL);
    return true;
}

bool MX_LOOP_DeInit(void)
{
    osThreadSuspend(loopThreadId);
    osThreadTerminate(loopThreadId);
    return true;
}

void MX_LOOP_Handle(void const * arg)
{
    // enable PM first
    MX_PM_Init();
    MX_PM_Boot();
    /** Init all the other function modules
     */
    // if init parameter error, just beep and shutdown.
    if (MX_PARAM_Init() == false ||
        MX_HAND_Init() == false ||
        MX_KEY_Init() == false)
    {
        MX_Audio_PlayBeep();
        MX_PM_Shutdown();
        while (1)
          DEBUG_BKPT();
    }
    MX_MUX_Init();
    MX_LED_Init();
    // SimpleLED_Init();

    if (!canBoot())
    {
        MX_PM_Shutdown();
        while(1)
            ;
    }

    MX_PM_Boot();

    MX_WTDG_FEED();

    /* Break point here */
    //DEBUG_BKPT();

    USR.sys_status = System_Restart;
    //vTaskPrioritySet(loopThreadId, osPriorityRealtime);
    usr_switch_bank(0, 1);
    //vTaskPrioritySet(loopThreadId, osPriorityNormal);
    USR.bank_color = 0;

    MX_LED_bankUpdate(&USR);

#if 0
    if (MX_PM_needWarning())
    {
        uint8_t buf = USR.mute_flag;
        USR.mute_flag = 0;
        MX_Audio_Play_Start(Audio_LowPower);
        MX_Audio_Play_Start(Audio_LowPower);
        USR.mute_flag = buf;
    }
#endif

    MX_Audio_Play_Start(Audio_Boot);

    //wait power key to release
    while(KEY_PRESS==MX_KEY_GetStatus(KEY_PWR)) {
        MX_WTDG_FEED();
        osDelay(MX_LOOP_INTERVAL);
    }

    USR.sys_status = System_Ready;

    for (;;)
    {
        if (MX_PM_needPowerOff())
        {
            USR.mute_flag = 0;
            SimpleLED_ChangeStatus(SIMPLELED_STATUS_SLEEP);
            MX_Audio_Play_Start(Audio_PowerOff);
            MX_PM_Shutdown();
        }
        switch(USR.sys_status)
        {
            case System_Ready: {
                handleReady();
                break;
            }
            case System_Running: {
                handleRunning();
                break;
            }
            case System_Charged: {
                handleCharged();
                break;
            }
            case System_Charging: {
                handleCharging();
                break;
            }
        }

        osDelay(MX_LOOP_INTERVAL);
    }
}

uint8_t scanKey(void)
{
    // 第一次检测时可以确定Power按键按下|User按键未按下
    static uint8_t status = KEY_PWR_PRESS;
    uint8_t buf = 0, pre_buf, res = 0;

    if (MX_KEY_GetStatus(KEY_PWR) == KEY_PRESS)
    {
        buf |= KEY_PWR_PRESS;
    }
    if (MX_KEY_GetStatus(KEY_SUB) == KEY_PRESS)
    {
        buf |= KEY_SUB_PRESS;
    }

    pre_buf = buf;
    buf ^= status;
    status = pre_buf;

    if (buf & KEY_PWR_PRESS)
    {
        res |= pre_buf & KEY_PWR_PRESS ? KEY_PWR_PRESS : KEY_PWR_RELEASE;
    }
    if (buf & KEY_SUB_PRESS)
    {
        res |= pre_buf & KEY_SUB_PRESS ? KEY_SUB_PRESS : KEY_SUB_RELEASE;
    }

    MX_WTDG_FEED();

    if (res == 0)
        osDelay(MX_LOOP_INTERVAL);
#if 0 // this is to test key value changed event
    else
        DEBUG_BKPT();
#endif

    return res;
}

void handleReady(void)
{
    uint8_t keyRes = scanKey();
    uint16_t timeout = 0;

    if (keyRes & KEY_PWR_PRESS)
    {
        uint16_t maxTimeout = USR.config->Tpoff > USR.config->Tout ?
            USR.config->Tpoff :
            USR.config->Tout;
        // wait for key release
        while (!(scanKey() & KEY_PWR_RELEASE) && timeout < maxTimeout)
        {
            timeout += MX_LOOP_INTERVAL;
        }

        // power off
        if(timeout >= maxTimeout || USR.config->Tpoff <= USR.config->Tout)
        {
            DEBUG(5, "System going to close");
            USR.sys_status = System_Close;
            MX_Audio_Play_Start(Audio_PowerOff);
            osDelay(3000); //3s
            MX_PM_Shutdown();
        }
        // trigger 'out'
        else
        {
            DEBUG(5, "System going to running");
            USR.sys_status = System_Running;
            MX_LED_bankUpdate(&USR);
            MX_Audio_Play_Start(Audio_intoRunning);
            update_param(MX_Audio_getLastHumPos(), HUM);
            MX_LED_applySets();
            update_param(MX_Audio_getLastTriggerPos(), OUT);
            MX_LED_startTrigger(LED_Trigger_Start);
            SimpleLED_ChangeStatus(SIMPLELED_STATUS_ON);
            autoTimeout[0] = 0;
        }
    }
    else if (keyRes & KEY_SUB_PRESS)
    {
        uint16_t maxTimeout = USR.config->Ts_switch;
        while ((!(scanKey() & KEY_SUB_RELEASE)) && timeout < maxTimeout)
        {
            timeout += MX_LOOP_INTERVAL;
        }

        if (timeout >= maxTimeout)
        {
            DEBUG(5, "System Bank switch");
            //vTaskPrioritySet(loopThreadId, osPriorityRealtime);
            usr_switch_bank((USR.bank_now + 1) % USR.nBank, 0);
            //vTaskPrioritySet(loopThreadId, osPriorityNormal);
            MX_LED_bankUpdate(&USR);
            SimpleLED_ChangeStatus(SIMPLELED_STATUS_STANDBY);
            MX_Audio_Play_Start(Audio_BankSwitch);
        }
    }

    if (MX_PM_isCharging())
    {
        DEBUG(5, "System going to Charging");
        USR.sys_status = System_Charging;
    }

    if (keyRes != 0)
    {
        autoTimeout[1] = 0;
    }
    autoTimeout[1] += MX_LOOP_INTERVAL;
    if (autoTimeout[1] >= USR.config->Tautooff && USR.config->Tautooff)
    {
        DEBUG(5, "System going to close");
        USR.sys_status = System_Close;
        MX_Audio_Play_Start(Audio_PowerOff);
        osDelay(3000); //3s
        MX_PM_Shutdown();
    }
}

void handleRunning(void)
{
    uint8_t keyRes = scanKey();
    uint16_t timeout = 0;
    if (keyRes & KEY_PWR_PRESS)
    {
        uint16_t maxTimeout = USR.config->Tin;
        while ((!((keyRes = scanKey()) & KEY_PWR_RELEASE)) && // Waiting for PowerKey release
               (!(keyRes & KEY_SUB_PRESS)) &&                 // Waiting for UserKey press
               timeout < maxTimeout)                          // Waiting for timeout
        {
            timeout += MX_LOOP_INTERVAL;
        }

        if (timeout < maxTimeout && (keyRes & KEY_SUB_PRESS))
        {
            DEBUG(5, "System ColorSwitch");
            USR.bank_color += 1;
            MX_LED_bankUpdate(&USR);
            MX_Audio_Play_Start(Audio_ColorSwitch);
            MX_LED_startTrigger(LED_Trigger_ColorSwitch);
        }
        else if (timeout >= maxTimeout)
        {
            DEBUG(5, "System going to ready")
            USR.sys_status = System_Ready;
            MX_Audio_Play_Start(Audio_intoReady);
            update_param(MX_Audio_getLastTriggerPos(), IN);
            MX_LED_startTrigger(LED_Trigger_Stop);
            SimpleLED_ChangeStatus(SIMPLELED_STATUS_STANDBY);
            USR.bank_color = 0;
            MX_LED_bankUpdate(&USR);
        }
    }
    else if (keyRes & KEY_SUB_PRESS)
    {
        uint16_t maxTimeout = USR.config->TEtrigger;
        while (!(scanKey() & KEY_SUB_RELEASE))
        {
            timeout += MX_LOOP_INTERVAL;

            if (timeout >= maxTimeout)
            {
                DEBUG(5, "Trigger E");
                SimpleLED_ChangeStatus(SIMPLELED_STATUS_LOCKUP);
                MX_Audio_Play_Start(Audio_TriggerE);
                update_param(MX_Audio_getLastTriggerPos(), E);
                MX_LED_startTrigger(LED_TriggerE);
                while (!(scanKey() & KEY_SUB_RELEASE)) {
                }
                DEBUG(5, "Trigger E End");
                SimpleLED_ChangeStatus(SIMPLELED_STATUS_ON);
                MX_Audio_Play_Stop(Audio_TriggerE);
                MX_LED_startTrigger(LED_TriggerE_END);
                break;
            }
        }
        if (timeout < USR.config->TEtrigger && askTrigger(2))
        {
            DEBUG(5, "Trigger D");
            MX_Audio_Play_Start(Audio_TriggerD);
            update_param(MX_Audio_getLastTriggerPos(), D);
            MX_LED_startTrigger(LED_TriggerD);
        }
    }

    // move detection
    HAND_TriggerId_t handTrigger = MX_HAND_GetTrigger();
    if (handTrigger.hex != 0) {
        if (handTrigger.unio.isClash && askTrigger(1)) {
            MX_Audio_Play_Start(Audio_TriggerC);
            update_param(MX_Audio_getLastTriggerPos(), C);
            MX_LED_startTrigger(LED_TriggerC);
        }
        else if (handTrigger.unio.isSwing && askTrigger(0)) {
            MX_Audio_Play_Start(Audio_TriggerB);
            update_param(MX_Audio_getLastTriggerPos(), B);
            MX_LED_startTrigger(LED_TriggerB);
        }
    }

    if (MX_PM_isCharging())
    {
            DEBUG(5, "System going to charging")
            USR.sys_status = System_Ready;
            MX_Audio_Play_Start(Audio_Charging);
            MX_LED_startTrigger(LED_Trigger_Stop);
            SimpleLED_ChangeStatus(SIMPLELED_STATUS_STANDBY);
            USR.bank_color = 0;
            MX_LED_bankUpdate(&USR);

            USR.sys_status = System_Charging;
    }

    // clear auto 'in' trigger timer if there is any trigger.
    if (handTrigger.hex != 0 || keyRes != 0)
    {
        autoTimeout[0] = 0;
    }

    autoTimeout[0] += MX_LOOP_INTERVAL;

    if (autoTimeout[0] >= USR.config->Tautoin && USR.config->Tautoin)
    {
        DEBUG(5, "System going to ready")
        USR.sys_status = System_Ready;
        MX_Audio_Play_Start(Audio_intoReady);
        update_param(MX_Audio_getLastTriggerPos(), IN);
        MX_LED_startTrigger(LED_Trigger_Stop);
        USR.bank_color = 0;
        MX_LED_bankUpdate(&USR);
        SimpleLED_ChangeStatus(SIMPLELED_STATUS_STANDBY);
    }

    // update frozen trigger
    for (int i = 0; i < 3; i++)
    {
        if (frozenTimer[i] > 0)
            frozenTimer[i] -= MX_LOOP_INTERVAL;
        if (frozenTimer[i] < 0)
            frozenTimer[i] = 0;
    }
}

void handleCharged(void)
{
    // plug-out, just shutdown
    if (MX_PM_isCharging() == false)
    {
        DEBUG(5, "System shutdown");
        USR.sys_status = System_Close;
        MX_Audio_Play_Start(Audio_PowerOff);
        osDelay(3000); // 3s
        MX_PM_Shutdown();
    }
}

void handleCharging(void)
{
    // any key press, play 'charging'
    uint8_t res = scanKey();
    if ( (res & KEY_PWR_PRESS) ||
         (res & KEY_SUB_PRESS) )
    {
        MX_Audio_Play_Start(Audio_Charging);
    }
    // charging -> charged
    if (MX_PM_isCharged())
    {
        DEBUG(5, "System going to Charged");
        USR.sys_status = System_Charged;
    }
    // plug-out, just shutdown
    if (MX_PM_isCharging() == false)
    {
        DEBUG(5, "System shutdown");
        USR.sys_status = System_Close;
        MX_Audio_Play_Start(Audio_PowerOff);
        osDelay(3000); // 3s
        MX_PM_Shutdown();
    }
}

/**
 * @brief check if can trigger this.
 * @param id : 0 trigger B
 *             1 trigger C
 *             2 trigger D
 */
bool askTrigger(uint8_t id)
{
    if (frozenTimer[id] == 0)
    {
        switch(id) {
            case 0: {
                frozenTimer[0] = USR.config->TBfreeze > INT16_MAX ?
                    INT16_MAX : USR.config->TBfreeze;
                break;
            }
            case 1: {
                frozenTimer[1] = USR.config->TCfreeze > INT16_MAX ?
                    INT16_MAX : USR.config->TCfreeze;
                break;
            }
            case 2: {
                frozenTimer[2] = USR.config->TDfreeze > INT16_MAX ?
                    INT16_MAX : USR.config->TDfreeze;
                break;
            }
        }
        return true;
    }
    return false;
}


bool canBoot(void)
{
    uint32_t overtime = 0;

    if (USR.mute_flag)
        overtime = USR.config->Tmute;
    else
        overtime = USR.config->Tpon;
    
    return osKernelSysTick() >= osKernelSysTickMicroSec(overtime);
}
