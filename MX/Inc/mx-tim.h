#ifndef _MX_TIM_H_
#define _MX_TIM_H_

#include "mx-config.h"
#include "tim.h"

void MX_TIM_PowerLEDStart(void);
uint16_t MX_TIM_PowerLEDRead(uint8_t ch);
void MX_TIM_PowerLEDOpra(uint8_t ch, uint16_t max_1024);
#endif
