#pragma once

#include "iBlade_ll.hpp"
#include "iBlade_ulti.hpp"
#include "color.hpp"
#include "flame.hpp"

#define MX_LED_MS2CNT(x) ((x) / MX_LED_INTERVAL)

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

    inline void pushColors(void)
    {
        MC_backup = MC;
        SC_backup = SC;
    }
    inline void popColors(void)
    {
        MC = MC_backup;
        SC = SC_backup;
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
    RGB MC;
    RGB SC;
    RGB TC;
    RGB MC_backup;
    RGB SC_backup;
    int maxLight;
    int minLight;
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
        pushColors();
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
    enum { idle, out, in, Run} status;
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
    } modeBackGround;
    enum modeTrigger_t
    {
        NoTrigger = 0,
        Flip = 1,
        Flip_Partial = 2,
        Drift = 3,
        Speard = 4,
        Comet = 5,
        Accelerate = 6,
    } modeTrigger;
    enum modeFilter_t
    {
        NoFilter = 0,
        Breath = 1,
        Flicker = 2,
        Wave = 3,
        Fade = 4,
    } modeFilter;
    step_t stepBackGround;
    step_t stepBackGround_backup;
    step_t stepTrigger;
    step_t stepFilter;
    step_t stepFilter_backup;
    /** @} */


    void setNormalParam(void);
    void setBackGroudParam(modeBackGround_t mode);
    void setTriggerParam(modeTrigger_t mode);
    void setFilterParam(modeFilter_t mode);
private:
    void backGroundRender(void);
};
