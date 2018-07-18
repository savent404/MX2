#ifndef _AUDIO_H_
#define _AUDIO_H_

#include "stm32f1xx_hal.h"
#include "FreeRTOS.h"
#include "cmsis_os.h"

#define AUDIO_FIFO_NUM  3
#define AUDIO_FIFO_SIZE 512

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

int8_t Audio_Play_Start(Audio_ID_t id);
int8_t Audio_Play_Stop(Audio_ID_t id);
uint8_t Audio_IsSimplePlayIsReady(void);

/**
 * @brief 获取音频文件的持续时间
 * @return 时间(ms)
 */
uint32_t Audio_getCurrentTriggerT();
void Audio_Play_LOOP(void);

#endif
