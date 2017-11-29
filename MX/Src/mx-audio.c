#include "mx-audio.h"
#include "tim.h"

/**
 * brief  初始化音频所需外设
 */
__weak void MX_Audio_Init(void)
{
  HAL_TIM_Base_Start(&htim7);
  MX_Audio_Mute(true);
}

/**
 * brief  将有符号音频信号转为DAC输出所需信号
 * pt     数据指针
 * vol    当前音量(0~16)
 * cnt    数据量
 */
__weak void pcm_convert(int16_t *_pt, uint8_t vol, uint32_t cnt)
{
  int16_t *pt = _pt;
  while (cnt--)
  {
    int ans = (int)*pt * vol >> (4 + 4) + 0x1000 / 2;
    *pt = (int16_t)ans;
    pt += 1;
  }
}

/**
 * brief  Audio.c DACHandle调用的底层函数
 */
__weak void MX_Audio_Start(uint16_t* pt1, uint16_t *pt2, uint8_t vol, uint32_t cnt)
{
  if (vol == 0) return;
  // pcm_convert((int16_t*)pt1, 4 + 3 - vol, cnt);
  // pcm_convert((int16_t*)pt2, 4 + 3 - vol, cnt);
  pcm_convert((int16_t*)pt1, vol, cnt);
  pcm_convert((int16_t*)pt2, vol, cnt);
  MX_Audio_Mute(false);
  HAL_DAC_Start_DMA(&hdac, DAC_CHANNEL_1, (uint32_t*)pt1, cnt, DAC_ALIGN_12B_R);
  HAL_DAC_Start_DMA(&hdac, DAC_CHANNEL_2, (uint32_t*)pt2, cnt, DAC_ALIGN_12B_R);
}

/**
 * brief  调用 MX_Audio_Callback， 此函数在DMA完成输出后调用
 */
__weak void HAL_DAC_ConvCpltCallbackCh1(DAC_HandleTypeDef* hdac)
{
  MX_Audio_Callback();
}

/**
 * brief  硬件产生 HWBeep
 * note   延时使用的osDelay
 */
__weak void MX_Audio_HWBeep(void)
{
  /// SD card can't initialize, so make a warning wave by soft.
  uint16_t *pt = (uint16_t *)pvPortMalloc(sizeof(uint16_t) * 1024);
  uint16_t *ppt = pt;
  uint16_t cnt = 1024;
  MX_GPIO_Enable(true);
  MX_Audio_Mute(false);
  while (cnt--)
  {
    *pt = ((float)(sin(cnt * 3.1514926 / 20) / 2) * 0x1000) + 0x1000 / 2;
    pt += 1;
  }
  cnt = 10;
  while (cnt--)
  {
    HAL_DAC_Start_DMA(&hdac, DAC_CHANNEL_1, (uint32_t *)ppt, 1024, DAC_ALIGN_12B_R);
    osDelay(50);
  }
  MX_Audio_Mute(true);
  MX_GPIO_Enable(false);
  while (1)
    ;
}

/**
 * brief  不播放音频的时候消除电流音
 */
__weak void MX_Audio_Mute(bool en)
{
  HAL_GPIO_WritePin(AUDIO_EN_GPIO_Port,
                    AUDIO_EN_Pin,
                    en ? GPIO_PIN_RESET : GPIO_PIN_SET);
}
