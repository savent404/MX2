#include "iBlade.h"
#include "LED.h"
#include "cmsis_os.h"
#include "debug.h"
#include "param.h"

iBlade::iBlade(size_t num)
    : iBladeDriver(num), status(idle)
{
    pFlame = nullptr;
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

    switch (modeTrigger)
    {
    case modeTrigger_t::Flip:
        if (stepTrigger.now == 0)
        {
            pushColors();
            flip_switchColor(flipMode);
        }
        if (stepTrigger.now == stepTrigger.total / 2)
        {
            popColors();
        }
        break;
    case modeTrigger_t::Drift:
        // teardown this in the `handle of stepTrigger.walk()`
        if (stepTrigger.now == 0)
        {
            HSV mc(MC);
            HSV sc(SC);
            mc.h += driftShift;
            sc.h += driftShift;
            MC = mc.convert2RGB();
            SC = sc.convert2RGB();
        }
        break;
    case modeTrigger_t::Accelerate:
        // teardown this in the `handle of stepTrigger.walk()`
        if (stepTrigger.now == 0)
        {
            stepBackGround.total = int((float)stepBackGround / accelerateRate);
			stepBackGround.now = int((float)stepBackGround / accelerateRate);
            stepFilter.total = int((float)stepFilter / accelerateRate);
			stepFilter.now = int((float)stepFilter / accelerateRate);
        }
        break;
    }

    backGroundRender();

    switch (modeTrigger)
    {
    case modeTrigger_t::Flip_Partial:
    {
        static int pos = 0;
        if (stepTrigger.now == 0)
        {
            pos = rand() % getPixelNum();
        }
        if (stepTrigger.now <= stepTrigger.total/2)
        {
            int startPos = pos - int(flipLength*getPixelNum()*0.5);
            int endPos = pos + int(flipLength*getPixelNum()*0.5);
            startMask() = startPos;
            endMask() = endPos;
            pushColors();
            flip_switchColor(flipMode);

            // draw BackGround again
            backGroundRender();
            
            //
            startMask() = 0;
            endMask() = getPixelNum();
            popColors();
        }
    }
    break;
    case modeTrigger_t::Speard:
    {
        if (stepTrigger.now == 0)
            speardPos = rand() % getPixelNum();
        int dist = int((float)stepTrigger * getPixelNum());
        drawLine(TC, speardPos - dist, speardPos + dist);
    }
    break;
    case modeTrigger_t::Comet:
    {
        int startPos = int(((float)stepTrigger - comentLength)*getPixelNum());
        int realStartPos = startPos >= 0 ? startPos : 0;
        int endPos = int((float)stepTrigger*getPixelNum());
        drawShade(c_ptr()[realStartPos], TC, startPos, endPos);
    }
    break;
    }

    switch (modeFilter)
    {
    case modeFilter_t::Breath:
    {
        // float rate = sin((float)stepFilter * 2 * M_PI);
        float rate = ((float)stepFilter - 0.5f) * 2.0f;
        filterSin(rate, maxLight, minLight);
    }
    break;
    case modeFilter_t::Flicker:
        if ((float)stepFilter < 0.01f)
        {
            int ans = rand() % (maxLight - minLight);
            ans += minLight;
            filterSet(ans);
        }
        break;
    case modeFilter_t::Wave:
        filterWave((float)stepFilter * waveDirection,
                   waveLength,
                   maxLight,
                   minLight);
        break;
    case modeFilter_t::Fade:
        {
            int startPos = int((filterStartPos + filterDirection*float(stepFilter))*getPixelNum());
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

    stepBackGround.walk();

    if (stepFilter.walk() && modeFilter == modeFilter_t::Fade)
    {
        if (status == out)
        {
            status = Run;
            popSet();
        }
        else if (status == in)
        {
            // close all
            popSet();
            status = idle;
            modeTrigger = modeTrigger_t::NoTrigger;
            RGB black(0, 0, 0);
            drawLine(black, 0, getPixelNum());
        }
    }

    if (stepTrigger.walk())
    {
        if (status == InTrigger && modeTrigger != modeTrigger_t::Accelerate)
        {
            popSet();
        }
        else if (status == InTrigger && modeTrigger == modeTrigger_t::Accelerate)
        {
            // means keep stepFilter&stepBackGround's now and repeatCnt
            step_t B = stepBackGround, F = stepFilter;
            popSet();
            stepBackGround.now = int(B.now * accelerateRate);
            stepBackGround.repeatCnt = B.repeatCnt;
            stepFilter.now = int(F.now * accelerateRate);
            stepFilter.repeatCnt = F.repeatCnt;
        }
        else
        {
            modeTrigger = modeTrigger_t::NoTrigger;
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


    if (status == Run)
    {
        pushSet();
    }
    switch (cmd)
    {
    case LED_Trigger_Start:
    {
        // setup default set first
        setFilterParam(modeFilter_t::NoFilter);
        RGB black(0, 0, 0);
        drawLine(black, 0, getPixelNum());
        // push set
        pushSet(); 
        // set trigger'Out'
        status = out;
        // setFilterParam(modeFilter_t::Fade);
        stepFilter = step_t(0, MX_LED_MS2CNT(alt), 0);
        modeFilter = modeFilter_t::Fade;
        filterDirection = 1;
        filterStartPos = 0.0f;
    }
    break;
    case LED_Trigger_Stop:
        status = in;
        stepFilter = step_t(0, MX_LED_MS2CNT(alt), 0);
        modeFilter = modeFilter_t::Fade;
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
        break;

    case LED_TriggerB:
    case LED_TriggerC:
    case LED_TriggerD:
        status = InTrigger;
        setTriggerParam(modeTrigger_t::Drift);
        break;
    case LED_TriggerE:
        status = InTrigger;
        setTriggerParam(modeTrigger_t::Flip);
        break;
    case LED_Trigger_ColorSwitch:
        status = InTrigger;
        setTriggerParam(modeTrigger_t::Flip);
        break;
    default:
        break;
    }
}

void iBlade::backGroundRender(void)
{
    switch (modeBackGround)
    {
    case modeBackGround_t::Static:
        drawLine(MC, 0, getPixelNum());
        break;
    case modeBackGround_t::Gradient:
        drawShade(MC, SC, 0, getPixelNum());
        break;
    case modeBackGround_t::Blink:
        if ((float)stepBackGround >= 0.5)
            drawLine(SC, 0, getPixelNum());
        else
            drawLine(MC, 0, getPixelNum());
        break;
    case modeBackGround_t::Pulse:
    {
        // MC maintain
        // MC->SC
        // SC maintain
        // SC->MC
        int mcm = MX_LED_MS2CNT(msMCMaintain);
        int mcs = MX_LED_MS2CNT(msMCSwitch);
        int scm = MX_LED_MS2CNT(msSCMaintain);
        int scs = MX_LED_MS2CNT(msSCSwitch);
        if (stepBackGround.now < mcm)
        {
            drawLine(MC, 0, getPixelNum());
        }
        else if (stepBackGround.now < mcm + mcs)
        {
            float rate = (float)(stepBackGround.now - mcm) / mcs;
            RGB mid = RGB::midColor(MC, SC, rate);
            drawLine(mid, 0, getPixelNum());
        }
        else if (stepBackGround.now < mcm + mcs + scm)
        {
            drawLine(SC, 0, getPixelNum());
        }
        else
        {
            float rate = (float)(stepBackGround.now - (mcm + mcs + scm)) / scs;
            RGB mid = RGB::midColor(SC, MC, rate);
            drawLine(mid, 0, getPixelNum());
        }
    }
    break;
    case modeBackGround_t::ColorBreath:
    {
        HSV hsv((float)stepBackGround * 360.0f, 1.0f, 1.0f);
        RGB ans = hsv.convert2RGB();
        drawLine(ans, 0, getPixelNum());
    }
    break;
    case modeBackGround_t::Spark:
        if (stepBackGround.now == 0)
            drawRandownSpot(MC, SC, fSparkRate);
        break;
    case modeBackGround_t::Rainbow:
        drawRainbow(float(stepBackGround), rainbowLength);
        break;
    case modeBackGround_t::Flame:
        if (pFlame == nullptr)
        {
            pFlame = new Flame_t(getPixelNum());
        }
        // use this pointer to call `setColor` func
        pFlame->update(this, flameRate);
        break;
    default:
        DEBUG(5, "Unknow modeBackGround:%d", modeBackGround);
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
}
__attribute__((weak)) void iBlade::setBackGroudParam(iBlade::modeBackGround_t mode)
{
}
__attribute__((weak)) void iBlade::setTriggerParam(iBlade::modeTrigger_t mode)
{
}
__attribute__((weak)) void iBlade::setFilterParam(iBlade::modeFilter_t mode)
{
}
