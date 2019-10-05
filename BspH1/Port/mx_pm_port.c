#include "pm.h"

#include "FreeRTOS.h"
#include "cmsis_os.h"
#include "stm32f1xx.h"
#include "main.h"
#include "adc.h"
#include <stdint.h>
#include "param.h"

#include "HW_CONFIG.h"

extern osSemaphoreId VBAT_LOW_FLAGHandle;

static uint16_t vdd_adc_val;

static uint16_t AdcCalibration(void);
static uint16_t BatteryGet(void);
static uint16_t GetVoltage(void);
static void ADC3_AWD_CFG(uint16_t lower, uint16_t higher, uint16_t v_avdd);

static bool isPluged(void)
{
    return HAL_GPIO_ReadPin(CHG_VALID_GPIO_Port,
                            CHG_VALID_Pin) == GPIO_PIN_RESET;
}

bool MX_PM_Init(void)
{
    // init hw
    MX_ADC1_Init();
    MX_ADC3_Init();
    vdd_adc_val=AdcCalibration();
    ADC3_AWD_CFG(STATIC_USR.vol_poweroff, vdd_adc_val, vdd_adc_val);
}

void MX_PM_Boot(void)
{
    HAL_GPIO_WritePin(POWER_EN_GPIO_Port,
                      POWER_EN_Pin,
                      GPIO_PIN_SET);
}

void MX_PM_Shutdown(void)
{
    HAL_GPIO_WritePin(POWER_EN_GPIO_Port,
                      POWER_EN_Pin,
                      GPIO_PIN_RESET);
    while(1);
}

bool MX_PM_needWarning(void)
{
    return GetVoltage() <= STATIC_USR.vol_warning;
}

bool MX_PM_needPowerOff(void)
{
    if(osOK==osSemaphoreWait(VBAT_LOW_FLAGHandle, 0)) {
        return GetVoltage() <= STATIC_USR.vol_poweroff;
    }
    else {
        return 0;
    }
}

bool MX_PM_isCharging(void)
{
    return isPluged();
}

bool MX_PM_isCharged(void)
{
    if (isPluged() == false)
        return false;
    return GetVoltage() > STATIC_USR.vol_chargecomplete;
}

bool MX_PM_CurrentCapacity(uint16_t* v)
{
    *v = GetVoltage();
    return true;
}

static uint16_t AdcCalibration(void)
{
  HAL_ADCEx_Calibration_Start(&hadc1);
  HAL_ADCEx_Calibration_Start(&hadc3);

  ADC_ChannelConfTypeDef sConfig;

  /**Configure Regular Channel 
  */
  sConfig.Channel = ADC_CHANNEL_VREFINT;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_71CYCLES_5;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  
  uint8_t i;
  uint16_t temp;
  temp=0;
  for(i=0;i<3;i++)
  {
    HAL_ADC_Start(&hadc1);
    HAL_ADC_PollForConversion(&hadc1, 500);
    temp+=((uint32_t)(1200*4095))/HAL_ADC_GetValue(&hadc1);
  }

  HAL_ADC_Stop(&hadc1);

  return temp/3;
}

static uint16_t BatteryGet(void)
{
  uint16_t temp;
  ADC3->CR2&=0xFFF1FEFF;
  ADC3->CR2|=0x000E0000;
  ADC3->CR2|=0x00500000;
  HAL_ADC_Start(&hadc3);
  while(!(ADC3->SR & ADC_FLAG_EOC)) {
    osDelay(5);
  }
  temp=HAL_ADC_GetValue(&hadc3);
  ADC3->CR2&=0xFFF1FFFF;
  ADC3->CR2|=0x00020100;
  return temp;
}

static uint16_t GetVoltage(void)
{
  uint32_t val;
  uint8_t i;
  val=0x0000;
  for(i=0;i<3;i++) {
    val+=BatteryGet();
    osDelay(25);
  }
  val=val*BAT_DIVIDER_RATIO/3;
  return (uint16_t)(((uint32_t)(val*vdd_adc_val))/4095);
}

static void ADC3_AWD_CFG(uint16_t lower, uint16_t higher, uint16_t v_avdd)
{
  ADC3->LTR=(uint16_t)((uint32_t)(lower*4095)/v_avdd/BAT_DIVIDER_RATIO);
  ADC3->HTR=(uint16_t)((uint32_t)(higher*4095)/v_avdd/BAT_DIVIDER_RATIO);
}

void HAL_ADC_LevelOutOfWindowCallback(ADC_HandleTypeDef* hadc)
{
    if(hadc->Instance==ADC3)
    {
        osSemaphoreRelease(VBAT_LOW_FLAGHandle);
    }
}

void MX_WTDG_HW_Feed(void)
{
}