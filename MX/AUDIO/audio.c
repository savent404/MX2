#include "audio.h"
#include "mux.h"
#include "path.h"
#include "param.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static MUX_Slot_Id_t sid_hum;
static MUX_Slot_Id_t sid_lockup;

bool MX_Audio_Play_Start(Audio_ID_t id)
{
    static char path[64];
    MUX_Slot_Mode_t mode = SlotMode_Once;

    const char* prefix = MX_PARAM_GetPrefix();

    unsigned pos;

    if (id == Audio_intoRunning)
    {
        sprintf(path, "%s/Bank%d/hum.wav", MX_PARAM_GetPrefix(), USR.bank_now + 1);
        sid_hum = MX_MUX_Start(TrackId_MainLoop,
                              SlotMode_Loop,
                              path);
    }
    else if (id == Audio_intoReady)
    {
        MX_MUX_Start(TrackId_MainLoop, sid_hum, NULL);
    }

    switch(id)
    {
        case Audio_Boot:
            sprintf(path, "%s/"WAV_BOOT, prefix);
            break;
        case Audio_Erro:
            sprintf(path, "%s/"WAV_ERROR, prefix);
            break;
        case Audio_LowPower:
            sprintf(path, "%s/"WAV_LOWPOWER, prefix);
            break;
        case Audio_BankSwitch:
            sprintf(path, "%s/Bank%d/BankSwitch.wav",
                    prefix,
                    USR.bank_now + 1);
            break;
        case Audio_Charging:
            sprintf(path, "%s/"WAV_CHARGING, prefix);
            break;
        case Audio_PowerOff:
            sprintf(path, "%s/"WAV_POWEROFF, prefix);
            break;
        case Audio_Recharge:
            sprintf(path, "%s/"WAV_RECHARGE, prefix);
            break;
        case Audio_TriggerB:
            pos = rand() % (USR.triggerB + 1)->number;
            sprintf(path, "%s/Bank%d/"TRIGGER(B)"/%s",
                    prefix,
                    USR.bank_now + 1,
                    (USR.triggerB + 1)->path_arry + 30*pos);
        case Audio_TriggerC:
            pos = rand() % (USR.triggerB + 1)->number;
            sprintf(path, "%s/Bank%d/"TRIGGER(C)"/%s",
                    prefix,
                    USR.bank_now + 1,
                    (USR.triggerB + 1)->path_arry + 30*pos);
        case Audio_TriggerD:
            pos = rand() % (USR.triggerB + 1)->number;
            sprintf(path, "%s/Bank%d/"TRIGGER(D)"/%s",
                    prefix,
                    USR.bank_now + 1,
                    (USR.triggerB + 1)->path_arry + 30*pos);
        case Audio_TriggerE:
            pos = rand() % (USR.triggerB + 1)->number;
            sprintf(path, "%s/Bank%d/"TRIGGER(E)"/%s",
                    prefix,
                    USR.bank_now + 1,
                    (USR.triggerB + 1)->path_arry + 30*pos);
            mode = SlotMode_Loop;
        case Audio_TriggerE|0x80:
            MX_Audio_Play_Stop(id);
            break;
        case Audio_ColorSwitch:
            sprintf(path, "%s/"WAV_COLORSWITCH, prefix);
            break;
        case Audio_intoReady:
            pos = rand() % (USR.triggerB + 1)->number;
            sprintf(path, "%s/Bank%d/"TRIGGER(IN)"/%s",
                    prefix,
                    USR.bank_now + 1,
                    (USR.triggerB + 1)->path_arry + 30*pos);
            break;
        case Audio_intoRunning:
            pos = rand() % (USR.triggerB + 1)->number;
            sprintf(path, "%s/Bank%d/"TRIGGER(OUT)"/%s",
                    prefix,
                    USR.bank_now + 1,
                    (USR.triggerB + 1)->path_arry + 30*pos);
            break;
    }
    if (id == Audio_TriggerE)
        sid_lockup = MX_MUX_Start(TrackId_Trigger, mode, path);
    else
        MX_MUX_Start(TrackId_Trigger, mode, path);
    return true;
}

bool MX_Audio_Play_Stop(Audio_ID_t id)
{
    if (id == Audio_TriggerE)
        MX_MUX_Stop(TrackId_Trigger, sid_lockup);
    return true;
}

bool MX_Audio_isReady(void)
{
    return MX_MUX_hasSlotsIdle(TrackId_Trigger);
}

uint32_t MX_Audio_getTriggerLastTime(void)
{
    return 0;
}