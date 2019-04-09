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
#define __LED_APPLE(x) ((x) = (x##_ready))
#define __LED_STASH(x) ((x##_ready) = (x))

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
    friend void applySets(iBlade&);
    
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
        __LED_PUSH(modeL1);
        __LED_PUSH(modeL2);
        __LED_PUSH(modeL3);
        __LED_PUSH(stepL1);
        __LED_PUSH(stepL2);
        __LED_PUSH(stepL3);
        __LED_PUSH(maxLight);
        __LED_PUSH(minLight);
    }
    inline void popSet(void)
    {
        popColors();
        if (modeL1 == modeL1_backup)
        {
            float t = (float)stepL1;
            __LED_POP(stepL1);
            stepL1.now = t * stepL1.total;
        }
        else
        {
            __LED_POP(modeL1);
            __LED_POP(stepL1);
        }

        if (modeL2 == modeL2_backup)
        {
            float t = (float)stepL2;
            __LED_POP(stepL2);
            stepL2.now = t * stepL2.total;
        }
        else
        {
            __LED_POP(modeL2);
            __LED_POP(stepL2);
        }

        if (modeL3 == modeL3_backup)
        {
            float t = (float)stepL3;
            __LED_POP(stepL3);
            stepL3.now = t * stepL3.total;
        }
        else
        {
            __LED_POP(modeL3);
            __LED_POP(stepL3);
        }
        __LED_POP(maxLight);
        __LED_POP(minLight);
    }
    inline void stashSet(void)
    {
        __LED_STASH(modeL1);
        __LED_STASH(modeL2);
        __LED_STASH(modeL3);
        __LED_STASH(stepL1);
        __LED_STASH(stepL2);
        __LED_STASH(stepL3);
        __LED_STASH(maxLight);
        __LED_STASH(minLight);
    }
    inline void applySet(void)
    {
        if (modeL1 == modeL1_ready)
        {
            float t = (float)stepL1;
            __LED_APPLE(stepL1);
            stepL1.now = t * stepL1.total;
        }
        else
        {
            __LED_APPLE(modeL1);
            __LED_APPLE(stepL1);
        }

        if (modeL2 == modeL2_ready)
        {
            float t = (float)stepL2;
            __LED_APPLE(stepL2);
            stepL2.now = t * stepL2.total;
        }
        else
        {
            __LED_APPLE(modeL2);
            __LED_APPLE(stepL2);
        }

        if (modeL3 == modeL3_ready)
        {
            float t = (float)stepL3;
            __LED_APPLE(stepL3);
            stepL3.now = t * stepL3.total;
        }
        else
        {
            __LED_APPLE(modeL3);
            __LED_APPLE(stepL3);
        }
        __LED_APPLE(maxLight);
        __LED_APPLE(minLight);
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
    int DEF_WITH_BACKUP(maxLight);
    int DEF_WITH_BACKUP(minLight);
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
    int flameMulti;
    Flame_t* pFlame;
    /** @} */
    /**
     * @name Storaged Param about BackGround:Rainbow
     ** @{ */
    float rainbowLength;
    int rainbowDirection;
    /** @} */

    /**
     * @name Storaged Param about BackGround:Gradient
     * @{ */
    int gradientDirection;
    int gradientLength;
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
     * @name Storage Param about Trigger:Comet
     ** @{ */
    int cometStartPos;
    int cometLength;
    int cometRange;
    int cometColorShift;
    int cometType;
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
    bool flipNeedFresh;
    float driftShift;
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
            // use flipColors API
        }
        break;
        }
    }
    /** @} */

                    
    /**
     * @name Storage Param about Trigger:Speard
     ** @{ */
    int speardMode;
    int speardLength;
    int speardPos;
    int speardColorShift;
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
private:
    enum modeL1_t
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
    enum modeL2_t
    {
        NoTrigger = 0,
        Flip = 1,
        Flip_Partial = 2,
        Comet = 3,
        Speard = 4,
        Accelerate = 5,
    };
    enum modeL3_t
    {
        NoFilter = 0,
        Breath = 1,
        Flicker = 2,
        Wave = 3,
        Fade = 4,
    };
    enum modeL1_t DEF_WITH_BACKUP(modeL1);
    enum modeL2_t DEF_WITH_BACKUP(modeL2);
    enum modeL3_t DEF_WITH_BACKUP(modeL3);

    step_t DEF_WITH_BACKUP(stepL1);
    step_t DEF_WITH_BACKUP(stepL2);
    step_t DEF_WITH_BACKUP(stepL3);

    step_t stepProcess;
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
