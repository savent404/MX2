#include "LED_NP.h"
#include "iBlade.h"

#include "iBlade_ulti.hpp"
#include "triggerSets.h"

#define DEFAULT(var, defaultVal) ((var) == -1 ? (defaultVal) : (var))
static iBlade* blade = nullptr;

bool LED_NP_Init(void* arg)
{
    triggerSets_HW_t hw = static_cast<triggerSets_HW_t>(arg);

    // get pixel number, default:50
    int num = triggerSets_getHW(hw, "NP_NUM");
    num     = (num < 0 || num > 256) ? 128 : num;

    blade = new iBlade(num);
    if (!blade) {
        return false;
    }

    if (LED_NP_HW_Init(blade->getPixelNum(), hw) == false) {
        return false;
    }
    blade->update();
    return true;
}

void LED_NP_Handle(PARA_DYNAMIC_t* ptr)
{
    blade->handle(ptr);
}

bool LED_NP_Update(PARA_DYNAMIC_t* ptr, bool needBlock)
{
    return blade->parameterUpdate(ptr, needBlock);
}
bool iBlade::parameterUpdate(void* arg, bool needBlock)
{
    auto parm = static_cast<PARA_DYNAMIC_t*>(arg);

    if (needBlock) {
        while (status != idle)
            osDelay(50);
        mutex.lock();

        resetSets();
        setNormalParam();
        pushSets();
    } else {
        mutex.lock();
        setNormalParam();
        pushColors();
    }
    clearStashSets();
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
    switch (mode) {
    case 1: {
        a.BLADE_VAR_READY(modeL1) = modeL1_t::Static;
        a.BLADE_VAR_READY(stepL1) = step_t(0, 0, step_t::infinity);
        break;
    }
    case 2: {
        a.BLADE_VAR_READY(modeL1) = modeL1_t::Gradient;
        int16_t tmp;

        tmp                               = triggerSets_getBG(t, "NP_GLength");
        tmp                               = tmp == -1 ? a.getPixelNum() : tmp;
        a.BLADE_VAR_READY(gradientLength) = tmp;

        tmp                       = triggerSets_getBG(t, "NP_Gspeed");
        tmp                       = tmp == -1 ? 1 : tmp;
        a.BLADE_VAR_READY(stepL1) = step_t(0, MX_LED_MS2CNT(1000 * a.getPixelNum() / tmp), step_t::infinity);

        tmp                                  = triggerSets_getBG(t, "NP_Gdirection");
        tmp                                  = tmp == -1 ? 0 : tmp;
        a.BLADE_VAR_READY(gradientDirection) = tmp == 0 ? 1 : -1;
        break;
    }
    case 3: {
        a.BLADE_VAR_READY(modeL1) = modeL1_t::Blink;
        int16_t t_ms              = triggerSets_getBG(t, "T_MC");
        int16_t t_sc              = triggerSets_getBG(t, "T_SC");
        if (t_ms == -1)
            t_ms = 0;
        if (t_sc == -1)
            t_sc = 0;
        a.BLADE_VAR_READY(stepL1)         = step_t(0, MX_LED_MS2CNT(t_ms + t_sc), step_t::infinity);
        a.BLADE_VAR_READY(cntBlinkSwitch) = MX_LED_MS2CNT(t_ms);
        break;
    }
    case 4: {

        int     ans                     = 0;
        int16_t tmp                     = triggerSets_getBG(t, "T_MC");
        a.BLADE_VAR_READY(msMCMaintain) = tmp == -1 ? 0 : tmp;
        ans += a.BLADE_VAR_READY(msMCMaintain);

        tmp                             = triggerSets_getBG(t, "T_SC");
        a.BLADE_VAR_READY(msSCMaintain) = tmp == -1 ? 0 : tmp;
        ans += a.BLADE_VAR_READY(msSCMaintain);

        tmp                           = triggerSets_getBG(t, "T_MS");
        a.BLADE_VAR_READY(msMCSwitch) = tmp == -1 ? 0 : tmp;
        ans += a.BLADE_VAR_READY(msMCSwitch);

        tmp                           = triggerSets_getBG(t, "T_SM");
        a.BLADE_VAR_READY(msSCSwitch) = tmp == -1 ? 0 : tmp;
        ans += a.BLADE_VAR_READY(msSCSwitch);

        a.BLADE_VAR_READY(modeL1) = modeL1_t::Pulse;
        a.BLADE_VAR_READY(stepL1) = step_t(0, MX_LED_MS2CNT(ans), step_t::infinity);
        break;
    }

    case 5: {
        int16_t tmp = triggerSets_getBG(t, "NP_BreathCycle");
        if (tmp == -1 || tmp == 0)
            break;
        a.BLADE_VAR_READY(modeL1) = modeL1_t::ColorBreath;
        a.BLADE_VAR_READY(stepL1) = step_t(0, MX_LED_MS2CNT(tmp), step_t::infinity);
        break;
    }
    case 6: {
        a.BLADE_VAR_READY(modeL1)     = modeL1_t::Spark;
        int16_t tmp                   = triggerSets_getBG(t, "NP_Tspark");
        tmp                           = tmp == -1 ? 0 : tmp;
        a.BLADE_VAR_READY(stepL1)     = step_t(0, MX_LED_MS2CNT(tmp), step_t::infinity);
        tmp                           = triggerSets_getBG(t, "NP_SparkDensity");
        a.BLADE_VAR_READY(fSparkRate) = float(tmp == -1 ? 0 : tmp) / 100.0f;
        break;
    }
    case 7: {
        a.BLADE_VAR_READY(modeL1) = modeL1_t::Rainbow;

        int16_t tmp                      = triggerSets_getBG(t, "RainbowLength");
        a.BLADE_VAR_READY(rainbowLength) = float(tmp == -1 ? 0 : tmp) / a.getPixelNum();
        if (a.BLADE_VAR_READY(rainbowLength) < 0)
            a.BLADE_VAR_READY(rainbowLength) = 0;
        if (a.BLADE_VAR_READY(rainbowLength) > 1.0f)
            a.BLADE_VAR_READY(rainbowLength) = 1.0f;

        tmp                                 = triggerSets_getBG(t, "RainbowDirection");
        tmp                                 = tmp == -1 ? 0 : tmp;
        a.BLADE_VAR_READY(rainbowDirection) = tmp == 0 ? 1 : -1;

        tmp                       = triggerSets_getBG(t, "RainbowSpeed");
        tmp                       = tmp == -1 ? 0 : tmp;
        tmp                       = tmp == 0 ? 1 : tmp;
        tmp                       = tmp > a.getPixelNum() ? a.getPixelNum() : tmp;
        a.BLADE_VAR_READY(stepL1) = step_t(0, a.getPixelNum() / tmp, step_t::infinity);
        break;
    }
    case 8: {
        a.BLADE_VAR_READY(modeL1)    = modeL1_t::Chaos;
        int16_t tmp                  = triggerSets_getBG(t, "CHAOSRATE");
        a.BLADE_VAR_READY(chaosRate) = tmp == -1 ? 0 : tmp;

        tmp                       = triggerSets_getBG(t, "CHAOSFREQ");
        tmp                       = tmp == -1 ? 1 : tmp;
        a.BLADE_VAR_READY(stepL1) = step_t(0, tmp, step_t::infinity);

        tmp                           = triggerSets_getBG(t, "CHAOSMULTI");
        tmp                           = tmp == -1 ? 1 : tmp;
        a.BLADE_VAR_READY(chaosMulti) = tmp;
        break;
    }
    case 9: {
        int tmp;
        a.BLADE_VAR_READY(modeL1) = modeL1_t::Flame;
        a.BLADE_VAR_READY(stepL1) = step_t(0, 1, step_t::infinity);

        tmp                           = triggerSets_getBG(t, "FLAME_SPEED");
        a.BLADE_VAR_READY(flameSpeed) = float(DEFAULT(tmp, 50)) * MX_LED_INTERVAL / 1000.0f;

        tmp                              = triggerSets_getBG(t, "FLAME_COLDDOWN");
        a.BLADE_VAR_READY(flameColdDown) = float(DEFAULT(tmp, 50)) * 0.01f * a.BLADE_VAR_READY(flameSpeed);

        tmp                            = triggerSets_getBG(t, "FLAME_AVG_LENGTH");
        tmp                            = DEFAULT(tmp, a.getPixelNum() / 3);
        a.BLADE_VAR_READY(flameRangeH) = tmp;

        tmp = triggerSets_getBG(t, "FLAME_RANGE_LENGTH");
        tmp = DEFAULT(tmp, a.BLADE_VAR_READY(flameRangeH) / 2) / 2;
        a.BLADE_VAR_READY(flameRangeH) += tmp;
        a.BLADE_VAR_READY(flameRangeL) -= tmp;

        tmp                            = triggerSets_getBG(t, "FLAME_AVG_LIGHT");
        tmp                            = DEFAULT(tmp, 128);
        a.BLADE_VAR_READY(flameLightH) = tmp;

        tmp = triggerSets_getBG(t, "FLAME_RANGE_LIGHT");
        tmp = DEFAULT(tmp, a.BLADE_VAR_READY(flameLightH) * 2) / 2;
        a.BLADE_VAR_READY(flameLightH) += tmp;
        a.BLADE_VAR_READY(flameLightL) -= tmp;

        // range check
        if (a.BLADE_VAR_READY(flameRangeL) < 0)
            a.BLADE_VAR_READY(flameRangeL) = 0;
        if (a.BLADE_VAR_READY(flameLightH) > 255)
            a.BLADE_VAR_READY(flameLightH) = 255;
        if (a.BLADE_VAR_READY(flameLightL) < 0)
            a.BLADE_VAR_READY(flameLightL) = 0;
        break;
    }
    }
    a.mutex.unlock();
}

/**
 * Rand range(0.25~1.0)
 */
static int randParam(int val)
{
    float rate = (rand() % 256) / 256.0f;
    return (val * 3 * rate / 4) + (val / 4);
}
void updateTG(iBlade& a, int16_t* p)
{
    triggerSets_TG_t t    = static_cast<triggerSets_TG_t>(p);
    int16_t          mode = triggerSets_getTG(t, "MODE");

    if (mode == -1)
        return;

    a.mutex.lock();
    switch (mode) {
    case 1:
    case 2: {
        a.BLADE_VAR_READY(modeL2)   = modeL2_t::Flip;
        int16_t tmp                 = triggerSets_getTG(t, "NP_FlipColor");
        tmp                         = tmp == -1 ? 0 : tmp;
        tmp                         = tmp == 0 ? 1 : tmp;
        a.BLADE_VAR_READY(flipMode) = tmp;

        tmp     = triggerSets_getTG(t, "NP_Tflip");
        tmp     = tmp == -1 ? 0 : tmp;
        int cnt = MX_LED_MS2CNT(tmp);
        if (tmp != 0 && cnt <= 1) {
            cnt = 2;
        }
        int16_t tmp2 = triggerSets_getTG(t, "NP_MaxFlipCount");
        tmp2         = tmp2 == -1 ? 0 : tmp2;

        a.BLADE_VAR_READY(stepL2) = step_t(0, cnt, tmp2 == 0 ? step_t::infinity : tmp2);

        if (mode == 2) {
            a.BLADE_VAR_READY(modeL2)     = modeL2_t::Flip_Partial;
            tmp                           = triggerSets_getTG(t, "NP_FlipLenth");
            tmp                           = tmp == -1 ? 0 : tmp;
            a.BLADE_VAR_READY(flipLength) = float(tmp) / a.getPixelNum();
        }
        if (a.BLADE_VAR_READY(flipMode) == 5) {
            tmp                           = triggerSets_getTG(t, "NP_Cdrift");
            a.BLADE_VAR_READY(driftShift) = float(tmp);
        }
        a.BLADE_VAR_READY(flipNeedFresh) = true;
        break;
    }
    case 3: {
        int16_t tmp;

        a.BLADE_VAR_READY(modeL2) = modeL2_t::Comet;

        int16_t cometMode = triggerSets_getTG(t, "NP_CometMode");
        cometMode         = cometMode == -1 ? 0 : cometMode;

        tmp                          = triggerSets_getTG(t, "NP_CometType");
        tmp                          = tmp == -1 ? 0 : tmp;
        a.BLADE_VAR_READY(cometType) = tmp;

        tmp                            = triggerSets_getTG(t, "NP_CometLength");
        tmp                            = tmp == -1 ? a.getPixelNum() / 5 : tmp;
        a.BLADE_VAR_READY(cometLength) = cometMode == 1 ? randParam(tmp) : tmp;

        tmp                           = triggerSets_getTG(t, "NP_CometRange");
        tmp                           = tmp == -1 ? a.getPixelNum() : tmp;
        a.BLADE_VAR_READY(cometRange) = tmp;

        tmp                       = triggerSets_getTG(t, "NP_CometSpeed");
        tmp                       = tmp <= 0 ? 1 : tmp;
        int tt                    = int(a.getPixelNum() * 1000 / tmp);
        a.BLADE_VAR_READY(stepL2) = step_t(0, MX_LED_MS2CNT(tt), 0);

        tmp                              = triggerSets_getTG(t, "NP_CometLocation");
        tmp                              = tmp == -1 ? a.getPixelNum() / 2 : tmp;
        a.BLADE_VAR_READY(cometStartPos) = cometMode == 1 ? randParam(tmp) : tmp;

        tmp                                = triggerSets_getTG(t, "NP_Cdrift");
        tmp                                = tmp == -1 ? 60 : tmp;
        a.BLADE_VAR_READY(cometColorShift) = tmp;
        break;
    }
    case 4: {
        a.BLADE_VAR_READY(modeL2) = modeL2_t::Speard;
        int16_t tmp;
        int     mode;

        tmp                           = triggerSets_getTG(t, "NP_SpeardMode");
        tmp                           = tmp == -1 ? 0 : tmp;
        a.BLADE_VAR_READY(speardMode) = tmp;
        mode                          = tmp;

        tmp                             = triggerSets_getTG(t, "NP_SpeardLength");
        a.BLADE_VAR_READY(speardLength) = tmp == -1 ? 0 : tmp;
        if (mode == 1) {
            tmp = rand() % (a.BLADE_VAR_READY(speardLength) / 2) - a.BLADE_VAR_READY(speardLength) / 4;
            a.BLADE_VAR_READY(speardLength) += tmp;
        }

        tmp       = triggerSets_getTG(t, "NP_SpeardSpeed");
        int speed = tmp == -1 ? 0 : tmp;
        if (mode == 1) {
            tmp = rand() % (speed / 2) - speed / 4;
            speed += tmp;
        }
        a.BLADE_VAR_READY(stepL2) = step_t(0, MX_LED_MS2CNT(1000 * a.getPixelNum() / speed), 0);

        tmp                          = triggerSets_getTG(t, "NP_SpeardLocation");
        a.BLADE_VAR_READY(speardPos) = tmp == -1 ? 0 : tmp;
        if (mode == 1) {
            tmp = rand() % (a.getPixelNum() / 2) - a.getPixelNum() / 4;
            a.BLADE_VAR_READY(speardPos) += tmp;
        }

        tmp                                 = triggerSets_getTG(t, "NP_Cdrift");
        a.BLADE_VAR_READY(speardColorShift) = tmp == -1 ? 0 : tmp;
        break;
    }
    case 5: {
        a.BLADE_VAR_READY(modeL2)         = modeL2_t::Accelerate;
        a.BLADE_VAR_READY(stepL2)         = step_t(0, 0xFFFFFF, 0);
        int16_t tmp                       = triggerSets_getTG(t, "NP_AccN");
        a.BLADE_VAR_READY(accelerateRate) = float(tmp == -1 ? 0 : tmp);
        break;
    }
    }
    a.mutex.unlock();
}

void updateFT(iBlade& a, int16_t* p)
{
    triggerSets_FT_t t    = static_cast<triggerSets_FT_t>(p);
    int16_t          mode = triggerSets_getFT(t, "MODE");
    if (mode == -1)
        return;
    a.mutex.lock();
    switch (mode) {
    case 1: {
        a.BLADE_VAR_READY(modeL3)   = modeL3_t::Breath;
        int16_t tmp                 = triggerSets_getFT(t, "NP_Tbreath");
        tmp                         = tmp == -1 ? 0 : tmp;
        a.BLADE_VAR_READY(stepL3)   = step_t(0, MX_LED_MS2CNT(tmp), step_t::infinity);
        tmp                         = triggerSets_getFT(t, "NP_BrightMax");
        tmp                         = tmp == -1 ? 255 : tmp;
        a.BLADE_VAR_READY(maxLight) = tmp;

        tmp                         = triggerSets_getFT(t, "NP_BrightMin");
        tmp                         = tmp == -1 ? 0 : tmp;
        a.BLADE_VAR_READY(minLight) = tmp;
        break;
    }
    case 2: {
        a.BLADE_VAR_READY(modeL3) = modeL3_t::Flicker;
        int16_t tmp               = triggerSets_getFT(t, "NP_Tflicker");
        tmp                       = tmp == -1 ? 0 : tmp;
        a.BLADE_VAR_READY(stepL3) = step_t(0, MX_LED_MS2CNT(tmp), step_t::infinity);
        // TODO: 设置变化点密度
        tmp                         = triggerSets_getFT(t, "NP_BrightMax");
        tmp                         = tmp == -1 ? 255 : tmp;
        a.BLADE_VAR_READY(maxLight) = tmp;

        tmp                         = triggerSets_getFT(t, "NP_BrightMin");
        tmp                         = tmp == -1 ? 0 : tmp;
        a.BLADE_VAR_READY(minLight) = tmp;

        tmp = triggerSets_getFT(t, "NP_FlickerDensity");
        break;
    }
    case 3: {
        a.BLADE_VAR_READY(modeL3)     = modeL3_t::Wave;
        int16_t tmp                   = triggerSets_getFT(t, "NP_WaveLength");
        tmp                           = tmp == -1 ? 0 : tmp;
        a.BLADE_VAR_READY(waveLength) = float(tmp) / a.getPixelNum();
        tmp                           = triggerSets_getFT(t, "NP_WaveSpeed");
        tmp                           = tmp == -1 ? 0 : tmp;
        tmp                           = tmp == 0 ? 1 : tmp;

        int16_t tt                  = triggerSets_getFT(t, "NP_WaveCycle");
        tt                          = tt == 0 ? step_t::infinity : tt;
        a.BLADE_VAR_READY(stepL3)   = step_t(0, MX_LED_MS2CNT(int(1000 * a.getPixelNum() / tmp)), tt);
        tmp                         = triggerSets_getFT(t, "NP_BrightMax");
        tmp                         = tmp == -1 ? 255 : tmp;
        a.BLADE_VAR_READY(maxLight) = tmp;

        tmp                         = triggerSets_getFT(t, "NP_BrightMin");
        tmp                         = tmp == -1 ? 0 : tmp;
        a.BLADE_VAR_READY(minLight) = tmp;

        a.BLADE_VAR_READY(waveDirection) = 1;
        break;
    }
    case 4: {
        int16_t tmp;
        a.BLADE_VAR_READY(modeL3)          = modeL3_t::Fade;
        tmp                                = triggerSets_getFT(t, "NP_FadeDirection");
        tmp                                = tmp == -1 ? 0 : tmp;
        a.BLADE_VAR_READY(filterDirection) = tmp == 0 ? 1 : -1;
        tmp                                = triggerSets_getFT(t, "NP_Tfade");
        tmp                                = tmp == -1 ? 0 : tmp;
        a.BLADE_VAR_READY(stepL3)          = step_t(0, MX_LED_MS2CNT(tmp), 0);
        tmp                                = triggerSets_getFT(t, "NP_BrightMax");
        tmp                                = tmp == -1 ? 255 : tmp;
        a.BLADE_VAR_READY(maxLight)        = tmp;

        tmp                         = triggerSets_getFT(t, "NP_BrightMin");
        tmp                         = tmp == -1 ? 0 : tmp;
        a.BLADE_VAR_READY(minLight) = tmp;
        break;
    }
    case 5: {
        a.BLADE_VAR_READY(modeL3) = modeL3_t::RandomWave;
        int16_t tmp;

        tmp                           = triggerSets_getFT(t, "NP_WaveLength");
        tmp                           = tmp == -1 ? 0 : tmp;
        a.BLADE_VAR_READY(waveLength) = float(tmp) / a.getPixelNum();

        tmp                             = triggerSets_getFT(t, "NP_WaveSpeed");
        tmp                             = tmp <= 0 ? 1 : tmp;
        a.BLADE_VAR_READY(waveMaxSpeed) = tmp;

        tmp                       = triggerSets_getFT(t, "NP_WaveCycle");
        tmp                       = tmp == -1 ? 0 : tmp;
        tmp                       = tmp == 0 ? step_t::infinity : tmp;
        a.BLADE_VAR_READY(stepL3) = step_t(0, MX_LED_MS2CNT(int(1000 * a.getPixelNum() / a.BLADE_VAR_READY(waveMaxSpeed))), tmp);

        tmp                                 = triggerSets_getFT(t, "NP_Wavecount");
        tmp                                 = tmp == -1 ? 1 : tmp;
        a.BLADE_VAR_READY(randomWaveMaxCnt) = tmp;

        tmp                         = triggerSets_getFT(t, "NP_BrightMax");
        tmp                         = tmp == -1 ? 255 : tmp;
        a.BLADE_VAR_READY(maxLight) = tmp;

        tmp                         = triggerSets_getFT(t, "NP_BrightMin");
        tmp                         = tmp == -1 ? 0 : tmp;
        a.BLADE_VAR_READY(minLight) = tmp;

        a.BLADE_VAR_READY(waveDirection) = 1;
        break;
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

void applySets(iBlade& a)
{
    a.applyStashSets();
}

void stashSets(iBlade& a)
{
    a.stashSets();
}

void LED_NP_applySets(void)
{
    applySets(*blade);
}

void LED_NP_stashSets(void)
{
    stashSets(*blade);
}
