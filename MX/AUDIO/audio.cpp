#include "audio.h"
#include "mux.h"
#include "param.h"
#include "path.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static MUX_Slot_Id_t sid_hum;
static MUX_Slot_Id_t sid_trigger   = 0;
static MUX_Slot_Id_t sid_move[ 2 ] = { -1, -1 };

static int static_hum_pos = -1;
static int static_trg_pos = -1;

static void getTriggerFullPath(char* out, TRIGGER_PATH_t* ptr, int* log)
{
    *log = rand() % MX_TriggerPath_getNum(ptr);
    sprintf(out, "%s/%s",
            MX_TriggerPath_GetPrefix(ptr),
            MX_TriggerPath_GetName(ptr, *log));
}
bool MX_Audio_Play_Start(Audio_ID_t id)
{
    static char path[ 64 ];

    MUX_Slot_Mode_t mode = SlotMode_Once;

    const char* prefix = MX_PARAM_GetPrefix();

    unsigned pos;

    if (id == Audio_intoRunning) {
        getTriggerFullPath(path, USR.triggerHUM, &static_hum_pos);
        sid_hum = MX_MUX_Start(TrackId_MainLoop,
                               SlotMode_Loop,
                               path,
                               USR.config->Vol);

        if (USR.triggerB->number) {
            // get H first
            static_trg_pos = rand() % USR.triggerB->number;
            sprintf(path, "%s/%s", MX_TriggerPath_GetPrefix(USR.triggerB),
                    MX_TriggerPath_GetPairHName(USR.triggerB, static_trg_pos));
            sid_move[ 0 ] = MX_MUX_Start(TrackId_MainLoop, SlotMode_Loop, path, 0);
            // then L
            sprintf(path, "%s/%s", MX_TriggerPath_GetPrefix(USR.triggerB),
                    MX_TriggerPath_GetPairLName(USR.triggerB, static_trg_pos));
            sid_move[ 1 ] = MX_MUX_Start(TrackId_MainLoop, SlotMode_Loop, path, 0);
        }
    } else if (id == Audio_intoReady) {
        MX_MUX_Stop(TrackId_MainLoop, sid_hum);
        if (sid_move[ 0 ] != -1)
            MX_MUX_Stop(TrackId_MainLoop, sid_move[ 0 ]);
        if (sid_move[ 1 ] != -1)
            MX_MUX_Stop(TrackId_MainLoop, sid_move[ 1 ]);
        sid_move[ 0 ] = -1;
        sid_move[ 1 ] = -1;
    }

    switch (id) {
    case Audio_Boot:
        sprintf(path, "%s/" WAV_BOOT, prefix);
        break;
    case Audio_Erro:
        sprintf(path, "%s/" WAV_ERROR, prefix);
        break;
    case Audio_LowPower:
        sprintf(path, "%s/" WAV_LOWPOWER, prefix);
        break;
    case Audio_BankSwitch:
        sprintf(path, "%s/Bank%d/BankSwitch.wav",
                prefix,
                USR.bank_now + 1);
        break;
    case Audio_Charging:
        sprintf(path, "%s/" WAV_CHARGING, prefix);
        break;
    case Audio_PowerOff:
        sprintf(path, "%s/" WAV_POWEROFF, prefix);
        break;
    case Audio_Recharge:
        sprintf(path, "%s/" WAV_RECHARGE, prefix);
        break;
    case Audio_TriggerB: {
        // adjust sid_move's vol
        if (sid_move[ 0 ] < 0 || sid_move[ 1 ] < 0)
            return false;
        break;
    } break;
    case Audio_TriggerC:
        getTriggerFullPath(path, USR.triggerC, &static_trg_pos);
        break;
    case Audio_TriggerD:
        getTriggerFullPath(path, USR.triggerD, &static_trg_pos);
        break;
    case Audio_TriggerE:
        getTriggerFullPath(path, USR.triggerE, &static_trg_pos);
        mode = SlotMode_Loop;
        break;
    case Audio_TriggerE | 0x80:
        MX_Audio_Play_Stop(id);
        break;
    case Audio_ColorSwitch:
        sprintf(path, "%s/" WAV_COLORSWITCH, prefix);
        break;
    case Audio_intoReady:
        getTriggerFullPath(path, USR.triggerIN, &static_trg_pos);
        break;
    case Audio_intoRunning:
        getTriggerFullPath(path, USR.triggerOUT, &static_trg_pos);
        break;
    }
    sid_trigger = MX_MUX_Start(TrackId_Trigger, mode, path, USR.config->Vol);
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

int MX_Audio_getLastHumPos(void)
{
    return static_hum_pos;
}
int MX_Audio_getLastTriggerPos(void)
{
    return static_trg_pos;
}

/**
 * @brief adjust tirgger move's vol
 * @param vol 0~100
 */
void MX_Audio_adjMoveVol(int l_vol, int h_vol)
{
    // constexpr int const_maxium_vol = 2 ^ MX_MUX_WAV_VOL_LEVEL;
    int maxium_vol = USR.config->Vol;
    l_vol          = l_vol * maxium_vol / 100;
    h_vol          = h_vol * maxium_vol / 100;

    // adjust instance vols
    auto pSlot = MX_MUX_getInstance(TrackId_MainLoop, sid_move[ 0 ]);
    if (pSlot != nullptr)
        pSlot->vol = h_vol;

    pSlot = MX_MUX_getInstance(TrackId_MainLoop, sid_move[ 1 ]);
    if (pSlot != nullptr)
        pSlot->vol = l_vol;
}
