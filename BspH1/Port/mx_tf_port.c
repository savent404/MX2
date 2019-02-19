#include <stdint.h>
#include <stdbool.h>
#include "main.h"
#include "cmsis_os.h"

extern SPI_HandleTypeDef hspi1;
extern osSemaphoreId SdOperate_Cplt_FlagHandle;

void Mmcsd_CS(bool status)
{
  if(true==status) {
    HAL_GPIO_WritePin(TF_CSN_GPIO_Port, TF_CSN_Pin, GPIO_PIN_RESET);
  }
  else {
    HAL_GPIO_WritePin(TF_CSN_GPIO_Port, TF_CSN_Pin, GPIO_PIN_SET);
  }
}

bool Mmcsd_Present(void)
{
  if(GPIO_PIN_RESET==HAL_GPIO_ReadPin(SD_DETn_GPIO_Port, SD_DETn_Pin)) {
    return true;
  }
  else {
    return false;
  }
}

void Mmcsd_SlowClock_Switch(bool status)
{
    if(true==status) {
        hspi1.Instance->CR1 &= ~(SPI_CR1_BR_Msk); 
        hspi1.Instance->CR1 |= SPI_BAUDRATEPRESCALER_256;
    }
    else {
        hspi1.Instance->CR1 &= ~(SPI_CR1_BR_Msk); 
        hspi1.Instance->CR1 |= SPI_BAUDRATEPRESCALER_2;
    }
}



HAL_StatusTypeDef Mmcsd_Spi_Send(uint8_t *txbuf, uint16_t datanum)
{
    return HAL_SPI_Transmit(&hspi1, txbuf, datanum, 100);
}

HAL_StatusTypeDef Mmcsd_Spi_Exchange(uint8_t *txbuf, uint8_t *rxbuf, uint16_t datanum)
{
    return HAL_SPI_TransmitReceive(&hspi1, txbuf, rxbuf, datanum, 100);
}

HAL_StatusTypeDef Mmcsd_Spi_Receive(uint8_t *rxbuf, uint16_t datanum)
{
    HAL_StatusTypeDef status;

    GPIO_InitTypeDef GPIO_InitStruct;

    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_SET);
      
    GPIO_InitStruct.Pin = GPIO_PIN_5;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;    
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    
    if(1==datanum) {
        status = HAL_SPI_Receive(&hspi1, rxbuf, datanum, 10);
    }
    else {
        status = HAL_SPI_Receive_DMA(&hspi1, rxbuf, datanum);
        if(HAL_OK==status) {
            osSemaphoreWait(SdOperate_Cplt_FlagHandle, osWaitForever);
        }
    }

    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;    
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    return status;
}

void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi)
{
    osSemaphoreRelease(SdOperate_Cplt_FlagHandle);
}

