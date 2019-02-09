#include "LED.h"
#include "cmsis_os.h"
#include "audio.h"
#include "debug.h"
#include "param.h"
#include "tim.h"
#if USE_NP == 1
#include "LED_NP.h"
#else
#include "LED_PWM.h"
#endif
/* Var *************************************************/
extern osThreadId defaultTaskHandle;
extern osThreadId DACTaskHandle;
extern osThreadId LEDTaskHandle;
extern osThreadId WavTaskHandle;
extern osMessageQId DAC_BufferHandle;
extern osMessageQId DAC_CMDHandle;
extern osMessageQId LED_CMDHandle;
extern osSemaphoreId DAC_Complete_FlagHandle;

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
void LED_Start_Trigger(LED_Message_t message)
{
    DEBUG(3, "LED send Message:%d", message);
    osMessagePut(LED_CMDHandle, message, osWaitForever);
}
osEvent LED_GetMessage(uint32_t timeout)
{
    return osMessageGet(LED_CMDHandle, timeout);
}
/**
 * @Breif  改变当前颜色Bank后(BankSwitch or ColorSwitch)， 更新任务中的局部变量
 */
void LED_Bank_Update(PARA_DYNAMIC_t* pt)
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
    if (ledIf.init != NULL && ledIf.init(NULL) == false) {
        DEBUG(0, "LED Driver Inited error");
        exit(-1);
    }
    if (ledIf.handle != NULL) {
        ledIf.handle(NULL);
    }
    DEBUG(2, "LED not working");
    while (1)
        ;
}

void MX_LED_Init(void)
{
    osThreadDef(self, LEDOpra, osPriorityLow, 0, 1024);
    osThreadCreate(osThread(self), NULL);
}
