#ifndef HW_CONFIG_H_
#define HW_CONFIG_H_

#include "MX_def.h"

#define TF_DMADISABLE       (1)

#ifndef USE_NP
#define USE_NP              (0)
#endif

#if __GNUC__
#define APPMODE             (0)
#else
#define APPMODE             (1)
#endif

#if USE_NP == 1
#define ENABLE_WS2811

#define NP_LEDNUM           (118)

#define NP_TIM_PERIOD       (149)
#define NP_TIM_WSCNT0       (48)
#define NP_TIM_WSCNT1       (102)
#define NP_TIM_SKCNT0       (36)
#define NP_TIM_SKCNT1       (72)

//#define USE_SK              (1)
#define USE_WS              (1)


#if USE_WS
#define NP_DAT0             NP_TIM_WSCNT0
#define NP_DAT1             NP_TIM_WSCNT1
#elif USE_SK
#define NP_DAT0             NP_TIM_SKCNT0
#define NP_DAT1             NP_TIM_SKCNT1
#endif

#endif

#define COLOR_MAX           (16)

#define CHG_DIVIDER_RATIO   (3)
#define BAT_DIVIDER_RATIO   (2)

//set the voltage supply to MCU
//if the ADC get the value lower than BOARD_VOL_VALUE-BOARD_VOL_THRE,
//the board is abnormal
#define BOARD_VOL_VALUE     (2800)
#define BOARD_VOL_THRE      (90)

//the step in ADC software trigger mode when check whether convertion completed
//unit in ms
#define ADC_CHECK_STEP      (5)
#define ADC_CONVET_STEP     (25)

//date format:
//BYTE[0] Valid Code, when power on, erase to 0xFF; when power off, write to 0x5A
//when power up with 0x5A, mean the configuration is valid
//BYTE[1] Bank Number
//BYTE[2] Current Bank
//BYTE[3] NULL
//BYTE[4-7] Bank1 Color
//BYTE[8-11] Bank2 Color
//......
#define CFG_STORAGE_BASEADDR    (0x0807F000)
#define CFG_STORAGE_VALIDADDR   CFG_STORAGE_BASEADDR
#define CFG_STORAGE_TOTALBANK   (0x0807F001)
#define CFG_STORAGE_CURRENTBANK (0x0807F002)
#define CFG_STORAGE_BCBASE      (0x0807F004)

#define CFG_STORAGE_VALIDCODE   (0x5A)

//use to storage the color table
//date format
//uint16 uint16 uint16 uint16 uint16 uint16 uint16 uint16
//R      G      B      L      L1     L2     L3     L4
#define COLOR_TABLE_ADDR        (0x0807F800)

#define FORCE_PWROFF_TIMEOUT    (10000)

#define ENABLECONSOLE (0)

#endif
