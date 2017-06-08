#include "Audio.h"
#include "USR_CONFIG.h"
#include "dac.h"
#include "AF.h"
#include "ff.h"
#include "DEBUG.h"
#include "main.h"
#include "path.h"
/* Variables -----------------------------------------------------------------*/
extern osThreadId defaultTaskHandle;
extern osThreadId DACTaskHandle;
extern osThreadId LEDTaskHandle;
extern osMessageQId DAC_BufferHandle;
extern osMessageQId DAC_CMDHandle;
extern osMessageQId LED_CMDHandle;
extern osSemaphoreId DAC_Complete_FlagHandle;

///循环音文件偏移量(运行态每次循环只会读取一部分音频)
__IO static UINT hum_offset = 0;
__IO static UINT trigger_offset = 0;
static char trigger_path[50];
__IO static char pri_now = 0x0F;

__IO static float audio_convert_f = 1;

static uint16_t dac_buffer[AUDIO_FIFO_NUM][AUDIO_FIFO_SIZE];
static uint16_t trigger_buffer[AUDIO_FIFO_SIZE];
__IO static uint16_t dac_buffer_pos = 0;
/* Function prototypes -------------------------------------------------------*/
static void Play_simple_wav(char *filepath);
static void Play_IN_wav(void);
static void Play_OUT_wav(void);
static void Play_Trigger_wav(uint8_t);
static void Play_TriggerE(void);
static void Play_TriggerE_END(void);
static void Play_RunningLOOP(void);
static void Play_RunningLOOPwithTrigger(char *triggerpath, uint8_t pri);
static void play_a_buffer(uint16_t*);

__STATIC_INLINE void pcm_convert(int16_t*);
__STATIC_INLINE void pcm_convert2(int16_t*, int16_t*);



int8_t Audio_Play_Start(Audio_ID_t id)
{
  if (!USR.mute_flag && USR.config->Vol != 0) {
    DEBUG(4, "[Message] Put AudioID:%02x", id);
    while (osMessagePut(DAC_CMDHandle, id, osWaitForever) != osOK);
  }
  return 0;
}

int8_t Audio_Play_Stop(Audio_ID_t id)
{
  if (!USR.mute_flag) {
    DEBUG(4, "[Message] Put AudioID:%02x", id);
    while (osMessagePut(DAC_CMDHandle, id | 0x80, osWaitForever) != osOK);
  }
  return 0;
}

void DACOutput(void const * argument)
{
  while (1) {
    osEvent evt = osMessageGet(DAC_BufferHandle, osWaitForever);
    if (evt.status != osEventMessage) continue;
    if (evt.value.v == 0) continue;
    osSemaphoreWait(DAC_Complete_FlagHandle, osWaitForever);
    HAL_DAC_Start_DMA(&hdac, DAC_CHANNEL_1, (uint32_t*)evt.value.p, AUDIO_FIFO_SIZE, DAC_ALIGN_12B_R);
  }
}


void Wav_Task(void const * argument)
{

  while (1)
  {
    osEvent  evt;
    /**< 当不在运行态时音频的播放使用简单模式 */
  	if (USR.sys_status != System_Running) {

	    evt = osMessageGet(DAC_CMDHandle, osWaitForever);

	    if (evt.status != osEventMessage) continue;

    /**< 由于运行态会产生两段音频同时播放的情况，特殊处理 */
   	} else {

			evt = osMessageGet(DAC_CMDHandle, 5);

			if (evt.status != osEventMessage) {

        /**< 未接到有效消息，播放背景音后结束 */
        Play_RunningLOOP();

        continue;
			}
  	}

    if (USR.sys_status == System_Restart || USR.sys_status == System_Ready)
    {
      switch(evt.value.v)
      {
        case Audio_Boot:
					Play_simple_wav(WAV_BOOT);
					break;
        case Audio_Erro:
					Play_simple_wav(WAV_ERROR);
        	break;
        case Audio_LowPower:
					Play_simple_wav(WAV_LOWPOWER);
					break;
        case Audio_intoReady:
					Play_IN_wav();
					break;
        case Audio_BankSwitch:
					{ char path[25];
            sprintf(path, "0:/Bank%d/BankSwitch.wav", USR.bank_now + 1);
            Play_simple_wav(path);}
					break;
      }
    }
    else if (USR.sys_status == System_Charged)
    {

    }
    else if (USR.sys_status == System_Charging)
    {
      switch(evt.value.v)
      {
        case Audio_Charging:
					Play_simple_wav(WAV_CHARGING);
					break;
      }
    }
    else if (USR.sys_status == System_Running)
    {
      switch(evt.value.v)
      {
        case Audio_TriggerB:
					Play_Trigger_wav(0);
					break;
        case Audio_TriggerC:
					Play_Trigger_wav(1);
					break;
        case Audio_TriggerD:
					Play_Trigger_wav(2);
					break;
        case Audio_TriggerE:
					Play_TriggerE();
					break;
        case Audio_TriggerE|0x80:
					Play_TriggerE_END();
					break;
        case Audio_ColorSwitch:
					Play_Trigger_wav(3);
					break;
				case Audio_intoRunning:
				  pri_now = 0x0F;
					Play_OUT_wav();
					break;

      }
    }
    else if (USR.sys_status == System_Close)
    {
      switch(evt.value.v) {
        case Audio_PowerOff:
					Play_simple_wav(WAV_POWEROFF);
					break;
        case Audio_Erro:
					Play_simple_wav(WAV_ERROR);
					break;
      }
    }
  }
}

/**
 * @Brief Play some simple wave file like:boot.wav
 * @Para filepath file's path
 */
static void Play_simple_wav(char *filepath)
{
  FIL file;
  FRESULT f_err;
  UINT f_cnt;
  struct _AF_DATA data;
  taskENTER_CRITICAL();
  if((f_err = f_open(&file, filepath, FA_READ)) != FR_OK)
  {
    DEBUG(1, "Open wave file:%s Error:%d", filepath, f_err);
    taskEXIT_CRITICAL();
    return;
  }

  // Ignore about pcm structure
  f_lseek(&file, sizeof(struct _AF_PCM));
  // Read about this file's length
  if ((f_err = f_read(&file, &data, sizeof(struct _AF_DATA), &f_cnt)) != FR_OK)
  {
    DEBUG(1, "Read wave file:%s Error:%d", filepath, f_err);
    f_close(&file);
    taskEXIT_CRITICAL();
    return;
  }
  taskEXIT_CRITICAL();

  while (data.size >= AUDIO_FIFO_SIZE*sizeof(uint16_t))
  {
    taskENTER_CRITICAL();
    /**< Read a Block */
    if ((f_err = f_read(&file, dac_buffer[dac_buffer_pos], sizeof(uint16_t)*AUDIO_FIFO_SIZE, &f_cnt)) != FR_OK && f_cnt != 0)
    {
      DEBUG(1, "Read wave file:%s Error:%d", filepath, f_err);
      f_close(&file);
      return;
    }
    taskEXIT_CRITICAL();
    pcm_convert((int16_t*)dac_buffer[dac_buffer_pos]);

    play_a_buffer(dac_buffer[dac_buffer_pos]);

    dac_buffer_pos += 1;
    dac_buffer_pos %= AUDIO_FIFO_NUM;
    data.size -= f_cnt;
  }
  f_close(&file);
}

/**
 * @Brief  在循环音下插入触发
 * @Para triggerid 0~2 TriggerB~TriggerD, 3-Colorswitch
 */
static void Play_Trigger_wav(uint8_t triggerid)
{

  char path[50];

  if (triggerid < 3)
  {
    uint8_t cnt;
    uint8_t num;
    uint8_t pri;
    switch (triggerid)
    {
      case 0: cnt = (USR.triggerB + USR.bank_now)->number; pri = 3;break;
      case 1: cnt = (USR.triggerC + USR.bank_now)->number; pri = 2;break;
      case 2: cnt = (USR.triggerD + USR.bank_now)->number; pri = 1;break;
    } num = HAL_GetTick() % cnt;
    //sprintf(path, "0:/Bank%d/Trigger_%c/", USR.bank_now + 1, triggerid + 'B');
    switch (triggerid) {
      case 0: sprintf(path, "0:/Bank%d/"TRIGGER(B)"/", USR.bank_now + 1); break;
      case 1: sprintf(path, "0:/Bank%d/"TRIGGER(C)"/", USR.bank_now + 1); break;
      case 2: sprintf(path, "0:/Bank%d/"TRIGGER(D)"/", USR.bank_now + 1); break;
    }
    switch (triggerid)
    {
      case 0: strcat(path, (USR.triggerB + USR.bank_now)->path_arry + 30*num); break;
      case 1: strcat(path, (USR.triggerC + USR.bank_now)->path_arry + 30*num); break;
      case 2: strcat(path, (USR.triggerD + USR.bank_now)->path_arry + 30*num); break;
    }
    Play_RunningLOOPwithTrigger(path, pri);
  }
  else if (triggerid == 3)
  {
    // Change Colorswitch.wav as mixture mode, pri:4
    // Play_simple_wav(WAV_COLORSWITCH);
    Play_RunningLOOPwithTrigger(WAV_COLORSWITCH, 4);
  }
}
static void Play_TriggerE(void)
{
  char path[50];
  uint8_t cnt = (USR.triggerE + USR.bank_now)->number;
  uint8_t pri = 0;
  uint8_t num = HAL_GetTick() % cnt;
  sprintf(path, "0:/Bank%d/"TRIGGER(E)"/", USR.bank_now + 1);
  strcat(path, (USR.triggerE + USR.bank_now)->path_arry + 30*num);
  Play_RunningLOOPwithTrigger(path, pri);

}
static void Play_TriggerE_END(void)
{
  trigger_path[0] = 0;
  trigger_offset = 0;
  pri_now = 0x0F;
}
static void Play_IN_wav(void)
{
  uint8_t cnt = (USR.triggerIn + USR.bank_now)->number;
  uint8_t num = HAL_GetTick() % cnt;
  char path[50];

  sprintf(path, "0:/Bank%d/"TRIGGER(IN)"/", USR.bank_now+1);
  strcat(path, (USR.triggerIn + USR.bank_now)->path_arry + 30*num);

  Play_simple_wav(path);
}
static void Play_OUT_wav(void)
{
  uint8_t cnt = (USR.triggerOut + USR.bank_now)->number;
  uint8_t num = HAL_GetTick() % cnt;
  char path[50];

  sprintf(path, "0:/Bank%d/"TRIGGER(OUT)"/", USR.bank_now+1);
  strcat(path, (USR.triggerOut + USR.bank_now)->path_arry + 30*num);

  Play_simple_wav(path);
}
static void Play_RunningLOOP(void)
{
  FIL file;
  char path[30];
  FRESULT f_err;
  UINT f_cnt;

  sprintf(path, "0:/Bank%d/hum.wav", USR.bank_now + 1);

  if (hum_offset + AUDIO_FIFO_SIZE*sizeof(uint16_t) > USR.humsize[USR.bank_now])
  {
    hum_offset = 0;
  }

  taskENTER_CRITICAL();
  if ((f_err = f_open(&file, path, FA_READ)) != FR_OK)
  {
    DEBUG(0, "[Hum.wav] can't open[%s]:%d", path, f_err);
    taskEXIT_CRITICAL();
    return;
  }
  if ((f_err = f_lseek(&file, sizeof(struct _AF_PCM) + sizeof(struct _AF_PCM) + hum_offset)) != FR_OK)
  {
    DEBUG(0, "lseek hum.wav Error:%d", f_err);
    f_close(&file);
    taskEXIT_CRITICAL();
    return;
  }
  if ((f_err = f_read(&file, dac_buffer[dac_buffer_pos], AUDIO_FIFO_SIZE*sizeof(uint16_t), &f_cnt)) != FR_OK && f_cnt != 0)
  {
    DEBUG(0, "Read hum.wave Error:%d", f_err);
    f_close(&file);
    taskEXIT_CRITICAL();
    return;
  }
  f_close(&file);
  if (pri_now < 0x0F)
  {
    if ((f_err = f_open(&file, trigger_path, FA_READ)) != FR_OK)
    {
      DEBUG(0, "trigger:%s can't open:%d", path, f_err);
      taskEXIT_CRITICAL();
      return;
    }
    if (file.fsize < sizeof(struct _AF_PCM) + sizeof(struct _AF_PCM) + trigger_offset + AUDIO_FIFO_SIZE*sizeof(uint16_t))
    {
      if (pri_now) {
        trigger_path[0] = '\0';
        trigger_offset = 0;
        pri_now = 0x0F;
        f_close(&file);
        goto outoftrigger;
      } else { //TriggerE
        trigger_offset = 0;
      }
    }

    if ((f_err = f_lseek(&file, sizeof(struct _AF_PCM) + sizeof(struct _AF_PCM) + trigger_offset)) != FR_OK && f_cnt != 0)
    {
      DEBUG(1, "lseek trigger:%s Error:%d", trigger_path, f_err);
      f_close(&file);
      taskEXIT_CRITICAL();
      return;
    }
    if ((f_err = f_read(&file, trigger_buffer, AUDIO_FIFO_SIZE*sizeof(uint16_t), &f_cnt)) != FR_OK && f_cnt != 0)
    {
      DEBUG(1, "Read tirrger:%s Error:%d", trigger_path, f_err);
      f_close(&file);
      taskEXIT_CRITICAL();
      return;
    }
    f_close(&file);
    trigger_offset += AUDIO_FIFO_SIZE*sizeof(uint16_t);
  }

outoftrigger:
  taskEXIT_CRITICAL();
  if (pri_now == 0x0F)
    pcm_convert((int16_t*)dac_buffer[dac_buffer_pos]);
  else {
    pcm_convert2((int16_t*)dac_buffer[dac_buffer_pos], (int16_t*)trigger_buffer);
  }
  play_a_buffer(dac_buffer[dac_buffer_pos]);
  dac_buffer_pos += 1;
  dac_buffer_pos %= AUDIO_FIFO_NUM;
  hum_offset += f_cnt;
}
static void Play_RunningLOOPwithTrigger(char *triggerpath, uint8_t pri)
{
  if (pri_now < pri) return;
  else {
    strcpy(trigger_path, triggerpath);
    pri_now = pri;
    trigger_offset = 0;
  }
}
__STATIC_INLINE void pcm_convert(int16_t* _pt)
{
  int16_t *pt = (int16_t*)_pt;
  uint8_t offset = 4 + 3 - USR.config->Vol;
  if (USR.config->Vol == 0) offset = 15;
  for (uint32_t i = 0; i < AUDIO_FIFO_SIZE; i++)
  {
    if (*pt > INT16_MAX / 2) {
        audio_convert_f = (float)INT16_MAX / 2 / (float) *pt;
        *pt = INT16_MAX / 2;
    } *pt *= audio_convert_f;
    if (*pt < INT16_MIN / 2) {
        audio_convert_f = (float)INT16_MIN / 2 / (float) *pt;
        *pt = INT16_MIN / 2;
    }
    if (audio_convert_f < 1)
    {
      audio_convert_f += ((float)1 - audio_convert_f) / (float) 32;
    }
    *pt = (*pt >> offset) + 0x1000/2;
    pt += 1;
  }
}
__STATIC_INLINE void pcm_convert2(int16_t* pt1, int16_t* pt2)
{
  uint8_t offset = 4 + 3 - USR.config->Vol;
  int16_t *p1 = (int16_t*)pt1, *p2 = (int16_t*)pt2;
  static float f = 1;

  if (USR.config->Vol == 0) offset = 15;
  for (uint32_t i = 0; i < AUDIO_FIFO_SIZE; i++)
  {
    int32_t buf = *p1 + *p2;
    if (buf > INT16_MAX / 2) {
        audio_convert_f = (float)INT16_MAX / 2 / (float)buf;
        buf = INT16_MAX / 2;
    } buf *= audio_convert_f;
    if (buf < INT16_MIN / 2) {
        audio_convert_f = (float)INT16_MIN /2 / (float)buf;
        buf = INT16_MIN / 2;
    }
    if (audio_convert_f < 1) {
      audio_convert_f += ((float)1 - f) / (float)32;
    }

    *p1 = (buf >> offset) + 0x1000/2;

    p1 += 1, p2 += 1;
  }
}


/**
 * @Brief Play a buffer
 */
static void play_a_buffer(uint16_t* pt)
{
  USR.audio_busy = 1;
  while (osMessagePut(DAC_BufferHandle, (uint32_t)pt, osWaitForever) != osOK);
}

void HAL_DAC_ConvCpltCallbackCh1(DAC_HandleTypeDef* hdac)
{
  osSemaphoreRelease(DAC_Complete_FlagHandle);
  USR.audio_busy = 0;
}
