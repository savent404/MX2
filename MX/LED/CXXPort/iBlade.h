#pragma once

#include "iBlade_ll.hpp"
#include "iBlade_ulti.hpp"
#include "color.hpp"

#define MX_LED_MS2CNT(x) ((x) / MX_LED_INTERVAL)

class iBlade : public iBladeDriver {
public:
    iBlade(size_t num);
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
private:
    mutex_t mutex;
    /**
     * @name Storaged Colors
     * @param MC main color
     * @param SC second main color
     * @param TC trigger color
     * @param natrualDiffDegree 自然渐变中色相角度值
     * @param maxLight 最大亮度
     * @param minLight 最小亮度
     * @param rainRate filter:rain的密度参数
     * @param inDuration trigger in持续时间
     * @param outDuration trigger out持续时间
     * @param [draft]triggerDuration 普通trigger的持续时间
     * @{ */
    RGB MC;
    RGB SC;
    RGB TC;
    float natrualDiffDegree;
    int maxLight;
    int minLight;
    float rainRate;
    int inDuration;
    int outDuration;
    int triggerDuration;
    int triggerRepeatCnt;
    /** @} */
    /**
     * @name 运行时参数
     * @{ */
    enum { idle, out, in, fullRun} status;
    int modeBackGround;
    int modeTrigger;
    int modeFilter;
    step_t stepBackGround;
    step_t stepTrigger;
    step_t stepFilter;
    /** @} */
};