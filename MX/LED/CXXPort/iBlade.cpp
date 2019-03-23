#include "iBlade.h"
#include "LED.h"
#include "cmsis_os.h"
#include "debug.h"
#include "param.h"

iBlade::iBlade(size_t num)
    : iBladeDriver(num), status(idle)
{
    pFlame = nullptr;
    setNormalParam();
    setBackGroudParam(modeL1_t::Static);
    setTriggerParam(modeL2_t::NoTrigger);
    setFilterParam(modeL3_t::NoFilter);
    MC = RGB(255, 0, 0);
    SC = RGB(0, 255, 0);
    TC = RGB(0, 0, 255);
    pushSet();
    modeL1_ready = modeL1;
    modeL2_ready = modeL2;
    modeL3_ready = modeL3;
    stepL1_ready = stepL1;
    stepL2_ready = stepL2;
    stepL3_ready = stepL3;
}

iBlade::~iBlade()
{
    if (pFlame)
        delete pFlame;
}

void iBlade::handle(void *arg)
{
    auto *pConfig = static_cast<PARA_DYNAMIC_t *>(arg);

    // 用于标志临界区，当进入In/Out状态时不释放mutex
    bool isInCritical = false;

    {
        RGB black(0, 0, 0);
        drawLine(black, 0, getPixelNum());
        update();
    }

    osEvent evt;

    for (;;)
    {
        evt = MX_LED_GetMessage(MX_LED_INTERVAL);

        if (!isInCritical)
            mutex.lock();

        handleTrigger(&evt);

        // handle into critical or out critical
        if (!isInCritical && (status == out || status == in))
            isInCritical = true;

        if (isInCritical && (status != out && status != in))
            isInCritical = false;

        if (status != idle)
        {
            handleLoop(arg);

            update();
        }

        if (!isInCritical)
            mutex.unlock();
    }
}

void iBlade::handleLoop(void *arg)
{
    if (status == idle)
        return;

    switch (modeL2)
    {
    case modeL2_t::Flip:
        if (flipMode == 5)
        {
            // do it later
        }
        else
        {
            if (stepL2.now == 0)
            {
                flip_switchColor(flipMode);
            }
            if (stepL2.now == stepL2.total / 2)
            {
                popColors();
            }
        }
        break;
    case modeL2_t::Drift:
        // teardown this in the `handle of stepL2.walk()`
        if (stepL2.now == 0)
        {
            HSV mc(MC);
            HSV sc(SC);
            mc.h += driftShift;
            sc.h += driftShift;
            pushColors();
            MC = mc.convert2RGB();
            SC = sc.convert2RGB();
        }
        else if (stepL2.now == stepL2.total / 2)
        {
            popColors();
        }
        break;
    case modeL2_t::Accelerate:
        // teardown this in the `handle of stepL2.walk()`
        if (stepL2.now == 0)
        {
            stepL1.total = int((float)stepL1 / accelerateRate);
			stepL1.now = int((float)stepL1 / accelerateRate);
            stepL3.total = int((float)stepL3 / accelerateRate);
			stepL3.now = int((float)stepL3 / accelerateRate);
        }
        break;
    }

    backGroundRender();
    if (modeL2 == modeL2_t::Flip && flipMode == 5 && stepL2.now < stepL2.total / 2)
    {
        // this function will take a long time
        // cause each pixel should use RGB->HSV->RGB
        flipColors();
    }

    switch (modeL2)
    {
    case modeL2_t::Flip_Partial:
    {
        static int pos = 0;

        // when trigger at the begining, step.total==infinity(-1)
        if (stepL2.now == 0 && stepL2.total == step_t::infinity)
        {
            pos = rand() % getPixelNum();
        }
        int startPos = pos - int(flipLength*getPixelNum()*0.5);
        int endPos = pos + int(flipLength*getPixelNum()*0.5);

        // set protected mask
        startMask() = startPos;
        endMask() = endPos;

        if (flipMode != 5)
        {
            if (stepL2.now <= stepL2.total/2)
            {
                pushColors();
                flip_switchColor(flipMode);
                backGroundRender();
                popColors();
            }
        }
        // flipMode=5
        else if (stepL2.now < stepL2.total / 2)
        {
            flipColors();
        }

        // set protected mask
        startMask() = 0;
        endMask() = getPixelNum();
    }
    break;
    case modeL2_t::Speard:
    {
        if (stepL2.now == 0 && speardMode != 0)
            speardPos = rand() % getPixelNum();
        int dist = int((float)stepL2 * getPixelNum());
        drawLine(TC, speardPos - dist, speardPos + dist);
    }
    break;
    case modeL2_t::Comet:
    {
        int startPos = int(((float)stepL2 - comentLength)*getPixelNum());
        int realStartPos = startPos >= 0 ? startPos : 0;
        int endPos = int((float)stepL2*getPixelNum());
        drawShade(c_ptr()[realStartPos], TC, startPos, endPos);
    }
    break;
    }

    switch (modeL3)
    {
    case modeL3_t::Breath:
    {
        // float rate = sin((float)stepL3 * 2 * M_PI);
        float rate = ((float)stepL3 - 0.5f) * 2.0f;
        filterSin(rate, maxLight, minLight);
    }
    break;
    case modeL3_t::Flicker:
        if ((float)stepL3 < 0.01f)
        {
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
    case modeL3_t::Fade:
        {
            int startPos = int((filterStartPos + filterDirection*float(stepL3))*getPixelNum());
            if (filterDirection == -1)
                startPos += getPixelNum();
            int endPos = getPixelNum() - startPos > getPixelNum() * 0.2 ?
                         int(getPixelNum()*0.2 + startPos) :
                         getPixelNum();
            filterSet(maxLight, 0, startPos);
            filterShade(startPos, endPos, maxLight, minLight);
            filterSet(0, endPos, getPixelNum());
        }
        break;
    default:
        filterLimit(maxLight, minLight);
        break;
    }

    // Back Ground & filter should be infinity loop
    // but trigger can be end up.
    stepL1.walk();
    stepL3.walk();
    if (stepL2.walk())
    {
        modeL2 = modeL2_t::NoTrigger;
    }

    if (stepProcess.walk())
    {
        if (status == InTrigger && modeL2 == modeL2_t::Accelerate)
        {
            // means keep stepL3&stepL1's now and repeatCnt
            step_t B = stepL1, F = stepL3;
            popSet();
            stepL1.now = int(B.now * accelerateRate);
            stepL1.repeatCnt = B.repeatCnt;
            stepL3.now = int(F.now * accelerateRate);
            stepL3.repeatCnt = F.repeatCnt;
        }
        if (status == InTrigger)
        {
            status = Run;
            popSet();
        }
        else if (status == out)
        {
            status = Run;
            popSet();
        }
        else if (status == in)
        {
            status = idle;
            RGB black(0, 0, 0);
            drawLine(black, 0, getPixelNum());
        }
        else if (status == InTrigger)
        {
        }
    }
}

void iBlade::handleTrigger(const void *evt)
{
    auto *p = static_cast<const osEvent *>(evt);

    if (p->status == osEventTimeout)
        return;
#if LED_SUPPORT_FOLLOW_AUDIO==0
    auto cmd = static_cast<LED_CMD_t>(p->value.v);
    int alt = -1; // iBlade need LED_SUPPORT_FOLLOW_AUDIO
                  // It's just for fun.
#elif LED_SUPPORT_FOLLOW_AUDIO==1
    LED_Message_t message;
    message.hex = p->value.v;
    auto cmd = message.pair.cmd;
    auto alt = message.pair.alt;
#endif


    if (status == Run || cmd == LED_Trigger_Start || cmd == LED_Trigger_Stop)
    {
        pushSet();
        applySet();
    }

    // default: step up stepProcess to handle trigger
    // if is triggerE(force), it should be endless.
    stepProcess = step_t(0, MX_LED_MS2CNT(alt), 0);

    switch (cmd)
    {
    case LED_Trigger_Start:
    {
        RGB black(0, 0, 0);
        drawLine(black, 0, getPixelNum());
        // set trigger'Out'
        status = out;
        stepL3 = step_t(0, MX_LED_MS2CNT(alt), 0);
        modeL3 = modeL3_t::Fade;
        filterDirection = 1;
        filterStartPos = 0.0f;
    }
    break;
    case LED_Trigger_Stop:
        status = in;
        stepL3 = step_t(0, MX_LED_MS2CNT(alt), 0);
        modeL3 = modeL3_t::Fade;
        filterStartPos = 0.0f;
        filterDirection = -1;
        break;

    // stop trigger
    case LED_TriggerE_END:
    case LED_Trigger_EXIT:
        if (status == out)
            status = Run;
        if (status == InTrigger)
            status = Run;
        popSet();
        stashSet();
        break;

    case LED_TriggerB:
    case LED_TriggerC:
    case LED_TriggerD:
    case LED_Trigger_ColorSwitch:
        status = InTrigger;
        // setTriggerParam(modeL2_t::Drift);
        break;
    case LED_TriggerE:
        status = InTrigger;
        stepProcess.repeatCnt = step_t::infinity;
        // setTriggerParam(modeL2_t::Flip);
        break;
    default:
        break;
    }
}

void iBlade::backGroundRender(void)
{
    switch (modeL1)
    {
    case modeL1_t::Static:
        drawLine(MC, 0, getPixelNum());
        break;
    case modeL1_t::Gradient:
        drawShade(MC, SC, 0, getPixelNum());
        break;
    case modeL1_t::Blink:
        if (stepL1.now >= cntBlinkSwitch)
            drawLine(SC, 0, getPixelNum());
        else
            drawLine(MC, 0, getPixelNum());
        break;
    case modeL1_t::Pulse:
    {
        // MC maintain
        // MC->SC
        // SC maintain
        // SC->MC
        int mcm = MX_LED_MS2CNT(msMCMaintain);
        int mcs = MX_LED_MS2CNT(msMCSwitch);
        int scm = MX_LED_MS2CNT(msSCMaintain);
        int scs = MX_LED_MS2CNT(msSCSwitch);
        if (stepL1.now < mcm)
        {
            drawLine(MC, 0, getPixelNum());
        }
        else if (stepL1.now < mcm + mcs)
        {
            float rate = (float)(stepL1.now - mcm) / mcs;
            RGB mid = RGB::midColor(MC, SC, rate);
            drawLine(mid, 0, getPixelNum());
        }
        else if (stepL1.now < mcm + mcs + scm)
        {
            drawLine(SC, 0, getPixelNum());
        }
        else
        {
            float rate = (float)(stepL1.now - (mcm + mcs + scm)) / scs;
            RGB mid = RGB::midColor(SC, MC, rate);
            drawLine(mid, 0, getPixelNum());
        }
    }
    break;
    case modeL1_t::ColorBreath:
    {
        HSV hsv((float)stepL1 * 360.0f, 1.0f, 1.0f);
        RGB ans = hsv.convert2RGB();
        drawLine(ans, 0, getPixelNum());
    }
    break;
    case modeL1_t::Spark:
        if (stepL1.now == 0)
            drawRandownSpot(MC, SC, fSparkRate);
        break;
    case modeL1_t::Rainbow:
        drawRainbow(float(stepL1) * rainbowDirection, rainbowLength);
        break;
    case modeL1_t::Flame:
        if (pFlame == nullptr)
        {
            pFlame = new Flame_t(getPixelNum());
        }
        // use this pointer to call `setColor` func
        pFlame->update(this, flameRate);
        break;
    default:
        DEBUG(5, "Unknow modeL1:%d", modeL1);
        break;
    }
}

__attribute__((weak)) bool iBlade::parameterUpdate(void* arg)
{
    // mutex.lock();
    // mutex.unlock();
    return true;
}
__attribute__((weak)) void iBlade::setNormalParam(void)
{
    pushColors();

    maxLight = 255;
    minLight = 0;
   // get MC color
   if (MX_ColorMatrix_isOutOfRange(&USR.colorMatrix, USR.config->MCIndex - 1))
   {
       DEBUG(5, "MC index out of range %d/%d", USR.config->MCIndex - 1,
             USR.colorMatrix.num);
       MC = RGB(125, 125, 0); // default Yellow
   }
   else
   {
       uint8_t *rgb = USR.colorMatrix.arr[USR.config->MCIndex - 1].arr;
       MC = RGB(rgb[0], rgb[1], rgb[2]);
   }

   // get SC color
   if (MX_ColorMatrix_isOutOfRange(&USR.colorMatrix, USR.config->SCIndex - 1))
   {
       DEBUG(5, "SC index out of range %d/%d", USR.config->SCIndex - 1,
           USR.colorMatrix.num);
       SC = RGB(255, 0, 0); // default green
   }
   else
   {
       uint8_t *rgb = USR.colorMatrix.arr[USR.config->SCIndex - 1].arr;
       SC = RGB(rgb[0], rgb[1], rgb[2]);
   }

   // get TC color
   if (MX_ColorMatrix_isOutOfRange(&USR.colorMatrix, USR.config->TCIndex - 1))
   {
       DEBUG(5, "TC index out of range %d/%d", USR.config->TCIndex - 1,
           USR.colorMatrix.num);
       TC = RGB(255, 0, 255); // default 
   }
   else
   {
       uint8_t *rgb = USR.colorMatrix.arr[USR.config->TCIndex - 1].arr;
       TC = RGB(rgb[0], rgb[1], rgb[2]);
   }
}
__attribute__((weak)) void iBlade::setBackGroudParam(iBlade::modeL1_t mode)
{
    modeL1 = mode;
    switch(modeL1)
    {
        case modeL1_t::Static:
            break;
        case modeL1_t::Gradient:
            break;
        case modeL1_t::Blink:
            stepL1 = step_t(0, MX_LED_MS2CNT(1000), step_t::infinity);
            break;
        case modeL1_t::Pulse:
            msMCMaintain = 500;
            msMCSwitch = 200;
            msSCMaintain = 1000;
            msMCSwitch = 200;
            stepL1 =
                step_t(0, MX_LED_MS2CNT(msMCMaintain + msMCSwitch + msSCMaintain + msSCSwitch),
                                        step_t::infinity);
            break;
        case modeL1_t::ColorBreath:
            break;
        case modeL1_t::Spark:
            break;
        case modeL1_t::Rainbow:
            rainbowLength = 1.0f;
            stepL1 = step_t(0, MX_LED_MS2CNT(2000), step_t::infinity);
            rainbowDirection = 1;
            break;
        case modeL1_t::Flame:
            flameRate = 128;
            break;
    }
}
__attribute__((weak)) void iBlade::setTriggerParam(iBlade::modeL2_t mode)
{
    modeL2 = mode;
    switch (mode)
    {
    case modeL2_t::NoTrigger:
        break;
    case modeL2_t::Flip:
    case modeL2_t::Flip_Partial:
        flipMode = 1;
        flipTime = 1000;
        flipMaxCnt = 5;
        flipLength = 0.4;
        stepL2 = step_t(0, MX_LED_MS2CNT(flipTime), flipMaxCnt - 1);
        break;
    case modeL2_t::Drift:
        driftShift = 30.0f;
        stepL2 = step_t(0, MX_LED_MS2CNT(800), 0);
        break;
    case modeL2_t::Speard:
        speardLength = 0.2f;
        stepL2 = step_t(0, MX_LED_MS2CNT(800), 0);
        break;
    case modeL2_t::Comet:
        comentLength = 0.1f;
        stepL2 = step_t(0, MX_LED_MS2CNT(700), 0);
        break;
    case modeL2_t::Accelerate:
        accelerateRate = 2.0f;
        stepL2 = step_t(0, MX_LED_MS2CNT(2000), 0);
        break;
    }
}
__attribute__((weak)) void iBlade::setFilterParam(iBlade::modeL3_t mode)
{
    modeL3 = mode;
    switch(mode)
    {
    case modeL3_t::Breath:
        stepL3 = step_t(0, MX_LED_MS2CNT(2000), step_t::infinity);
        break;
    case modeL3_t::Flicker:
        stepL3 = step_t(0, MX_LED_MS2CNT(500), step_t::infinity);
        break;
    case modeL3_t::Wave:
        stepL3 = step_t(0, MX_LED_MS2CNT(2000), step_t::infinity);
        waveLength = 1.0;
        waveDirection = 1;
        break;
    case modeL3_t::Fade:
        stepL3 = step_t(0, MX_LED_MS2CNT(1000), step_t::infinity);
        filterDirection = 1;
        filterStartPos = 0.0f;
        break;
    }
    DEBUG(5, "There is");
}
