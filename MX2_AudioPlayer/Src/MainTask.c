// STD Lib
/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

// STM32 Lib
#include "stm32f1xx_hal.h"
#include "adc.h"
#include "dac.h"

// OS Lib
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "cmsis_os.h"

// User Lib
#include "main.h"
#include "ff.h"
#include "debug.h"
#include "USR_CONFIG.h"
#include "dac.h"
#include "Audio.h"
#include "Lis3D.h"
#include "LED.h"

/* Variables -----------------------------------------------------------------*/
extern osThreadId defaultTaskHandle;
extern osThreadId DACTaskHandle;
extern osThreadId LEDTaskHandle;
extern osThreadId WavTaskHandle;
extern osMessageQId DAC_BufferHandle;
extern osMessageQId DAC_CMDHandle;
extern osMessageQId LED_CMDHandle;
extern osSemaphoreId DAC_Complete_FlagHandle;
///NOTE: 由于各种触发为低概率事件， 循环中的定时器的定时间隔又软件延时产生，间隔时间为LOOP_DELAY
///切勿将LOOP_DELAY改为0
#define LOOP_DELAY 15

// 冻结时间计数值
static uint16_t frozen_trigger_cnt[3] = {0, 0, 0};
// 电源电压检测采集到的ADC值
static uint16_t power_adc_val;
// 自动待机定时器值
static uint32_t auto_intoready_cnt = 0;
// 自动关机定时器值
static uint32_t auto_poweroff_cnt = 0;
FATFS fatfs;
/* Function prototypes -------------------------------------------------------*/
static void filesystem_init(void);
static void beep_error(void);
static uint8_t key_scan(void);
static uint8_t ask_trigger(uint8_t triggerid);
static void ticktock_trigger(void);
static void move_detected(void);
static uint8_t auto_off(uint32_t* ready_cnt, uint32_t *poweroff_cnt);
extern void MX_FATFS_Init(void);
#define AUTO_CNT_CLEAR()  auto_intoready_cnt = 0, auto_poweroff_cnt = 0

void StartDefaultTask(void const * argument)
{
  // 检测是否静音启动
  if (HAL_GPIO_ReadPin(KEY_GPIO_Port, KEY_Pin) == GPIO_PIN_RESET)
  {
    USR.mute_flag = 1;
  } else {
    USR.mute_flag = 0;
  }

  // Fatfs 初始化
  MX_FATFS_Init();

  // SD卡信息初始化到内存
  filesystem_init();

  // 启动中的按键检测， 延时中未打开电源使能端，若用户松开按键则会断开单片机电源
  if (USR.mute_flag)
    osDelay(USR.config->Tmute);
  else
    osDelay(USR.config->Tpon);

  // 使能电源使能端
  HAL_GPIO_WritePin(Power_EN_GPIO_Port, Power_EN_Pin, GPIO_PIN_SET);

  #if USE_DEBUG
  __ASM("BKPT 0");
  #endif

  /**< 由于多Bank可能不同的Vol值，初始化时判断Vol为0不再是安全的操作 */
  // // 当配置音频音量为0时，默认与静音启动相同操作
  // if (USR.config->Vol == 0) {
  //     USR.mute_flag = 1;
  // }

  // 初始化结构体中的参数
  USR.sys_status = System_Restart;
  USR.bank_now = 0;
  USR.bank_color = 0;

  // 更新LED用到的变量
  LED_Bank_Update(&USR);

  ///Lis3DHTR initialize
  {
    Lis3dConfig config;
    config.CD = USR.config->CD;
    config.CL = USR.config->CL;
    config.CT = USR.config->CT;
    config.CW = USR.config->CW;
    config.MD = USR.config->MD;
    config.MT = USR.config->MT;
    Lis3d_Set(&config);
  }

  // Power voltage check
  HAL_ADC_Start_DMA(&hadc1, (uint32_t*)&power_adc_val, 1);
  osDelay(100);

  // 低电压检测：忽略静音标志发送2次低电压报警
  if (power_adc_val <= STATIC_USR.vol_warning)
  {
    uint8_t buf = USR.mute_flag;
    USR.mute_flag = 0;
    Audio_Play_Start(Audio_LowPower);
    Audio_Play_Start(Audio_LowPower);
    USR.mute_flag = buf;
  }

  // 初始化音
  Audio_Play_Start(Audio_Boot);

  USR.sys_status = System_Ready;

  for (;;)
  {

    // 检测按键上升/下降沿
    uint8_t key_status = key_scan();

    // 系统参数分析
    #if USE_DEBUG
    USR.Stack_Free[0] = uxTaskGetStackHighWaterMark(defaultTaskHandle);
    USR.Stack_Free[1] = uxTaskGetStackHighWaterMark(DACTaskHandle);
    USR.Stack_Free[2] = uxTaskGetStackHighWaterMark(LEDTaskHandle);
    USR.Stack_Free[3] = uxTaskGetStackHighWaterMark(WavTaskHandle);
    #endif

    ///Button check ant its function
    if (USR.sys_status == System_Ready)
    {

      /**< Power Key down*/
      if (key_status & 0x04)
      {
        uint16_t timeout = 0;

        //Wait for POWER Key rising
        while (!(key_scan() & 0x01) &&
               (timeout < (USR.config->Tpoff > USR.config->Tout ? USR.config->Tpoff : USR.config->Tout)))
        {
          osDelay(10);
          timeout += 10;
        }
        if (timeout > (USR.config->Tpoff>USR.config->Tout?USR.config->Tout:USR.config->Tpoff))
        {
          if ((USR.config->Tpoff >= USR.config->Tout && USR.config->Tpoff <= timeout) ||
              (USR.config->Tpoff < USR.config->Tout && USR.config->Tout > timeout))
          {
            DEBUG(5, "System going to close");
            USR.sys_status = System_Close;
            Audio_Play_Start(Audio_PowerOff);
            osDelay(3000); //3s
            HAL_GPIO_WritePin(Power_EN_GPIO_Port, Power_EN_Pin, GPIO_PIN_RESET);
          }
          else
          {
            DEBUG(5, "System going to running");
            auto_intoready_cnt = 0;
            USR.sys_status = System_Running;
            Audio_Play_Start(Audio_intoRunning);
            LED_Start_Trigger(LED_Trigger_Start);
          }
        }
      }
      /**< User Key down */
      else if (key_status & 0x08)
      {
        uint16_t timeout = 0;
        while ((!(key_scan()& 0x02)) && (timeout < USR.config->Ts_switch))
        {
          osDelay(10);
          timeout += 10;
        }
        if (timeout >= USR.config->Ts_switch)
        {
          USR.bank_now += 1;
          USR.bank_now %= USR.nBank;
          DEBUG(5, "System Bank Switch");
          USR.config = USR._config + USR.bank_now;
          LED_Bank_Update(&USR);
          Audio_Play_Start(Audio_BankSwitch);
        }
      }
    }

    else if (USR.sys_status == System_Running)
    {
      // PowerKey
      if (key_status & 0x04)
      {
        uint16_t timeout = 0;
        while (
         (!((key_status = key_scan()) & 0x01))  //Waiting for PowerKey UP
         &&(!(key_status & 0x08))               //Waiting for UserKey  DOWN
         &&timeout < USR.config->Tin)           //Waiting for Timeout
        {
          osDelay(10);
          timeout += 10;
        }

        if (timeout < USR.config->Tin && (key_status & 0x08))
        {
          DEBUG(5, "System ColorSwitch");
          USR.bank_color += 1;
          LED_Bank_Update(&USR);
          Audio_Play_Start(Audio_ColorSwitch);
        }
        else if (timeout >= USR.config->Tin)
        {
          DEBUG(5, "System going to ready");
          USR.sys_status = System_Ready;
          LED_Start_Trigger(LED_Trigger_Stop);
          Audio_Play_Start(Audio_intoReady);
          USR.bank_color   = 0; //每次退出都将清零Colorswitch的值
        }
      }

      // User Key
      else if (key_status & 0x08)
      {
        uint16_t timeout = 0;
        while (!(key_scan() & 0x02))
        {
          osDelay(10);
          timeout += 10;

          if (timeout >= USR.config->TEtrigger)
          {
            DEBUG(5, "Trigger E");
            LED_Start_Trigger(LED_TriggerE);
            Audio_Play_Start(Audio_TriggerE);
            while (!(key_scan() & 0x02)) {
                osDelay(10);
            }
            DEBUG(5, "Trigger E END");
            LED_Start_Trigger(LED_TriggerE_END);
            Audio_Play_Stop(Audio_TriggerE);
            break;
          }
        }
        if (timeout < USR.config->TEtrigger && ask_trigger(2))
        {
          DEBUG(5, "Trigger D");
          LED_Start_Trigger(LED_TriggerD);
          Audio_Play_Start(Audio_TriggerD);
        }
      }
      move_detected();
    }

    else if (USR.sys_status == System_Charged)
    {

    }

    else if (USR.sys_status == System_Charging)
    {
      if (key_status & 0xC) // Keys DOWN
      {
        Audio_Play_Start(Audio_Charging);
      }
    }
    /// 电源管理部分
    if (power_adc_val <= STATIC_USR.vol_poweroff)
    {
      DEBUG(5, "System should be Power off");
      USR.sys_status = System_Close;
      Audio_Play_Start(Audio_Recharge);
      osDelay(2000);
      HAL_GPIO_WritePin(Power_EN_GPIO_Port, Power_EN_Pin, GPIO_PIN_RESET);
    }
    /// 充电检测
    if (HAL_GPIO_ReadPin(Charge_Check_GPIO_Port, Charge_Check_Pin) == GPIO_PIN_SET)
    {
      if (USR.sys_status == System_Charged || USR.sys_status == System_Charging) {

      } else if (STATIC_USR.vol_chargecomplete < power_adc_val){
        USR.sys_status = System_Charged;
      } else {
        USR.sys_status = System_Charging;
      }
    } else if (USR.sys_status == System_Charged || USR.sys_status == System_Charging)
    {
      USR.sys_status = System_Close;
      Audio_Play_Start(Audio_PowerOff);
      osDelay(2000); // 2s
      HAL_GPIO_WritePin(Power_EN_GPIO_Port, Power_EN_Pin, GPIO_PIN_RESET);
    }

    ///当有按键动作时清空自动关机\待机定时器
    if (key_status != 0) {
        AUTO_CNT_CLEAR();
    }

    /// 自动关机\待机定时器 当达到配置文件的计数值后触发动作
    uint8_t t;
    if ((t = auto_off(&auto_intoready_cnt, &auto_poweroff_cnt)) != 0)
    {
      if (t == 1) {
          DEBUG(5, "System going to ready");
          USR.sys_status = System_Ready;
          LED_Start_Trigger(LED_Trigger_Stop);
          Audio_Play_Start(Audio_intoReady);
      } else if (t == 2) {
          DEBUG(5, "System going to close");
          USR.sys_status = System_Close;
          Audio_Play_Start(Audio_PowerOff);
          osDelay(3000); //3s
          HAL_GPIO_WritePin(Power_EN_GPIO_Port, Power_EN_Pin, GPIO_PIN_RESET);
      }
    }

    /// 触发的冻结时间计数器
    ticktock_trigger();

    ///NOTE: 由于各种触发为低概率事件， 循环中的定时器的定时间隔又软件延时产生，间隔时间为LOOP_DELAY
    ///切勿将LOOP_DELAY改为0或者将此函数删除
    osDelay(LOOP_DELAY);
  }
}
/**
 * @Brief 自动关机/待机功能
 * @Para  ready_cnt 运行态转待机的计时器
 * @Para  poweroff_cnt 待机态转关机的计时器
 * @Retvl 0-No trigger | 1-转待机 | 2-转关机
 */
static uint8_t auto_off(uint32_t* ready_cnt, uint32_t *poweroff_cnt)
{
  if (USR.sys_status == System_Ready) {
      *poweroff_cnt += LOOP_DELAY;
      if (*poweroff_cnt >= USR.config->Tautooff && USR.config->Tautooff != 0)
        return 2;
  } else if (USR.sys_status == System_Running) {
      *ready_cnt += LOOP_DELAY;
      if (*ready_cnt >= USR.config->Tautoin && USR.config->Tautoin != 0)
        return 1;
  } return 0;
}

/**
 * @Brief  加速度计检测，当有触发后触发相应动作并清空自动待机定时器
 */
static void move_detected(void) {
    uint8_t move,click, __move, __click;
    static  uint8_t _move = 0, _click = 0;
    __move = Lis3d_isMove();
    __click = Lis3d_isClick();

    if (__move > 0 && _move == 0) move = 1;
    else move = 0;
    if (__click > 0 && _click == 0) click = 1;
    else click = 0;
    _move = __move;
    _click = __click;
    //Lis3DH parts
    //TriggerC
    if (click && ask_trigger(1)) {
        LED_Start_Trigger(LED_TriggerC);
        Audio_Play_Start(Audio_TriggerC);
        AUTO_CNT_CLEAR();
    }
    //TriggerB
    if (move && !ask_trigger(0)) {
        LED_Start_Trigger(LED_TriggerB);
        Audio_Play_Start(Audio_TriggerB);
        AUTO_CNT_CLEAR();
    }
}
/**
 * @Brief  Init fatfs and then initial all user configuration
 */
static void filesystem_init(void)
{
  int f_err;
  if ((f_err = f_mount(&fatfs, "0:/", 1)) != FR_OK)
  {
    DEBUG(0, "mount error:%d", f_err);
    beep_error();
  }

  if ((f_err = usr_config_init()) != 0)
  {
    DEBUG(0, "User Configuration has some problem:%d", f_err);
    beep_error();
  }
}

/**
 * @Brief When fatfs init error, call this function notifi user by Beep
 */
static void beep_error(void)
{
  /// SD card can't initialize, so make a warning wave by soft.
  uint16_t *pt = (uint16_t*)pvPortMalloc(sizeof(uint16_t)*1024*15);
  uint16_t *ppt = pt;
  uint16_t cnt = 1024*15;
  HAL_GPIO_WritePin(Power_EN_GPIO_Port, Power_EN_Pin, GPIO_PIN_SET);
  while (cnt--)
  {
    *pt = ((float)(sin(cnt*3.1514926/20)/2)*0x1000) + 0x1000/2;
    pt += 1;
  }
  cnt = 1;
  while (cnt--) {
    HAL_DAC_Start_DMA(&hdac, DAC_CHANNEL_1, (uint32_t*)ppt, 1024*20, DAC_ALIGN_12B_R);
    osDelay(1000);
  }
  HAL_GPIO_WritePin(Power_EN_GPIO_Port, Power_EN_Pin, GPIO_PIN_RESET);
  while (1);

}

/**
 * @Brief  Scan User Key and then return Active logic
 * @Retval 0x01-PowerKeyUP 0x02-UserKeyUP 0x04-PowerKeyDOWN 0x08-UserKeyDOWN
 */
static uint8_t key_scan(void)
{
  // 第一次检测时可以确定Power按键按下|User按键未按下
  static uint8_t status = 1;
  uint8_t buf = 0, pre_buf;

  if (HAL_GPIO_ReadPin(POWER_GPIO_Port, POWER_Pin) == GPIO_PIN_SET) {
    buf |= 0x01;
  } if (HAL_GPIO_ReadPin(KEY_GPIO_Port, KEY_Pin) == GPIO_PIN_RESET) {
    buf |= 0x02;
  }
  pre_buf = buf;
  buf ^= status;
  status = pre_buf;

  if (buf & 0x01) {
    if (pre_buf & 0x01) buf = 0x04;
    else buf = 0x01;
  } if (buf & 0x02) {
    if (pre_buf & 0x02) buf = 0x08;
    else buf = 0x02;
  } return buf;
}

/**
 * @Brief  ask for trigger
 * @Para trigger id Trigger 0~2 -> TriggerB~TriggerD
 * @Retval 1-ok|0-busy
 */
static uint8_t ask_trigger(uint8_t triggerid)
{
  if (!frozen_trigger_cnt[triggerid]) {
    switch (triggerid) {
      case 0: frozen_trigger_cnt[0] = USR.config->TBfreeze; break;
      case 1: frozen_trigger_cnt[1] = USR.config->TCfreeze; break;
      case 2: frozen_trigger_cnt[2] = USR.config->TDfreeze; break;
    } return 1;
  } return 0;
}

/**
 * @Brief  Update trigger's frozen time, put it in MainTask's loop.
 */
static void ticktock_trigger(void) {
  for (uint8_t i = 0; i < 3; i++)
  {
    if (frozen_trigger_cnt[i] > 0) {
      frozen_trigger_cnt[i] -= LOOP_DELAY;
    }
  }
}
