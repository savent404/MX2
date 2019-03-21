#pragma once

#include "MX_def.h"
#include <stdbool.h>
#include <stdint.h>

typedef enum _play_audio_id {
  Audio_Erro = 0x00,
  Audio_Boot = 0x01,
  Audio_PowerOff = 0x02,
  Audio_Recharge = 0x03,
  Audio_Charging = 0x04,
  Audio_ColorSwitch = 0x05,
  Audio_LowPower = 0x06,
  Audio_intoRunning = 0x07,
  Audio_intoReady = 0x08,
  Audio_TriggerB = 0x09,
  Audio_TriggerC = 0x0a,
  Audio_TriggerD = 0x0b,
  Audio_Humi = 0x0c,
  Audio_TriggerE = 0x0d,
  Audio_BankSwitch = 0x0e,
} Audio_ID_t;

MX_C_API bool MX_Audio_Play_Start(Audio_ID_t id);
MX_C_API bool MX_Audio_Play_Stop(Audio_ID_t id);
MX_C_API bool MX_Audio_isReady(void);
MX_C_API int MX_Audio_getTriggerLastTime(void);
MX_C_API int MX_Audio_getLastTriggerPos(void);
MX_C_API int MX_Audio_getLastHumPos(void);
MX_PORT_API void MX_Audio_PlayBeep(void);
