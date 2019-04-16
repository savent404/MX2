#include "audio.h"

__MX_WEAK void MX_Audio_PlayBeep(void)
{
    MX_Audio_Play_Start(Audio_Erro);
}