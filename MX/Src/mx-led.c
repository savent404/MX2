#include "mx-led.h"

void LED_Start_Trigger(LED_Message_t message)
{
  log_v("LED send Message ID:%d", message);
  osMessagePut(LED_CMDHandle, message, osWaitForever);
}

void LEDOpra(void const *argument)
{
  LED_Opra_t *opra = MX_LED_GetType();
  enum _led_message message;

  uint32_t loop_step = 0;

  opra->io_init();

  while (1)
  {
    osEvent evt = osMessageGet(LED_CMDHandle, 20);

    message = evt.value.v;

  GETMESSAGE:
    switch (message)
    {
    case LED_NoTrigger:
      break;

    case LED_Trigger_Start:
    case LED_Trigger_Stop:
    case LED_TriggerB:
    case LED_TriggerC:
    case LED_TriggerD:
    case LED_TriggerE:
    case LED_TriggerE_END:
      if ((message = opra->trigger(message) != LED_NoTrigger))
        goto GETMESSAGE;
      break;

    default:
      log_w("led get unknow message id:%d", (int)message);
      break;
    }
    if (USR.sys_status == System_Running)
    {
      while ((message = opra->run_loop(&loop_step, 30)) == LED_NoTrigger)
        ;
      goto GETMESSAGE;
    }
    else if (USR.sys_status == System_Charging)
    {
      opra->charging_loop(20, 5000);
    }
    else if (USR.sys_status == System_Charged)
    {
      opra->charged_loop(20, 5000);
    }
  }
}
