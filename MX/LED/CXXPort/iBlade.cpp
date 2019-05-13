#include "iBlade.h"
#include "LED.h"
#include "cmsis_os.h"
#include "debug.h"
#include "param.h"

iBlade::iBlade(size_t num)
    : iBladeDriver(num)
    , status(idle)
{
    setNormalParam();
    setBackGroudParam(modeL1_t::Static);
    setTriggerParam(modeL2_t::NoTrigger);
    setFilterParam(modeL3_t::NoFilter);
    MC       = RGB(255, 0, 0);
    SC       = RGB(0, 255, 0);
    TC       = RGB(0, 0, 255);
    maxLight = 255;
    minLight = 0;
    pushSets();
    stashSets();
    modeL1_ready = modeL1;
    modeL2_ready = modeL2;
    modeL3_ready = modeL3;
    stepL1_ready = stepL1;
    stepL2_ready = stepL2;
    stepL3_ready = stepL3;
    pFlame       = new Flame_t(getPixelNum(), MC, SC);
    pRandomWave  = nullptr;
}

iBlade::~iBlade()
{
    if (pFlame)
        delete pFlame;
}

void iBlade::handle(void* arg)
{
    auto* pConfig = static_cast<PARA_DYNAMIC_t*>(arg);

    {
        RGB black(0, 0, 0);
        drawLine(black, 0, getPixelNum());
        update();
    }

    osEvent evt;

    for (;;) {
        evt = MX_LED_GetMessage(MX_LED_INTERVAL);

        mutex.lock();

        handleTrigger(&evt);

        if (status != idle) {
            handleLoop(arg);

            update();
        } else {
            RGB b(0, 0, 0);
            drawLine(b, 0, getPixelNum());
        }

        mutex.unlock();
    }
}

void iBlade::handleLoop(void* arg)
{
    if (status == idle)
        return;

    switch (modeL2) {
    case modeL2_t::Flip:
        if (flipMode == 5) {
            // do it later
        } else {
            if (stepL2.now == 0) {
                flip_switchColor(flipMode);
            }
            if (stepL2.now == stepL2.total / 2) {
                popColors();
            }
        }
        break;
    case modeL2_t::Accelerate:
        if (stepL2.now == 0) {
            float r = 1 / accelerateRate;

            stepL1.total = int(r * stepL1.total);
            stepL1.now   = int(r * stepL1.now);

            stepL3.total = int(r * stepL3.total);
            stepL3.now   = int(r * stepL3.now);
        }
        break;
    }

    backGroundRender();
    if (modeL2 == modeL2_t::Flip && flipMode == 5 && stepL2.now < stepL2.total / 2) {
        // this function will take a long time
        // cause each pixel should use RGB->HSV->RGB
        flipColors(driftShift);
    }

    switch (modeL2) {
    case modeL2_t::Comet: {
        // get trigger color
        HSV hsvTmp = SC;
        hsvTmp.h += cometColorShift;
        RGB c = RGB(hsvTmp);

        int shiftPos = float(stepL2) * getPixelNum();
        drawComet(c, cometStartPos, shiftPos, cometLength, cometRange, cometType);
        break;
    }
    case modeL2_t::Flip_Partial: {
        static int pos = 0;

        // when trigger at the begining
        if (stepL2.now == 0 && flipNeedFresh) {
            flipNeedFresh = false;
            pos           = rand() % getPixelNum();
        }
        int startPos = pos - int(flipLength * getPixelNum() * 0.5);
        int endPos   = pos + int(flipLength * getPixelNum() * 0.5);

        // set protected mask
        startMask() = startPos;
        endMask()   = endPos;

        if (flipMode != 5) {
            if (stepL2.now <= stepL2.total / 2) {
                pushColors();
                flip_switchColor(flipMode);
                backGroundRender();
                popColors();
            }
        }
        // flipMode=5
        else if (stepL2.now < stepL2.total / 2) {
            flipColors(driftShift);
        }

        // set protected mask
        startMask() = 0;
        endMask()   = getPixelNum();
    } break;
    case modeL2_t::Speard: {
        if (stepL2.now == 0 && speardMode != 0)
            speardPos = rand() % getPixelNum();
        int dist = int((float)stepL2 * speardLength / 2);
        HSV hTmp = TC;
        hTmp.h += speardColorShift;
        RGB tmp = hTmp;
        drawLine(tmp, speardPos - dist, speardPos + dist);
    } break;
    }

    switch (modeL3) {
    case modeL3_t::NoFilter:
        if (status == in || status == idle) {
            filterSet(0);
        } else {
            filterSet(maxLight);
        }
        break;
    case modeL3_t::Breath: {
        // float rate = sin((float)stepL3 * 2 * M_PI);
        float rate = ((float)stepL3 - 0.5f) * 2.0f;
        filterSin(rate, maxLight, minLight);
    } break;
    case modeL3_t::Flicker:
        if ((float)stepL3 < 0.01f) {
            int ans = rand() % (maxLight - minLight);
            ans += minLight;
            filterSet(ans);
        }
        break;
    case modeL3_t::Wave:
        filterWave((float)stepL3 * waveDirection,
                   waveLength,
                   maxLight,
                   minLight);
        break;
    case modeL3_t::Fade: {
        int startPos = int(getPixelNum() * float(stepL3));
        if (filterDirection == -1)
            startPos = getPixelNum() - startPos;
        if ((status == out && filterDirection == 1) || (status == in && filterDirection == -1)) {
            filterSet(maxLight, 0, startPos);
            filterSet(0, startPos, getPixelNum());
        } else {

            filterSet(0, 0, startPos);
            filterSet(maxLight, startPos, getPixelNum());
        }
    } break;
    case modeL3_t::RandomWave:
        if (stepL3.now == 0 && pRandomWave == nullptr) {
            pRandomWave = new RandomWave_t(randomWaveMaxCnt, stepL3, waveLength, maxLight, minLight);
        }
        filterSet(minLight);
        if (pRandomWave && pRandomWave->update(this, waveMaxSpeed) == false) {
            delete pRandomWave;
            pRandomWave = nullptr;
        }
        break;
    default:
        filterLimit(maxLight, minLight);
        break;
    }

    // Back Ground & filter should be infinity loop
    // but trigger can be end up.
    if (stepL1.walk()) {
        clearL1();
    }
    if (stepL2.walk()) {
        clearL2();
    }
    if (stepL3.walk()) {
        clearL3();
    }
    if (stepProcess.walk()) {
        clearProcess();
        stashSets();
    }
}

void iBlade::handleTrigger(const void* evt)
{
    auto* p = static_cast<const osEvent*>(evt);

    if (p->status == osEventTimeout)
        return;
#if LED_SUPPORT_FOLLOW_AUDIO == 0
    auto cmd = static_cast<LED_CMD_t>(p->value.v);
    int  alt = -1; // iBlade need LED_SUPPORT_FOLLOW_AUDIO
        // It's just for fun.
#elif LED_SUPPORT_FOLLOW_AUDIO == 1
    LED_Message_t message;
    message.hex = p->value.v;
    auto cmd    = message.pair.cmd;
    auto alt    = message.pair.alt;
#endif

    if (status == InTrigger || status == out || status == in)
        clearProcess();
    if (status == Run || cmd == LED_Trigger_Start || cmd == LED_Trigger_Stop) {
        pushSets();
        applyStashSets();
        // when pop or apply, update flame color
        pFlame->initColor(MC, SC);
    }

    // default: step up stepProcess to handle trigger
    // if is triggerE(force), it should be endless.
    stepProcess = step_t(0, MX_LED_MS2CNT(alt), 0);

    switch (cmd) {
    case LED_Trigger_Start: {
        RGB black(0, 0, 0);
        drawLine(black, 0, getPixelNum());
        status = out;

        // if already prepare a 'fade' trigger, ignore default config
        // else set L3 as default fade
        if (modeL3 != modeL3_t::Fade) {
            modeL3          = modeL3_t::Fade;
            filterDirection = 1;
            if (stepL3.total != 0 && stepL3.repeatCnt == 0) {

            } else {
                stepL3 = stepProcess;
            }
        } else {
            // stepL3 = stepProcess;
        }
        break;
    }
    case LED_Trigger_Stop:
        status = in;

        // if already prepare a 'fade' trigger, ignore default config
        // else set L3 as default fade
        if (modeL3 != modeL3_t::Fade) {
            modeL3          = modeL3_t::Fade;
            filterDirection = -1;
            if (stepL3.total != 0 && stepL3.repeatCnt == 0) {

            } else {
                stepL3 = stepProcess;
            }
        } else {
            // stepL3 = stepProcess;
        }
        break;

    // stop trigger
    case LED_TriggerE_END:
    case LED_Trigger_EXIT:
        if (status == out)
            status = Run;
        if (status == InTrigger)
            status = Run;
        break;

    case LED_TriggerB:
    case LED_TriggerC:
    case LED_TriggerD:
    case LED_Trigger_ColorSwitch:
    case LED_TriggerStab:
        status = InTrigger;
        break;
    case LED_TriggerE:
        status                = InTrigger;
        stepL1.repeatCnt      = step_t::infinity;
        stepL2.repeatCnt      = step_t::infinity;
        stepL3.repeatCnt      = step_t::infinity;
        stepProcess.repeatCnt = step_t::infinity;
        break;
    default:
        break;
    }

    // Fade的特殊判断:音乐跟随
    if (modeL3 == modeL3_t::Fade && stepL3.total == 0) {
        stepL3.total = stepProcess.total;
    }
}

void iBlade::backGroundRender(void)
{
    switch (modeL1) {
    case modeL1_t::Static:
        drawLine(MC, 0, getPixelNum());
        break;
    case modeL1_t::Gradient:
        drawGradient(MC, SC, (float)stepL1 * gradientDirection, gradientLength);
        break;
    case modeL1_t::Blink:
        if (stepL1.now >= cntBlinkSwitch)
            drawLine(SC, 0, getPixelNum());
        else
            drawLine(MC, 0, getPixelNum());
        break;
    case modeL1_t::Pulse: {
        // MC maintain
        // MC->SC
        // SC maintain
        // SC->MC
        int mcm = MX_LED_MS2CNT(msMCMaintain);
        int mcs = MX_LED_MS2CNT(msMCSwitch);
        int scm = MX_LED_MS2CNT(msSCMaintain);
        int scs = MX_LED_MS2CNT(msSCSwitch);
        if (stepL1.now < mcm) {
            drawLine(MC, 0, getPixelNum());
        } else if (stepL1.now < mcm + mcs) {
            float rate = (float)(stepL1.now - mcm) / mcs;
            RGB   mid  = RGB::midColor(MC, SC, rate);
            drawLine(mid, 0, getPixelNum());
        } else if (stepL1.now < mcm + mcs + scm) {
            drawLine(SC, 0, getPixelNum());
        } else {
            float rate = (float)(stepL1.now - (mcm + mcs + scm)) / scs;
            RGB   mid  = RGB::midColor(SC, MC, rate);
            drawLine(mid, 0, getPixelNum());
        }
    } break;
    case modeL1_t::ColorBreath: {
        HSV hsv((float)stepL1 * 360.0f, 1.0f, 1.0f);
        RGB ans = hsv.convert2RGB();
        drawLine(ans, 0, getPixelNum());
    } break;
    case modeL1_t::Spark:
        if (stepL1.now == 0)
            drawRandownSpot(MC, SC, fSparkRate);
        break;
    case modeL1_t::Rainbow:
        drawRainbow(float(stepL1) * rainbowDirection, rainbowLength);
        break;
    case modeL1_t::Flame:
        // use this pointer to call `setColor` func
        if (stepL1.now == 0)
            for (int i = 0; i < flameMulti; i++)
                pFlame->update(this, flameRate);
        break;
    default:
        DEBUG(5, "Unknow modeL1:%d", modeL1);
        break;
    }
}

__attribute__((weak)) void iBlade::setNormalParam(void)
{
    /*
    pushColors();

    maxLight = 255;
    minLight = 0;
    // get MC color
    int index = USR.np_colorIndex * 3;
    if (MX_ColorMatrix_isOutOfRange(&USR.colorMatrix, index)) {
        DEBUG(5, "MC index out of range %d/%d", index,
              USR.colorMatrix.num);
        MC = RGB(125, 125, 0); // default Yellow
    } else {
        uint8_t* rgb = USR.colorMatrix.arr[ index ].arr;
        MC           = RGB(rgb[ 0 ], rgb[ 1 ], rgb[ 2 ]);
    }

    // get SC color
    index = USR.np_colorIndex * 3 + 1;
    if (MX_ColorMatrix_isOutOfRange(&USR.colorMatrix, index)) {
        DEBUG(5, "SC index out of range %d/%d", index,
              USR.colorMatrix.num);
        SC = RGB(255, 0, 0); // default green
    } else {
        uint8_t* rgb = USR.colorMatrix.arr[ index ].arr;
        SC           = RGB(rgb[ 0 ], rgb[ 1 ], rgb[ 2 ]);
    }

    // get TC color
    index = USR.np_colorIndex * 3 + 2;
    if (MX_ColorMatrix_isOutOfRange(&USR.colorMatrix, index)) {
        DEBUG(5, "TC index out of range %d/%d", index,
              USR.colorMatrix.num);
        TC = RGB(255, 0, 255); // default
    } else {
        uint8_t* rgb = USR.colorMatrix.arr[ index ].arr;
        TC           = RGB(rgb[ 0 ], rgb[ 1 ], rgb[ 2 ]);
    }
    pFlame->initColor(MC, SC);
    */
}
__attribute__((weak)) void iBlade::setBackGroudParam(modeL1_t mode)
{
    /*
    modeL1 = mode;
    switch (modeL1) {
    case modeL1_t::Static:
        break;
    case modeL1_t::Gradient:
        break;
    case modeL1_t::Blink:
        stepL1 = step_t(0, MX_LED_MS2CNT(1000), step_t::infinity);
        break;
    case modeL1_t::Pulse:
        msMCMaintain = 500;
        msMCSwitch   = 200;
        msSCMaintain = 1000;
        msMCSwitch   = 200;
        stepL1       = step_t(0, MX_LED_MS2CNT(msMCMaintain + msMCSwitch + msSCMaintain + msSCSwitch),
                        step_t::infinity);
        break;
    case modeL1_t::ColorBreath:
        break;
    case modeL1_t::Spark:
        break;
    case modeL1_t::Rainbow:
        rainbowLength    = 1.0f;
        stepL1           = step_t(0, MX_LED_MS2CNT(2000), step_t::infinity);
        rainbowDirection = 1;
        break;
    case modeL1_t::Flame:
        flameRate = 128;
        break;
    }
    */
}
__attribute__((weak)) void iBlade::setTriggerParam(modeL2_t mode)
{
    /*
    modeL2 = mode;
    switch (mode) {
    case modeL2_t::NoTrigger:
        break;
    case modeL2_t::Flip:
    case modeL2_t::Flip_Partial:
        flipMode   = 1;
        flipTime   = 1000;
        flipMaxCnt = 5;
        flipLength = 0.4;
        stepL2     = step_t(0, MX_LED_MS2CNT(flipTime), flipMaxCnt - 1);
        break;
    case modeL2_t::Drift:
        driftShift = 30.0f;
        stepL2 = step_t(0, MX_LED_MS2CNT(800), 0);
        break;
    case modeL2_t::Speard:
        speardLength = 0.2f;
        stepL2       = step_t(0, MX_LED_MS2CNT(800), 0);
        break;
    case modeL2_t::Comet:
        comentLength = 0.1f;
        stepL2       = step_t(0, MX_LED_MS2CNT(700), 0);
        break;
    case modeL2_t::Accelerate:
        accelerateRate = 2.0f;
        stepL2         = step_t(0, MX_LED_MS2CNT(2000), 0);
        break;
    }
    */
}
__attribute__((weak)) void iBlade::setFilterParam(modeL3_t mode)
{
    /*
    modeL3 = mode;
    switch (mode) {
    case modeL3_t::Breath:
        stepL3 = step_t(0, MX_LED_MS2CNT(2000), step_t::infinity);
        break;
    case modeL3_t::Flicker:
        stepL3 = step_t(0, MX_LED_MS2CNT(500), step_t::infinity);
        break;
    case modeL3_t::Wave:
        stepL3        = step_t(0, MX_LED_MS2CNT(2000), step_t::infinity);
        waveLength    = 1.0;
        waveDirection = 1;
        break;
    case modeL3_t::Fade:
        stepL3          = step_t(0, MX_LED_MS2CNT(1000), step_t::infinity);
        filterDirection = 1;
        break;
    }
    DEBUG(5, "There is");
    */
}

void iBlade::clearL1(void)
{
}

void iBlade::clearL2(void)
{
    modeL2 = modeL2_t::NoTrigger;
}

void iBlade::clearL3(void)
{
    modeL3 = modeL3_t::NoFilter;
    if (pRandomWave != nullptr) {
        delete pRandomWave;
        pRandomWave = nullptr;
    }
}

void iBlade::clearProcess(void)
{
    stepProcess.repeatCnt = -1;
    if (status == InTrigger || status == out) {
        status = Run;
        popSets();
        // when pop or apply, update flame color
        pFlame->initColor(MC, SC);
    } else if (status == in) {
        status = idle;
        RGB black(0, 0, 0);
        drawLine(black, 0, getPixelNum());
    }
}
