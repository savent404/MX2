#ifndef _MX_AUDIO_H_
#define _MX_AUDIO_H_

#include "dac.h"

void MX_Audio_Init(void);
void MX_Audio_Start(uint16_t* pt, uint32_t cnt);
void MX_Audio_Callback(void);
#endif
