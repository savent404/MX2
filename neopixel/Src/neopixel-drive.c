#include "neopixel-drive.h"

static inline void neopixel_convert(const uint8_t *src, uint8_t *des, int cnt);

bool NP_Init(NP_Handle_t *pt)
{
  pt->isInited = false;

  if (pt->TransmitHandle == NULL)
  {
    return false;
  }

  if (pt->MaxPixelInDMABuffer <= 0)
  {
    return false;
  }

  if (pt->MaxPixelInDMABuffer > NP_DMA_MAX_BITS)
  {
    pt->MaxPixelInDMABuffer = NP_DMA_MAX_BITS;
  }

  pt->dma_buffer = (uint8_t *)NP_MALLOC(sizeof(uint8_t) * (pt->MaxPixelInDMABuffer) * 24 + 2);

  if (pt->dma_buffer == NULL)
  {
    return false;
  }

  pt->isRunning = false;
  pt->source = NULL;
  // pt->transmitedNumber = 0;
  // pt->transmitNumber = 0;
  pt->isInited = true;
  pt->sync = false;
  return true;
}

bool NP_DeInit(NP_Handle_t *pt)
{
  if (pt->isInited)
  {
    if (pt->isRunning)
      return false;
    if (pt->dma_buffer == NULL)
      return true;
    NP_FREE(pt->dma_buffer);
    return true;
  }
  return true;
}

bool NP_Update(NP_Handle_t *pt, const uint8_t *GRB_24bit, int size)
{
  if (!pt->isInited)
    return false;
  if (pt->isRunning)
    return false;
  if (pt->dma_buffer == NULL)
    return false;

  pt->isRunning = true;
  pt->source = GRB_24bit;

  int cnt = size;
  if (cnt > pt->MaxPixelInDMABuffer)
    cnt = pt->MaxPixelInDMABuffer;
  // pt->transmitNumber = size;
  // pt->transmitedNumber = 0;
  neopixel_convert(GRB_24bit, pt->dma_buffer, cnt);
  NP_TRANSMIT_DMA(pt->TransmitHandle, pt->dma_buffer, cnt * 24 + 2);
  return true;
}

void NP_DMA_CallbackHandle(NP_Handle_t *pt)
{
  pt->isRunning = false;
  // pt->transmitedNumber += pt->MaxPixelInDMABuffer;
  // if (pt->transmitedNumber >= pt->transmitNumber)
  // {
  //   if (pt->sync)
  //   {
  //     pt->sync = false;
  //     pt->isRunning = false;
  //   }
  //   else
  //   {
  //     pt->sync = true;
  //     NP_TRANSMIT_DMA(pt->TransmitHandle,
  //                     eof_data,
  //                     sizeof(eof_data));
  //   }
  // }
  // else
  // {
  //   int cnt = ((pt->transmitNumber - pt->transmitedNumber) > pt->MaxPixelInDMABuffer)
  //                 ? pt->MaxPixelInDMABuffer
  //                 : (pt->transmitNumber - pt->transmitedNumber);
  //   neopixel_convert(pt->source + pt->transmitedNumber * 3,
  //                    pt->dma_buffer,
  //                    cnt);
  //   NP_TRANSMIT_DMA(pt->TransmitHandle,
  //                   pt->dma_buffer,
  //                   cnt * 24);
  // }
}

static inline void neopixel_convert(const uint8_t *src, uint8_t *des, int cnt)
{
  *des++ = 0;
  for (int pixel = 0; pixel < cnt; pixel++)
  {
    uint8_t buf = src[1];
    for (int bits = 0; bits < 8; bits++)
    {
      if (buf & 0x80)
        *des++ = NP_LOGIC(1);
      else
        *des++ = NP_LOGIC(0);
      buf <<= 1;
    }
    buf = src[0];
    for (int bits = 0; bits < 8; bits++)
    {
      if (buf & 0x80)
        *des++ = NP_LOGIC(1);
      else
        *des++ = NP_LOGIC(0);
      buf <<= 1;
    }
    buf = src[2];
    for (int bits = 0; bits < 8; bits++)
    {
      if (buf & 0x80)
        *des++ = NP_LOGIC(1);
      else
        *des++ = NP_LOGIC(0);
      buf <<= 1;
    }
    src += 3;
  }
  *des = 0x00;
}
