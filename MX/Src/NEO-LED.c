#include "NEO-LED.h"
/** Protocol function *************************/
static void neo_io_init(void);
static void neo_charged_loop(uint32_t step_ms, uint32_t period_ms);
static void neo_charging_loop(uint32_t step_ms, uint32_t period_ms);
static LED_Message_t neo_run_loop(uint32_t *step, uint32_t step_ms);
static LED_Message_t neo_trigger(LED_Message_t method);

/** Static function ***************************/

/** Public var ********************************/
const LED_Opra_t NEO_LED_Opra = {
    .io_init = neo_io_init,
    .charged_loop = neo_charged_loop,
    .charging_loop = neo_charging_loop,
    .run_loop = neo_run_loop,
    .trigger = neo_trigger,
};
/** Static var ********************************/
static NP_Handle_t neo_io_handle;

static void neo_io_init(void)
{
  // IO init
  neo_io_handle.TransmitHandle = 0;
  neo_io_handle.MaxPixelInDMABuffer = 30;
  if (!NP_Init(&neo_io_handle))
    log_w("Init neopixel io interface error");
}

static void neo_charged_loop(uint32_t step_ms, uint32_t period_ms)
{
  log_w("TODO::charged trigger rised");
  while (1)
    osDelay(100);
}
static void neo_charging_loop(uint32_t step_ms, uint32_t period_ms)
{
  log_w("TODO::charging trigger rised");
  while (1)
    osDelay(100);
}
static LED_Message_t neo_run_loop(uint32_t *step, uint32_t step_ms)
{
  log_w("TODO::run_loop trigger rised");
  while (1)
    osDelay(100);
  return LED_NoTrigger;
}
static LED_Message_t neo_trigger(LED_Message_t method)
{
  switch (method)
  {
  case LED_Trigger_Start:
  {
    const char path[] = "0:/demo.neo";
    const MX_NeoPixel_Structure_t *spt;
    uint8_t *buffer;
    spt = MX_File_NeoPixel_OpenFile(path);
    if (spt == NULL)
    {
      return LED_NoTrigger;
    }

    buffer = (uint8_t *)pvPortMalloc(sizeof(char) *
                                     spt->frame_len *
                                     3);
    if (buffer == NULL)
    {
      log_w("no enough mem");
      MX_File_NeoPixel_CloseFile((MX_NeoPixel_Structure_t *)spt);
      return LED_NoTrigger;
    }
    for (int i = 0; i < spt->frame_cnt; i++)
    {
      if (!MX_File_NeoPixel_GetLine(spt, i, buffer, 0))
      {
        log_w("get line error");
        MX_File_NeoPixel_CloseFile((MX_NeoPixel_Structure_t *)spt);
        spt = MX_File_NeoPixel_OpenFile(path);
        continue;
      }
      log_v("output");
      osDelay(1000 / spt->Hz);
    }
    vPortFree(buffer);
    MX_File_NeoPixel_CloseFile((MX_NeoPixel_Structure_t *)spt);
  }
  break;
  default:
    log_w("TODO::unknow trigger id:%d", method);
  }
  return LED_NoTrigger;
}
