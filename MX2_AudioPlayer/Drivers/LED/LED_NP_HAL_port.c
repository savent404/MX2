#include "LED_NP.h"
#include "stm32f1xx_hal.h"
#include "tim.h"
#include "DEBUG.h"

// TODO: makes compiler feel ok, not use 'extern'
// extern TIM_HandleTypeDef htim4;
// extern DMA_HandleTypeDef hdma_tim4_ch2;
// extern unsigned char DmaCpltFlag;
TIM_HandleTypeDef htim4;
DMA_HandleTypeDef hdma_tim4_ch2;
unsigned char DmaCpltFlag;


__weak void Np_Tim_Init(unsigned short period) 
{
    //Timer Initial
  TIM_MasterConfigTypeDef sMasterConfig;
  TIM_OC_InitTypeDef sConfigOC;

  htim4.Instance = TIM4;
  htim4.Init.Prescaler = 0;
  htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim4.Init.Period = period;
  htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim4.Init.RepetitionCounter = 0;
  // htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_PWM_Init(&htim4) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim4, &sMasterConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim4, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  if (HAL_TIM_PWM_ConfigChannel(&htim4, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  HAL_TIM_MspPostInit(&htim4);
}


__weak void Np_Tim_Start(unsigned char * pDataBuffer, unsigned int BufferSize) 
{
    HAL_TIM_PWM_Start_DMA(&htim4, TIM_CHANNEL_2, (uint32_t *)(pDataBuffer), BufferSize);
}

__weak void Np_Tim_Stop(void) 
{
    HAL_TIM_PWM_Stop_DMA(&htim4, TIM_CHANNEL_2);
}

__weak void HAL_TIM_PWM_PulseFinishedCallback(TIM_HandleTypeDef *htim)
{
    DmaCpltFlag=1;
}
