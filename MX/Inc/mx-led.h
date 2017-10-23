#ifndef _MX_LED_H_
#define _MX_LED_H_

#ifndef LOG_TAG
#define LED_TAG "mx-led"
#endif

#include "DEBUG.h"
#include "cmsis_os.h"
#include "mx-config.h"

typedef enum _led_message {
  LED_Trigger_EXIT = 0x00,
  LED_NoTrigger = 0x01,
  LED_TriggerB = 0x02,
  LED_TriggerC = 0x03,
  LED_TriggerD = 0x04,
  LED_TriggerE = 0x05,
  LED_TriggerE_END = 0x06,
  LED_Trigger_ColorSwitch = 0x07,
  LED_Trigger_Start = 0x08,
  LED_Trigger_Stop = 0x09,
} LED_Message_t;

typedef struct _LED_Opra_Method
{
  void (*io_init)(void);
  void (*charged_loop)(uint32_t step_ms, uint32_t period_ms);
  void (*charging_loop)(uint32_t step_ms, uint32_t period_ms);
  LED_Message_t (*run_loop)(uint32_t *step, uint32_t step_ms);
  LED_Message_t (*trigger)(LED_Message_t method);
} LED_Opra_t;

// PWM LED drive
#include "PWM-LED.h"
// NEO LED drive

const LED_Opra_t *MX_LED_GetType(void);
void LED_Start_Trigger(LED_Message_t message);
#endif
