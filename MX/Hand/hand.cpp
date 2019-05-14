#include "hand.h"
#include "PARAM_def.h"
#include "debug.h"

static float              stashed_gyro[ 3 ];
static MX_HAND_Instance_t instance;

/**
 * @brief hasMainPower 检测某个轴（如x）是否具有绝大部分的分量
 * @param x 单轴分量
 * @param scalar 整体值
 * @return 是否占有绝大部分值
 * @note 按照方向半径45°判断是否为绝大部分分量
 */
inline static bool hasMainPower(const float x, const float scalar)
{
    return fabs(x) * 2.0f > fabs(scalar);
}

bool MX_HAND_Init(void)
{
    extern PARA_DYNAMIC_t USR;

    MX_HAND_HW_Init();

    // default for stab paramter
    instance.stab_threshold[ 0 ] = 2.0f;
    instance.stab_threshold[ 1 ] = 2.0f;
    instance.stab_window         = 3;
    instance.stab_gyroThreshold  = 360;

    // set from user's parameter
    instance.stab_threshold[ 0 ] = float(USR.config->StabThreshold / 1000.0f);
    instance.stab_threshold[ 1 ] = float(USR.config->StabThreshold / 1000.0f);
    instance.stab_window         = USR.config->StabWindow;

    instance.spin_threshold[ 0 ]  = 50;
    instance.spin_threshold[ 1 ]  = 50;
    instance.spin_min_circle_time = 300;
    instance.spin_min_counter     = 4;
    instance.spin_now_circle      = 0;
    instance.spin_now_counter     = 0;

    memset(instance.bActive, 0, sizeof(instance.bActive));
    memset(instance.u32TimeStamp, 0, sizeof(instance.u32TimeStamp));
    return true;
}

bool MX_HAND_DeInit(void)
{
    MX_HAND_HW_DeInit();
    return true;
}

HAND_TriggerId_t MX_HAND_GetTrigger(uint32_t timestamp)
{
    HAND_TriggerId_t a;
    a.hex = 0;

    float gyro[ 3 ];
    float acc[ 3 ];
    float Scalar_acc = 0, Scalar_gyro = 0, Scalar_gyro_yandz = 0;

    /* get first:
     * swing
     * clash
     */
    if (MX_HAND_isSwing(&instance, timestamp)) {
        a.unio.isSwing = true;
    }
    if (MX_HAND_isClash(&instance, timestamp)) {
        a.unio.isClash = true;
    }
   
    /** try to get acc&gyro data */
    if (!MX_HAND_HW_getData(acc, gyro)) {
        if (a.hex == 0)
            a.unio.isNoData = true;
        return a;
    }

    // storage gyro
    memcpy(stashed_gyro, gyro, sizeof(stashed_gyro));

    /** measure scalar */
    for (int i = 0; i < 3; i++) {
        Scalar_acc += acc[ i ] * acc[ i ];
        Scalar_gyro += gyro[ i ] * gyro[ i ];
        if (i != 0)
            Scalar_gyro_yandz += gyro[ i ] * gyro[ i ];
    }
    Scalar_acc        = sqrtf(Scalar_acc) - 1.0f;
    Scalar_gyro       = sqrtf(Scalar_gyro);
    Scalar_gyro_yandz = sqrtf(Scalar_gyro_yandz);
    bool statusIsSpin = instance.bActive[ HAND_ID_Spin ];
    bool isSpin       = MX_HAND_isSpin(&instance, timestamp, Scalar_gyro_yandz);
    int  circleCount  = instance.spin_now_circle;
    if (isSpin) {
        a.unio.spinStatus = HAND_TriggerId_t::unioStruct::spinStatus::spin_start;
        if (statusIsSpin && circleCount < instance.spin_now_circle)
            a.unio.spinStatus = HAND_TriggerId_t::unioStruct::spinStatus::spin_circle;
    }

    if (MX_HAND_isStab(&instance, timestamp, acc, Scalar_acc, Scalar_gyro)) {
        DEBUG(5, "triggered a stab, acc:%.2f\t%.2f\t%.2f",
              acc[ 0 ],
              acc[ 1 ],
              acc[ 2 ]);
        a.unio.isStab = true;
    }

    return a;
}

bool MX_HAND_isSpin(MX_HAND_Instance_t* p, uint32_t time, float scalar)
{
    // get last time first
    uint32_t LT_startSwing    = time - p->u32TimeStamp[ HAND_ID_Swing ];
    uint32_t LT_startSpin     = time - p->u32TimeStamp[ HAND_ID_Spin ];
    float    quadraCircleTime = p->spin_min_circle_time / 4.0f;

    // increase spin quadra circle counter
    if (p->bActive[ HAND_ID_Swing ] && LT_startSwing - quadraCircleTime * p->spin_min_counter > 90.0f) {
        p->spin_now_counter++;
    }

    // check start condition
    if (p->bActive[ HAND_ID_Spin ] == false && p->spin_now_counter >= p->spin_min_counter && scalar > p->spin_threshold[ 0 ] && LT_startSwing > p->spin_min_circle_time / scalar) {
        p->bActive[ HAND_ID_Spin ] = true;
    }
    // check if increase a circle counter or end condition
    else if (p->bActive[ HAND_ID_Spin ] == true) {
        uint32_t minimalCircleTime = p->spin_min_circle_time * (p->spin_now_circle + 1);
        if (scalar < p->spin_threshold[ 1 ]) {
            p->bActive[ HAND_ID_Spin ] = false;
            p->spin_now_counter        = 1; // speed on to re-active spin
            p->spin_now_circle         = 0;
        } else if (LT_startSpin > minimalCircleTime) {
            p->spin_now_circle++;
        }
    }

    // disable 're-active speed on' if no more spin for a long time
    if (p->bActive[ HAND_ID_Swing ] == 0 && p->bActive[ HAND_ID_Spin ] == false && LT_startSwing > p->spin_min_circle_time * 4) {
        p->spin_now_counter = 0;
    }

    return p->bActive[ HAND_ID_Spin ];
}

bool MX_HAND_isStab(MX_HAND_Instance_t* p, uint32_t time, float acc[ 3 ], float scalar_acc, float scalar_gyro)
{
    if (!p->bActive[ HAND_ID_Stab ] && scalar_acc > p->stab_threshold[ 0 ] && hasMainPower(acc[ 0 ], scalar_acc)) {
        p->bActive[ HAND_ID_Stab ]      = true;
        p->u32TimeStamp[ HAND_ID_Stab ] = time;
    } else if (p->bActive[ HAND_ID_Stab ]) {
        if (scalar_gyro > p->stab_gyroThreshold) {
            p->bActive[ HAND_ID_Stab ] = false;
            return false;
        }
        if (scalar_acc < p->stab_threshold[ 1 ] || time - p->u32TimeStamp[ HAND_ID_Stab ]) {
            p->bActive[ HAND_ID_Stab ] = false;
            if (time - p->u32TimeStamp[ HAND_ID_Stab ] > p->stab_window) {
                return true;
            }
        }
    }
    return false;
}

bool MX_HAND_isSwing(MX_HAND_Instance_t* p, uint32_t time)
{
    (void)p;
    (void)time;
    return MX_HAND_HW_isSwing();
    /*
    bool isSwing = false;

    if (isSwing && !p->bActive[ HAND_ID_Swing ]) {
        p->bActive[ HAND_ID_Swing ]      = true;
        p->u32TimeStamp[ HAND_ID_Swing ] = time;
    }

    if (!isSwing && p->bActive[ HAND_ID_Swing ]) {
        p->bActive[ HAND_ID_Swing ] = false;
    }
    return p->bActive[ HAND_ID_Swing ];
    */
}

bool MX_HAND_isClash(MX_HAND_Instance_t* p, uint32_t time)
{
    (void)p;
    (void)time;
    return MX_HAND_HW_isClash();
    /*
    bool isSwing = false;

    if (isSwing && !p->bActive[ HAND_ID_Clash ]) {
        p->bActive[ HAND_ID_Clash ]      = true;
        p->u32TimeStamp[ HAND_ID_Clash ] = time;
    }

    if (!isSwing && p->bActive[ HAND_ID_Clash ]) {
        p->bActive[ HAND_ID_Clash ] = false;
    }
    return p->bActive[ HAND_ID_Clash ];
    */
}

float MX_HAND_GetScalarGyro(void)
{
    float gyro[ 3 ];
    float ans = 0;

    /*
    while (!MX_HAND_HW_getData(acc, gyro)) {

    }
    */
    memcpy(gyro, stashed_gyro, sizeof(gyro));

    for (int i = 0; i < 3; i++) {
        if (i != 0) {
            ans += gyro[ i ] * gyro[ i ];
        }
    }

    return sqrtf(ans);
}
