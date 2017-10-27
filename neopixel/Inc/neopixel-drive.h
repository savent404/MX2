#ifndef _NEOPIXEL_H_
#define _NEOPIXEL_H_

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include "spi.h"
#include "freeRTOS.h"

#ifndef NP_MALLOC
#define NP_MALLOC pvPortMalloc
#endif

#ifndef NP_FREE
#define NP_FREE   vPortFree
#endif

#ifndef NP_DMA_MAX_BITS
#define NP_DMA_MAX_BITS (7)
#endif

#ifndef NP_LOGIC_1
#define NP_LOGIC_1 (0xFC)
#endif

#ifndef NP_LOGIC_0
#define NP_LOGIC_0 (0xC0)
#endif

#ifndef NP_TRANSMIT_DMA
#define NP_TRANSMIT_DMA(pvHandle, pu8Data, size) \
HAL_SPI_Transmit_DMA((SPI_HandleTypeDef*)(pvHandle), (uint8_t*)(pu8Data), (size))
#endif

#define NP_LOGIC(x) (NP_LOGIC_##x)

typedef struct _neopixel_ctl_structure
{
  // Config paramter
  void *TransmitHandle;
  int MaxPixelInDMABuffer;

  // status
  bool isRunning;
  bool isInited;
  // private var
  uint8_t *dma_buffer;
  const uint8_t *source;
  // int transmitedNumber;
  // int transmitNumber;
  bool sync;
} NP_Handle_t;

bool NP_Init(NP_Handle_t *pt);
bool NP_DeInit(NP_Handle_t *pt);
bool NP_Update(NP_Handle_t *pt, const uint8_t *GRB_24bit, int size);
void NP_DMA_CallbackHandle(NP_Handle_t *pt);

#endif
