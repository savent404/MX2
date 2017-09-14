#include "Audio.h"

/** #001 说明  ：为提升音质、消除循环中由于重复的open/close操作造成的间断感--------*/
/**
 [1] 为避免运行态时重复open/close以及seek音频文件，添加静态文件变量"file_1"、"file_2"
     来保存文件信息避免重复的open/close操作
 [2] 由于运行态实行中断机制，为保证被中断的音频文件关闭，需在Play_RunningLOOPwithTrigger
     中添加中断发生时关闭文件的操作
 [3] read_a_buffer通过当前文件偏移量seek来判断是否open/close文件
     当seek为0时代表需要执行open操作
     当seek到文件结尾时代表需要执行close操作
 [4] 为保证从运行态到待机态再到运行态时变量处于初始化状态，从运行态到待机态的时候关闭
     "file_1"以及"file_2"，将hum以及trigger的偏移量置0
 */
/* Variables -----------------------------------------------------------------*/
static uint8_t SIMPLE_PLAY_READY = 1;
///循环音文件偏移量(运行态每次循环只会读取一部分音频)
static UINT hum_offset = 0;
static UINT trigger_offset = 0;
static char trigger_path[50];
__IO static char pri_now = PRI(NULL);

__IO static float audio_convert_f = 1;

static uint16_t dac_buffer[AUDIO_FIFO_NUM][AUDIO_FIFO_SIZE];
static uint16_t trigger_buffer[AUDIO_FIFO_SIZE];
__IO static uint16_t dac_buffer_pos = 0;
// 详见 #001 [1]
static FIL file_1, file_2;
/* Function prototypes -------------------------------------------------------*/
static void Play_simple_wav(char *filepath);
static void Play_IN_wav(void);
static void Play_OUT_wav(void);
static void Play_Trigger_wav(uint8_t);
static void Play_TriggerE(void);
static void Play_TriggerE_END(void);
static void Play_RunningLOOP(void);
static void Play_RunningLOOPwithTrigger(char *triggerpath, uint8_t pri);
static void play_a_buffer(uint16_t *);
__STATIC_INLINE FRESULT read_a_buffer(FIL *fpt, const TCHAR *path, void *buffer, UINT *seek);
__STATIC_INLINE void AUDIO_SoftMUX(int16_t *, int16_t *);
#define convert_filesize2MS(size) (size / 22 / sizeof(uint16_t))
#define convert_ms2filesize(ms) (ms * sizeof(uint16_t) * 22)
#define RESET_Buffer() \
  f_close(&file_1);    \
  f_close(&file_2);    \
  hum_offset = 0;      \
  trigger_offset = 0;
int8_t Audio_Play_Start(Audio_ID_t id)
{
  if (!USR.mute_flag && USR.config->Vol != 0)
  {
    DEBUG(4, "[Message] Put AudioID:%02x", id);
    while (osMessagePut(DAC_CMDHandle, id, osWaitForever) != osOK)
      ;
  }
  return 0;
}

int8_t Audio_Play_Stop(Audio_ID_t id)
{
  if (!USR.mute_flag)
  {
    DEBUG(4, "[Message] Put AudioID:%02x", id);
    while (osMessagePut(DAC_CMDHandle, id | 0x80, osWaitForever) != osOK)
      ;
  }
  return 0;
}

void DACOutput(void const *argument)
{
  while (1)
  {
    osEvent evt = osMessageGet(DAC_BufferHandle, osWaitForever);
    if (evt.status != osEventMessage)
      continue;
    if (evt.value.v == 0)
      continue;
    osSemaphoreWait(DAC_Complete_FlagHandle, osWaitForever);
    
    MX_Audio_Start((uint16_t*)evt.value.p, USR.config->Vol, AUDIO_FIFO_SIZE);
  }
}

void Wav_Task(void const *argument)
{

  while (1)
  {
    osEvent evt;
    /**< 当不在运行态时音频的播放使用简单模式 */
    if (USR.sys_status != System_Running)
    {

      evt = osMessageGet(DAC_CMDHandle, osWaitForever);

      if (evt.status != osEventMessage)
        continue;

      /**< 由于运行态会产生两段音频同时播放的情况，特殊处理 */
    }
    else
    {

      evt = osMessageGet(DAC_CMDHandle, 5);

      if (evt.status != osEventMessage)
      {

        /**< 未接到有效消息，播放背景音后结束 */
        Play_RunningLOOP();

        continue;
      }
    }

    if (USR.sys_status == System_Restart || USR.sys_status == System_Ready)
    {
      switch (evt.value.v)
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
        pri_now = PRI(NULL);
        RESET_Buffer();
        Play_IN_wav();
        break;
      case Audio_BankSwitch:
      {
        char path[25];
        sprintf(path, "0:/Bank%d/BankSwitch.wav", USR.bank_now + 1);
        Play_simple_wav(path);
      }
      break;
      }
    }
    else if (USR.sys_status == System_Charged)
    {
    }
    else if (USR.sys_status == System_Charging)
    {
      switch (evt.value.v)
      {
      case Audio_Charging:
        Play_simple_wav(WAV_CHARGING);
        break;
      }
    }
    else if (USR.sys_status == System_Running)
    {
      switch (evt.value.v)
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
      case Audio_TriggerE | 0x80:
        Play_TriggerE_END();
        break;
      case Audio_ColorSwitch:
        Play_Trigger_wav(3);
        break;
      case Audio_intoRunning:
        pri_now = PRI(NULL);
        // 详见 #001 [4]
        RESET_Buffer();
        Play_OUT_wav();
        break;
      }
    }
    else if (USR.sys_status == System_Close)
    {
      RESET_Buffer();
      switch (evt.value.v)
      {
      case Audio_PowerOff:
        Play_simple_wav(WAV_POWEROFF);
        break;
      case Audio_Erro:
        Play_simple_wav(WAV_ERROR);
        break;
      case Audio_Recharge:
        Play_simple_wav(WAV_RECHARGE);
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
  SIMPLE_PLAY_READY = 0;
  if ((f_err = f_open(&file, filepath, FA_READ)) != FR_OK)
  {
    DEBUG(1, "Open wave file:%s Error:%d", filepath, f_err);
    SIMPLE_PLAY_READY = 1;
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
    SIMPLE_PLAY_READY = 1;
    taskEXIT_CRITICAL();
    return;
  }
  taskEXIT_CRITICAL();

  while (data.size >= AUDIO_FIFO_SIZE * sizeof(uint16_t))
  {
    taskENTER_CRITICAL();
    /**< Read a Block */
    if ((f_err = f_read(&file, dac_buffer[dac_buffer_pos], sizeof(uint16_t) * AUDIO_FIFO_SIZE, &f_cnt)) != FR_OK && f_cnt != 0)
    {
      DEBUG(1, "Read wave file:%s Error:%d", filepath, f_err);
      f_close(&file);
      SIMPLE_PLAY_READY = 1;
      return;
    }
    taskEXIT_CRITICAL();

    play_a_buffer(dac_buffer[dac_buffer_pos]);

    dac_buffer_pos += 1;
    dac_buffer_pos %= AUDIO_FIFO_NUM;
    data.size -= f_cnt;
  }
  f_close(&file);
  SIMPLE_PLAY_READY = 1;
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
    case 0:
      cnt = (USR.triggerB + USR.bank_now)->number;
      pri = PRI(B);
      break;
    case 1:
      cnt = (USR.triggerC + USR.bank_now)->number;
      pri = PRI(C);
      break;
    case 2:
      cnt = (USR.triggerD + USR.bank_now)->number;
      pri = PRI(D);
      break;
    }
    num = HAL_GetTick() % cnt;
    //sprintf(path, "0:/Bank%d/Trigger_%c/", USR.bank_now + 1, triggerid + 'B');
    switch (triggerid)
    {
    case 0:
      sprintf(path, "0:/Bank%d/" TRIGGER(B) "/", USR.bank_now + 1);
      break;
    case 1:
      sprintf(path, "0:/Bank%d/" TRIGGER(C) "/", USR.bank_now + 1);
      break;
    case 2:
      sprintf(path, "0:/Bank%d/" TRIGGER(D) "/", USR.bank_now + 1);
      break;
    }
    switch (triggerid)
    {
    case 0:
      strcat(path, (USR.triggerB + USR.bank_now)->path_ptr[num]);
      break;
    case 1:
      strcat(path, (USR.triggerC + USR.bank_now)->path_ptr[num]);
      break;
    case 2:
      strcat(path, (USR.triggerD + USR.bank_now)->path_ptr[num]);
      break;
    }
    Play_RunningLOOPwithTrigger(path, pri);
  }
  else if (triggerid == 3)
  {
    // Change Colorswitch.wav as mixture mode, pri:0
    // Play_simple_wav(WAV_COLORSWITCH);
    Play_RunningLOOPwithTrigger(WAV_COLORSWITCH, PRI(COLORSWITCH));
  }
}
static void Play_TriggerE(void)
{
  char path[50];
  uint8_t cnt = (USR.triggerE + USR.bank_now)->number;
  uint8_t pri = PRI(E);
  uint8_t num = HAL_GetTick() % cnt;
  sprintf(path, "0:/Bank%d/" TRIGGER(E) "/", USR.bank_now + 1);
  strcat(path, (USR.triggerE + USR.bank_now)->path_arry + 30 * num);
  Play_RunningLOOPwithTrigger(path, pri);
}
static void Play_TriggerE_END(void)
{
  f_close(&file_2);
  trigger_path[0] = 0;
  trigger_offset = 0;
  pri_now = PRI(NULL);
}
static void Play_IN_wav(void)
{
  uint8_t cnt = (USR.triggerIn + USR.bank_now)->number;
  uint8_t num = HAL_GetTick() % cnt;
  char path[50];

  sprintf(path, "0:/Bank%d/" TRIGGER(IN) "/", USR.bank_now + 1);
  strcat(path, (USR.triggerIn + USR.bank_now)->path_ptr[num]);

  Play_simple_wav(path);
}
static void Play_OUT_wav(void)
{
  uint8_t cnt = (USR.triggerOut + USR.bank_now)->number;
  uint8_t num = HAL_GetTick() % cnt;
  char path[50];
  char hum_path[50];

  UINT point = convert_ms2filesize(USR.config->Out_Delay);
  sprintf(hum_path, "0:/Bank%d/hum.wav", USR.bank_now + 1);
  sprintf(path, "0:/Bank%d/" TRIGGER(OUT) "/", USR.bank_now + 1);
  strcat(path, (USR.triggerOut + USR.bank_now)->path_ptr[num]);

  // Play_simple_wav(path);
  while (1)
  {
    // 读取Out
    if (read_a_buffer(&file_2, path, dac_buffer[dac_buffer_pos], &trigger_offset) != FR_OK)
      continue;
    // 达到Out_Delay延时读取hum.wav
    if (trigger_offset >= point)
    {
    read_hum_again_1:
      if (read_a_buffer(&file_1, hum_path, trigger_buffer, &hum_offset) != FR_OK)
        continue;
      if (!hum_offset)
        goto read_hum_again_1;
    }
    // 播放一个缓冲块
    if (trigger_offset >= point)
    {
      AUDIO_SoftMUX((int16_t *)dac_buffer[dac_buffer_pos], (int16_t *)trigger_buffer);
    }
    else
    {
    }
    play_a_buffer(dac_buffer[dac_buffer_pos]);
    dac_buffer_pos += 1;
    dac_buffer_pos %= AUDIO_FIFO_NUM;
    // Out播放完毕则退出
    if (!trigger_offset)
      break;
  }
  trigger_offset = 0;
}
static void Play_RunningLOOP(void)
{
  char path[30];

  sprintf(path, "0:/Bank%d/hum.wav", USR.bank_now + 1);

read_hum_again:
  if (read_a_buffer(&file_1, path, dac_buffer[dac_buffer_pos], &hum_offset) != FR_OK)
    return;
  if (!hum_offset)
    goto read_hum_again;
  if (pri_now < PRI(NULL))
  {
  read_trigger_again:
    if (read_a_buffer(&file_2, trigger_path, trigger_buffer, &trigger_offset) != FR_OK)
      return;
    if (!trigger_offset)
    {
      if (pri_now == PRI(E))
        goto read_trigger_again;
      trigger_path[0] = '\0';
      trigger_offset = 0;
      pri_now = PRI(NULL);
    }
  }

  if (pri_now == PRI(NULL))
    ;
  else
  {
    AUDIO_SoftMUX((int16_t *)dac_buffer[dac_buffer_pos], (int16_t *)trigger_buffer);
  }
  play_a_buffer(dac_buffer[dac_buffer_pos]);
  dac_buffer_pos += 1;
  dac_buffer_pos %= AUDIO_FIFO_NUM;
}
static void Play_RunningLOOPwithTrigger(char *triggerpath, uint8_t pri)
{
  if (pri_now < pri)
    return;
  else
  {
    // 详见 #001 [2]
    if (pri_now != PRI(NULL))
    {
      f_close(&file_2);
    }
    strcpy(trigger_path, triggerpath);
    pri_now = pri;
    trigger_offset = 0;
  }
}

__STATIC_INLINE void AUDIO_SoftMUX(int16_t *pt1, int16_t *pt2)
{
  uint8_t offset = 4 + 3 - USR.config->Vol;
  int16_t *p1 = (int16_t *)pt1, *p2 = (int16_t *)pt2;
  static float f = 1;

  if (USR.config->Vol == 0)
    offset = 15;
  for (uint32_t i = 0; i < AUDIO_FIFO_SIZE; i++)
  {
    *p1= *p1 + *p2;
    p1 += 1, p2 += 1;
  }
}

/**
 * @brief  读取一个缓存块
 * @NOTE   当读取到文件结尾处不能填满一个缓存块，将seek指向变量置0，不做读取操作退出
 */
__STATIC_INLINE FRESULT read_a_buffer(FIL *fpt, const TCHAR *path, void *buffer, UINT *seek)
{
  FRESULT f_err;
  UINT f_cnt;

  taskENTER_CRITICAL();
  // 详见 #001 [3]
  if (*seek == 0 && (f_err = f_open(fpt, path, FA_READ)) != FR_OK)
  {
    DEBUG(0, "Can't open file:%s:%d", path, f_err);
    taskEXIT_CRITICAL();
    return f_err;
  }

  if (*seek + AUDIO_FIFO_SIZE * sizeof(uint16_t) > fpt->fsize - sizeof(struct _AF_PCM) - sizeof(struct _AF_DATA))
  {
    *seek = 0;
    f_close(fpt);
    taskEXIT_CRITICAL();
    return FR_OK;
  }

  if ((f_err = f_lseek(fpt, *seek + sizeof(struct _AF_PCM) + sizeof(struct _AF_DATA))) != FR_OK)
  {
    DEBUG(0, "Can't seek file:%s:%d", path, f_err);
    f_close(fpt);
    taskEXIT_CRITICAL();
    return f_err;
  }

  if ((f_err = f_read(fpt, buffer, sizeof(uint16_t) * AUDIO_FIFO_SIZE, &f_cnt)) != FR_OK)
  {
    DEBUG(0, "Can't read file:%s:%c", path, f_err);
    f_close(fpt);
    taskEXIT_CRITICAL();
    return f_err;
  }

  *seek += f_cnt;

  // 详见 #001 [3]
  // f_close(fpt);
  taskEXIT_CRITICAL();
  return FR_OK;
}
/**
 * @Brief Play a buffer
 */
static void play_a_buffer(uint16_t *pt)
{
  USR.audio_busy = 1;
  while (osMessagePut(DAC_BufferHandle, (uint32_t)pt, osWaitForever) != osOK)
    ;
}

void MX_Audio_Callback(void)
{
  static osEvent evt;
  evt = osMessageGet(DAC_BufferHandle, osWaitForever);
  if (evt.status != osEventMessage)
  {
    osSemaphoreRelease(DAC_Complete_FlagHandle);
    USR.audio_busy = 0;
  }
  else
  {
    MX_Audio_Start((uint16_t*)evt.value.p, USR.config->Vol, AUDIO_FIFO_SIZE);
  }
}


uint8_t Audio_IsSimplePlayIsReady(void)
{
  return SIMPLE_PLAY_READY;
}
