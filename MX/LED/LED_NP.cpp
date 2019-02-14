#include "LED_NP.h"
#include "iBlade.h"
#include "string.h"

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
    blade->hanlde(ptr);
}

bool LED_NP_Update(PARA_DYNAMIC_t *ptr)
{
    return LED_NP_HW_Update(blade->c_ptr(), blade->getPixelNum());
}
bool iBlade::parameterUpdate(void* arg)
{
    return true;
}
void iBlade::update()
{
    return LED_NP_HW_Update(this->c_ptr(), this->getPixelNum());
}