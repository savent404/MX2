#include "audio.h"
#include "mux.h"
#include "path.h"
#include "param.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static MUX_Slot_Id_t sid_hum;
static MUX_Slot_Id_t sid_trigger = 0;

static void getTriggerFullPath(char* out, TRIGGER_PATH_t* ptr)
{
    int pos = rand() % MX_TriggerPath_getNum(ptr);
    sprintf(out, "%s/%s", MX_TriggerPath_GetPrefix(ptr),
                         MX_TriggerPath_GetName(ptr, pos));
}
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
        MX_MUX_Stop(TrackId_MainLoop, sid_hum);
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
            getTriggerFullPath(path, USR.triggerB);
            break;
        case Audio_TriggerC:
            getTriggerFullPath(path, USR.triggerC);
            break;
        case Audio_TriggerD:
            getTriggerFullPath(path, USR.triggerD);
            break;
        case Audio_TriggerE:
            getTriggerFullPath(path, USR.triggerE);
            mode = SlotMode_Loop;
            break;
        case Audio_TriggerE|0x80:
            MX_Audio_Play_Stop(id);
            break;
        case Audio_ColorSwitch:
            sprintf(path, "%s/"WAV_COLORSWITCH, prefix);
            break;
        case Audio_intoReady:
            getTriggerFullPath(path, USR.triggerIN);
            break;
        case Audio_intoRunning:
            getTriggerFullPath(path, USR.triggerOUT);
            break;
    }
    sid_trigger = MX_MUX_Start(TrackId_Trigger, mode, path);
    return true;
}

bool MX_Audio_Play_Stop(Audio_ID_t id)
{
    if (id == Audio_TriggerE)
        MX_MUX_Stop(TrackId_Trigger, sid_trigger);
    return true;
}

bool MX_Audio_isReady(void)
{
    return MX_MUX_hasSlotsIdle(TrackId_Trigger);
}

int MX_Audio_getTriggerLastTime(void)
{
    return MX_MUX_getLastTime(TrackId_Trigger, sid_trigger);
}
