#include "mux.h"
#include "tim.h"
#include "dac.h"
//void MX_MUX_HW_Init(void* source, size_t size)
//{
//    MX_TIM7_Init();
//    MX_DAC_Init();
//    HAL_TIM_Base_Start(&htim7);
//    HAL_DAC_Start_DMA(&hdac,
//                      DAC_CHANNEL_1,
//                      (uint32_t*)source,
//                      (uint32_t)size,
//                      DAC_ALIGN_12B_R);
//}
//
//void MX_MUX_HW_DeInit(void)
//{
//    HAL_DAC_Stop_DMA(&hdac, DAC_CHANNEL_1);
//    HAL_TIM_Base_Stop(&htim7);
//}
