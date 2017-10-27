#ifndef _MX_AUDIO_H_
#define _MX_AUDIO_H_

#include "audio.h"
#include "dac.h"
#include "mx-config.h"
#include <stdbool.h>
#include <math.h>

#ifndef AUDIO_SOFTMIX
#define AUDIO_SOFTMIX 1
#endif

void MX_Audio_Init(void);
#if AUDIO_SOFTMIX
void MX_Audio_Start(uint16_t *pt, uint8_t vol, uint32_t cnt);
#else
void MX_Audio_Start(uint16_t *pt1, uint16_t *pt2, uint8_t vol, uint32_t cnt);
#endif
void MX_Audio_HWBeep(void);
void MX_Audio_Callback(void);
void MX_Audio_Mute(bool en);
#endif
