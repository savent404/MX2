#include "hand.h"
#include "Lis3D.h"
#include "USR_CONFIG.h"

bool MX_HAND_Init(void)
{
    Lis3d_Init();
    Lis3dConfig config;
    config.CD = USR.config->CD;
    config.CL = USR.config->CL;
    config.CT = USR.config->CT;
    config.CW = USR.config->CW;
    config.MD = USR.config->MD;
    config.MT = USR.config->MT;
    Lis3d_Set(&config);
    return true;
}

bool MX_HAND_DeInit(void)
{
    return true;
}

HAND_TriggerId_t MX_HAND_GetTrigger(void)
{
    uint8_t isClik = Lis3d_isClick();
    uint8_t isMove = Lis3d_isMove();

    if (isClik)
        return HAND_CLIK;
    if (isMove)
        return HAND_WAVE;
    return HAND_NULL;
}
