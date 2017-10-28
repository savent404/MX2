#ifndef _AUDIO_H_
#define _AUDIO_H_

#ifndef LOG_TAG
#define LOG_TAG "Audio"
#endif

#include "AF.h"
#include "DEBUG.h"
#include "FreeRTOS.h"
#include "MX_osID.h"
#include "USR_CONFIG.h"
#include "cmsis_os.h"
#include "ff.h"
#include "main.h"
#include "mx-audio.h"
#include "mx-config.h"
#include "path.h"

#if AUDIO_SOFTMIX
#undef AUDIO_TRACK_NUM
#define AUDIO_TRACK_NUM 1
#else
#undef AUDIO_TRACK_NUM
#define AUDIO_TRACK_NUM 2
#endif

#ifndef AUDIO_FIFO_NUM
#define AUDIO_FIFO_NUM 3
#endif

#ifndef AUDIO_FIFO_SIZE
#define AUDIO_FIFO_SIZE 512
#endif
typedef enum _track {
  Track_0 = 0,
  Track_1 = 1,
} Audio_Track_t;

typedef enum _play_audio_id {
  Audio_Erro = 0x00,
  Audio_Boot = 0x01,
  Audio_PowerOff = 0x02,
  Audio_Recharge = 0x03,
  Audio_Charging = 0x04,
  Audio_ColorSwitch = 0x05,
  Audio_LowPower = 0x06,
  Audio_intoRunning = 0x07,
  Audio_intoRunning_X = 0x08,
  Audio_intoRunning_Y = 0x09,
  Audio_intoRunning_Z = 0x0A,
  Audio_intoReady = 0x0B,
  Audio_intoReady_X = 0x0D,
  Audio_intoReady_Y = 0x0E,
  Audio_intoReady_Z = 0x0F,
  Audio_TriggerB = 0x10,
  Audio_TriggerC = 0x11,
  Audio_TriggerD = 0x12,
  Audio_Humi = 0x13,
  Audio_TriggerE = 0x14,
  Audio_BankSwitch = 0x15,
} Audio_ID_t;

#define PRI_TRIGGER_B 3
#define PRI_TRIGGER_C 2
#define PRI_TRIGGER_D 4
#define PRI_TRIGGER_E 1
#define PRI_TRIGGER_NULL 0x0F
#define PRI_TRIGGER_COLORSWITCH 5
#define PRI(x) PRI_TRIGGER_##x

int8_t Audio_Play_Start(Audio_ID_t id);
int8_t Audio_Play_Stop(Audio_ID_t id);
uint8_t Audio_IsSimplePlayIsReady(void);
bool Audio_IsPlayBankSwitch(void);
void Audio_Play_LOOP(void);

#endif
