#include "PWM-led.h"
/** Protocol function *********************************/
static void pwm_io_init(void);
static void pwm_charged_loop(uint32_t step_ms, uint32_t period_ms);
static void pwm_charging_loop(uint32_t step_ms, uint32_t period_ms);
static LED_Message_t pwm_run_loop(uint32_t *step, uint32_t step_ms);
static LED_Message_t pwm_trigger(LED_Message_t method);

/** Public var ****************************************/
const LED_Opra_t PWM_LED_Opra = {
    .io_init = pwm_io_init,
    .charged_loop = pwm_charged_loop,
    .charging_loop = pwm_charging_loop,
    .run_loop = pwm_run_loop,
    .trigger = pwm_trigger,
};
/* Config const var ***********************************/
const uint32_t T_SP = 300;        //慢速脉冲模式，亮度变化周期，单位ms
const uint32_t T_MP = 150;        //中速脉冲模式，亮度变化周期，单位ms
const uint32_t T_FP = 75;         //快速脉冲模式，亮度变化周期，单位ms
const uint32_t T_Spark = 200;     //Trigger Spark维持时长
const uint32_t T_nSpark = 150;    //Trigger nSpark维持时长
const uint32_t T_nSparkGap = 150; //Trigger nSpark间隔时长
const uint32_t nSparkCount = 2;   //Trigger nSpark翻转次数
const uint32_t T_Electricl = 100; //Trigger Electricl 间隔时长
uint16_t T_BREATH;                //LMode呼吸灯周期
/** Private var ***************************************/
static uint16_t BankColor[4];
static uint16_t FBankColor[4];
static uint16_t LBright = 0;
static uint16_t LDeep = 0;
static uint16_t LMode = 0;

/** static IO function ********************************/
__STATIC_INLINE void LED_RGB_Output(uint16_t r, uint16_t g, uint16_t b, uint16_t l)
{
  MX_TIM_PowerLEDOpra(1, r);
  MX_TIM_PowerLEDOpra(2, g);
  MX_TIM_PowerLEDOpra(3, b);
  MX_TIM_PowerLEDOpra(4, l);
}
__STATIC_INLINE void LED_Sync_Color(void)
{
  static int8_t bank_pos = -1;
  uint8_t bank = (USR.bank_now + USR.bank_color) % USR.nBank;

  if (bank_pos != bank)
  {
    bank_pos = bank;
    uint32_t *point = USR.BankColor + (bank * 4);

    BankColor[0] = (uint16_t)*point & 0xFFFF;
    BankColor[1] = (uint16_t)(*point >> 16);
    point += 1;
    BankColor[2] = (uint16_t)*point & 0xFFFF;
    BankColor[3] = (uint16_t)(*point >> 16);
    point += 1;
    FBankColor[0] = (uint16_t)*point & 0xFFFF;
    FBankColor[1] = (uint16_t)(*point >> 16);
    point += 1;
    FBankColor[2] = (uint16_t)*point & 0xFFFF;
    FBankColor[3] = (uint16_t)(*point >> 16);

    LBright = USR.config->Lbright;
    LDeep = USR.config->Ldeep;
    LMode = USR.config->LMode;
    T_BREATH = USR.config->T_Breath;
  }
}
__STATIC_INLINE void LED_RGB_Limited(uint16_t r, uint16_t g, uint16_t b, uint16_t l)
{
  float ave = (r + g + b + l) / 1024.0 / 4.0;
  float a = LBright / 1024.0;
  if (ave > a)
    ave = a / ave;
  else
    ave = 1;
  MX_TIM_PowerLEDOpra(1, r * ave);
  MX_TIM_PowerLEDOpra(2, g * ave);
  MX_TIM_PowerLEDOpra(3, b * ave);
  MX_TIM_PowerLEDOpra(4, l * ave);
}
__STATIC_INLINE void LED_RGB_SoftRise_Single(uint8_t channel, uint32_t delay_ms, uint32_t step, uint32_t step_ms, uint32_t total_ms)
{
  const float pi_div2 = 3.141592654 / 2;
  uint32_t step_num = total_ms / step_ms;
  uint32_t delay = delay_ms / step_ms;

  if (step <= delay || step >= delay + step_num)
    return;
  // Get a function like y = 1 - cos(x)
  float d = sin(pi_div2 * (float)(step - delay) / (float)step_num);
  float l = (BankColor[0] + BankColor[1] + BankColor[2] + BankColor[3]) / 4.0 / 1024.0;
  float a = LBright / 1024.0;
  if (l > a)
    l = a / l;
  else
    l = 1;
  d *= l;
  switch (channel)
  {
  case 0:
    LED_RGB_Output(BankColor[0] * d, MX_TIM_PowerLEDRead(2), MX_TIM_PowerLEDRead(3), MX_TIM_PowerLEDRead(4));
    break;
  case 1:
    LED_RGB_Output(MX_TIM_PowerLEDRead(1), BankColor[1] * d, MX_TIM_PowerLEDRead(3), MX_TIM_PowerLEDRead(4));
    break;
  case 2:
    LED_RGB_Output(MX_TIM_PowerLEDRead(1), MX_TIM_PowerLEDRead(2), BankColor[2] * d, MX_TIM_PowerLEDRead(4));
    break;
  case 3:
    LED_RGB_Output(MX_TIM_PowerLEDRead(1), MX_TIM_PowerLEDRead(2), MX_TIM_PowerLEDRead(3), BankColor[3] * d);
    break;
  }
}
__STATIC_INLINE LED_Message_t LED_RGB_Toggle(uint32_t step, uint32_t step_ms)
{
  step %= 2;
  if (!step)
    LED_RGB_Limited(FBankColor[0], FBankColor[1], FBankColor[2], FBankColor[3]);
  else
    LED_RGB_Limited(BankColor[0], BankColor[1], BankColor[2], BankColor[3]);
  osEvent evt = osMessageGet(LED_CMDHandle, step_ms);
  if (evt.status != osEventMessage)
    return LED_NoTrigger;
  else
    return evt.value.v;
}
/* Structure Protocol Define *************************/
static void pwm_io_init(void)
{
  MX_TIM_PowerLEDStart();
  LED_RGB_Output(0, 0, 0, 0);
}

static void pwm_charged_loop(uint32_t step_ms, uint32_t period_ms)
{
  const float pi_2 = 3.141592654 * 2;
  uint32_t period_step = period_ms / step_ms;
  uint32_t step = 0;
  while (1)
  {
    step %= period_step;
    float d = cos(pi_2 * (float)step / (float)period_step) / 2 + 0.5;
    LED_RGB_Output(0, 0, 1024 * 0.15 * d, 0);
    osMessageGet(LED_CMDHandle, step_ms);
  }
}

static void pwm_charging_loop(uint32_t step_ms, uint32_t period_ms)
{
  const float pi_2 = 3.141592654 * 2;
  uint32_t period_step = period_ms / step_ms;
  uint32_t step = 0;
  while (1)
  {
    step %= period_step;
    float d = cos(pi_2 * (float)step / (float)period_step) / 2 + 0.5;
    LED_RGB_Output(1024 * 0.15 * d, 0, 0, 0);
    osMessageGet(LED_CMDHandle, step_ms);
    step += step_ms;
  }
}

static LED_Message_t pwm_run_loop(uint32_t *step, uint32_t step_ms)
{
  LED_Sync_Color();
  switch (USR.config->LMode)
  {
  case LED_LMode_Method_Static:
  {
    LED_RGB_Limited(BankColor[0], BankColor[1], BankColor[2], BankColor[3]);
  }
  break;
  case LED_LMode_Method_Breath:
  {
    const float pi_2 = 3.141592654 * 2;
    uint32_t period_step = T_BREATH / step_ms;
    float A = (LBright - LDeep) / 1024.0;
    *step %= period_step;
    float d = cos(pi_2 * (float)*step / (float)period_step) * A / 2 + 0.5 * A + LDeep / 1024.0;
    LED_RGB_Output(BankColor[0] * d,
                   BankColor[1] * d,
                   BankColor[2] * d,
                   BankColor[3] * d);
  }
  break;
  case LED_LMode_Method_SlowPulse:
  case LED_LMode_Method_MidPulse:
  case LED_LMode_Method_FastPulse:
  {
    uint16_t buf = rand() % (LBright - LDeep) + LDeep;
    float light = (float)buf / 1024;
    LED_RGB_Output(BankColor[0] * light,
                   BankColor[1] * light,
                   BankColor[2] * light,
                   BankColor[3] * light);
  }
  break;
  }
  *step += step_ms;
  osEvent evt = osMessageGet(LED_CMDHandle, step_ms);
  if (evt.status != osEventMessage)
    return LED_NoTrigger;
  else
    return evt.value.v;
}

static LED_Message_t pwm_trigger(LED_Message_t method)
{
  uint8_t trigger_mode = 0;
  uint8_t using_mode;
  LED_Message_t message;

  switch (method)
  {
  case LED_Trigger_Start:
  {
    uint32_t step = 0;
    uint32_t step_ms = 10;
    uint32_t max_step = USR.config->ChDelay[0] / step_ms;
    uint8_t cnt = 0;
    for (cnt = 0; cnt < 3; cnt++)
    {
      if (max_step < USR.config->ChDelay[cnt + 1] / step_ms)
        ;
      max_step = USR.config->ChDelay[cnt + 1] / step_ms;
    }
    while (step < max_step + USR.config->TLon / step_ms)
    {
      LED_RGB_SoftRise_Single(0, USR.config->ChDelay[0], step, step_ms, USR.config->TLon);
      LED_RGB_SoftRise_Single(1, USR.config->ChDelay[1], step, step_ms, USR.config->TLon);
      LED_RGB_SoftRise_Single(2, USR.config->ChDelay[2], step, step_ms, USR.config->TLon);
      LED_RGB_SoftRise_Single(3, USR.config->ChDelay[3], step, step_ms, USR.config->TLon);
      osDelay(step_ms);
      step += 1;
    }
  }
  break;

  case LED_Trigger_Stop:
  {
    uint32_t step = 0;
    while (step < USR.config->TLoff / 10)
    {
      uint16_t r, g, b, l;
      const float pi_div2 = 3.141592654 / 2;
      uint32_t step_num = USR.config->TLoff / 10;
      float d = cos(pi_div2 * (float)step / (float)step_num);
      if (step == 0)
      {
        r = MX_TIM_PowerLEDRead(1);
        g = MX_TIM_PowerLEDRead(2);
        b = MX_TIM_PowerLEDRead(3);
        l = MX_TIM_PowerLEDRead(4);
      }
      LED_RGB_Limited(r * d, g * d, b * d, l * d);
      osEvent evt = osMessageGet(LED_CMDHandle, 10);
      if (evt.status == osEventMessage)
        return evt.value.v;
    }
  }
  break;

  case LED_TriggerB:
  {
    trigger_mode = USR.config->TBMode;
    trigger_mode &= 0x07;
  }
  break;

  case LED_TriggerC:
  {
    trigger_mode = USR.config->TCMode;
    trigger_mode &= 0x07;
  }
  break;

  case LED_TriggerD:
  {
    trigger_mode = USR.config->TDMode;
    trigger_mode &= 0x07;
  }
  break;

  case LED_TriggerE:
  {
    trigger_mode = USR.config->TEMode;
  }
  break;

  case LED_TriggerE_END:
    break;

  default:
  {
    log_w("Unkonw Message ID:%d", (int)method);
  }
  break;
  }

  // Identify if a trigger, not start/stop
  if (trigger_mode != 0)
  {
    while ((trigger_mode & (0x01 << ((using_mode = rand() % 4 + 1) - 1))) == 0)
      ;
    switch (using_mode)
    {
    case LED_Trigger_Method_Static:
      break;
    case LED_Trigger_Method_Spark:
    {
      message = LED_RGB_Toggle(0, T_Spark);
      if (message > method)
        return message;
    }
    break;
    case LED_Trigger_Method_nSpark:
    {
      uint8_t cnt = nSparkCount;
      while (--cnt)
      {
        message = LED_RGB_Toggle(0, T_nSpark);
        if (message > method)
          return message;
        message = LED_RGB_Toggle(1, T_nSparkGap);
        if (message > method)
          return message;
      }
      message = LED_RGB_Toggle(0, T_nSpark);
      if (message > method)
        return message;
    }
    break;
    case LED_Trigger_Method_Electricl:
    {
      uint32_t cnt = 0;
      while (1)
      {
        message = LED_RGB_Toggle(cnt++, T_Electricl);
        if (message > method)
          return message;
      }
    }
    break;
    }
  }

  return LED_NoTrigger;
}
