#ifndef PWM_LED_H_
#define PWM_LED_H_

#ifndef LOG_TAG
#define LOG_TAG "PWM-LED"
#endif

#include "DEBUG.h"
#include "MX_osID.h"
#include "USR_CONFIG.h"
#include "cmsis_os.h"
#include "mx-config.h"
#include "mx-led.h"
#include "mx-tim.h"
#include <math.h>
#include <stdint.h>

typedef enum _led_method {
  LED_Method_Static,
  LED_Method_Breath,
  LED_Method_Puls,
  LED_Method_Toggle,
  LED_Method_SoftRise,
  LED_Method_SoftDown,
} LED_Method_t;

typedef enum _led_lmode_method {
  LED_LMode_Method_Static = 0x01,    // 静止
  LED_LMode_Method_Breath = 0x02,    // 呼吸, 以T_Breath为周期 LBright~Ldeep的亮度
  LED_LMode_Method_SlowPulse = 0x03, // 慢速脉冲  以T_SP为周期，Lbright-Ldeep为下限，亮度在范围内随机跳变
  LED_LMode_Method_MidPulse = 0x04,  // 中速脉冲  以T_SP为周期，Lbright-Ldeep为下限，亮度在范围内随机跳变
  LED_LMode_Method_FastPulse = 0x05, // 快速脉冲  以T_SP为周期，Lbright-Ldeep为下限，亮度在范围内随机跳变
} LED_LMode_Method_t;

typedef enum _led_trigger_method {
  LED_Trigger_Method_Static = 0x01,    // 静止 不做操作
  LED_Trigger_Method_Spark = 0x02,     // 闪烁 翻转为FBank色一次， 时长为T_Spark
  LED_Trigger_Method_nSpark = 0x03,    // 闪烁 翻转为FBank色n次，时长为T_nSpark,间隔时间为T_nSparkGap
  LED_Trigger_Method_Electricl = 0x04, // 持续闪烁 单次保持时长为T_Electricl
} LED_Trigger_Method_t;

void LED_Bank_Update(PARA_DYNAMIC_t *);
extern const LED_Opra_t PWM_LED_Opra;

#endif
