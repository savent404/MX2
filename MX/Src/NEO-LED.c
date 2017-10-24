#include "NEO-LED.h"
/** Protocol function *************************/
static void neo_io_init(void);
static void neo_charged_loop(uint32_t step_ms, uint32_t period_ms);
static void neo_charging_loop(uint32_t step_ms, uint32_t period_ms);
static LED_Message_t neo_run_loop(uint32_t *step, uint32_t step_ms);
static LED_Message_t neo_trigger(LED_Message_t method);
/** Static var ********************************/
static SPI_HandleTypeDef *spi_handle;
static NP_Handle_t neo_io_handle;
const uint8_t reset_buffer[3 * NP_DMA_MAX_BITS] = {0};
// static NP_Handle_t neo_io_handle;
/** Static function ***************************/
static LED_Message_t neo_playfile(const char *filepath);
/** Public var ********************************/
const LED_Opra_t NEO_LED_Opra = {
    .io_init = neo_io_init,
    .charged_loop = neo_charged_loop,
    .charging_loop = neo_charging_loop,
    .run_loop = neo_run_loop,
    .trigger = neo_trigger,
};

static void neo_io_init(void)
{
  // IO init
  spi_handle = NEO_LED_LL_Init();
  neo_io_handle.TransmitHandle = spi_handle;
  neo_io_handle.MaxPixelInDMABuffer = NP_DMA_MAX_BITS;
  if (!NP_Init(&neo_io_handle))
    log_w("Init neopixel io interface error");
  NP_Update(&neo_io_handle, reset_buffer, NP_DMA_MAX_BITS);
  while (!NP_DeInit(&neo_io_handle))
  {
    osDelay(10);
  }
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
    neo_playfile("0:/demo.neo");
  }
  break;
  default:
    log_w("TODO::unknow trigger id:%d", method);
  }
  return LED_NoTrigger;
}

void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
{
  if (hspi == (SPI_HandleTypeDef *)(neo_io_handle.TransmitHandle))
  {
    NP_DMA_CallbackHandle(&neo_io_handle);
    log_v("spi dma callback");
  }
}

static LED_Message_t neo_playfile(const char *filepath)
{
  const MX_NeoPixel_Structure_t *spt = MX_File_NeoPixel_OpenFile(filepath);
  if (spt == NULL)
  {
    return LED_NoTrigger;
  }

  neo_io_handle.TransmitHandle = spi_handle;
  neo_io_handle.MaxPixelInDMABuffer = spt->frame_len;
  if (!NP_Init(&neo_io_handle))
  {
    log_w("Init neopixel io interface error");
    MX_File_NeoPixel_CloseFile((MX_NeoPixel_Structure_t *)spt);
    return LED_NoTrigger;
  }

  uint8_t *buffer = (uint8_t *)pvPortMalloc(sizeof(uint8_t *) *
                                            spt->frame_len *
                                            3);

  if (buffer == NULL)
  {
    log_e("no enough mem");
    MX_File_NeoPixel_CloseFile((MX_NeoPixel_Structure_t *)spt);
    while (!NP_DeInit(&neo_io_handle))
      osDelay(10);
    return LED_NoTrigger;
  }

  for (int i = 0; i < spt->frame_cnt; i++)
  {
    if (!MX_File_NeoPixel_GetLine(spt, i, buffer, 0))
    {
      log_w("get line error");
      MX_File_NeoPixel_CloseFile((MX_NeoPixel_Structure_t *)spt);
      spt = MX_File_NeoPixel_OpenFile(filepath);
      continue;
    }
    NP_Update(&neo_io_handle, buffer, spt->frame_len);

    osEvent evt = osMessageGet(LED_CMDHandle, 1000 / spt->Hz);
    if (evt.status == osEventMessage)
    {
      NP_Update(&neo_io_handle, reset_buffer, NP_DMA_MAX_BITS);
      vPortFree(buffer);
      MX_File_NeoPixel_CloseFile((MX_NeoPixel_Structure_t *)spt);
      while (!NP_DeInit(&neo_io_handle))
        osDelay(10);
      return evt.value.v;
  }
  }
  NP_Update(&neo_io_handle, reset_buffer, NP_DMA_MAX_BITS);
  vPortFree(buffer);
  MX_File_NeoPixel_CloseFile((MX_NeoPixel_Structure_t *)spt);
  while (!NP_DeInit(&neo_io_handle))
    osDelay(10);
  return LED_NoTrigger;
}
