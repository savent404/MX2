#include "iBlade.h"
#include "LED.h"
#include "cmsis_os.h"
#include "debug.h"
#include "param.h"

iBlade::iBlade(size_t num)
    : iBladeDriver(num)
    , status(idle)
{
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
        }

        mutex.unlock();
    }
}

void iBlade::handleLoop(void* arg)
{
    if (status == fullRun) {
        switch (modeBackGround) {
        // static
        case 1:
            drawLine(MC, 0, getPixelNum());
            break;
        // natrual shade
        case 2:
            drawNaturalShade(MC, natrualDiffDegree, 0, getPixelNum());
            break;
        // static ranbow
        case 3:
            drawRainbow(0.0f);
            break;
        // dynamic ranbow
        case 4:
            drawRainbow(float(stepBackGround));
            break;
        // flame
        case 5:
            break;
        default:
            DEBUG(5, "Unknow modeBackGround:%d", modeBackGround);
            break;
        }
    }

    switch (modeTrigger) {
    // no trigger
    case 0:
        break;
    // None Trigger
    case 1:
        break;
    // Spark
    case 2:
        if (float(stepTrigger) < 0.8f)
            drawNaturalShade(TC, natrualDiffDegree, 0, getPixelNum());
        break;
    // PartialSpark
    case 3:
        break;
    // FollowVol
    case 4:
        break;
    // Comet
    case 5:
        break;
    // trigger 'In'
    case 6:
        drawLine(MC, 0, int(getPixelNum() * float(stepTrigger)));
        break;
    // trigger 'Out'
    case 7: {
        RGB black(0, 0, 0);
        drawLine(black, int((1 - float(stepTrigger)) * getPixelNum()), getPixelNum());
    } break;
    default:
        DEBUG(5, "Unknow modeTrigger:%d", modeTrigger);
        break;
    }

    switch (modeFilter) {
    // static
    case 1:
        filterLimit(maxLight, minLight);
        break;
    // breath
    case 2:
        filterSin(float(stepFilter) * 2.0f - 1.0f, maxLight, minLight);
        break;
    // flow
    case 3:
        break;
    // filter spark
    case 4: {
        int offset = rand() % (maxLight - minLight);
        filterSet(offset + minLight);
    } break;
    // rain
    case 5:
        filterRandomRain(rainRate);
        break;
    // vol follow
    case 6:
        break;
    default:
        DEBUG(5, "Unknow modeFilter:%d", modeFilter);
        break;
    }

    stepBackGround.walk();
    stepFilter.walk();

    if (stepTrigger.walk()) {
        modeTrigger = 0;
        // trigger is done
        if (status == out) {
            status = fullRun;
        } else if (status == in) {
            status = idle;
        }
    }
}

void iBlade::handleTrigger(const void* evt)
{
    auto* p = static_cast<const osEvent*>(evt);

    if (p->status == osEventTimeout)
        return;
    auto message = static_cast<LED_Message_t>(p->value.v);

    switch (message) {
    case LED_Trigger_Start:
        status = out;
        stepTrigger = step_t(0, MX_LED_MS2CNT(outDuration), 0);
        modeTrigger = 7;
        break;
    case LED_Trigger_Stop:
        status = in;
        stepTrigger = step_t(0, MX_LED_MS2CNT(inDuration), 0);
        modeTrigger = 6;
        break;

    // stop trigger
    case LED_TriggerE_END:
    case LED_Trigger_EXIT:
        if (status == out)
            status = fullRun;
        else if (status == in)
            status = idle;
        modeTrigger = 0;
        break;

    case LED_TriggerB:
    case LED_TriggerC:
    case LED_TriggerD:
        stepTrigger = step_t(0, MX_LED_MS2CNT(triggerDuration), triggerRepeatCnt);
        modeTrigger = 2;
        break;
    case LED_TriggerE:
        stepTrigger = step_t(0, MX_LED_MS2CNT(triggerDuration), -1);
        modeTrigger = 2;
        break;
    case LED_Trigger_ColorSwitch:
        break;
    default:
        break;
    }
}