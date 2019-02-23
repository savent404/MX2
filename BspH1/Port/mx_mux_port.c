#include "mux.h"
#include "tim.h"
#include "dac.h"
#include "main.h"

void Audio_Enable(bool status, MUX_Track_Id_t tid)
{
    static bool enable[2] = {false, false};
    enable[tid] = status;

    /** If any track enabled, pull up gpio */
    bool res = false;
    for (int i = 0; i < 2; i++)
    {
        if (enable[i])
            res = true;
    }
    if (res == status) {
        HAL_GPIO_WritePin(AUDIO_ENABLE_GPIO_Port, AUDIO_ENABLE_Pin, GPIO_PIN_SET);
        osDelay(150);
    }
    else {
        HAL_GPIO_WritePin(AUDIO_ENABLE_GPIO_Port, AUDIO_ENABLE_Pin, GPIO_PIN_RESET);
    }
}


void MX_MUX_HW_Init(MUX_Track_Id_t tid)
{
    switch(tid)
    {
    case 0:
        MX_TIM7_Init();
        MX_DAC_Init();
        hdac.Instance->DHR12R1=0x800;
        HAL_TIM_Base_Start(&htim7);
        break;
    case 1:
        MX_TIM6_Init();
        hdac.Instance->DHR12R2=0x800;
        HAL_TIM_Base_Start(&htim6);
        break;
    }
}

void MX_MUX_HW_DeInit(MUX_Track_Id_t tid)
{
    Audio_Enable(false, tid);
    switch(tid)
    {
    case 0:
        HAL_DAC_Stop_DMA(&hdac, DAC_CHANNEL_1);
        HAL_TIM_Base_Stop(&htim7);
        break;
    case 1:
        HAL_DAC_Stop_DMA(&hdac, DAC_CHANNEL_2);
        HAL_TIM_Base_Stop(&htim6);
        break;
    }
}

void MX_MUX_HW_Start(MUX_Track_Id_t tid, void* source, int size)
{
    switch (tid)
    {
    case 0:
        HAL_DAC_Start_DMA(&hdac,
                    DAC_CHANNEL_1,
                    (uint32_t*)source,
                    (uint32_t)size,
                    DAC_ALIGN_12B_R);
        break;
    case 1:
        HAL_DAC_Start_DMA(&hdac,
                    DAC_CHANNEL_2,
                    (uint32_t*)source,
                    (uint32_t)size,
                    DAC_ALIGN_12B_R);
        break;
    }
    Audio_Enable(true, tid);
}


/** 重载DAC的 DMA Callback 函数 */
void HAL_DAC_ConvCpltCallbackCh1(DAC_HandleTypeDef* hdac)
{
    // 处理一般事物
    
    // 调用 MUX提供的callback
    MX_MUX_HW_DMA_Callback(TrackId_MainLoop);
}

void HAL_DAC_ConvHalfCpltCallbackCh1(DAC_HandleTypeDef* hdac)
{
    // 处理一般事物
    
    // 调用 MUX提供的callback
    MX_MUX_HW_DMA_HalfCallback(TrackId_MainLoop);
}


void HAL_DACEx_ConvCpltCallbackCh2(DAC_HandleTypeDef* hdac)
{
    // 处理一般事物
    
    // 调用 MUX提供的callback
    MX_MUX_HW_DMA_Callback(TrackId_Trigger);
}

void HAL_DACEx_ConvHalfCpltCallbackCh2(DAC_HandleTypeDef* hdac)
{
    // 处理一般事物
    
    // 调用 MUX提供的callback
    MX_MUX_HW_DMA_HalfCallback(TrackId_Trigger);
}

