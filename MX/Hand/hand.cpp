#include "hand.h"

static MX_HAND_Instance_t instance;

/**
 * @brief hasMainPower 检测某个浮点数是否是该数组中的绝对值最大值
 * @param pdata 浮点数数组
 * @param index 指定检测的最大值的标号
 * @param maxNum 数组大小
 * @return 是否绝对值最大值
 */
inline static bool hasMainPower(float* pdata, int index, int maxNum)
{
    for (int i = 0; i < maxNum; i++) {
        if (index == i)
            continue;
        if (fabsf(pdata[i] >= fabsf(pdata[index])))
            return false;
    }
    return true;
}

bool MX_HAND_Init(void)
{
    MX_HAND_HW_Init();
    
    instance.stab_threshold[0] = 2.0f;
    instance.stab_threshold[1] = 2.0f;
    instance.stab_window = 50;

    instance.spin_threshold[0] = 50;
    instance.spin_threshold[1] = 50;
    instance.spin_min_circle_time = 300;
    instance.spin_min_counter = 4;
    instance.spin_now_circle = 0;
    instance.spin_now_counter = 0;

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
    
    float gyro[3];
    float acc[3];
    float Scalar_acc = 0, Scalar_gyro = 0, Scalar_gyro_yandz = 0;

    /** get acc&gyro data */
    if (!MX_HAND_HW_getData(acc, gyro)) {
        a.unio.isNoData = true;
        return a;
    }

    /** measure scalar */
    for (int i = 0; i < 3; i++) {
        Scalar_acc += acc[i] * acc[i];
        Scalar_gyro += gyro[i] * gyro[i];
        if (i != 0)
            Scalar_gyro_yandz += gyro[i] * gyro[i];
    }
    Scalar_acc = sqrtf(Scalar_acc) - 1.0f;
    Scalar_gyro = sqrtf(Scalar_gyro);
    Scalar_gyro_yandz = sqrtf(Scalar_gyro_yandz);

    if (MX_HAND_isSwing(&instance, timestamp)) {
        a.unio.isSwing = true;
    }
    if (MX_HAND_isClash(&instance, timestamp)) {
        a.unio.isClash = true;
    }

    bool statusIsSpin = instance.bActive[HAND_ID_Spin];
    bool isSpin = MX_HAND_isSpin(&instance, timestamp, Scalar_gyro_yandz);
    int circleCount = instance.spin_now_circle;
    if (isSpin) {
        a.unio.spinStatus = HAND_TriggerId_t::unioStruct::spinStatus::spin_start;
        if (statusIsSpin && circleCount < instance.spin_now_circle)
            a.unio.spinStatus = HAND_TriggerId_t::unioStruct::spinStatus::spin_circle;
    }

    if (MX_HAND_isStab(&instance, timestamp, acc, Scalar_acc, Scalar_gyro)) {
        a.unio.isStab = true;
    }

    return a;
}

bool MX_HAND_isSpin(MX_HAND_Instance_t* p, uint32_t time, float scalar)
{
    // get last time first
    uint32_t LT_startSwing = time - p->u32TimeStamp[HAND_ID_Swing];
    uint32_t LT_startSpin = time - p->u32TimeStamp[HAND_ID_Spin];
    float quadraCircleTime = p->spin_min_circle_time / 4.0f;

    // increase spin quadra circle counter
    if (p->bActive[HAND_ID_Swing] &&
        LT_startSwing - quadraCircleTime*p->spin_min_counter > 90.0f) {
        p->spin_now_counter++;
    }

    // check start condition
    if (p->bActive[HAND_ID_Spin] == false &&
        p->spin_now_counter >= p->spin_min_counter &&
        scalar > p->spin_threshold[0] &&
        LT_startSwing > p->spin_min_circle_time / scalar) {
        p->bActive[HAND_ID_Spin] = true;
    }
    // check if increase a circle counter or end condition
    else if (p->bActive[HAND_ID_Spin] == true) {
        uint32_t minimalCircleTime = p->spin_min_circle_time * (p->spin_now_circle + 1);
        if (scalar < p->spin_threshold[1]) {
            p->bActive[HAND_ID_Spin] = false;
            p->spin_now_counter = 1; // speed on to re-active spin
            p->spin_now_circle = 0;
        }
        else if (LT_startSpin > minimalCircleTime) {
            p->spin_now_circle++;
        }
    }
    
    // disable 're-active speed on' if no more spin for a long time
    if (p->bActive[HAND_ID_Swing] == 0 &&
        p->bActive[HAND_ID_Spin] == false &&
        LT_startSwing > p->spin_min_circle_time*4) {
        p->spin_now_counter = 0;
    }

    return p->bActive[HAND_ID_Spin];
}

bool MX_HAND_isStab(MX_HAND_Instance_t* p, uint32_t time, float acc[3], float scalar_acc, float scalar_gyro)
{
    if (!p->bActive[HAND_ID_Stab] &&
        scalar_acc > p->stab_threshold[0] &&
        hasMainPower(acc, 0, 3)) {
        p->bActive[HAND_ID_Stab] = true;
        p->u32TimeStamp[HAND_ID_Stab] = time;
    }
    else if (p->bActive[HAND_ID_Stab]) {
        if (scalar_gyro > 300.0f) {
            p->bActive[HAND_ID_Stab] = false;
            return false;
        }
        if (scalar_acc < p->stab_threshold[1]) {
            p->bActive[HAND_ID_Stab] = false;
            if (time - p->u32TimeStamp[HAND_ID_Stab] > p->stab_window) {
                return true;
            }
        }
    }
    return false;
}

bool MX_HAND_isSwing(MX_HAND_Instance_t* p, uint32_t time)
{
    bool isSwing = false;

    if (isSwing && !p->bActive[HAND_ID_Swing]) {
        p->bActive[HAND_ID_Swing] = true;
        p->u32TimeStamp[HAND_ID_Swing] = time;
    }

    if (!isSwing && p->bActive[HAND_ID_Swing]) {
        p->bActive[HAND_ID_Swing] = false;
    }
    return p->bActive[HAND_ID_Swing];
}

bool MX_HAND_isClash(MX_HAND_Instance_t* p, uint32_t time)
{
    bool isSwing = false;

    if (isSwing && !p->bActive[HAND_ID_Clash]) {
        p->bActive[HAND_ID_Clash] = true;
        p->u32TimeStamp[HAND_ID_Clash] = time;
    }

    if (!isSwing && p->bActive[HAND_ID_Clash]) {
        p->bActive[HAND_ID_Clash] = false;
    }
    return p->bActive[HAND_ID_Clash];
}

