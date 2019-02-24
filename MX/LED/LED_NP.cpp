#include "LED_NP.h"
#include "iBlade.h"

static iBlade* blade = nullptr;

bool LED_NP_Init(void* arg)
{
    blade = new iBlade(50);
    if (!blade)
    {
        return false;
    }
    if (LED_NP_HW_Init(blade->getPixelNum()) == false)
    {
        return false;
    }
    blade->update();
    return true;
}

void LED_NP_Handle(PARA_DYNAMIC_t* ptr)
{
    blade->handle(ptr);
}

bool LED_NP_Update(PARA_DYNAMIC_t *ptr)
{
    return blade->parameterUpdate(ptr);
}
bool iBlade::parameterUpdate(void* arg)
{
    auto parm = static_cast<PARA_DYNAMIC_t*>(arg);

    mutex.lock();

    MC = RGB(255, 0, 0);
    TC = RGB(0, 255, 0);
    natrualDiffDegree = 30.0f;
    maxLight = 255;
    minLight = 50;
    rainRate = 0.6;
    inDuration = 500;
    outDuration = 500;
    triggerDuration = 800;
    triggerRepeatCnt = 1;

    stepBackGround.total = MX_LED_MS2CNT(2000);
    stepBackGround.repeatCnt = -1;
    stepTrigger.total = MX_LED_MS2CNT(1000);
    stepTrigger.repeatCnt = -1;

    modeBackGround = 2;
    modeTrigger = 0;
    modeFilter = 1;

    mutex.unlock();
    return true;
}
void iBlade::update()
{
    LED_NP_HW_Update(this->c_ptr(), this->getPixelNum());
}