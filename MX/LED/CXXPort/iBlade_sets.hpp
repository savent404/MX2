#pragma once

#include "color.hpp"
#include "flame.hpp"
#include "iBlade_ulti.hpp"
#include "randomWave.hpp"

#define BLADE_VAR_NORMAL(name) name
#define BLADE_VAR_BACKUP(name) name##_backup
#define BLADE_VAR_READY(name) name##_ready

#define DEF_BLADE_VAR(name) BLADE_VAR_NORMAL(name), BLADE_VAR_BACKUP(name), BLADE_VAR_READY(name)

// normal <-> backup
#define BLADE_VAR_PUSH(name) (BLADE_VAR_BACKUP(name) = BLADE_VAR_NORMAL(name))
#define BLADE_VAR_POP(name) (BLADE_VAR_NORMAL(name) = BLADE_VAR_BACKUP(name))

// normal <-> ready
#define BLADE_VAR_APPLY(name) (BLADE_VAR_NORMAL(name) = BLADE_VAR_READY(name))
#define BLADE_VAR_STASH(name) (BLADE_VAR_READY(name) = BLADE_VAR_NORMAL(name))

// ready <-> backup
#define BLADE_VAR_CLEARSTASH(name) (BLADE_VAR_READY(name) = BLADE_VAR_BACKUP(name))

enum modeL1_t {
    Static      = 1,
    Gradient    = 2,
    Blink       = 3,
    Pulse       = 4,
    ColorBreath = 5,
    Spark       = 6,
    Rainbow     = 7,
    Chaos       = 8,
    Flame       = 9,
};
enum modeL2_t {
    NoTrigger    = 0,
    Flip         = 1,
    Flip_Partial = 2,
    Comet        = 3,
    Speard       = 4,
    Accelerate   = 5,
};
enum modeL3_t {
    NoFilter   = 0,
    Breath     = 1,
    Flicker    = 2,
    Wave       = 3,
    Fade       = 4,
    RandomWave = 5,
};

/**
 * @brief 需要实现精简的参数堆栈
 * @note  每个参数共需要两个拷贝:
 *        - backup 将当前参数入栈，期待后面恢复该参数
 *        - ready 等待写入读写区的参数，从配置文件等地方获取
 * 
 */
class iBladeParam {
public:
    iBladeParam()
    {
        resetSets();
        pushSets();
        stashSets();
    }

    void resetSets(void)
    {
        MC                = RGB(255, 0, 0);
        SC                = RGB(0, 255, 0);
        TC                = RGB(0, 0, 255);
        maxLight          = 255;
        minLight          = 0;
        cntBlinkSwitch    = 0;
        msMCMaintain      = 0;
        msSCMaintain      = 0;
        msMCSwitch        = 0;
        msSCSwitch        = 0;
        fSparkRate        = 0;
        chaosRate         = 0;
        chaosMulti        = 0;
        flameSpeed        = 0;
        flameColdDown     = 0;
        flameRangeL       = 0;
        flameRangeH       = 0;
        flameLightL       = 0;
        flameLightH       = 0;
        rainbowLength     = 0;
        rainbowDirection  = 0;
        gradientDirection = 0;
        gradientLength    = 0;
        waveLength        = 0;
        waveDirection     = 0;
        waveMaxSpeed      = 0;
        filterDirection   = 0;
        cometStartPos     = 0;
        cometLength       = 0;
        cometRange        = 0;
        cometColorShift   = 0;
        cometType         = 0;
        flipMode          = 0;
        flipTime          = 0;
        flipMaxCnt        = 0;
        flipLength        = 0;
        flipNeedFresh     = 0;
        driftShift        = 0;
        speardMode        = 0;
        speardLength      = 0;
        speardColorShift  = 0;
        pRandomWave       = nullptr;
        randomWaveMaxCnt  = 0;
        comentLength      = 0;
        accelerateRate    = 0;

        modeL1 = modeL1_t::Static;
        modeL2 = modeL2_t::NoTrigger;
        modeL3 = modeL3_t::NoFilter;

        // no need to set step_t
    }

protected:
    mutex_t mutex;
    /**
     * @name Storaged Colors
     * @param MC main color
     * @param SC second main color
     * @param TC trigger color
     * @param maxLight 最大亮度
     * @param minLight 最小亮度
     * @{ */
    RGB DEF_BLADE_VAR(MC);
    RGB DEF_BLADE_VAR(SC);
    RGB DEF_BLADE_VAR(TC);
    int DEF_BLADE_VAR(maxLight);
    int DEF_BLADE_VAR(minLight);
    /** @} */
    /**
     * @name Storaged Param about BackGround:Blink
     * @{ */
    int DEF_BLADE_VAR(cntBlinkSwitch);
    /** @} */
    /**
     * @name Storaged Param about BackGround:Pulse
     * @{ */
    int DEF_BLADE_VAR(msMCMaintain);
    int DEF_BLADE_VAR(msSCMaintain);
    int DEF_BLADE_VAR(msMCSwitch);
    int DEF_BLADE_VAR(msSCSwitch);
    /** @} */
    /**
     * @name Storaged Param about BackGround:Spark
     * @{ */
    float DEF_BLADE_VAR(fSparkRate);
    /** @} */
    /**
     * @name Storaged Param about BackGround:Chaos
     * @chaosRate
     * @chaosMulti
     * @{ */
    int DEF_BLADE_VAR(chaosRate);
    int DEF_BLADE_VAR(chaosMulti);
    /** @} */
    /** 
     * @name Storaged Param about BackGround:Flame
     * @{ */
    float DEF_BLADE_VAR(flameSpeed);
    float DEF_BLADE_VAR(flameColdDown);
    int   DEF_BLADE_VAR(flameRangeL);
    int   DEF_BLADE_VAR(flameRangeH);
    int   DEF_BLADE_VAR(flameLightL);
    int   DEF_BLADE_VAR(flameLightH);
    /** @} */
    /**
     * @name Storaged Param about BackGround:Rainbow
     ** @{ */
    float DEF_BLADE_VAR(rainbowLength);
    int   DEF_BLADE_VAR(rainbowDirection);
    /** @} */

    /**
     * @name Storaged Param about BackGround:Gradient
     * @{ */
    int DEF_BLADE_VAR(gradientDirection);
    int DEF_BLADE_VAR(gradientLength);
    /** @} */

    /**
     * @name Storage Param about Filter:Wave
     * @param waveDirection :+1 wave out, -1 wave in
     ** @{ */
    float DEF_BLADE_VAR(waveLength);
    int   DEF_BLADE_VAR(waveDirection);
    int   DEF_BLADE_VAR(waveMaxSpeed);
    /** @} */

    /**
     * @name Storage Param about Filter:Fade
     ** @{ */
    int DEF_BLADE_VAR(filterDirection);
    /** @} */

    /**
     * @name Storage Param about Trigger:Comet
     ** @{ */
    int DEF_BLADE_VAR(cometStartPos);
    int DEF_BLADE_VAR(cometLength);
    int DEF_BLADE_VAR(cometRange);
    int DEF_BLADE_VAR(cometColorShift);
    int DEF_BLADE_VAR(cometType);
    /** @} */

    /**
     * @name Storage Param about Trigger:Flip & Partial Flip
     * @param flipMode
     *                  - 1: MC->FC
     *                  - 2: SC->FC
     *                  - 3: MC->FC & SC->FC
     *                  - 4: MC->SC & SC->MC
     *                  - 5: MC=~MC
     * @param flipLimitH used in Partial Flip, limit the start location
     * @param flipLimitL used in Partial Flip, limit the stop location
     ** @{*/
    int   DEF_BLADE_VAR(flipMode);
    int   DEF_BLADE_VAR(flipTime);
    int   DEF_BLADE_VAR(flipMaxCnt);
    float DEF_BLADE_VAR(flipLength);
    bool  DEF_BLADE_VAR(flipNeedFresh);
    float DEF_BLADE_VAR(driftShift);
    int   DEF_BLADE_VAR(flipLimitH);
    int   DEF_BLADE_VAR(flipLimitL);
    /** @} */

    /**
     * @name Storage Param about Trigger:Speard
     ** @{ */
    int DEF_BLADE_VAR(speardMode);
    int DEF_BLADE_VAR(speardLength);
    int DEF_BLADE_VAR(speardPos);
    int DEF_BLADE_VAR(speardColorShift);
    /** @} */

    /**
     * @name Storage Param about Filter:RandomWave
     ** @{ */
    RandomWave_t* pRandomWave;
    int           DEF_BLADE_VAR(randomWaveMaxCnt);
    /** @} */

    /**
     * @name Storage Param about Trigger:Coment
     * @{ */
    float DEF_BLADE_VAR(comentLength);
    /** @} */
    /**
     * @name Storage Param about Trigger:Accelerate
     ** @{ */
    float DEF_BLADE_VAR(accelerateRate);
    /** @} */

    enum modeL1_t DEF_BLADE_VAR(modeL1);
    enum modeL2_t DEF_BLADE_VAR(modeL2);
    enum modeL3_t DEF_BLADE_VAR(modeL3);
    step_t        DEF_BLADE_VAR(stepL1);
    step_t        DEF_BLADE_VAR(stepL2);
    step_t        DEF_BLADE_VAR(stepL3);
    step_t        DEF_BLADE_VAR(stepProcess);

public:
    friend class FlameGen_t;
    friend class FlameBase_t;
    friend class FlameMode1_t;
    friend class FlameMode2_t;
    /**
     * @brief normal->backup
     */
    void pushSets(void)
    {
        BLADE_VAR_PUSH(MC);
        BLADE_VAR_PUSH(SC);
        BLADE_VAR_PUSH(TC);
        BLADE_VAR_PUSH(maxLight);
        BLADE_VAR_PUSH(minLight);
        BLADE_VAR_PUSH(cntBlinkSwitch);
        BLADE_VAR_PUSH(msMCMaintain);
        BLADE_VAR_PUSH(msSCMaintain);
        BLADE_VAR_PUSH(msMCSwitch);
        BLADE_VAR_PUSH(msSCSwitch);
        BLADE_VAR_PUSH(fSparkRate);
        BLADE_VAR_PUSH(chaosRate);
        BLADE_VAR_PUSH(chaosMulti);
        BLADE_VAR_PUSH(flameSpeed);
        BLADE_VAR_PUSH(flameColdDown);
        BLADE_VAR_PUSH(flameRangeL);
        BLADE_VAR_PUSH(flameRangeH);
        BLADE_VAR_PUSH(flameLightL);
        BLADE_VAR_PUSH(flameLightH);
        BLADE_VAR_PUSH(rainbowLength);
        BLADE_VAR_PUSH(rainbowDirection);
        BLADE_VAR_PUSH(gradientDirection);
        BLADE_VAR_PUSH(gradientLength);
        BLADE_VAR_PUSH(waveLength);
        BLADE_VAR_PUSH(waveDirection);
        BLADE_VAR_PUSH(waveMaxSpeed);
        BLADE_VAR_PUSH(filterDirection);
        BLADE_VAR_PUSH(cometStartPos);
        BLADE_VAR_PUSH(cometLength);
        BLADE_VAR_PUSH(cometRange);
        BLADE_VAR_PUSH(cometColorShift);
        BLADE_VAR_PUSH(cometType);
        BLADE_VAR_PUSH(flipMode);
        BLADE_VAR_PUSH(flipTime);
        BLADE_VAR_PUSH(flipMaxCnt);
        BLADE_VAR_PUSH(flipLength);
        BLADE_VAR_PUSH(flipNeedFresh);
        BLADE_VAR_PUSH(driftShift);
        BLADE_VAR_PUSH(speardMode);
        BLADE_VAR_PUSH(speardLength);
        BLADE_VAR_PUSH(speardPos);
        BLADE_VAR_PUSH(speardColorShift);
        BLADE_VAR_PUSH(randomWaveMaxCnt);
        BLADE_VAR_PUSH(cometLength);
        BLADE_VAR_PUSH(accelerateRate);
        BLADE_VAR_PUSH(modeL1);
        BLADE_VAR_PUSH(modeL2);
        BLADE_VAR_PUSH(modeL3);
        BLADE_VAR_PUSH(stepL1);
        BLADE_VAR_PUSH(stepL2);
        BLADE_VAR_PUSH(stepL3);
        BLADE_VAR_PUSH(stepProcess);
        BLADE_VAR_PUSH(flipLimitH);
        BLADE_VAR_PUSH(flipLimitL);
    }
    /**
     * @brief backup->normal
     * 
     */
    void popSets(void)
    {
        BLADE_VAR_POP(MC);
        BLADE_VAR_POP(SC);
        BLADE_VAR_POP(TC);
        BLADE_VAR_POP(maxLight);
        BLADE_VAR_POP(minLight);
        BLADE_VAR_POP(cntBlinkSwitch);
        BLADE_VAR_POP(msMCMaintain);
        BLADE_VAR_POP(msSCMaintain);
        BLADE_VAR_POP(msMCSwitch);
        BLADE_VAR_POP(msSCSwitch);
        BLADE_VAR_POP(fSparkRate);
        BLADE_VAR_POP(chaosRate);
        BLADE_VAR_POP(chaosMulti);
        BLADE_VAR_POP(flameSpeed);
        BLADE_VAR_POP(flameColdDown);
        BLADE_VAR_POP(flameRangeL);
        BLADE_VAR_POP(flameRangeH);
        BLADE_VAR_POP(flameLightL);
        BLADE_VAR_POP(flameLightH);
        BLADE_VAR_POP(rainbowLength);
        BLADE_VAR_POP(rainbowDirection);
        BLADE_VAR_POP(gradientDirection);
        BLADE_VAR_POP(gradientLength);
        BLADE_VAR_POP(waveLength);
        BLADE_VAR_POP(waveDirection);
        BLADE_VAR_POP(waveMaxSpeed);
        BLADE_VAR_POP(filterDirection);
        BLADE_VAR_POP(cometStartPos);
        BLADE_VAR_POP(cometLength);
        BLADE_VAR_POP(cometRange);
        BLADE_VAR_POP(cometColorShift);
        BLADE_VAR_POP(cometType);
        BLADE_VAR_POP(flipMode);
        BLADE_VAR_POP(flipTime);
        BLADE_VAR_POP(flipMaxCnt);
        BLADE_VAR_POP(flipLength);
        BLADE_VAR_POP(flipNeedFresh);
        BLADE_VAR_POP(driftShift);
        BLADE_VAR_POP(speardMode);
        BLADE_VAR_POP(speardLength);
        BLADE_VAR_POP(speardPos);
        BLADE_VAR_POP(speardColorShift);
        BLADE_VAR_POP(randomWaveMaxCnt);
        BLADE_VAR_POP(cometLength);
        BLADE_VAR_POP(accelerateRate);
        if (BLADE_VAR_NORMAL(modeL1) == BLADE_VAR_BACKUP(modeL1)) {
            auto t                   = float(BLADE_VAR_NORMAL(stepL1));
            BLADE_VAR_NORMAL(stepL1) = BLADE_VAR_BACKUP(stepL1);
            BLADE_VAR_NORMAL(stepL1) = t;
        } else {
            BLADE_VAR_NORMAL(stepL1) = BLADE_VAR_BACKUP(stepL1);
        }
        if (BLADE_VAR_NORMAL(modeL2) == BLADE_VAR_BACKUP(modeL2)) {
            auto t                   = float(BLADE_VAR_NORMAL(stepL2));
            BLADE_VAR_NORMAL(stepL2) = BLADE_VAR_BACKUP(stepL2);
            BLADE_VAR_NORMAL(stepL2) = t;
        } else {
            BLADE_VAR_NORMAL(stepL2) = BLADE_VAR_BACKUP(stepL2);
        }
        if (BLADE_VAR_NORMAL(modeL3) == BLADE_VAR_BACKUP(modeL3)) {
            auto t                   = float(BLADE_VAR_NORMAL(stepL3));
            BLADE_VAR_NORMAL(stepL3) = BLADE_VAR_BACKUP(stepL3);
            BLADE_VAR_NORMAL(stepL3) = t;
        } else {
            BLADE_VAR_NORMAL(stepL3) = BLADE_VAR_BACKUP(stepL3);
        }
        BLADE_VAR_POP(modeL1);
        BLADE_VAR_POP(modeL2);
        BLADE_VAR_POP(modeL3);
        BLADE_VAR_POP(stepProcess);
        BLADE_VAR_POP(flipLimitH);
        BLADE_VAR_POP(flipLimitL);
    }
    /**
     * @brief normal->ready
     * 
     */
    void stashSets(void)
    {
        BLADE_VAR_STASH(MC);
        BLADE_VAR_STASH(SC);
        BLADE_VAR_STASH(TC);
        BLADE_VAR_STASH(maxLight);
        BLADE_VAR_STASH(minLight);
        BLADE_VAR_STASH(cntBlinkSwitch);
        BLADE_VAR_STASH(msMCMaintain);
        BLADE_VAR_STASH(msSCMaintain);
        BLADE_VAR_STASH(msMCSwitch);
        BLADE_VAR_STASH(msSCSwitch);
        BLADE_VAR_STASH(fSparkRate);
        BLADE_VAR_STASH(chaosRate);
        BLADE_VAR_STASH(chaosMulti);
        BLADE_VAR_STASH(flameSpeed);
        BLADE_VAR_STASH(flameColdDown);
        BLADE_VAR_STASH(flameRangeL);
        BLADE_VAR_STASH(flameRangeH);
        BLADE_VAR_STASH(flameLightL);
        BLADE_VAR_STASH(flameLightH);
        BLADE_VAR_STASH(rainbowLength);
        BLADE_VAR_STASH(rainbowDirection);
        BLADE_VAR_STASH(gradientDirection);
        BLADE_VAR_STASH(gradientLength);
        BLADE_VAR_STASH(waveLength);
        BLADE_VAR_STASH(waveDirection);
        BLADE_VAR_STASH(waveMaxSpeed);
        BLADE_VAR_STASH(filterDirection);
        BLADE_VAR_STASH(cometStartPos);
        BLADE_VAR_STASH(cometLength);
        BLADE_VAR_STASH(cometRange);
        BLADE_VAR_STASH(cometColorShift);
        BLADE_VAR_STASH(cometType);
        BLADE_VAR_STASH(flipMode);
        BLADE_VAR_STASH(flipTime);
        BLADE_VAR_STASH(flipMaxCnt);
        BLADE_VAR_STASH(flipLength);
        BLADE_VAR_STASH(flipNeedFresh);
        BLADE_VAR_STASH(driftShift);
        BLADE_VAR_STASH(speardMode);
        BLADE_VAR_STASH(speardLength);
        BLADE_VAR_STASH(speardPos);
        BLADE_VAR_STASH(speardColorShift);
        BLADE_VAR_STASH(randomWaveMaxCnt);
        BLADE_VAR_STASH(cometLength);
        BLADE_VAR_STASH(accelerateRate);
        BLADE_VAR_STASH(modeL1);
        BLADE_VAR_STASH(modeL2);
        BLADE_VAR_STASH(modeL3);
        BLADE_VAR_STASH(stepL1);
        BLADE_VAR_STASH(stepL2);
        BLADE_VAR_STASH(stepL3);
        BLADE_VAR_STASH(stepProcess);
        BLADE_VAR_STASH(flipLimitH);
        BLADE_VAR_STASH(flipLimitL);
    }
    /**
     * @brief ready->normal, then backup->ready
     * 
     */
    void applyStashSets(void)
    {
        BLADE_VAR_APPLY(MC);
        BLADE_VAR_APPLY(SC);
        BLADE_VAR_APPLY(TC);
        BLADE_VAR_APPLY(maxLight);
        BLADE_VAR_APPLY(minLight);
        BLADE_VAR_APPLY(cntBlinkSwitch);
        BLADE_VAR_APPLY(msMCMaintain);
        BLADE_VAR_APPLY(msSCMaintain);
        BLADE_VAR_APPLY(msMCSwitch);
        BLADE_VAR_APPLY(msSCSwitch);
        BLADE_VAR_APPLY(fSparkRate);
        BLADE_VAR_APPLY(chaosRate);
        BLADE_VAR_APPLY(chaosMulti);
        BLADE_VAR_APPLY(flameSpeed);
        BLADE_VAR_APPLY(flameColdDown);
        BLADE_VAR_APPLY(flameRangeL);
        BLADE_VAR_APPLY(flameRangeH);
        BLADE_VAR_APPLY(flameLightL);
        BLADE_VAR_APPLY(flameLightH);
        BLADE_VAR_APPLY(rainbowLength);
        BLADE_VAR_APPLY(rainbowDirection);
        BLADE_VAR_APPLY(gradientDirection);
        BLADE_VAR_APPLY(gradientLength);
        BLADE_VAR_APPLY(waveLength);
        BLADE_VAR_APPLY(waveDirection);
        BLADE_VAR_APPLY(waveMaxSpeed);
        BLADE_VAR_APPLY(filterDirection);
        BLADE_VAR_APPLY(cometStartPos);
        BLADE_VAR_APPLY(cometLength);
        BLADE_VAR_APPLY(cometRange);
        BLADE_VAR_APPLY(cometColorShift);
        BLADE_VAR_APPLY(cometType);
        BLADE_VAR_APPLY(flipMode);
        BLADE_VAR_APPLY(flipTime);
        BLADE_VAR_APPLY(flipMaxCnt);
        BLADE_VAR_APPLY(flipLength);
        BLADE_VAR_APPLY(flipNeedFresh);
        BLADE_VAR_APPLY(driftShift);
        BLADE_VAR_APPLY(speardMode);
        BLADE_VAR_APPLY(speardLength);
        BLADE_VAR_APPLY(speardPos);
        BLADE_VAR_APPLY(speardColorShift);
        BLADE_VAR_APPLY(randomWaveMaxCnt);
        BLADE_VAR_APPLY(cometLength);
        BLADE_VAR_APPLY(accelerateRate);
        if (BLADE_VAR_READY(modeL1) != BLADE_VAR_NORMAL(modeL1)) {
            BLADE_VAR_APPLY(stepL1);
        }
        BLADE_VAR_APPLY(stepL2);
        BLADE_VAR_APPLY(stepL3);
        BLADE_VAR_APPLY(modeL1);
        BLADE_VAR_APPLY(modeL2);
        BLADE_VAR_APPLY(modeL3);
        BLADE_VAR_APPLY(stepProcess);
        BLADE_VAR_APPLY(flipLimitH);
        BLADE_VAR_APPLY(flipLimitL);

        clearStashSets();
    }
    /**
     * @brief backup->ready
     * 
     */
    void clearStashSets(void)
    {
        BLADE_VAR_CLEARSTASH(MC);
        BLADE_VAR_CLEARSTASH(SC);
        BLADE_VAR_CLEARSTASH(TC);
        BLADE_VAR_CLEARSTASH(maxLight);
        BLADE_VAR_CLEARSTASH(minLight);
        BLADE_VAR_CLEARSTASH(cntBlinkSwitch);
        BLADE_VAR_CLEARSTASH(msMCMaintain);
        BLADE_VAR_CLEARSTASH(msSCMaintain);
        BLADE_VAR_CLEARSTASH(msMCSwitch);
        BLADE_VAR_CLEARSTASH(msSCSwitch);
        BLADE_VAR_CLEARSTASH(fSparkRate);
        BLADE_VAR_CLEARSTASH(chaosRate);
        BLADE_VAR_CLEARSTASH(chaosMulti);
        BLADE_VAR_CLEARSTASH(flameSpeed);
        BLADE_VAR_CLEARSTASH(flameColdDown);
        BLADE_VAR_CLEARSTASH(flameRangeL);
        BLADE_VAR_CLEARSTASH(flameRangeH);
        BLADE_VAR_CLEARSTASH(flameLightL);
        BLADE_VAR_CLEARSTASH(flameLightH);
        BLADE_VAR_CLEARSTASH(rainbowLength);
        BLADE_VAR_CLEARSTASH(rainbowDirection);
        BLADE_VAR_CLEARSTASH(gradientDirection);
        BLADE_VAR_CLEARSTASH(gradientLength);
        BLADE_VAR_CLEARSTASH(waveLength);
        BLADE_VAR_CLEARSTASH(waveDirection);
        BLADE_VAR_CLEARSTASH(waveMaxSpeed);
        BLADE_VAR_CLEARSTASH(filterDirection);
        BLADE_VAR_CLEARSTASH(cometStartPos);
        BLADE_VAR_CLEARSTASH(cometLength);
        BLADE_VAR_CLEARSTASH(cometRange);
        BLADE_VAR_CLEARSTASH(cometColorShift);
        BLADE_VAR_CLEARSTASH(cometType);
        BLADE_VAR_CLEARSTASH(flipMode);
        BLADE_VAR_CLEARSTASH(flipTime);
        BLADE_VAR_CLEARSTASH(flipMaxCnt);
        BLADE_VAR_CLEARSTASH(flipLength);
        BLADE_VAR_CLEARSTASH(flipNeedFresh);
        BLADE_VAR_CLEARSTASH(driftShift);
        BLADE_VAR_CLEARSTASH(speardMode);
        BLADE_VAR_CLEARSTASH(speardLength);
        BLADE_VAR_CLEARSTASH(speardPos);
        BLADE_VAR_CLEARSTASH(speardColorShift);
        BLADE_VAR_CLEARSTASH(randomWaveMaxCnt);
        BLADE_VAR_CLEARSTASH(cometLength);
        BLADE_VAR_CLEARSTASH(accelerateRate);
        BLADE_VAR_CLEARSTASH(modeL1);
        BLADE_VAR_CLEARSTASH(modeL2);
        BLADE_VAR_CLEARSTASH(modeL3);
        BLADE_VAR_CLEARSTASH(stepL1);
        BLADE_VAR_CLEARSTASH(stepL2);
        BLADE_VAR_CLEARSTASH(stepL3);
        BLADE_VAR_CLEARSTASH(stepProcess);
        BLADE_VAR_CLEARSTASH(flipLimitH);
        BLADE_VAR_CLEARSTASH(flipLimitL);
    }

    void pushColors(void)
    {
        BLADE_VAR_PUSH(MC);
        BLADE_VAR_PUSH(SC);
        BLADE_VAR_PUSH(TC);
    }
    void popColors(void)
    {
        BLADE_VAR_POP(MC);
        BLADE_VAR_POP(SC);
        BLADE_VAR_POP(TC);
    }
};
