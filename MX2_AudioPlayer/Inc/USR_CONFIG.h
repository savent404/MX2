#ifndef USR_CONFIG_H_
#define USR_CONFIG_H_

#include <stdio.h>
#include <stdint.h>

typedef enum {
  System_Restart,
  System_Running,
  System_Ready,
  System_Charging,
  System_Charged,
  System_Close
} System_t;


typedef struct _usr_config_structure {
  uint8_t   Vol;       //音量设置(0无声，1小声，2中等，3大声)

  uint16_t  Tpon;      //关机转待机，Main按钮延时触发值，单位ms,
  uint16_t  Tpoff;     //待机转关机，Main按钮延时触发值，单位ms,
  uint16_t  Tout;      //待机转运行，Main按钮延时触发值，单位ms,
  uint16_t  Tin;       //运行转待机，Main按钮延时触发值，单位ms,
  uint16_t  Ts_switch; //切换LED色彩配置以及音频的AUX按钮延时触发值，单位ms,
  uint32_t  Tautoin;   //运行状态下，加速度计以及按钮连续无触发计时，到达则自动转待机，0表示不自动转待机，单位ms,
  uint32_t  Tautooff;  //待机状态下，按钮连续无动作计时触发关机，0表示不自动关机，单位ms,

  uint16_t  Tmute;     //关机状态下，进入静音模式的双键复合触发时长
  uint16_t  TLcolor;   //开机状态下，LEDbank变更的双键复合触发时长
                       //若TLcolor>TEtrigger,或TLcolor>Tin,则此参数无效

  uint16_t  TBfreeze;  //触发B重复触发间隔，单位ms
  uint8_t   TBMode;    //TriggerLMode,可复选

  uint16_t  TCfreeze;  //触发C重复触发间隔，单位ms
  uint8_t   TCMode;    // TriggerLMode,可复选，单次触发只随机执行一种

  uint16_t  TDfreeze;  //触发D重复触发间隔，单位ms
  uint8_t   TDMode;    // TriggerLMode,可复选，单次触发只随机执行一种

  uint16_t  TEtrigger; //开机状态下，触发E的AUX按钮触发时长，单位ms
  uint8_t   TEMode;    // TriggerLMode,可复选

  uint16_t  TLon;      // LED从起辉到达到设定全局亮度的时间，单位ms
  uint16_t  TLoff;     // LED从全局亮度到熄灭的时间，单位ms
  uint16_t  Lbright;   // LED全局亮度，范围0~1023
  uint16_t  Ldeep;     // LED全局亮度下探值，范围0~1023，当LDeep>Lbright时，此值归0
  uint16_t  LMode;     //开机运行时LED基础工作模式，详见Config
  uint16_t  MD;
  uint16_t  MT;
  uint16_t  CD;
  uint16_t  CT;
  uint16_t  CL;
  uint16_t  CW;
  uint16_t SimpleLED_MASK;   /**< 小型LED掩码，标志1允许亮，标志0则不亮，LSB代表LED0 */
  uint16_t ChDelay[4];

  uint16_t T_Breath; //LMode 呼吸延时

  uint16_t Out_Delay; // 触发Out时， 延时Hum播放时间
} USR_CONFIG_t;

// 小型LED动作
typedef struct _simple_led {
  uint16_t Delay;
  uint8_t Num;
  uint8_t *Action;
} SimpleLED_Acction_t;

typedef struct _accent_structure {
  SimpleLED_Acction_t *Standby;
  SimpleLED_Acction_t *On;
  SimpleLED_Acction_t *Lockup;
  SimpleLED_Acction_t *Clash;
} Accent_t;

typedef struct _file_number_limits {
  uint16_t bank_max;
  uint16_t trigger_in_max;
  uint16_t trigger_out_max;
  uint16_t trigger_B_max;
  uint16_t trigger_C_max;
  uint16_t trigger_D_max;
  uint16_t trigger_E_max;
} File_NumberLimits_t;

typedef struct _usr_static_parameter {
  uint16_t vol_warning;
  uint16_t vol_poweroff;
  uint16_t vol_chargecomplete;
  File_NumberLimits_t filelimits;
} PARA_STATIC_t;

typedef struct _usr_trigger_path {
  uint16_t number;
  char *path_arry;
} TRIGGER_PATH_t;

typedef uint32_t HumSize_t;

typedef struct _usr_dynamic_parameter {
  uint8_t nBank;            /**< 定义SD卡中存在的Bank数 */
  HumSize_t *humsize;       /**< 定义每个循环音频文件有效数据量 */
  uint32_t *BankColor;      /**< 定义每个Bank的颜色，Bank色(2*uint32_t)+FBank色(2*uint32_t) */
  TRIGGER_PATH_t *triggerB; /**< 定义每个Bank中TriggerB的信息 */
  TRIGGER_PATH_t *triggerC; /**< 定义每个Bank中TriggerC的信息 */
  TRIGGER_PATH_t *triggerD; /**< 定义每个Bank中TriggerD的信息 */
  TRIGGER_PATH_t *triggerE; /**< 定义每个Bank中TriggerE的信息 */
  TRIGGER_PATH_t *triggerIn;
  TRIGGER_PATH_t *triggerOut;
  Accent_t *accent;         /**< 简单LEDAccent配置信息 */
  USR_CONFIG_t backUpConfig;
  USR_CONFIG_t *config;     /**< 用户config文本配置信息 */
  USR_CONFIG_t *_config;    /**< 各Bank的配置信息 */
  System_t sys_status;      /**< 系统状态 */
  uint8_t bank_now;         /**< 当前Bank值，包括0 */
  uint8_t bank_color;       /**< 当前LED使用的Bank(相对位移,需与Bank_now一起使用) */
  uint8_t mute_flag:1;      /**< 静音标志位(1Bit) */
  uint8_t audio_busy:1;     /**< 音频输出标志 */
  
  #ifdef USE_DEBUG
  /// Analyze Paramter
  uint16_t Stack_Free[4];
  #endif
} PARA_DYNAMIC_t;

extern PARA_DYNAMIC_t USR;
extern const PARA_STATIC_t STATIC_USR;
uint8_t usr_config_init(void);
uint8_t usr_switch_bank(int dest, int freshFlag);
uint8_t usr_init_bank(int bankPos, int storagePos);
uint8_t usr_update_triggerPah(int bankPos);
#define TRIGGER_MAX_NUM(x) (STATIC_USR.filelimits.trigger_##x##_max)

#endif
