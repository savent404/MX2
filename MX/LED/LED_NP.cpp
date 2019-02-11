#include "LED_NP.h"
#include "iBlade.h"
#include "string.h"

static iBlade* blade = nullptr;

static _sNpParaConfig configParam;

bool LED_NP_Init(void* arg)
{
    _pNpHwConfig hwConfig = nullptr;
    _pNpParaConfig paraConfig = nullptr;
    _pNpDrv driver = nullptr;

    memcpy(&configParam, paraConfig, sizeof(configParam));

    _eNPStatus status = NP_DataIF_Init(hwConfig, paraConfig, driver);

    if (status != NP_OpSuccess)
    {
        return false;
    }

    blade = new iBlade(paraConfig->NpNumber);
    blade->update();
    return true;
}

void LED_NP_Handle(PARA_DYNAMIC_t* ptr)
{
    blade->hanlde(ptr);
}

bool LED_NP_Update(PARA_DYNAMIC_t *ptr)
{
    return blade->parameterUpdate(ptr);
}
bool iBlade::parameterUpdate(void* arg)
{
    return false;
}
void iBlade::update()
{
    // for each pixel
    size_t n = this->getPixelNum();
    RGB* ptrColor = this->ptr();

    for (size_t i = 0; i < n; i++)
    {
        uint8_t RGB[3] = {
            ptrColor->wR(),
            ptrColor->wG(),
            ptrColor->wB()
        };

        // TODO: fill RGB[3] into DMA Buffer & shift pointer

        ptrColor++;
    }
    NP_DataIF_Refresh(&configParam);
}