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
        MC = RGB(255, 0, 0); // default green
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