#include "neopixel-drive.h"


static inline void neopixel_convert(uint8_t *src, uint8_t *des, int cnt);

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

  pt->dma_buffer = (uint8_t*)NP_MALLOC(sizeof(uint8_t)*(pt->MaxPixelInDMABuffer)*24);

  if (pt->dma_buffer == NULL)
  {
    return false;
  }

  pt->isRunning = false;
  pt->source = NULL;
  pt->transmitedNumber = 0;
  pt->transmitNumber = 0;
  pt->isInited = true;
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

bool NP_Update(NP_Handle_t *pt, uint8_t *GRB_24bit, int size)
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
  pt->transmitNumber = size;
  pt->transmitedNumber = 0;
  neopixel_convert(GRB_24bit, pt->dma_buffer, cnt);
  NP_TRANSMIT_DMA(pt->TransmitHandle, pt->dma_buffer, cnt * 24);
  return true;
}

void NP_DMA_CallbackHandle(NP_Handle_t *pt)
{
  pt->transmitedNumber += pt->MaxPixelInDMABuffer;
  if (pt->transmitedNumber >= pt->transmitNumber)
  {
    pt->isRunning = false;
  }
  else
  {
    int cnt = (pt->transmitNumber - pt->transmitedNumber) % (pt->MaxPixelInDMABuffer + 1);
    neopixel_convert(pt->source + pt->transmitedNumber*3,
                     pt->dma_buffer,
                     cnt);
    NP_TRANSMIT_DMA(pt->TransmitHandle,
                    pt->dma_buffer,
                    cnt * 24);
  }
}

static inline void neopixel_convert(uint8_t *src, uint8_t *des, int cnt)
{
  for(int pixel = 0; pixel < cnt; pixel++)
  {
    for(int byte = 0; byte < 3; byte++)
    {
      uint8_t buff = *src++;
      for (int bits = 0; bits < 8; bits++)
      {
        if (buff & 0x80)
          *des++ = NP_LOGIC(1);
        else
          *des++ = NP_LOGIC(0);
        buff <<= 1;
      }
    }
  }
}
