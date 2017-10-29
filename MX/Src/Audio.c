#include "Audio.h"
/* Variables -----------------------------------------------------------------*/
static uint8_t SIMPLE_PLAY_READY = 1;
static bool isPlaySwitch = false;
static FIL audio_file[2];
static UINT file_offset[2] = {0, 0};
static char trigger_path[50];
static char pri_now = PRI(NULL);
static uint16_t dac_buffer[AUDIO_TRACK_NUM][AUDIO_FIFO_NUM][AUDIO_FIFO_SIZE];
static uint16_t dac_buffer_pos = 0;
#if AUDIO_SOFTMIX
static uint16_t trigger_buffer[AUDIO_FIFO_SIZE];
#endif
/* Function prototypes -------------------------------------------------------*/
static void Play_simple_wav(char *filepath);
static void Play_IN_wav(Audio_ID_t id);
static void Play_OUT_wav(Audio_ID_t id);
static void Play_Trigger_wav(uint8_t);
static void Play_TriggerE(void);
static void Play_TriggerE_END(void);
static void Play_RunningLOOP(void);
static void Play_RunningLOOPwithTrigger(char *triggerpath, uint8_t pri);
static osEvent Play_PlayerWave(const char *path);
static void play_a_buffer(uint16_t buffer_pos);
static FRESULT read_a_buffer(FIL *fpt, const TCHAR *path, void *buffer, UINT *seek);
static void SoftMix(int16_t *, int16_t *);
#define convert_filesize2MS(size) (size / 22 / sizeof(uint16_t))
#define convert_ms2filesize(ms) (ms * sizeof(uint16_t) * 22)
#define RESET_FILE_Buffer()      \
  f_close(&audio_file[Track_0]); \
  f_close(&audio_file[Track_1]); \
  file_offset[Track_0] = 0;      \
  file_offset[Track_1] = 0;

bool Audio_IsPlayBankSwitch(void)
{
  return isPlaySwitch;
}

int8_t Audio_Play_Start(Audio_ID_t id)
{
  if (!USR.mute_flag && USR.config->Vol != 0)
  {
    log_v("Audio start:%d", id);
    while (osMessagePut(DAC_CMDHandle, id, osWaitForever) != osOK)
      ;
  }
  return 0;
}

int8_t Audio_Play_Stop(Audio_ID_t id)
{
  if (!USR.mute_flag)
  {
    log_v("Audio stop:%d", id);
    while (osMessagePut(DAC_CMDHandle, id | 0x80, osWaitForever) != osOK)
      ;
  }
  return 0;
}

void DACOutput(void const *argument)
{
  while (1)
  {
    osEvent evt = osMessageGet(DAC_BufferHandle, 50);
    if (evt.status != osEventMessage)
    {
      MX_Audio_Mute(true);
      continue;
    }
    osSemaphoreWait(DAC_Complete_FlagHandle, osWaitForever);

#if AUDIO_SOFTMIX
    MX_Audio_Start((uint16_t *)dac_buffer[Track_0][evt.value.v], USR.config->Vol, AUDIO_FIFO_SIZE);
#else
    MX_Audio_Start((uint16_t *)dac_buffer[Track_0][evt.value.v],
                   (uint16_t *)dac_buffer[Track_1][evt.value.v],
                   USR.config->Vol,
                   AUDIO_FIFO_SIZE);
#endif
  }
}

void Wav_Task(void const *argument)
{

  while (1)
  {
    osEvent evt;
    /**< 当不在运行态时音频的播放使用简单模式 */
    if (USR.sys_status != System_Running &&
        USR.sys_status != System_Player)
    {

      evt = osMessageGet(DAC_CMDHandle, osWaitForever);

      if (evt.status != osEventMessage)
        continue;

      /**< 由于运行态会产生两段音频同时播放的情况，特殊处理 */
    }

    /**< 播放器模式 */
    else if (USR.sys_status == System_Player)
    {
      log_w("TODO, unknow opration");
    }

    /**< 运行模式 */
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
  INTER_MESSAGE:

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
      case Audio_intoReady_X:
      case Audio_intoReady_Y:
      case Audio_intoReady_Z:
      case Audio_intoReady:
        pri_now = PRI(NULL);
        RESET_FILE_Buffer();
        Play_IN_wav(evt.value.v);
        break;
      case Audio_BankSwitch:
      {

        char path[25];
        sprintf(path, "0:/Bank%d/BankSwitch.wav", USR.bank_now + 1);
        isPlaySwitch = true;
        Play_simple_wav(path);
        isPlaySwitch = false;
        break;
      }
      }
    }
    else if (USR.sys_status == System_Player)
    {
      static uint16_t audio_player_pos = 0;
      static uint16_t audio_player_num = 0;

      switch (evt.value.v)
      {
      case Audio_Player_Enter:
      {
        char path[] = "0:/Player.wav";
        audio_player_num = MX_File_SearchFile("0:/", "Multimedia", ".wav");
        audio_player_pos = 0;
        Play_simple_wav(path);
        break;
      }
      case Audio_Player_Exit:
        break;
      case Audio_Player_Start:
      {
        char path[50];
        if (MX_File_SearchFileName("0:/",
                                   "Multimedia",
                                   ".wav",
                                   audio_player_pos,
                                   path,
                                   50))
        {
          evt = Play_PlayerWave(path);
          audio_player_pos += 1;
          audio_player_pos %= audio_player_num;
          if (evt.status == osEventMessage)
            goto INTER_MESSAGE;
        }
        break;
      }
      case Audio_Player_Stop:
      case Audio_Player_Switch:
        log_w("TODO, unknow opration");
        break;

      default:
        log_e("unknow audio message:%d", (int)evt.value.v);
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
      case Audio_intoRunning_X:
      case Audio_intoRunning_Y:
      case Audio_intoRunning_Z:
      case Audio_intoRunning:
        pri_now = PRI(NULL);
        RESET_FILE_Buffer();
        Play_OUT_wav(evt.value.v);
        break;
      }
    }
    else if (USR.sys_status == System_Close)
    {
      RESET_FILE_Buffer();
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
    log_w("Open wave file:%s:%d", filepath, f_err);
    SIMPLE_PLAY_READY = 1;
    taskEXIT_CRITICAL();
    return;
  }

  // Ignore about pcm structure
  f_lseek(&file, sizeof(struct _AF_PCM));
  // Read about this file's length
  if ((f_err = f_read(&file, &data, sizeof(struct _AF_DATA), &f_cnt)) != FR_OK)
  {
    log_w("Read wave file:%s Error:%d", filepath, f_err);
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
    if ((f_err = f_read(&file, dac_buffer[Track_0][dac_buffer_pos], sizeof(uint16_t) * AUDIO_FIFO_SIZE, &f_cnt)) != FR_OK && f_cnt != 0)
    {
      log_w("Read wave file:%s Error:%d", filepath, f_err);
      f_close(&file);
      SIMPLE_PLAY_READY = 1;
      return;
    }
    taskEXIT_CRITICAL();

    play_a_buffer(dac_buffer_pos);

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
    uint8_t pri;
    uint8_t num;

    switch (triggerid)
    {
    case 0:
      cnt = (USR.triggerB + USR.bank_now)->number;
      num = HAL_GetTick() % cnt;
      pri = PRI(B);
      sprintf(path, "0:/Bank%d/" TRIGGER(B) "/", USR.bank_now + 1);
      strcat(path, (USR.triggerB + USR.bank_now)->path_ptr[num]);
      break;
    case 1:
      cnt = (USR.triggerC + USR.bank_now)->number;
      num = HAL_GetTick() % cnt;
      pri = PRI(C);
      sprintf(path, "0:/Bank%d/" TRIGGER(C) "/", USR.bank_now + 1);
      strcat(path, (USR.triggerC + USR.bank_now)->path_ptr[num]);
      break;
    case 2:
      cnt = (USR.triggerD + USR.bank_now)->number;
      num = HAL_GetTick() % cnt;
      pri = PRI(D);
      sprintf(path, "0:/Bank%d/" TRIGGER(D) "/", USR.bank_now + 1);
      strcat(path, (USR.triggerD + USR.bank_now)->path_ptr[num]);
      break;
    }

    Play_RunningLOOPwithTrigger(path, pri);
  }
  else if (triggerid == 3)
  {
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
  f_close(&audio_file[Track_1]);
  trigger_path[0] = 0;
  file_offset[Track_1] = 0;
  pri_now = PRI(NULL);
}
static void Play_IN_wav(Audio_ID_t id)
{
  uint8_t num;
  char path[50];

  switch (id)
  {
  case Audio_intoReady:
    sprintf(path, "0:/Bank%d/" TRIGGER(IN) "/", USR.bank_now + 1);
    num = HAL_GetTick() % USR.triggerIn->number;
    strcat(path, (USR.triggerIn->path_ptr[num]));
    break;

  case Audio_intoReady_X:
    sprintf(path, "0:/Bank%d/" TRIGGER(IN) "/X/", USR.bank_now + 1);
    num = HAL_GetTick() % USR.triggerIn_X->number;
    strcat(path, (USR.triggerIn_X->path_ptr[num]));
    break;

  case Audio_intoReady_Y:
    sprintf(path, "0:/Bank%d/" TRIGGER(IN) "/Y/", USR.bank_now + 1);
    num = HAL_GetTick() % USR.triggerIn_Y->number;
    strcat(path, (USR.triggerIn_Y->path_ptr[num]));
    break;

  case Audio_intoReady_Z:
    sprintf(path, "0:/Bank%d/" TRIGGER(IN) "/Z/", USR.bank_now + 1);
    num = HAL_GetTick() % USR.triggerIn_Z->number;
    strcat(path, (USR.triggerIn_Z->path_ptr[num]));
    break;
  default:
    return;
  }
  Play_simple_wav(path);
}
static void Play_OUT_wav(Audio_ID_t id)
{
  uint8_t num;
  char path[50];
  char hum_path[50];
#if AUDIO_SOFTMIX
  uint16_t *pt_trigger = trigger_buffer;
#else
  uint16_t *pt_trigger = dac_buffer[Track_1][dac_buffer_pos];
#endif

  UINT point = convert_ms2filesize(USR.config->Out_Delay);
  sprintf(hum_path, "0:/Bank%d/hum.wav", USR.bank_now + 1);
  // sprintf(path, "0:/Bank%d/" TRIGGER(OUT) "/", USR.bank_now + 1);
  // strcat(path, (USR.triggerOut + USR.bank_now)->path_ptr[num]);
  switch (id)
  {
  case Audio_intoRunning:
    num = HAL_GetTick() % USR.triggerOut->number;
    sprintf(path, "0:/Bank%d/" TRIGGER(IN) "/", USR.bank_now + 1);
    strcat(path, USR.triggerOut->path_ptr[num]);
    break;
  case Audio_intoRunning_X:
    num = HAL_GetTick() % USR.triggerOut_X->number;
    sprintf(path, "0:/Bank%d/" TRIGGER(IN) "/X", USR.bank_now + 1);
    strcat(path, USR.triggerOut_X->path_ptr[num]);
    break;
  case Audio_intoRunning_Y:
    num = HAL_GetTick() % USR.triggerOut_Y->number;
    sprintf(path, "0:/Bank%d/" TRIGGER(IN) "/Y", USR.bank_now + 1);
    strcat(path, USR.triggerOut_Y->path_ptr[num]);
    break;
  case Audio_intoRunning_Z:
    num = HAL_GetTick() % USR.triggerOut_Z->number;
    sprintf(path, "0:/Bank%d/" TRIGGER(IN) "/Z", USR.bank_now + 1);
    strcat(path, USR.triggerOut_Z->path_ptr[num]);
    break;
  default:
    return;
  }

  while (1)
  {
#if AUDIO_SOFTMIX == 0
    pt_trigger = dac_buffer[Track_1][dac_buffer_pos];
#endif
    // 读取Out
    if (read_a_buffer(&audio_file[Track_1], path, dac_buffer[Track_0][dac_buffer_pos], &file_offset[Track_1]) != FR_OK)
      continue;
    // 达到Out_Delay延时读取hum.wav
    if (file_offset[Track_1] >= point)
    {
    read_hum_again_1:
      if (read_a_buffer(&audio_file[Track_0], hum_path, pt_trigger, &file_offset[Track_0]) != FR_OK)
        continue;
      if (!file_offset[Track_0])
        goto read_hum_again_1;
    }
    // 播放一个缓冲块
    if (file_offset[Track_1] >= point)
    {
#if AUDIO_SOFTMIX
      SoftMix((int16_t *)dac_buffer[Track_0][dac_buffer_pos], (int16_t *)pt_trigger);
#else
#endif
    }
    play_a_buffer(dac_buffer_pos);
    dac_buffer_pos += 1;
    dac_buffer_pos %= AUDIO_FIFO_NUM;
    // Out播放完毕则退出
    if (!file_offset[Track_1])
      break;
  }
  file_offset[Track_1] = 0;
}
static void Play_RunningLOOP(void)
{
  char path[30];
#if AUDIO_SOFTMIX
  uint16_t *pt_trigger = trigger_buffer;
#else
  uint16_t *pt_trigger = dac_buffer[Track_1][dac_buffer_pos];
#endif
  sprintf(path, "0:/Bank%d/hum.wav", USR.bank_now + 1);

read_hum_again:
  if (read_a_buffer(&audio_file[Track_0], path, dac_buffer[Track_0][dac_buffer_pos], &file_offset[Track_0]) != FR_OK)
    return;
  if (!file_offset[Track_0])
    goto read_hum_again;
  if (pri_now < PRI(NULL))
  {
  read_trigger_again:
    if (read_a_buffer(&audio_file[Track_1], trigger_path, pt_trigger, &file_offset[Track_1]) != FR_OK)
      return;
    if (!file_offset[Track_1])
    {
      if (pri_now == PRI(E))
        goto read_trigger_again;
      trigger_path[0] = '\0';
      file_offset[Track_1] = 0;
      pri_now = PRI(NULL);
    }
  }

  if (pri_now == PRI(NULL))
    ;
  else
  {
#if AUDIO_SOFTMIX
    SoftMix((int16_t *)dac_buffer[Track_0][dac_buffer_pos], (int16_t *)pt_trigger);
#endif
  }
  play_a_buffer(dac_buffer_pos);
  dac_buffer_pos += 1;
  dac_buffer_pos %= AUDIO_FIFO_NUM;
}
static void Play_RunningLOOPwithTrigger(char *triggerpath, uint8_t pri)
{
  if (pri_now < pri)
    return;
  else
  {
    if (pri_now != PRI(NULL))
    {
      f_close(&audio_file[Track_1]);
    }
    strcpy(trigger_path, triggerpath);
    pri_now = pri;
    file_offset[Track_1] = 0;
  }
}

__STATIC_INLINE void SoftMix(int16_t *pt1, int16_t *pt2)
{
#if AUDIO_SOFTMIX
  int16_t *p1 = (int16_t *)pt1, *p2 = (int16_t *)pt2;

  for (uint32_t i = 0; i < AUDIO_FIFO_SIZE; i++)
  {
    *p1 = *p1 + *p2;
    p1 += 1, p2 += 1;
  }
#else
#endif
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

  if (*seek == 0 && (f_err = f_open(fpt, path, FA_READ)) != FR_OK)
  {
    log_w("Can't open file:%s:%d", path, f_err);
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
    log_w("Can't seek file:%s:%d", path, f_err);
    f_close(fpt);
    taskEXIT_CRITICAL();
    return f_err;
  }

  if ((f_err = f_read(fpt, buffer, sizeof(uint16_t) * AUDIO_FIFO_SIZE, &f_cnt)) != FR_OK)
  {
    log_w("Can't read file:%s:%c", path, f_err);
    f_close(fpt);
    taskEXIT_CRITICAL();
    return f_err;
  }

  *seek += f_cnt;

  taskEXIT_CRITICAL();
  return FR_OK;
}
/**
 * @Brief Play a buffer
 */
static void play_a_buffer(uint16_t pos)
{
  USR.audio_busy = 1;
  while (osMessagePut(DAC_BufferHandle, (uint32_t)pos, osWaitForever) != osOK)
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
#if AUDIO_SOFTMIX
    MX_Audio_Start((uint16_t *)dac_buffer[Track_0][evt.value.v], USR.config->Vol, AUDIO_FIFO_SIZE);
#else
    MX_Audio_Start((uint16_t *)dac_buffer[Track_0][evt.value.v],
                   (uint16_t *)dac_buffer[Track_1][evt.value.v],
                   USR.config->Vol,
                   AUDIO_FIFO_SIZE);
#endif
  }
}
/**
 * @brief  播放器模式下播放音频，支持audio消息响应
 * @para   path 播放文件路径
 * @note   只响应播放器终止(Audio_Player_Exit),
 *         以及播放器停止(Aduio_Player_Stop)、
 *         切换(Audio_Player_Switch)
 * @retvl  停止标志， Audio_ID_NULL-播放完毕退出, 或者是退出消息
 */
static osEvent Play_PlayerWave(const char *filepath)
{
  FIL file;
  FRESULT f_err;
  UINT f_cnt;
  struct _AF_DATA data;
  osEvent res = {.status = osOK, .value.v = Audio_ID_NULL};

  taskENTER_CRITICAL();
  SIMPLE_PLAY_READY = 0;
  if ((f_err = f_open(&file, filepath, FA_READ)) != FR_OK)
  {
    log_w("Open wave file:%s:%d", filepath, f_err);
    SIMPLE_PLAY_READY = 1;
    taskEXIT_CRITICAL();
    return;
  }

  // Ignore about pcm structure
  f_lseek(&file, sizeof(struct _AF_PCM));
  // Read about this file's length
  if ((f_err = f_read(&file, &data, sizeof(struct _AF_DATA), &f_cnt)) != FR_OK)
  {
    log_w("Read wave file:%s Error:%d", filepath, f_err);
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
    if ((f_err = f_read(&file, dac_buffer[Track_0][dac_buffer_pos], sizeof(uint16_t) * AUDIO_FIFO_SIZE, &f_cnt)) != FR_OK && f_cnt != 0)
    {
      log_w("Read wave file:%s Error:%d", filepath, f_err);
      f_close(&file);
      SIMPLE_PLAY_READY = 1;
      return;
    }
    taskEXIT_CRITICAL();

    play_a_buffer(dac_buffer_pos);

    dac_buffer_pos += 1;
    dac_buffer_pos %= AUDIO_FIFO_NUM;
    data.size -= f_cnt;

    osEvent evt = osMessageGet(DAC_CMDHandle, 1);
    if (evt.status != osEventMessage)
    {
      if (evt.value.v == Audio_Player_Exit ||
          evt.value.v == Audio_Player_Stop ||
          evt.value.v == Audio_Player_Switch)
      {
        res = evt;
        break;
      }
    }
  }
  f_close(&file);
  SIMPLE_PLAY_READY = 1;
  return res;
}
uint8_t Audio_IsSimplePlayIsReady(void)
{
  return SIMPLE_PLAY_READY;
}
