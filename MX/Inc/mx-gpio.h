#ifndef _MX_GPIO_H_
#define _MX_GPIO_H_

#include "gpio.h"
#include <stdbool.h>
typedef enum
{
  KEY_MUX,
  KEY_USR
} MX_KEY_t;

#define AFIO_RELEASE() __HAL_AFIO_REMAP_SWJ_NONJTRST()

bool MX_GPIO_IsPress(MX_KEY_t key);
bool MX_GPIO_PlugIn(void);
void MX_GPIO_Enable(bool is);
void MX_GPIO_Lis3DCSEnable(bool);

void SimpleLED_Init(void);
void SimpleLED_DeInit(void);
void SimpleLED_Opra(uint8_t led);
#endif
