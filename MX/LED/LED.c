#include "LED.h"
#include "cmsis_os.h"
#include "debug.h"
#if USE_NP == 1
#include "LED_NP.h"
#else
#include "LED_PWM.h"
#endif
/* Var *************************************************/
static osMessageQId LED_CMDHandle;
static osThreadId   selfThreadId = NULL;
LED_IF_t ledIf = {
#if USE_NP == 1
    .init = LED_NP_Init,
    .updateParam = LED_NP_Update,
    .handle = LED_NP_Handle,
#else
    .init = LED_PWM_Init,
    .updateParam = LED_PWM_Update,
    .handle = LED_PWM_Handle,
#endif
};

/******************************************************/
/**
 * @Breif  触发条件后向LEDOpra任务发送触发信息
 */
void MX_LED_startTrigger(LED_Message_t message)
{
    DEBUG(5, "LED send Message:%d", message);
    osMessagePut(LED_CMDHandle, message, osWaitForever);
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
    if (ledIf.init != NULL && ledIf.init(NULL) == false) {
        DEBUG(0, "LED Driver Inited error");
        while (1)
            DEBUG_BKPT();
    }
    osThreadDef(LED, LEDOpra, osPriorityLow, 0, 1024);
    selfThreadId = osThreadCreate(osThread(LED), NULL);

    osMessageQDef(LED_CMD, 8, uint32_t);
    LED_CMDHandle = osMessageCreate(osMessageQ(LED_CMD), selfThreadId);
}
