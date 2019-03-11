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

    setNormalParam();
    setBackGroudParam(modeBackGround_t::Gradient);
    setTriggerParam(modeTrigger_t::NoTrigger);
    setFilterParam(modeFilter_t::NoFilter);

    mutex.unlock();
    return true;
}
void iBlade::update()
{
    LED_NP_HW_Update(this->c_ptr(), this->getPixelNum());
}

void iBlade::setNormalParam()
{
    MC = RGB(50, 0, 0);
    SC = RGB(0, 0, 50);
    TC = RGB(0, 50, 0);

    pushColors();

    maxLight = 255;
    minLight = 0;
//    // get MC color
//    if (MX_ColorMatrix_isOutOfRange(&USR.colorMatrix, USR.config->MCIndex - 1))
//    {
//        DEBUG(5, "MC index out of range %d/%d", USR.config->MCIndex - 1,
//              USR.colorMatrix.num);
//        MC = RGB(125, 125, 0); // default Yellow
//    }
//    else
//    {
//        uint8_t *rgb = USR.colorMatrix.arr[USR.config->MCIndex - 1].arr;
//        MC = RGB(rgb[0], rgb[1], rgb[2]);
//    }
//
//    // get SC color
//    if (MX_ColorMatrix_isOutOfRange(&USR.colorMatrix, USR.config->SCIndex - 1))
//    {
//        DEBUG(5, "SC index out of range %d/%d", USR.config->SCIndex - 1,
//            USR.colorMatrix.num);
//        SC = RGB(255, 0, 0); // default green
//    }
//    else
//    {
//        uint8_t *rgb = USR.colorMatrix.arr[USR.config->SCIndex - 1].arr;
//        SC = RGB(rgb[0], rgb[1], rgb[2]);
//    }
//
//    // get TC color
//    if (MX_ColorMatrix_isOutOfRange(&USR.colorMatrix, USR.config->TCIndex - 1))
//    {
//        DEBUG(5, "TC index out of range %d/%d", USR.config->TCIndex - 1,
//            USR.colorMatrix.num);
//        TC = RGB(255, 0, 255); // default 
//    }
//    else
//    {
//        uint8_t *rgb = USR.colorMatrix.arr[USR.config->TCIndex - 1].arr;
//        TC = RGB(rgb[0], rgb[1], rgb[2]);
//    }
}
void iBlade::setBackGroudParam(iBlade::modeBackGround_t mode)
{
    modeBackGround = mode;
    switch(modeBackGround)
    {
        case modeBackGround_t::Static:
            break;
        case modeBackGround_t::Gradient:
            break;
        case modeBackGround_t::Blink:
            stepBackGround = step_t(0, MX_LED_MS2CNT(1000), step_t::infinity);
            break;
        case modeBackGround_t::Pulse:
            msMCMaintain = 500;
            msMCSwitch = 200;
            msSCMaintain = 1000;
            msMCSwitch = 200;
            stepBackGround =
                step_t(0, MX_LED_MS2CNT(msMCMaintain + msMCSwitch + msSCMaintain + msSCSwitch),
                                        step_t::infinity);
            break;
        case modeBackGround_t::ColorBreath:
            break;
        case modeBackGround_t::Spark:
            break;
        case modeBackGround_t::Rainbow:
            rainbowLength = 1.0f;
            stepBackGround = step_t(0, MX_LED_MS2CNT(2000), step_t::infinity);
            break;
        case modeBackGround_t::Flame:
            flameRate = 128;
            break;
    }
}
void iBlade::setTriggerParam(iBlade::modeTrigger_t mode)
{
    modeTrigger = mode;
    switch (mode)
    {
    case modeTrigger_t::NoTrigger:
        break;
    case modeTrigger_t::Flip:
    case modeTrigger_t::Flip_Partial:
        flipMode = 1;
        flipTime = 1000;
        flipMaxCnt = 5;
        flipLength = 0.4;
        stepTrigger = step_t(0, MX_LED_MS2CNT(flipTime), flipMaxCnt - 1);
        break;
    case modeTrigger_t::Drift:
        driftShift = 30.0f;
        stepTrigger = step_t(0, MX_LED_MS2CNT(800), 0);
        break;
    case modeTrigger_t::Speard:
        speardLength = 0.2f;
        stepTrigger = step_t(0, MX_LED_MS2CNT(800), 0);
        break;
    case modeTrigger_t::Comet:
        comentLength = 0.1f;
        stepTrigger = step_t(0, MX_LED_MS2CNT(700), 0);
        break;
    case modeTrigger_t::Accelerate:
        accelerateRate = 2.0f;
        stepTrigger = step_t(0, MX_LED_MS2CNT(2000), 0);
        break;
    }
}
void iBlade::setFilterParam(iBlade::modeFilter_t mode)
{
    modeFilter = mode;
    switch(mode)
    {
    case modeFilter_t::Breath:
        stepFilter = step_t(0, MX_LED_MS2CNT(2000), step_t::infinity);
        break;
    case modeFilter_t::Flicker:
        stepFilter = step_t(0, MX_LED_MS2CNT(500), step_t::infinity);
        break;
    case modeFilter_t::Wave:
        stepFilter = step_t(0, MX_LED_MS2CNT(2000), step_t::infinity);
        waveLength = 1.0;
        waveDirection = 1;
        break;
    case modeFilter_t::Fade:
        stepFilter = step_t(0, MX_LED_MS2CNT(1000), step_t::infinity);
        filterDirection = 1;
        filterStartPos = 0.0f;
        break;
    }
    DEBUG(5, "There is");
}
