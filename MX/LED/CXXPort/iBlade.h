#pragma once

#include "MX_def.h"
#include "color.hpp"
#include "flame.hpp"
#include "iBlade_ll.hpp"
#include "iBlade_sets.hpp"
#include "iBlade_ulti.hpp"
#include "randomWave.hpp"

/**
 * @brief restore info at [x]_backup
 */
#define __LED_PUSH(x) ((x##_backup) = (x))
#define __LED_POP(x) ((x) = (x##_backup))
#define __LED_APPLE(x) ((x) = (x##_ready))
#define __LED_STASH(x) ((x##_ready) = (x))
#define __LED_APPLE_CLEAR(x) ((x##_ready) = (x##_backup))

/**
 * @brief def a var with backup
 */
#define DEF_WITH_BACKUP(x) x, x##_backup, x##_ready

class iBlade : public iBladeDriver, public iBladeParam {
public:
    iBlade(size_t num);
    ~iBlade();
    bool parameterUpdate(void* arg, bool needBlock);
    void handle(void* arg);

    virtual void update();

protected:
    /**
     * @brief 处理绘画事务
     * @param Param 结构体
     */
    void handleLoop(void* arg);
    /**
     * @brief 处理触发事件
     * @param osEvent 消息事件
     */
    void handleTrigger(const void* arg);

    friend void updateBG(iBlade&, int16_t*);
    friend void updateTG(iBlade&, int16_t*);
    friend void updateFT(iBlade&, int16_t*);
    friend void applyStashSets(iBlade&);
    friend void stashSets(iBlade&);

private:
    FlameBase_t* pFlame;
    inline void  flip_switchColor(int mode)
    {
        switch (mode) {
        case 1:
            MC = TC;
            break;
        case 2:
            SC = TC;
            break;
        case 3:
            MC = TC;
            SC = TC;
            break;
        case 4:
            MC = SC_backup;
            SC = MC_backup;
            break;
        case 5: {
            // use flipColors API
        } break;
        }
        if (helper_getFlameMode() && pFlame) {
            pFlame->initColor(MC, SC);
        }
    }
    inline void flip_switchColor_callback(int mode)
    {
        switch (mode) {
        default:
            popColors();
            if (helper_getFlameMode() && pFlame) {
                pFlame->initColor(MC, SC);
            }
        }
    }
    inline int helper_getFlameMode()
    {
        switch (modeL1) {
        case modeL1_t::Chaos:
            return 1;
        case modeL1_t::Flame:
            return 2;
        default:
            return 0;
        }
    }

    /**
     * @name 运行时参数
     * @{ */
    enum { idle,
           out,
           in,
           Run,
           InTrigger } status;

private:
    /** @} */

    void setNormalParam(void);
    void setBackGroudParam(modeL1_t mode);
    void setTriggerParam(modeL2_t mode);
    void setFilterParam(modeL3_t mode);

private:
    void backGroundRender(void);
    void clearL1(void);
    void clearL2(void);
    void clearL3(void);
    void clearProcess(void);
};
