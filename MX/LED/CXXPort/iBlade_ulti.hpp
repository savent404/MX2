#pragma once

#include "cmsis_os.h"

class mutex_t {
    osMutexId selfId;
public:
    mutex_t() {
        osMutexDef(self);
        selfId = osMutexCreate(osMutex(self));
    }
    ~mutex_t() {
        osMutexDelete(selfId);
    }
    bool lock(uint32_t timeout = osWaitForever)
    {
        return osMutexWait(selfId, timeout) == osOK;
    }
    bool unlock()
    {
        return osMutexRelease(selfId) == osOK;
    }
};
struct step_t {

    int now;
    int total;
    int repeatCnt;

    typedef enum {
      infinity = -1,
      noRepeat = 0
    } mode;
    /**
     * @param _now 当前step
     * @param _total 总的step
     * @param _repeat 小于0则几乎为无限循环
     */
    step_t(int _now = 0, int _total = 0, int _repeat = 0) {
        now = _now;
        total = _total;
        repeatCnt = _repeat;
    }

    step_t& operator = (const step_t step)
    {
        this->now = step.now;
        this->total = step.total;
        this->repeatCnt = step.repeatCnt;
        return *this;
    }
    
    /**
     * @brief 返回进度百分比
     * @return  0.0f ~ 1.0f
     */
    explicit operator float()
    {
        float fTotal = total == 0 ? 1 : total;
        float res = float(now) / fTotal;
        return res > 1.0f ? 1.0f : res;
    }
    /**
     * @brief update
     * @retvl is EOF
     * @note  如果repeatCnt小于零，那么几乎为无限次重复
     */
    bool walk() {
        bool finishLoop = ++now >= total;
        if (finishLoop) {
            if (repeatCnt-- == 0)
                return true;
            now = 0;
        }
        return false;
    }
};