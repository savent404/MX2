#pragma once

#include "iBlade_ll.hpp"
#include "iBlade_ulti.hpp"
#include "color.hpp"
#include "flame.hpp"

#define MX_LED_MS2CNT(x) ((x) / MX_LED_INTERVAL)


/**
 * @brief restore info at [x]_backup
 */
#define __LED_PUSH(x) ((x##_backup) = (x))
#define __LED_POP(x)  ((x) = (x##_backup))

/**
 * @brief def a var with backup
 */
#define DEF_WITH_BACKUP(x) x,x##_backup,x##_ready

class iBlade : public iBladeDriver {
public:
    iBlade(size_t num);
    ~iBlade();
    bool parameterUpdate(void* arg);
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
    
    inline void pushColors(void)
    {
        __LED_PUSH(MC);
        __LED_PUSH(SC);
    }
    inline void popColors(void)
    {
        __LED_POP(MC);
        __LED_POP(SC);
    }
    inline void pushSet(void)
    {
        pushColors();
        __LED_PUSH(modeBackGround);
        __LED_PUSH(modeTrigger);
        __LED_PUSH(modeFilter);
        __LED_PUSH(stepBackGround);
        __LED_PUSH(stepTrigger);
        __LED_PUSH(stepFilter);
    }
    inline void popSet(void)
    {
        popColors();
        __LED_POP(modeBackGround);
        __LED_POP(modeTrigger);
        __LED_POP(modeFilter);
        __LED_POP(stepBackGround);
        __LED_POP(stepTrigger);
        __LED_POP(stepFilter);
    }
private:
    mutex_t mutex;
    /**
     * @name Storaged Colors
     * @param MC main color
     * @param SC second main color
     * @param TC trigger color
     * @param maxLight 最大亮度
     * @param minLight 最小亮度
     * @{ */
    RGB DEF_WITH_BACKUP(MC);
    RGB DEF_WITH_BACKUP(SC);
    RGB DEF_WITH_BACKUP(TC);
    int maxLight;
    int minLight;
    /** @} */
    /**
     * @name Storaged Param about BackGround:Blink
     * @{ */
    int cntBlinkSwitch;
    /** @} */
    /**
     * @name Storaged Param about BackGround:Pulse
     * @{ */
    int msMCMaintain;
    int msSCMaintain;
    int msMCSwitch;
    int msSCSwitch;
    /** @} */
    /**
     * @name Storaged Param about BackGround:Spark
     * @{ */
    float fSparkRate;
    /** @} */
    /**
     * @name Storaged Param about BackGround:Flame
     * @flameRate
     * @pFlame
     * @{ */
    int flameRate;
    Flame_t* pFlame;
    /** @} */
    /**
     * @name Storaged Param about BackGround:Rainbow
     ** @{ */
    float rainbowLength;
    int rainbowDirection;
    /** @} */

    /**
     * @name Storage Param about Filter:Wave
     * @param waveDirection :+1 wave out, -1 wave in
     ** @{ */
    float waveLength;
    int waveDirection;
    /** @} */

    /**
     * @name Storage Param about Filter:Fade
     ** @{ */
    int filterDirection;
    float filterStartPos;
    /** @} */

    /**
     * @name Storage Param about Trigger:Flip & Partial Flip
     * @param flipMode
     *                  - 1: MC->FC
     *                  - 2: SC->FC
     *                  - 3: MC->FC & SC->FC
     *                  - 4: MC->SC & SC->MC
     *                  - 5: MC=~MC
     ** @{*/
    int flipMode;
    int flipTime;
    int flipMaxCnt;
    float flipLength;
    inline void flip_switchColor(int mode)
    {
        switch (mode)
        {
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
        case 5:
        {
            HSV x(MC);
            x.h += 180.0f;
            MC = x.convert2RGB();
        }
        break;
        }
    }

                    
    /**
     * @name Storage Param about Trigger::Drift
     * @{ */
    float driftShift;
    /** @} */
    /**
     * @name Storage Param about Trigger:Speard
     ** @{ */
    int speardMode;
    int speardLength;
    int speardPos;
    /** @} */


    /**
     * @name Storage Param about Trigger:Coment
     * @{ */
    float comentLength;
    /** @} */
    /**
     * @name Storage Param about Trigger:Accelerate
     ** @{ */
    float accelerateRate;
    /** @} */
    /**
     * @name 运行时参数
     * @{ */
    enum { idle, out, in, Run, InTrigger} status;
    enum modeBackGround_t
    {
        Static = 1,
        Gradient = 2,
        Blink = 3,
        Pulse = 4,
        ColorBreath = 5,
        Spark = 6,
        Rainbow = 7,
        Flame = 8
    };
    enum modeTrigger_t
    {
        NoTrigger = 0,
        Flip = 1,
        Flip_Partial = 2,
        Drift = 3,
        Speard = 4,
        Comet = 5,
        Accelerate = 6,
    };
    enum modeFilter_t
    {
        NoFilter = 0,
        Breath = 1,
        Flicker = 2,
        Wave = 3,
        Fade = 4,
    };
    enum modeBackGround_t DEF_WITH_BACKUP(modeBackGround);
    enum modeTrigger_t DEF_WITH_BACKUP(modeTrigger);
    enum modeFilter_t DEF_WITH_BACKUP(modeFilter);

    step_t DEF_WITH_BACKUP(stepBackGround);
    step_t DEF_WITH_BACKUP(stepTrigger);
    step_t DEF_WITH_BACKUP(stepFilter);
    /** @} */


    void setNormalParam(void);
    void setBackGroudParam(modeBackGround_t mode);
    void setTriggerParam(modeTrigger_t mode);
    void setFilterParam(modeFilter_t mode);
private:
    void backGroundRender(void);
};
