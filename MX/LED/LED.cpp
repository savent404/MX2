#include "LED.h"
#include "audio.h"
#include "cmsis_os.h"
#include "debug.h"
#if USE_NP == 1
#    include "LED_NP.h"
#else
#    include "LED_PWM.h"
#endif
/* Var *************************************************/
static osMessageQId LED_CMDHandle;
static osThreadId   selfThreadId = NULL;

LED_IF_t ledIf = {
#if USE_NP == 1
#    if __GNUC__ >= 4
    init : LED_NP_Init,
    handle : LED_NP_Handle,
    updateParam : LED_NP_Update,
    updateBG : LED_NP_updateBG,
    updateFT : LED_NP_updateFT,
    updateTG : LED_NP_updateTG,
    applySets : LED_NP_applySets,
#    elif __ICCARM__
    .init        = LED_NP_Init,
    .updateParam = LED_NP_Update,
    .handle      = LED_NP_Handle,
    .updateBG    = LED_NP_updateBG,
    .updateTG    = LED_NP_updateTG,
    .updateFT    = LED_NP_updateFT,
    .applySets   = LED_NP_applySets,
#    endif
#else
// .init = LED_PWM_Init,
// .updateParam = LED_PWM_Update,
// .handle = LED_PWM_Handle,
#endif
};

/******************************************************/
/**
 * @Breif  触发条件后向LEDOpra任务发送触发信息
 */
void MX_LED_startTrigger(LED_CMD_t message)
{
    LED_Message_t out;

#if LED_SUPPORT_FOLLOW_AUDIO == 0
    out = message;
    osMessagePut(LED_CMDHandle, out, osWaitForever);
    DEBUG(5, "LED send Message:0x%04x", out);
#elif LED_SUPPORT_FOLLOW_AUDIO == 1
    out.pair.cmd = message;
    out.pair.alt = MX_Audio_getTriggerLastTime();
    osMessagePut(LED_CMDHandle, out.hex, osWaitForever);
    DEBUG(5, "LED send Message:0x%04x", out.hex);
#endif
}
osEvent MX_LED_GetMessage(uint32_t timeout)
{
    return osMessageGet(LED_CMDHandle, timeout);
}
/**
 * @Breif  改变当前颜色Bank后(BankSwitch or ColorSwitch)， 更新任务中的局部变量
 */
void MX_LED_bankUpdate(PARA_DYNAMIC_t* pt)
{
    if (ledIf.updateParam != NULL && !ledIf.updateParam(pt)) {
        DEBUG(3, "LED update parameter error");
    }
}

/**
 * @Brief  LED处理函数,响应向LED_CMDHandle发送的LED动作命令
 */
void LEDOpra(void const* argument)
{
    if (ledIf.handle != NULL) {
        ledIf.handle(NULL);
    }
    DEBUG(2, "LED not working");
    while (1)
        DEBUG_BKPT();
}

void MX_LED_Init(void)
{
    if (ledIf.init != NULL && ledIf.init(USR.hwParam) == false) {
        DEBUG(0, "LED Driver Inited error");
        while (1)
            DEBUG_BKPT();
    }
    osThreadDef(LED, LEDOpra, osPriorityLow, 0, 1024);
    selfThreadId = osThreadCreate(osThread(LED), NULL);

    osMessageQDef(LED_CMD, 8, uint32_t);
    LED_CMDHandle = osMessageCreate(osMessageQ(LED_CMD), selfThreadId);
}

void MX_LED_updateBG(triggerSets_BG_t t)
{
    ledIf.updateBG(t);
}

void MX_LED_updateTG(triggerSets_TG_t t)
{
    ledIf.updateTG(t);
}

void MX_LED_updateFT(triggerSets_FT_t t)
{
    ledIf.updateFT(t);
}

void MX_LED_applySets(void)
{
    ledIf.applySets();
}
