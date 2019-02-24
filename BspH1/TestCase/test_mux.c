#include "ff.h"
#include "mux.h"
#include "param.h"
#include "pm.h"

extern FATFS fatfs;
extern PARA_DYNAMIC_t USR;

USR_CONFIG_t config;

int main(void)
{

    MX_MUX_Init();
    
    MX_PM_Boot();
    
    // init fatfs
    f_mount(&fatfs, "0:/", 1);
    
    // TODO:check fatfs works first

    USR.config=&config;
    USR.config->Vol=1;
    // 播放循环背景音 0:/loop.wav
    MX_MUX_Start(TrackId_MainLoop, SlotMode_Loop, "0:/hum.wav");
    
    for (;;)
    {
        // 添加触发(单次) 0:/trigger.wav
        //MX_MUX_Start(MUX_Track_Trigger, MUX_Mode_Once, "0:/clash.wav");
        osDelay(3000);
    }
}
