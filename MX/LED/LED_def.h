#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#include "param.h"
#include <stdint.h>
#include <stdbool.h>

typedef enum _led_message {
    LED_Trigger_EXIT = 0x00,
    LED_NoTrigger = 0x01,
    LED_TriggerB  = 0x02,
    LED_TriggerC  = 0x03,
    LED_TriggerD  = 0x04,
    LED_TriggerE  = 0x05,
    LED_TriggerE_END = 0x06,
    LED_Trigger_ColorSwitch = 0x07,
    LED_Trigger_Start = 0x08,
    LED_Trigger_Stop = 0x09,
} LED_Message_t;

typedef enum _led_method {
    LED_Method_Static,
    LED_Method_Breath,
    LED_Method_Puls,
    LED_Method_Toggle,
    LED_Method_SoftRise,
    LED_Method_SoftDown,
} LED_Method_t;

typedef enum _led_lmode_method {
    LED_LMode_Method_Static = 0x01,   // 静止
    LED_LMode_Method_Breath = 0x02,   // 呼吸, 以T_Breath为周期 LBright~Ldeep的亮度
    LED_LMode_Method_SlowPulse = 0x03, // 慢速脉冲  以T_SP为周期，Lbright-Ldeep为下限，亮度在范围内随机跳变
    LED_LMode_Method_MidPulse = 0x04,  // 中速脉冲  以T_SP为周期，Lbright-Ldeep为下限，亮度在范围内随机跳变
    LED_LMode_Method_FastPulse = 0x05, // 快速脉冲  以T_SP为周期，Lbright-Ldeep为下限，亮度在范围内随机跳变
} LED_LMode_Method_t;

typedef enum _led_trigger_method {
    LED_Trigger_Method_Static = 0x01,   // 静止 不做操作
    LED_Trigger_Method_Spark = 0x02,    // 闪烁 翻转为FBank色一次， 时长为T_Spark
    LED_Trigger_Method_nSpark = 0x03,   // 闪烁 翻转为FBank色n次，时长为T_nSpark,间隔时间为T_nSparkGap
    LED_Trigger_Method_Electricl = 0x04,// 持续闪烁 单次保持时长为T_Electricl
} LED_Trigger_Method_t;

typedef struct _interface_led {
    bool (*init)(void* arg);
    void (*handle)(PARA_DYNAMIC_t* ptr);
    bool (*updateParam)(PARA_DYNAMIC_t* param);
} LED_IF_t;

#ifdef __cplusplus
}
#endif