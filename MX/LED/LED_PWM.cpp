#include "LED_PWM.h"
#include <math.h>

/** Some private parameters */
static const uint32_t T_SP        = 300; //慢速脉冲模式，亮度变化周期，单位ms
static const uint32_t T_MP        = 150; //中速脉冲模式，亮度变化周期，单位ms
static const uint32_t T_FP        = 75; //快速脉冲模式，亮度变化周期，单位ms
static const uint32_t T_Spark     = 200; //Trigger Spark维持时长
static const uint32_t T_nSpark    = 150; //Trigger nSpark维持时长
static const uint32_t T_nSparkGap = 150; //Trigger nSpark间隔时长
static const uint32_t nSparkCount = 2; //Trigger nSpark翻转次数
static const uint32_t T_Electricl = 100; //Trigger Electricl 间隔时长
static uint16_t       T_BREATH; //LMode呼吸灯周期
static uint16_t       BankColor[ 4 ]; //R G B L
static uint16_t       FBankColor[ 4 ]; //R G B L
static uint16_t       LBright     = 0;
static uint16_t       LDeep       = 0;
static uint16_t       LMode       = 0;
static uint32_t       breath_step = 0;

__weak bool LED_PWM_Init(void* arg)
{
    arg = arg;
    HAL_TIM_Base_Start(&htim1);
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_3);
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_4);
    LED_RGB_Output(0, 0, 0, 0);

    return true;
}
bool LED_PWM_Update(PARA_DYNAMIC_t* pt)
{
    uint8_t   bank  = (pt->bank_now + pt->bank_color) % pt->nBank;
    uint32_t* point = pt->BankColor + (bank * 4);

    BankColor[ 0 ] = (uint16_t)*point & 0xFFFF;
    BankColor[ 1 ] = (uint16_t)(*point >> 16);
    point += 1;
    BankColor[ 2 ] = (uint16_t)*point & 0xFFFF;
    BankColor[ 3 ] = (uint16_t)(*point >> 16);
    point += 1;
    FBankColor[ 0 ] = (uint16_t)*point & 0xFFFF;
    FBankColor[ 1 ] = (uint16_t)(*point >> 16);
    point += 1;
    FBankColor[ 2 ] = (uint16_t)*point & 0xFFFF;
    FBankColor[ 3 ] = (uint16_t)(*point >> 16);

    LBright  = pt->config->Lbright;
    LDeep    = pt->config->Ldeep;
    LMode    = pt->config->LMode;
    T_BREATH = pt->config->T_Breath;

    return true;
}

void LED_PWM_Handle(PARA_DYNAMIC_t* ptr)
{
    LED_Message_t message;
    LED_CMD_t     cmd;
    osEvent       evt = MX_LED_GetMessage(20);

#if LED_SUPPORT_FOLLOW_AUDIO == 0
    message = static_cast<LED_CMD_t>(evt.value.v);
    cmd     = message;
#elif LED_SUPPORT_FOLLOW_AUDIO == 1
    message.hex = static_cast<LED_CMD_t>(evt.value.v);
    cmd         = message.pair.cmd;
#endif
// 当在LED执行函数中收到了额外的动作命令，将忽略上方代码并跳转到此处
GETMESSAGE:
    switch (cmd) {
    case LED_Trigger_Start: {
        uint32_t step     = 0;
        uint32_t step_ms  = 10;
        uint32_t max_step = USR.config->ChDelay[ 0 ] / step_ms;
        uint8_t  cnt      = 0;
        for (cnt = 0; cnt < 3; cnt++)
            if (max_step < USR.config->ChDelay[ cnt + 1 ] / step_ms)
                max_step = USR.config->ChDelay[ cnt + 1 ] / step_ms;
        while (step < max_step + USR.config->TLon / step_ms) {
            LED_RGB_SoftRise_Single(0, USR.config->ChDelay[ 0 ], step, step_ms, USR.config->TLon);
            LED_RGB_SoftRise_Single(1, USR.config->ChDelay[ 1 ], step, step_ms, USR.config->TLon);
            LED_RGB_SoftRise_Single(2, USR.config->ChDelay[ 2 ], step, step_ms, USR.config->TLon);
            LED_RGB_SoftRise_Single(3, USR.config->ChDelay[ 3 ], step, step_ms, USR.config->TLon);
            osDelay(step_ms);
            step += 1;
        }
        // 每次进入确保清空循环
        breath_step = 0;
    } break;

    case LED_Trigger_Stop: {
        uint32_t step = 0;
        while (step < USR.config->TLoff / 10) {
            LED_RGB_SoftDown(step++, 10, USR.config->TLoff);
        }
    } break;

    case LED_TriggerB:
    case LED_TriggerC:
    case LED_TriggerD:
    case LED_TriggerE: {
        LED_CMD_t buf = cmd;
        while ((((cmd = LED_Trigger_Method(buf)) != LED_Trigger_EXIT)) && cmd < buf)
            ;
    } break;
    case LED_TriggerE_END:
        break;
    default:
        break;
    }
    if (USR.sys_status == System_Running) {
        switch (USR.config->LMode) {
        case LED_LMode_Method_Static: {
            LED_RGB_Limited(BankColor[ 0 ], BankColor[ 1 ], BankColor[ 2 ], BankColor[ 3 ]);
        } break;
        case LED_LMode_Method_Breath: {
            while ((cmd = LED_RGB_Breath(breath_step++, 10, T_BREATH)) == LED_NoTrigger)
                ;
            goto GETMESSAGE;
        } break;
        case LED_LMode_Method_SlowPulse: {
            while ((cmd = LED_RGB_Pulse(T_SP)) == LED_NoTrigger)
                ;
            goto GETMESSAGE;
        } break;
        case LED_LMode_Method_MidPulse: {
            while ((cmd = LED_RGB_Pulse(T_MP)) == LED_NoTrigger)
                ;
            goto GETMESSAGE;
        } break;
        case LED_LMode_Method_FastPulse: {
            while ((cmd = LED_RGB_Pulse(T_FP)) == LED_NoTrigger)
                ;
            goto GETMESSAGE;
        } break;
        }
    } else if (USR.sys_status == System_Charged) {
        static uint32_t cnt = 0;
        LED_RGB_Charged(cnt++, 20, 5000);
    } else if (USR.sys_status == System_Charging) {
        static uint32_t cnt = 0;
        LED_RGB_Charging(cnt++, 20, 5000);
    } else if (USR.sys_status == System_Ready) {
        LED_RGB_Output(0, 0, 0, 0);
    }
}

static LED_CMD_t LED_Trigger_Method(LED_CMD_t trigger_bcd)
{
    uint8_t   trigger_mode;
    uint8_t   using_mode;
    LED_CMD_t message;
    switch (trigger_bcd) {
    case LED_TriggerB:
        trigger_mode = USR.config->TBMode;
        trigger_mode &= 0x07;
        break;
    case LED_TriggerC:
        trigger_mode = USR.config->TCMode;
        trigger_mode &= 0x07;
        break;
    case LED_TriggerD:
        trigger_mode = USR.config->TDMode;
        trigger_mode &= 0x07;
        break;
    case LED_TriggerE:
        trigger_mode = USR.config->TEMode;
        break;
    default:
        break;
    }
    while ((trigger_mode & (0x01 << ((using_mode = rand() % 4 + 1) - 1))) == 0)
        ;
    switch (using_mode) {
    case LED_Trigger_Method_Static:
        break;
    case LED_Trigger_Method_Spark: {
        message = LED_RGB_Toggle(0, T_Spark);
        if (message > trigger_bcd)
            return message;
        // message = LED_RGB_Toggle(1, 1);
        // if (message > trigger_bcd) return message;
    } break;
    case LED_Trigger_Method_nSpark: {
        uint32_t totalTime = MX_Audio_getTriggerLastTime();
        uint8_t  cnt       = totalTime / (T_nSpark + T_nSparkGap);
        while (cnt--) {
            message = LED_RGB_Toggle(0, T_nSpark);
            if (message > trigger_bcd)
                return message;
            message = LED_RGB_Toggle(1, T_nSparkGap);
            if (message > trigger_bcd)
                return message;
        }
        message = LED_RGB_Toggle(0, T_nSpark);
        if (message > trigger_bcd)
            return message;
    } break;
    case LED_Trigger_Method_Electricl: {
        uint32_t cnt = 0;
        while (1) {
            message = LED_RGB_Toggle(cnt++, T_Electricl);
            if (message > trigger_bcd)
                return message;
        }
    } break;
    }
    return LED_Trigger_EXIT;
}

static void LED_RGB_Output(uint16_t r, uint16_t g, uint16_t b, uint16_t l)
{
    CHx_VAL(1) = r;
    CHx_VAL(2) = g;
    CHx_VAL(3) = b;
    CHx_VAL(4) = l;
}
static void LED_RGB_Limited(uint16_t r, uint16_t g, uint16_t b, uint16_t l)
{
    float ave = (r + g + b + l) / 1024.0 / 4.0;
    float a   = LBright / 1024.0;
    if (ave > a)
        ave = a / ave;
    else
        ave = 1;
    CHx_VAL(1) = r * ave;
    CHx_VAL(2) = g * ave;
    CHx_VAL(3) = b * ave;
    CHx_VAL(4) = l * ave;
}
static void LED_RGB_Output_Light(uint16_t* colors, float light)
{
    uint16_t* pt  = colors;
    float     ave = (pt[ 0 ] + pt[ 1 ] + pt[ 2 ] + pt[ 3 ]) / 1024.0f / 4.0f;
    float     a   = light / ave;
    CHx_VAL(1)    = a * pt[ 0 ];
    CHx_VAL(2)    = a * pt[ 1 ];
    CHx_VAL(3)    = a * pt[ 2 ];
    CHx_VAL(4)    = a * pt[ 3 ];
}
// Ever time run single step, if a trigger is coming, return triggerid

// Color: Bank->FBank->Bank
static LED_CMD_t LED_RGB_Charging(uint32_t step, uint32_t step_ms, uint32_t period_ms)
{
    const float pi_2        = 3.141592654 * 2;
    uint32_t    period_step = period_ms / step_ms;
    step %= period_step;
    float    d = cos(pi_2 * (float)step / (float)period_step) / 2 + 0.5;
    uint16_t r = 1024 * 0.15 * d;
    uint16_t g = 0;
    uint16_t b = 0;
    uint16_t l = 0;
    LED_RGB_Output(r, g, b, l);
    osEvent evt = MX_LED_GetMessage(step_ms);
    if (evt.status != osEventMessage)
        return LED_NoTrigger;
    else
        return static_cast<LED_CMD_t>(evt.value.v);
}
static LED_CMD_t LED_RGB_Charged(uint32_t step, uint32_t step_ms, uint32_t period_ms)
{
    const float pi_2        = 3.141592654 * 2;
    uint32_t    period_step = period_ms / step_ms;
    step %= period_step;
    float    d = cos(pi_2 * (float)step / (float)period_step) / 2 + 0.5;
    uint16_t r = 0;
    uint16_t g = 0;
    uint16_t b = 1024 * 0.15 * d;
    uint16_t l = 0;
    LED_RGB_Output(r, g, b, l);
    osEvent evt = MX_LED_GetMessage(step_ms);
    if (evt.status != osEventMessage)
        return LED_NoTrigger;
    else
        return static_cast<LED_CMD_t>(evt.value.v);
}
static LED_CMD_t LED_RGB_Breath(uint32_t step, uint32_t step_ms, uint32_t period_ms)
{
    const float pi_2        = 3.141592654 * 2;
    uint32_t    period_step = period_ms / step_ms;
    float       A           = (LBright - LDeep) / 1024.0;
    step %= period_step;
    float d = cos(pi_2 * (float)step / (float)period_step) * A / 2 + 0.5 * A + LDeep / 1024.0;

    LED_RGB_Output_Light(BankColor, d);
    osEvent evt = MX_LED_GetMessage(step_ms);
    if (evt.status != osEventMessage)
        return LED_NoTrigger;
    else
        return static_cast<LED_CMD_t>(evt.value.v);
}
static LED_CMD_t LED_RGB_Toggle(uint32_t step, uint32_t step_ms)
{
    step %= 2;
    if (!step)
        LED_RGB_Limited(FBankColor[ 0 ], FBankColor[ 1 ], FBankColor[ 2 ], FBankColor[ 3 ]);
    else
        LED_RGB_Limited(BankColor[ 0 ], BankColor[ 1 ], BankColor[ 2 ], BankColor[ 3 ]);
    osEvent evt = MX_LED_GetMessage(step_ms);
    if (evt.status != osEventMessage)
        return LED_NoTrigger;
    else {
#if LED_SUPPORT_FOLLOW_AUDIO == 0
        return static_cast<LED_CMD_t>(evt.value.v);
#elif LED_SUPPORT_FOLLOW_AUDIO == 1
        LED_Message_t message;
        message.hex = static_cast<LED_CMD_t>(evt.value.v);
        return message.pair.cmd;
#endif
    }
}
static LED_CMD_t LED_RGB_Pulse(uint32_t step_ms)
{
    uint16_t buf   = rand() % (LBright - LDeep) + LDeep;
    float    light = (float)buf / 1024;
    LED_RGB_Output_Light(BankColor, light);
    osEvent evt = MX_LED_GetMessage(step_ms);
    if (evt.status != osEventMessage)
        return LED_NoTrigger;
    else
        return static_cast<LED_CMD_t>(evt.value.v);
}
static LED_CMD_t LED_RGB_SoftRise(uint32_t step, uint32_t step_ms, uint32_t total_ms)
{
    const float pi_div2  = 3.141592654 / 2;
    uint32_t    step_num = total_ms / step_ms;
    // Get a function like y = 1 - cos(x)
    float d = 1 - cos(pi_div2 * (float)step / (float)step_num);
    LED_RGB_Limited(BankColor[ 0 ] * d, BankColor[ 1 ] * d, BankColor[ 2 ] * d, BankColor[ 3 ] * d);
    osEvent evt = MX_LED_GetMessage(step_ms);
    if (evt.status != osEventMessage)
        return LED_NoTrigger;
    else
        return static_cast<LED_CMD_t>(evt.value.v);
}
static LED_CMD_t LED_RGB_SoftRise_Single(uint8_t channel, uint32_t delay_ms, uint32_t step, uint32_t step_ms, uint32_t total_ms)
{
    const float pi_div2  = 3.141592654 / 2;
    uint32_t    step_num = total_ms / step_ms;
    uint32_t    delay    = delay_ms / step_ms;

    if (step <= delay || step >= delay + step_num)
        return LED_NoTrigger;
    // Get a function like y = 1 - cos(x)
    float d = sin(pi_div2 * (float)(step - delay) / (float)step_num);
    //float d = 1 - cos(pi_div2 * (float)(step - delay) / (float)step_num);
    //float d = 0.1;
    float l = (BankColor[ 0 ] + BankColor[ 1 ] + BankColor[ 2 ] + BankColor[ 3 ]) / 4.0 / 1024.0;
    float a = LBright / 1024.0;
    if (l > a)
        l = a / l;
    else
        l = 1;

    d *= l;

    switch (channel) {

    case 0:
        LED_RGB_Output(BankColor[ 0 ] * d, CHx_VAL(2), CHx_VAL(3), CHx_VAL(4));
        break;
    case 1:
        LED_RGB_Output(CHx_VAL(1), BankColor[ 1 ] * d, CHx_VAL(3), CHx_VAL(4));
        break;
    case 2:
        LED_RGB_Output(CHx_VAL(1), CHx_VAL(2), BankColor[ 2 ] * d, CHx_VAL(4));
        break;
    case 3:
        LED_RGB_Output(CHx_VAL(1), CHx_VAL(2), CHx_VAL(3), BankColor[ 3 ] * d);
        break;
    }
    //LED_RGB_Limited(BankColor[0]*d, CHx_VAL(2), CHx_VAL(3), CHx_VAL(4));
    // osEvent evt = MX_LED_GetMessage(step_ms);
    // if (evt.status != osEventMessage) return LED_NoTrigger;
    // else return static_cast<LED_CMD_t>(evt.value.v);
    return LED_NoTrigger;
}
static LED_CMD_t LED_RGB_SoftDown(uint32_t step, uint32_t step_ms, uint32_t total_ms)
{
    static uint16_t r, g, b, l;
    const float     pi_div2  = 3.141592654 / 2;
    uint32_t        step_num = total_ms / step_ms;
    // Get a function like y = cos(x)
    float d = cos(pi_div2 * (float)step / (float)step_num);

    if (step == 0)
        r = CHx_VAL(1), g = CHx_VAL(2), b = CHx_VAL(3), l = CHx_VAL(4);
    LED_RGB_Limited(r * d, g * d, b * d, l * d);
    osEvent evt = MX_LED_GetMessage(step_ms);
    if (evt.status != osEventMessage)
        return LED_NoTrigger;
    else
        return static_cast<LED_CMD_t>(evt.value.v);
}
