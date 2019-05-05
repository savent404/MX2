#pragma once

#include "MX_def.h"
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

typedef enum {
    HAND_ID_Swing = 0,
    HAND_ID_Clash = 1,
    HAND_ID_Stab  = 2,
    HAND_ID_Spin  = 3,
    HAND_ID_MAXNUM
} HAND_Id_t;

typedef union {
    uint32_t hex;
    struct unioStruct {
        enum spinStatus {
            spin_stoped = 0,
            spin_start  = 1,
            spin_circle = 2,
        } spinStatus : 2;
        bool isStab : 1;
        bool isSwing : 1;
        bool isClash : 1;
        bool isNoData : 1;
    } unio;
} HAND_TriggerId_t;

typedef struct {
    float    stab_threshold[ 2 ];
    uint32_t stab_window;

    // 超过n个四分之1周期的旋转触发spin
    int spin_min_counter;
    // 整周期的最小时间(挥动一周)
    int   spin_min_circle_time;
    float spin_threshold[ 2 ];

    /** Run time vars */
    // 触发状态标志
    bool     bActive[ HAND_ID_MAXNUM ];
    uint32_t u32TimeStamp[ HAND_ID_MAXNUM ];
    // 记录当前旋转了n个四分之1周期
    int spin_now_counter;
    int spin_now_circle;
} MX_HAND_Instance_t;

MX_C_API bool MX_HAND_Init(void);
MX_C_API bool MX_HAND_DeInit(void);
MX_C_API HAND_TriggerId_t MX_HAND_GetTrigger(uint32_t timestamp);
MX_C_API float            MX_HAND_GetScalarGyro(void);

/**
 * @brief 初始化硬件(包括内部参数)
 */
MX_PORT_API void MX_HAND_HW_Init(void);
MX_PORT_API void MX_HAND_HW_DeInit(void);

/**
 * @brief 获取加速度值
 * 
 * @param[out] acc 单位为g/s
 * @param[out] gyro  单位°/s
 * @return 是否获取成功
 */
MX_PORT_API bool MX_HAND_HW_getData(float acc[ 3 ], float gyro[ 3 ]);
MX_PORT_API bool MX_HAND_HW_isSwing(void);
MX_PORT_API bool MX_HAND_HW_isClash(void);

MX_INTERNAL_API bool MX_HAND_isSwing(MX_HAND_Instance_t*, uint32_t time);
MX_INTERNAL_API bool MX_HAND_isClash(MX_HAND_Instance_t*, uint32_t time);
MX_INTERNAL_API bool MX_HAND_isStab(MX_HAND_Instance_t*, uint32_t time, float acc[ 3 ], float scalarA, float scalarG);
MX_INTERNAL_API bool MX_HAND_isSpin(MX_HAND_Instance_t*, uint32_t time, float scalarGwithoutZ);
