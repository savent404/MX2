// STD Lib
/* Includes ------------------------------------------------------------------*/
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LOG_TAG "Main"

#include "Audio.h"
#include "DEBUG.h"
#include "FreeRTOS.h"
#include "cmsis_os.h"
#include "mx-adc.h"
#include "mx-audio.h"
#include "mx-gpio.h"
#include "queue.h"
#include "task.h"
// #include "LED.h"
#include "Lis3D.h"
#include "MX_osID.h"
#include "SimpleLED.h"
#include "USR_CONFIG.h"
#include "ff.h"
#include "main.h"
#include "mx-led.h"

/* Variables -----------------------------------------------------------------*/
///NOTE: 由于各种触发为低概率事件， 循环中的定时器的定时间隔又软件延时产生，间隔时间为LOOP_DELAY
///切勿将LOOP_DELAY改为0
#define LOOP_DELAY 15

// 冻结时间计数值
static int16_t frozen_trigger_cnt[3] = {0, 0, 0};
// 电源电压检测采集到的ADC值
static uint16_t power_adc_val;
// 自动待机定时器值
static uint32_t auto_intoready_cnt = 0;
// 自动关机定时器值
static uint32_t auto_poweroff_cnt = 0;
FATFS fatfs;
/* Function prototypes -------------------------------------------------------*/
static void H_Close(void);
static void H_Ready(void);
static void H_IN(void);
static void H_OUT(void);
static void H_TriggerB(void);
static void H_TriggerC(void);
static void H_TriggerD(void);
static void H_TriggerE(void);
static void H_TriggerEOff(void);
static void H_BankSwitch(void);
static void H_ColorSwitch(void);
static void H_LowPower(void);
static void H_Recharge(void);
static void H_Charged(void);
static void H_Charging(void);
static void H_PlayerEnter(void);
static void H_PlayerExit(void);
static void H_PlayerStart(void);
static void H_PlayerStop(void);
static void H_PlayerSwitch(void);

static void filesystem_init(void);
static uint8_t key_scan(void);
static uint8_t ask_trigger(uint8_t triggerid);
static void ticktock_trigger(void);
static void move_detected(void);
static uint8_t auto_off(uint32_t *ready_cnt, uint32_t *poweroff_cnt);
static uint16_t GetVoltage(void);
extern void MX_FATFS_Init(void);
#define AUTO_CNT_CLEAR() auto_intoready_cnt = 0, auto_poweroff_cnt = 0

void StartDefaultTask(void const *argument)
{
  // 检测是否静音启动
  if (MX_GPIO_IsPress(KEY_USR))
  {
    USR.mute_flag = 1;
  }
  else
  {
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
  MX_GPIO_Enable(true);

#if USE_DEBUG
  __ASM("BKPT 0");
#endif

  H_Ready();

  for (;;)
  {

    Lis3dData lisData;

    // 检测按键上升/下降沿
    uint8_t key_status = key_scan();
    Lis3d_GetData(&lisData);

#if USE_DEBUG
    AFIO_RELEASE();
#endif

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
      int acc_ans;
      /**< Power Key down*/
      if (key_status & 0x04)
      {
        uint16_t timeout = 0;
        uint16_t max = USR.config->Tpoff > USR.config->Tout ? USR.config->Tpoff : USR.config->Tout;
        uint16_t min = USR.config->Tpoff > USR.config->Tout ? USR.config->Tout : USR.config->Tpoff;

        static uint8_t click_cnt = 0;
        static uint32_t tick_stick = 0;

        if (click_cnt == 0)
        {
          tick_stick = osKernelSysTick();
        }

        //Wait for POWER Key rising
        while (!(key_scan() & 0x01) && (timeout < max))
        {
          osDelay(10);
          timeout += 10;
        }
        if (timeout > min)
        {
          if ((USR.config->Tpoff >= USR.config->Tout && USR.config->Tpoff <= timeout) ||
              (USR.config->Tpoff < USR.config->Tout && USR.config->Tout > timeout))
          {
            H_Close();
          }
          else
          {
            if (!Audio_IsSimplePlayIsReady())
            {
              osDelay(10);
              continue;
            }
            H_OUT();
          }
        }
        else
        {
          click_cnt += 1;
        }

        // into Player mode
        if (click_cnt == 3 && (osKernelSysTick() - tick_stick < osKernelSysTickFrequency * 1))
        {
          tick_stick = 0;
          H_PlayerEnter();
        }
      }
      /**< User Key down */
      else if (key_status & 0x08)
      {
        uint16_t timeout = 0;
        uint16_t max = USR.config->Ts_switch;
        while (!(key_scan() & 0x02))
        {
          osDelay(10);
          timeout += 10;

          if (timeout >= max && Audio_IsPlayBankSwitch() == false)
            H_BankSwitch();
        }
      }

      else if ((acc_ans = (int)sqrt(lisData.Dx * lisData.Dx + lisData.Dy * lisData.Dy + lisData.Dz * lisData.Dz) * 8000 / 0x8000) > USR.config->ShakeOutG * 1000)
      {
        log_i("acc:%dmg", acc_ans);
        H_OUT();
      }
    }

    else if (USR.sys_status == System_Running)
    {
      int acc_ans;
      // PowerKey
      if (key_status & 0x04)
      {
        uint16_t timeout = 0;
        while (
            (!((key_status = key_scan()) & 0x01)) //Waiting for PowerKey UP
            && (!(key_status & 0x08))             //Waiting for UserKey  DOWN
            && timeout < USR.config->Tin)         //Waiting for Timeout
        {
          osDelay(10);
          timeout += 10;
        }

        if (timeout < USR.config->Tin && (key_status & 0x08))
        {
          H_ColorSwitch();
        }
        else if (timeout >= USR.config->Tin)
        {
          H_IN();
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
            uint16_t hold = 0;
            H_TriggerE();
            while (!(key_scan() & 0x02))
            {
              osDelay(10);
              hold += 10;
            }

            if (USR.config->LockupHold > 0 && hold < USR.config->LockupHold)
              H_TriggerEOff();
            else
            {
              // triggerE_HoldFlag = true;
              while (!(key_scan() & 0x08))
              {
                osDelay(10);
              }
              H_TriggerEOff();
              while (!(key_scan() & 0x02))
              {
                osDelay(10);
              }
            }
            break;
          }
        }
        if (timeout < USR.config->TEtrigger && ask_trigger(2))
        {
          H_TriggerD();
        }
      }
      else if (sqrt(lisData.Dx * lisData.Dx + lisData.Dy * lisData.Dy + lisData.Dz * lisData.Dz) > USR.config->ShakeInG)
      {
        H_IN();
      }
      move_detected();
    }

    else if (USR.sys_status == System_Player)
    {
      /**< Power Key down*/
      if (key_status & 0x04)
      {
        static uint32_t tick_cnt[3] = {0, 0, 0};

        tick_cnt[0] = tick_cnt[1];
        tick_cnt[1] = tick_cnt[2];
        tick_cnt[2] = osKernelSysTick();

        if (tick_cnt[0] != 0 && (tick_cnt[2] - tick_cnt[0] < osKernelSysTickFrequency))
        {
          H_PlayerExit();
        }
        else
        {
          if (Audio_IsSimplePlayIsReady())
          {
            H_PlayerStart();
          }
          else
          {
            H_PlayerStop();
          }
        }
      }
      /**< User Key down */
      else if (key_status & 0x08)
      {
        H_PlayerSwitch();
      }
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

    power_adc_val = GetVoltage();
#ifndef USE_DEBUG
    /// 电源管理部分
    if (power_adc_val <= STATIC_USR.vol_poweroff)
    {
      H_Recharge();
    }
#endif
    /// 充电检测
    if (MX_GPIO_PlugIn())
    {
      if (STATIC_USR.vol_chargecomplete < power_adc_val)
      {
        H_Charged();
      }
      else
      {
        H_Charging();
      }
    }
    else if (USR.sys_status == System_Charged || USR.sys_status == System_Charging)
    {
      H_Close();
    }

    ///当有按键动作时清空自动关机\待机定时器
    if (key_status != 0)
    {
      AUTO_CNT_CLEAR();
    }

    /// 自动关机\待机定时器 当达到配置文件的计数值后触发动作
    uint8_t t;
    if ((t = auto_off(&auto_intoready_cnt, &auto_poweroff_cnt)) != 0)
    {
      if (t == 1)
      {
        H_IN();
      }
      else if (t == 2)
      {
        H_Close();
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
 * @Para  player_cnt 播放器模式的计时器
 * @Retvl 0-No trigger | 1-转待机 | 2-转关机
 */
static uint8_t auto_off(uint32_t *ready_cnt, uint32_t *poweroff_cnt)
{
  if (USR.sys_status == System_Ready ||
      (USR.sys_status == System_Player && Audio_IsSimplePlayIsReady()))
  {
    *poweroff_cnt += LOOP_DELAY;
    if (*poweroff_cnt >= USR.config->Tautooff && USR.config->Tautooff != 0)
      return 2;
  }
  else if (USR.sys_status == System_Running)
  {
    *ready_cnt += LOOP_DELAY;
    if (*ready_cnt >= USR.config->Tautoin && USR.config->Tautoin != 0)
      return 1;
  }
  return 0;
}

/**
 * @Brief  加速度计检测，当有触发后触发相应动作并清空自动待机定时器
 */
static void move_detected(void)
{
  uint8_t move, click, __move, __click;
  static uint8_t _move = 0, _click = 0;
  __move = Lis3d_isMove();
  __click = Lis3d_isClick();

  if (__move > 0 && _move == 0)
    move = 1;
  else
    move = 0;
  if (__click > 0 && _click == 0)
    click = 1;
  else
    click = 0;
  _move = __move;
  _click = __click;
  //Lis3DH parts
  //TriggerC
  if (click && ask_trigger(1))
  {
    H_TriggerC();
  }
  //TriggerB
  if (move && ask_trigger(0))
  {
    H_TriggerB();
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
    elog_error("FATFS", "mount error:%d", f_err);
    MX_Audio_HWBeep();
  }

  if ((f_err = usr_config_init()) != 0)
  {
    log_e("User Configuration has some problem:%d", f_err);
    MX_Audio_HWBeep();
  }
}

/**
 * @Brief  Scan User Key and then return Active logic
 * @Retval 0x01-PowerKeyUP 0x02-UserKeyUP 0x04-PowerKeyDOWN 0x08-UserKeyDOWN
 */
static uint8_t key_scan(void)
{
  static uint8_t status = 1;
  uint8_t buf = 0, pre_buf;

  if (MX_GPIO_IsPress(KEY_MUX))
  {
    buf |= 0x01;
  }
  if (MX_GPIO_IsPress(KEY_USR))
  {
    buf |= 0x02;
  }
  pre_buf = buf;
  buf ^= status;
  status = pre_buf;

  if (buf & 0x01)
  {
    if (pre_buf & 0x01)
      buf = 0x04;
    else
      buf = 0x01;
  }
  if (buf & 0x02)
  {
    if (pre_buf & 0x02)
      buf = 0x08;
    else
      buf = 0x02;
  }
  return buf;
}

/**
 * @Brief  ask for trigger
 * @Para trigger id Trigger 0~2 -> TriggerB~TriggerD
 * @Retval 1-ok|0-busy
 */
static uint8_t ask_trigger(uint8_t triggerid)
{
  if (!frozen_trigger_cnt[triggerid])
  {
    switch (triggerid)
    {
    case 0:
      frozen_trigger_cnt[0] = (USR.config->TBfreeze > INT16_MAX ? INT16_MAX : USR.config->TBfreeze);
      break;
    case 1:
      frozen_trigger_cnt[1] = (USR.config->TCfreeze > INT16_MAX ? INT16_MAX : USR.config->TCfreeze);
      break;
    case 2:
      frozen_trigger_cnt[2] = (USR.config->TDfreeze > INT16_MAX ? INT16_MAX : USR.config->TDfreeze);
      break;
    }
    return 1;
  }
  return 0;
}

/**
 * @Brief  Update trigger's frozen time, put it in MainTask's loop.
 */
static void ticktock_trigger(void)
{
  for (uint8_t i = 0; i < 3; i++)
  {
    if (frozen_trigger_cnt[i] > 0)
    {
      frozen_trigger_cnt[i] -= LOOP_DELAY;
    }
    if (frozen_trigger_cnt[i] < 0)
    {
      frozen_trigger_cnt[i] = 0;
    }
  }
}

static uint16_t GetVoltage(void)
{
  static uint16_t val = 0;
  uint16_t buf = BatteryGet();
  int16_t div = buf - val;

  if ((div > 0 ? div : -div) >= 50)
  {
    val += div / 3;
  }
  else
  {
    val = buf;
  }

  return val;
}

static void H_Close(void)
{
  log_v("System going to close");
  SimpleLED_ChangeStatus(SIMPLELED_STATUS_SLEEP); // 小型LED进入休眠模式
  USR.sys_status = System_Close;
  Audio_Play_Start(Audio_PowerOff);
  osDelay(3000); //3s
  MX_GPIO_Enable(false);
}
static void H_Ready(void)
{
  // 初始化结构体中的参数
  USR.sys_status = System_Restart;
  USR.bank_now = 0;
  USR.bank_color = 0;

  // 更新LED用到的变量
  // LED_Bank_Update(&USR);

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
  BatteryStart();
  {
    uint32_t cnt = 10;
    while (cnt--)
    {
      power_adc_val = GetVoltage();
      osDelay(10);
    }
  }
#ifndef USE_DEBUG
  // 低电压检测：忽略静音标志发送2次低电压报警
  if (power_adc_val <= STATIC_USR.vol_warning)
  {
    H_LowPower();
  }
#endif

  Audio_Play_Start(Audio_Boot);
  osDelay(100);
  SimpleLED_Init();
  SimpleLED_ChangeStatus(SIMPLELED_STATUS_STANDBY);
  osTimerStart(SimpleLEDHandle, 10);

  USR.sys_status = System_Ready;
}
static void H_IN(void)
{
  log_v("System going to ready");
  SimpleLED_ChangeStatus(SIMPLELED_STATUS_STANDBY);
  USR.sys_status = System_Ready;
  LED_Start_Trigger(LED_Trigger_Stop);
  if (USR.config->Direction == 0)
  {
    Audio_Play_Start(Audio_intoReady);
  }
  else
  {
    Lis3dData data;
    Lis3d_GetData(&data);
    if (data.Dx < 0)
      data.Dx = -data.Dx;
    if (data.Dy < 0)
      data.Dy = -data.Dy;
    if (data.Dz < 0)
      data.Dz = -data.Dz;
    Audio_ID_t id = Audio_intoRunning_X;
    int16_t max = data.Dx;
    if (max < data.Dy)
    {
      max = data.Dy;
      id = Audio_intoRunning_Y;
    }
    if (max < data.Dz)
    {
      max = data.Dz;
      id = Audio_intoRunning_Z;
    }
    Audio_Play_Start(id);
  }

  USR.bank_color = 0;
  // LED_Bank_Update(&USR);
}
static void H_OUT(void)
{
  log_v("System going to running");
  auto_intoready_cnt = 0;
  SimpleLED_ChangeStatus(SIMPLELED_STATUS_ON);
  USR.sys_status = System_Running;
  // LED_Bank_Update(&USR);
  // Audio_Play_Start(Audio_intoRunning);
  if (USR.config->Direction == 0)
  {
    Audio_Play_Start(Audio_intoReady);
  }
  else
  {
    Lis3dData data;
    Lis3d_GetData(&data);
    if (data.Dx < 0)
      data.Dx = -data.Dx;
    if (data.Dy < 0)
      data.Dy = -data.Dy;
    if (data.Dz < 0)
      data.Dz = -data.Dz;
    int16_t max = data.Dx;
    Audio_ID_t id = Audio_intoRunning_X;
    if (max < data.Dy)
    {
      max = data.Dy;
      id = Audio_intoRunning_Y;
    }
    if (max < data.Dz)
    {
      max = data.Dy;
      id = Audio_intoRunning_Z;
    }
    Audio_Play_Start(id);
  }
  LED_Start_Trigger(LED_Trigger_Start);
}
static void H_TriggerB(void)
{
  LED_Start_Trigger(LED_TriggerB);
  Audio_Play_Start(Audio_TriggerB);
  AUTO_CNT_CLEAR();
}
static void H_TriggerC(void)
{
  SimpleLED_ChangeStatus(SIMPLELED_STATUS_CLASH);
  LED_Start_Trigger(LED_TriggerC);
  Audio_Play_Start(Audio_TriggerC);
  AUTO_CNT_CLEAR();
}
static void H_TriggerD(void)
{
  log_v("Trigger D");
  LED_Start_Trigger(LED_TriggerD);
  Audio_Play_Start(Audio_TriggerD);
}
static void H_TriggerE(void)
{
  log_v("Trigger E");
  SimpleLED_ChangeStatus(SIMPLELED_STATUS_LOCKUP);
  LED_Start_Trigger(LED_TriggerE);
  Audio_Play_Start(Audio_TriggerE);
}
static void H_TriggerEOff(void)
{
  log_v("Trigger E END");
  SimpleLED_ChangeStatus(SIMPLELED_STATUS_ON);
  LED_Start_Trigger(LED_TriggerE_END);
  Audio_Play_Stop(Audio_TriggerE);
}
static void H_LowPower(void)
{
  uint8_t buf = USR.mute_flag;
  USR.mute_flag = 0;
  Audio_Play_Start(Audio_LowPower);
  Audio_Play_Start(Audio_LowPower);
  USR.mute_flag = buf;
}
static void H_Recharge(void)
{
  log_v("System should be Power off");
  SimpleLED_ChangeStatus(SIMPLELED_STATUS_SLEEP); // 小型LED进入休眠模式
  USR.sys_status = System_Close;
  Audio_Play_Start(Audio_Recharge);
  osDelay(2000);
  MX_GPIO_Enable(false);
  while (1)
    ;
}
static void H_Charged(void)
{
  SimpleLED_ChangeStatus(SIMPLELED_STATUS_STANDBY);
  USR.sys_status = System_Charged;
}
static void H_Charging(void)
{
  SimpleLED_ChangeStatus(SIMPLELED_STATUS_STANDBY);
  USR.sys_status = System_Charging;
}
static void H_BankSwitch(void)
{
  log_v("System Bank Switch");
  USR.bank_now += 1;
  USR.bank_now %= USR.nBank;
  USR.config = USR._config + USR.bank_now;
  // LED_Bank_Update(&USR);
  usr_config_update();
  SimpleLED_ChangeStatus(SIMPLELED_STATUS_STANDBY);
  Audio_Play_Start(Audio_BankSwitch);
}
static void H_ColorSwitch(void)
{
  log_v("System ColorSwitch");
  USR.bank_color += 1;
  // LED_Bank_Update(&USR);
  Audio_Play_Start(Audio_ColorSwitch);
}
static void H_PlayerEnter(void)
{
  log_v("player enter");
  Audio_Play_Start(Audio_Player_Enter);
  USR.sys_status = System_Player;
}
static void H_PlayerExit(void)
{
  log_v("player exit");
  Audio_Play_Start(Audio_Player_Exit);
  USR.sys_status = System_Ready;
}
static void H_PlayerStart(void)
{
  log_v("player start");
  Audio_Play_Start(Audio_Player_Start);
}
static void H_PlayerStop(void)
{
  log_v("player stop");
  Audio_Play_Start(Audio_Player_Stop);
}
static void H_PlayerSwitch(void)
{
  log_v("player switch");
  Audio_Play_Start(Audio_Player_Switch);
}
