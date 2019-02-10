#include "LOOP.h"
#include "FreeRTOS.h"
#include "cmsis_os.h"

static osThreadId loopThreadId;

// [0]: auto 'in'
// [1]: auto 'powerOff'
static uint32_t autoTimeout[2] = {0, 0};
// [0]: trigger B timer
// [1]: trigger C timer
// [2]: trigger D timer
static uint32_t frozenTimer[3] = {0, 0, 0};

#define KEY_1_PRESS     (0x01)
#define KEY_2_PRESS     (0x02)
#define KEY_1_RELEASE   (0x04)
#define KEY_2_RELEASE   (0x08)

static uint8_t scanKey(void);
static bool askTrigger(uint8_t);
static void handleReady(void);
static void handleRunning(void);
static void handleCharged(void);
static void handleCharging(void);

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
    /** Init all the other function modules
     */
    // if init parameter error, just beep and shutdown.
    if (MX_PARAM_Init() == false ||
        MX_HAND_Init() == false ||
        MX_KEY_Init() == false
       )
    {
        MX_PM_Boot();
        MX_Audio_PlayBeep();
        MX_PM_Shutdown();
        while (1);
    }
    MX_MUX_Init();
    MX_PM_Init();
    MX_LED_Init();
    SimpleLED_Init();

    if (USR.mute_flag) {
        osDelay(USR.config->Tmute);
    } else {
        osDelay(USR.config->Tpon);
    }

    MX_PM_Boot();

    /* Break point here */
    DEBUG_BKPT();

    USR.sys_status = System_Restart;
    usr_switch_bank(0, 1);
    USR.bank_color = 0;

    LED_Bank_Update(&USR);

    if (MX_PM_needWarning())
    {
        uint8_t buf = USR.mute_flag;
        USR.mute_flag = 0;
        MX_Audio_Play_Start(Audio_LowPower);
        MX_Audio_Play_Start(Audio_LowPower);
        USR.mute_flag = buf;
    }

    MX_Audio_Play_Start(Audio_Boot);

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
    static uint8_t status = 1;
    uint8_t buf = 0, pre_buf, res = 0;

    if (MX_KEY_GetStatus(KEY_1) == KEY_PRESS)
    {
        buf |= 0x01;
    }
    if (MX_KEY_GetStatus(KEY_2) == KEY_PRESS)
    {
        buf |= 0x02;
    }

    pre_buf = buf;
    buf ^= status;
    status = pre_buf;

    if (buf & 0x01)
    {
        if (pre_buf & 0x01)
        {
            res |= KEY_1_PRESS;
        }
        else
        {
            res |= KEY_2_RELEASE;
        }
        
    }
    if (buf & 0x02)
    {
        if (pre_buf & 0x02)
        {
            res |= KEY_2_PRESS;
        }
        else
        {
            res |= KEY_2_RELEASE;
        }
        
    }
    return res;
}

void handleReady(void)
{
    uint8_t keyRes = scanKey();
    uint16_t timeout = 0;

    if (keyRes & KEY_1_PRESS)
    {
        uint16_t maxTimeout = USR.config->Tpoff > USR.config->Tout ?
            USR.config->Tpoff :
            USR.config->Tout;
        // wait for key release
        while (!(scanKey() & KEY_1_RELEASE) && timeout < maxTimeout)
        {
            osDelay(MX_LOOP_INTERVAL);
            timeout += MX_LOOP_INTERVAL;
        }

        // trigger 'out'
        if ((timeout > maxTimeout && USR.config->Tpoff <= USR.config->Tout) ||
            (timeout < maxTimeout && USR.config->Tpoff >  USR.config->Tout))
        {
            DEBUG(5, "System going to running");
            USR.sys_status = System_Running;
            LED_Bank_Update(&USR);
            MX_Audio_Play_Start(Audio_intoRunning);
            LED_Start_Trigger(LED_Trigger_Start);
            SimpleLED_ChangeStatus(SIMPLELED_STATUS_ON);
            autoTimeout[0] = 0;
        }
        // power off
        else 
        {
            DEBUG(5, "System going to close");
            USR.sys_status = System_Close;
            MX_Audio_Play_Start(Audio_PowerOff);
            osDelay(3000); //3s
            MX_PM_Shutdown();
        }
        
    }
    else if (keyRes & KEY_2_PRESS)
    {
        uint16_t maxTimeout = USR.config->Ts_switch;
        while ((!(scanKey() & KEY_2_RELEASE)) && timeout < maxTimeout)
        {
            osDelay(MX_LOOP_INTERVAL);
            timeout += MX_LOOP_INTERVAL;
        }

        if (timeout >= maxTimeout)
        {
            DEBUG(5, "System Bank switch");
            usr_switch_bank((USR.bank_now + 1) % USR.nBank, 0);
            LED_Bank_Update(&USR);
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
    if (autoTimeout[1] >= USR.config->Tautooff)
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
    if (keyRes & KEY_1_PRESS)
    {
        uint16_t maxTimeout = USR.config->Tin;
        while ((!((keyRes = scanKey()) & KEY_1_RELEASE)) &&  // Waiting for PowerKey release
            (!(keyRes = scanKey()) & KEY_2_PRESS) &&         // Waiting for UserKey press
            timeout < maxTimeout)                            // Waiting for timeout
        {
            osDelay(MX_LOOP_INTERVAL);
            timeout += MX_LOOP_INTERVAL;
        }

        if (timeout < maxTimeout && (keyRes & KEY_2_PRESS))
        {
            DEBUG(5, "System ColorSwitch");
            USR.bank_color += 1;
            LED_Bank_Update(&USR);
            MX_Audio_Play_Start(Audio_ColorSwitch);
        }
        else if (timeout >= maxTimeout)
        {
            DEBUG(5, "System going to ready")
            USR.sys_status = System_Ready;
            LED_Start_Trigger(LED_Trigger_Stop);
            MX_Audio_Play_Start(Audio_intoReady);
            SimpleLED_ChangeStatus(SIMPLELED_STATUS_STANDBY);
            USR.bank_color = 0;
            LED_Bank_Update(&USR);
        }
    }
    else if (keyRes & KEY_2_PRESS)
    {
        uint16_t maxTimeout = USR.config->TEtrigger;
        while (!(scanKey() & KEY_2_RELEASE))
        {
            osDelay(MX_LOOP_INTERVAL);
            timeout += MX_LOOP_INTERVAL;

            if (timeout >= maxTimeout)
            {
                DEBUG(5, "Trigger E");
                SimpleLED_ChangeStatus(SIMPLELED_STATUS_LOCKUP);
                LED_Start_Trigger(LED_TriggerE);
                MX_Audio_Play_Start(Audio_TriggerE);
                while (!(scanKey() & KEY_2_RELEASE)) {
                    osDelay(MX_LOOP_INTERVAL);
                }
                DEBUG(5, "Trigger E End");
                SimpleLED_ChangeStatus(SIMPLELED_STATUS_ON);
                LED_Start_Trigger(LED_TriggerE_END);
                MX_Audio_Play_Stop(Audio_TriggerE);
                break;
            }
        }
        if (timeout < USR.config->TEtrigger && askTrigger(2))
        {
            DEBUG(5, "Trigger D");
            LED_Start_Trigger(LED_TriggerD);
            MX_Audio_Play_Start(Audio_TriggerD);
        }
    }

    // move detection
    HAND_TriggerId_t handTrigger = MX_HAND_GetTrigger();
    if (handTrigger == HAND_CLIK && askTrigger(1)) {
        LED_Start_Trigger(LED_TriggerC);
        MX_Audio_Play_Start(Audio_TriggerC);
    }
    else if (handTrigger == HAND_WAVE && askTrigger(0)) {
        LED_Start_Trigger(LED_TriggerB);
        MX_Audio_Play_Start(Audio_TriggerB);
    }

    if (MX_PM_isCharging())
    {
            DEBUG(5, "System going to charging")
            USR.sys_status = System_Ready;
            LED_Start_Trigger(LED_Trigger_Stop);
            MX_Audio_Play_Start(Audio_intoReady);
            SimpleLED_ChangeStatus(SIMPLELED_STATUS_STANDBY);
            USR.bank_color = 0;
            LED_Bank_Update(&USR);

            USR.sys_status = System_Charging;
    }

    // clear auto 'in' trigger timer if there is any trigger.
    if (handTrigger != HAND_NULL || keyRes != 0)
    {
        autoTimeout[0] = 0;
    }

    autoTimeout[0] += MX_LOOP_INTERVAL;

    if (autoTimeout[0] >= USR.config->Tautoin)
    {
        DEBUG(5, "System going to ready")
        USR.sys_status = System_Ready;
        LED_Start_Trigger(LED_Trigger_Stop);
        MX_Audio_Play_Start(Audio_intoReady);
        USR.bank_color = 0;
        LED_Bank_Update(&USR);
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
    // any trigger, play 'charging'
    if (scanKey() != 0)
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
