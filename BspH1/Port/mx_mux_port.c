#include "mux.h"
#include "tim.h"
#include "dac.h"
#include "main.h"

void Audio_Enable(bool status)
{
    if(true==status) {
        HAL_GPIO_WritePin(AUDIO_ENABLE_GPIO_Port, AUDIO_ENABLE_Pin, GPIO_PIN_SET);
    }
    else {
        HAL_GPIO_WritePin(AUDIO_ENABLE_GPIO_Port, AUDIO_ENABLE_Pin, GPIO_PIN_RESET);
    }
}


void MX_MUX_HW_Init(void* source, size_t size)
{
    Audio_Enable(true);
    MX_TIM7_Init();
    MX_DAC_Init();
    HAL_TIM_Base_Start(&htim7);
    HAL_DAC_Start_DMA(&hdac,
                      DAC_CHANNEL_1,
                      (uint32_t*)source,
                      (uint32_t)size,
                      DAC_ALIGN_12B_R);
}

void MX_MUX_HW_DeInit(void)
{
    Audio_Enable(false);
    HAL_DAC_Stop_DMA(&hdac, DAC_CHANNEL_1);
    HAL_TIM_Base_Stop(&htim7);
}


/** 重载DAC的 DMA Callback 函数 */
void HAL_DAC_ConvCpltCallbackCh1(DAC_HandleTypeDef* hdac)
{
    // 处理一般事物
    
    // 调用 MUX提供的callback
    MX_MUX_Callback();
}

void HAL_DAC_ConvHalfCpltCallbackCh1(DAC_HandleTypeDef* hdac)
{
    // 处理一般事物
    
    // 调用 MUX提供的callback
    MX_MUX_HalfCallback();
}

