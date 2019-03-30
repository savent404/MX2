#include "LED_NP.h"
#include "iBlade.h"

#include "triggerSets.h"
#include "iBlade_ulti.hpp"

static iBlade* blade = nullptr;

bool LED_NP_Init(void* arg)
{
    triggerSets_HW_t hw = static_cast<triggerSets_HW_t>(arg);

    // get pixel number, default:50
    int num = triggerSets_getHW(hw, "NP_NUM");
    num = (num < 0 ||  num > 256) ? 128 : num;

    blade = new iBlade(num);
    if (!blade)
    {
        return false;
    }

    if (LED_NP_HW_Init(blade->getPixelNum(), hw) == false)
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
    setBackGroudParam(modeL1_t::Static);
    setTriggerParam(modeL2_t::NoTrigger);
    setFilterParam(modeL3_t::NoFilter);

    mutex.unlock();
    return true;
}
void iBlade::update()
{
    LED_NP_HW_Update(this->c_ptr(), this->getPixelNum());
}

void updateBG(iBlade& a, int16_t* p)
{
    triggerSets_BG_t t = static_cast<triggerSets_BG_t>(p);

    int16_t mode = triggerSets_getBG(t, "mode");

    if (mode == -1)
        return;

    a.mutex.lock();
    switch (mode)
    {
        case 1: {
            a.modeL1_ready = iBlade::modeL1_t::Static;
            a.stepL1_ready = step_t(0, 0, step_t::infinity);
            break;
        }
        case 2: {
            a.modeL1_ready = iBlade::modeL1_t::Gradient;
            a.stepL1_ready = step_t(0, 0, step_t::infinity);
            break;
        }
        case 3: {
            a.modeL1_ready = iBlade::modeL1_t::Blink;
            int16_t t_ms = triggerSets_getBG(t, "T_MC");
            int16_t t_sc = triggerSets_getBG(t, "T_SC");
            if (t_ms == -1)
                t_ms = 0;
            if (t_sc == -1)
                t_sc = 0;
            a.stepL1_ready = step_t(0, MX_LED_MS2CNT(t_ms + t_sc), step_t::infinity);
            a.cntBlinkSwitch = MX_LED_MS2CNT(t_ms);
            break;
        }
        case 4: {

            int ans = 0;
            int16_t tmp = triggerSets_getBG(t, "T_MC");
            a.msMCMaintain = tmp == -1 ? 0 : tmp;
            ans += a.msMCMaintain;

            tmp = triggerSets_getBG(t, "T_SC");
            a.msSCMaintain = tmp == -1 ? 0 : tmp;
            ans += a.msSCMaintain;

            tmp = triggerSets_getBG(t, "T_MS");
            a.msMCSwitch = tmp == -1 ? 0 : tmp;
            ans += a.msMCSwitch;

            tmp = triggerSets_getBG(t, "T_SM");
            a.msSCSwitch = tmp == -1 ? 0 : tmp;
            ans += a.msSCSwitch;

            a.modeL1_ready = iBlade::modeL1_t::Pulse;
            a.stepL1_ready = step_t(0, MX_LED_MS2CNT(ans), step_t::infinity);
            break;
        }

        case 5: {
            int16_t tmp = triggerSets_getBG(t, "NP_BreathCycle");
            if (tmp == -1 || tmp == 0)
                break;
            a.modeL1_ready = iBlade::modeL1_t::ColorBreath;
            a.stepL1_ready = step_t(0, MX_LED_MS2CNT(tmp), step_t::infinity);
            break;
        }
        case 6: {
            a.modeL1_ready = iBlade::modeL1_t::Spark;
            int16_t tmp = triggerSets_getBG(t, "NP_Tspark");
            tmp = tmp == -1 ? 0 : tmp;
            a.stepL1_ready = step_t(0, MX_LED_MS2CNT(tmp), step_t::infinity);
            tmp = triggerSets_getBG(t, "NP_SparkDensity");
            a.fSparkRate = float(tmp == -1 ? 0 : tmp) / 100.0f;
            break;
        }
        case 7: {
            a.modeL1_ready = iBlade::modeL1_t::Rainbow;

            int16_t tmp = triggerSets_getBG(t, "RainbowLength");
            a.rainbowLength = float(tmp == -1 ? 0 : tmp) / a.getPixelNum();
            if (a.rainbowLength < 0)
                a.rainbowLength = 0;
            if (a.rainbowLength > 1.0f)
                a.rainbowLength = 1.0f;

            tmp = triggerSets_getBG(t, "RainbowDirection");
            tmp = tmp == -1 ? 0 : tmp;
            a.rainbowDirection = tmp == 0 ? 1 : -1;

            tmp = triggerSets_getBG(t, "RainbowSpeed");
            tmp = tmp == -1 ? 0 : tmp;
            tmp = tmp == 0 ? 1 : tmp;
            tmp = tmp > a.getPixelNum() ? a.getPixelNum() : tmp;
            a.stepL1_ready = step_t(0, a.getPixelNum() / tmp, step_t::infinity);
            break;
        }
        case 8: {
            a.modeL1_ready = iBlade::modeL1_t::Flame;
            int16_t tmp = triggerSets_getBG(t, "FLAMERATE");
            a.flameRate = tmp == -1 ? 0 : tmp;

            tmp = triggerSets_getBG(t, "FLAMEFREQ");
            tmp = tmp == -1 ? 1 : tmp;
            a.stepL1_ready = step_t(0, tmp, step_t::infinity);
            break;
        }
    }
    a.mutex.unlock();
}
void updateTG(iBlade& a, int16_t* p)
{
    triggerSets_TG_t t = static_cast<triggerSets_TG_t>(p);
    int16_t mode = triggerSets_getTG(t, "MODE");
    
    if (mode == -1)
        return;

    a.mutex.lock();
    switch (mode)
    {
        case 1:
        case 2: {
            a.modeL2_ready = iBlade::modeL2_t::Flip;
            int16_t tmp = triggerSets_getTG(t, "NP_FlipColor");
            tmp = tmp == -1 ? 0 : tmp;
            tmp = tmp == 0 ? 1 : tmp;
            a.flipMode = tmp;
            
            tmp = triggerSets_getTG(t, "NP_Tflip");
            tmp = tmp == -1 ? 0 : tmp;
            int16_t tmp2 = triggerSets_getTG(t, "NP_MaxFlipCount");
            tmp2 = tmp2 == -1 ? 0 : tmp2;

            a.stepL2_ready = step_t(0, MX_LED_MS2CNT(tmp), tmp2 == 0 ? step_t::infinity : tmp2);

            if (mode == 2)
            {
                a.modeL2_ready = iBlade::modeL2_t::Flip_Partial;
                tmp = triggerSets_getTG(t, "NP_FlipLenth");
                tmp = tmp == -1 ? 0 : tmp;
                a.flipLength = float(tmp) / a.getPixelNum();
            }
            a.flipNeedFresh = true;
            break;
        }
        case 3: {
            a.modeL2_ready = iBlade::modeL2_t::Drift;
            int16_t tmp = triggerSets_getTG(t, "NP_Cdrift");
            a.driftShift = float(tmp);
            tmp = triggerSets_getTG(t, "NP_TDrift");
            a.stepL2_ready = step_t(0, MX_LED_MS2CNT(tmp), step_t::infinity);
            break;
        }
        case 4: {
            a.modeL2_ready = iBlade::modeL2_t::Speard;
            int16_t tmp;
            int mode;

            tmp = triggerSets_getTG(t, "NP_SpeardMode");
            tmp = tmp == -1 ? 0 : tmp;
            a.speardMode = tmp;
            mode = tmp;

            tmp = triggerSets_getTG(t, "NP_SpeardLength");
            a.speardLength = tmp == -1 ? 0 : tmp;
            if (mode == 1)
            {
                tmp = a.speardLength * 3 / 4;
                tmp = rand() % (a.speardLength / 2) - a.speardLength / 4;
                a.speardLength += tmp;
            }

            tmp = triggerSets_getTG(t, "NP_SpeardSpeed");
            int speed = tmp == -1 ? 0 : tmp;
            if (mode == 1)
            {
                tmp = rand() % (speed / 2) - speed / 4;
                speed += tmp;
            }
            a.stepL2_ready = step_t(0, MX_LED_MS2CNT(1000 * a.getPixelNum() / speed), 0);

            tmp = triggerSets_getTG(t, "NP_SpeardLocation");
            a.speardPos = tmp == -1 ? 0 : tmp;
            if (mode == 1)
            {
                tmp = rand() % (a.getPixelNum() / 2) - a.getPixelNum() / 4;
                a.speardPos += tmp;
            }
            break;
        }
        case 5: {
            a.modeL2_ready = iBlade::modeL2_t::Accelerate;
            a.stepL2_ready = step_t(0, 0xFFFFFF, 0);
            int16_t tmp = triggerSets_getTG(t, "NP_AccN");
            a.accelerateRate = float(tmp == -1 ? 0 : tmp);
            break;
        }
    }
    a.mutex.unlock();
}

void updateFT(iBlade& a, int16_t* p)
{
    triggerSets_FT_t t = static_cast<triggerSets_FT_t>(p);
    int16_t mode = triggerSets_getFT(t, "MODE");
    if (mode == -1)
        return;
    a.mutex.lock();
    switch (mode)
    {
        case 1: {
            a.modeL3_ready = iBlade::modeL3_t::Breath;
            int16_t tmp = triggerSets_getFT(t, "NP_Tbreath");
            tmp = tmp == -1 ? 0 : tmp;
            a.stepL3_ready = step_t(0, MX_LED_MS2CNT(tmp), step_t::infinity);
            tmp = triggerSets_getFT(t, "NP_BrightMax");
            tmp = tmp == -1 ? 255 : tmp;
            a.maxLight_ready = tmp;

            tmp = triggerSets_getFT(t, "NP_BrightMin");
            tmp = tmp == -1 ? 0 : tmp;
            a.minLight_ready = tmp;
            break;
        }
        case 2: {
            a.modeL3_ready = iBlade::modeL3_t::Flicker;
            int16_t tmp = triggerSets_getFT(t, "NP_Tflicker");
            tmp = tmp == -1 ? 0 : tmp;
            a.stepL3_ready = step_t(0, MX_LED_MS2CNT(tmp), step_t::infinity);
            // TODO: 设置变化点密度
            tmp = triggerSets_getFT(t, "NP_BrightMax");
            tmp = tmp == -1 ? 255 : tmp;
            a.maxLight_ready = tmp;

            tmp = triggerSets_getFT(t, "NP_BrightMin");
            tmp = tmp == -1 ? 0 : tmp;
            a.minLight_ready = tmp;

            tmp = triggerSets_getFT(t, "NP_FlickerDensity");
            break;
        }
        case 3: {
            a.modeL3_ready = iBlade::modeL3_t::Wave;
            int16_t tmp = triggerSets_getFT(t, "NP_WaveLength");
            tmp = tmp == -1 ? 0 : tmp;
            a.waveLength = float(tmp) / a.getPixelNum();
            tmp = triggerSets_getFT(t, "NP_WaveSpeed");
            tmp = tmp == -1 ? 0 : tmp;
            tmp = tmp == 0 ? 1 : tmp;

            int16_t tt = triggerSets_getFT(t, "NP_WaveCount");
            a.stepL3_ready = step_t(0, MX_LED_MS2CNT(int(1000 * a.getPixelNum() / tmp)), tt);
            tmp = triggerSets_getFT(t, "NP_BrightMax");
            tmp = tmp == -1 ? 255 : tmp;
            a.maxLight_ready = tmp;

            tmp = triggerSets_getFT(t, "NP_BrightMin");
            tmp = tmp == -1 ? 0 : tmp;
            a.minLight_ready = tmp;

            a.waveDirection = 1;
            break;
        }
        case 4: {
            a.modeL3_ready = iBlade::modeL3_t::Fade;
            int16_t tmp = triggerSets_getFT(t, "NP_FadePosition");
            tmp = tmp == -1 ? 0 : tmp;
            a.filterStartPos = float(tmp) / a.getPixelNum();
            tmp = triggerSets_getFT(t, "NP_FadeDirection");
            tmp = tmp == -1 ? 0 : tmp;
            a.filterDirection = tmp == 0 ? 1 : -1;
            tmp = triggerSets_getFT(t, "NP_Tfade");
            tmp = tmp == -1 ? 0 : tmp;
            a.stepL3_ready = step_t(0, MX_LED_MS2CNT(tmp), 0);
            tmp = triggerSets_getFT(t, "NP_BrightMax");
            tmp = tmp == -1 ? 255 : tmp;
            a.maxLight_ready = tmp;

            tmp = triggerSets_getFT(t, "NP_BrightMin");
            tmp = tmp == -1 ? 0 : tmp;
            a.minLight_ready = tmp;
        }
    }
    a.mutex.unlock();
}

void LED_NP_updateBG(triggerSets_BG_t p)
{
    updateBG(*blade, static_cast<int16_t*>(p));
}
void LED_NP_updateTG(triggerSets_TG_t p)
{
    updateTG(*blade, static_cast<int16_t*>(p));
}
void LED_NP_updateFT(triggerSets_FT_t p)
{
    updateFT(*blade, static_cast<int16_t*>(p));
}

void LED_NP_applySets(void)
{
    applySets(*blade);
}

#define apply(name) (name = name##_ready)
void applySets(iBlade& a)
{
    a.applySet();
}
