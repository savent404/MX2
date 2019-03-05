#include "hand.h"
#include "Sensor.h"
#include "USR_CONFIG.h"
#include "gpio.h"
#include "spi.h"
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#define STAB_FREEZE (1000)
#define STAB_GYRO (200)


typedef enum {
  Select=0x00,
  Deselect
}eOpStatus;

typedef enum {
  SP_NONE=0x00,
  SP_L1,
  SP_L2,
  SP_L3,
  SP_L4
}eSpStatus;

extern __IO uint32_t uwTick;

static void Sensor_Init(void);
static void Sensor_Set(SensorConfig *para);
static uint8_t Sensor_isClick(void);
static uint8_t Sensor_isMove(void);
static bool Sensor_isStab(int16_t *data_buf);
static uint8_t Sensor_isSpin(int16_t * data_buf);


uint8_t Sensor_RD(uint8_t addr);
static void Sensor_WR(uint8_t addr, uint8_t wrdata);
static void Sensor_CS(eOpStatus status);
static void Sensor_SPI_WR(uint16_t Data);



extern SPI_HandleTypeDef hspi2;

void Sensor_MultiDataRD(uint8_t addr, uint16_t *pRxData, uint8_t Size);


bool MX_HAND_Init(void)
{
    Sensor_Init();
    SensorConfig config;
    config.CD = USR.config->CD;
    config.CL = USR.config->CL;
    config.CT = USR.config->CT;
    config.CW = USR.config->CW;
    config.MD = USR.config->MD;
    config.MT = USR.config->MT;
    config.GB = USR.config->GB;
    config.ST = USR.config->ST;
    config.SPL1 = USR.config->SPL1;
    config.SPL2 = USR.config->SPL2;
    config.SPL3 = USR.config->SPL3;
    config.SPL4 = USR.config->SPL4;
    Sensor_Set(&config);
    return true;
}

bool MX_HAND_DeInit(void)
{
    return true;
}

HAND_TriggerId_t MX_HAND_GetTrigger(void)
{
    int16_t SensorRawData[6];
    HAND_TriggerId_t res;
    res.hex = 0;
    uint8_t isClik = Sensor_isClick();
    uint8_t isMove = Sensor_isMove();
    Sensor_MultiDataRD(OUTX_L_G, (uint16_t *)SensorRawData, 6);
    bool isStab = Sensor_isStab(SensorRawData);
    uint8_t isSpin = Sensor_isSpin(SensorRawData);

    if (isClik)
        res.unio.isClash = true;
    if (isMove)
        res.unio.isSwing = true;
    if (isStab)
        res.unio.isStab = true;
    return res;
}

static void Sensor_Init(void)
{
  __HAL_SPI_ENABLE(&hspi2);
  
  //check who I am
  while(0x69!=Sensor_RD(WHO_AM_I));     

  /* Configuration Description
   * Trigger software reset
   */
  Sensor_WR(CTRL3_C, 0x01);

  osDelay(50);

  /* Configuration Description
   * Disable I2C
   */
  Sensor_WR(CTRL4_C, 0x04);

  /* Configuration Description
   * Swap LSB & MSB
   */
  Sensor_WR(CTRL3_C, 0x06);

  /* Configuration Description
   * Enable XYZ accelerometer output
   */
  Sensor_WR(CTRL9_XL, 0x38);
  
  /* Configuration Description
   * DOR : 833Hz
   * Full-scale selecion 8G
   * Anti-aliasing filter bandwidth selection 400Hz
   */
  Sensor_WR(CTRL1_XL, 0x7C);


  /* Configuration Description
   * Enable Gyro X, Y, Z
   */
  Sensor_WR(CTRL10_C, 0x38);

  /* Configuration Description
   * Gyroscope ODR: 104Hz
   * Full-scale selecion 1000dps
   */
  Sensor_WR(CTRL2_G, 0x78);


  /* Configuration Description
   * Enable TAP_X/Y/Z
   */
  Sensor_WR(TAP_CFG, 0x0E);

  /* Configuration Description
   * Duration of maximun time gap for double tap 4
   * Quiet time after a tap 4 ODR_TIME
   * Shock duration 8 ODR_TIME
   */
  Sensor_WR(INT_DUR2, 0x02);

  /* CLICK_THS Threshold
   * Disable D4D
   * Set D6D threshold 80 degrees
   */
  Sensor_WR(TAP_THS_6D, 0x10);


  Sensor_WR(WAKE_UP_THS, 0x3F);
  Sensor_WR(WAKE_UP_DUR, 0x60);
  
}

static void Sensor_Set(SensorConfig *para)
{
  uint8_t temp;


  //globale sample rate setting
  if(para->GB>0 && para->GB<11) {
    temp=(10-para->GB)<<4;
  }
  else {
    temp=0x80;
  }
  
  Sensor_WR(CTRL1_XL, temp);
  Sensor_WR(CTRL2_G, temp);

  //click setting
  if(para->CT>0 && para->CT<33) {
    Sensor_WR(TAP_THS_6D, para->CT-1);
  }
  else {
    Sensor_WR(TAP_THS_6D, 0x1F);
  }

  if(para->CD>0 && para->CD<5) {
    Sensor_WR(INT_DUR2, para->CD-1);
  }
  else {
    Sensor_WR(INT_DUR2, 0x03);
  }

  //swing setting
  if(para->MT>0 && para->MT<65) {
    Sensor_WR(WAKE_UP_THS, para->MT-1);
  }
  else {
    Sensor_WR(WAKE_UP_THS, 0x3F);
  }

  if(para->MD>0 && para->MD<5) {
    Sensor_WR(WAKE_UP_DUR, (para->MD-1)<<6);
  }
  else {
    Sensor_WR(WAKE_UP_DUR, 0x60);
  }

}

static uint8_t Sensor_isClick(void)
{
  return Sensor_RD(TAP_SRC);
}
static uint8_t Sensor_isMove(void)
{
  return Sensor_RD(WAKE_UP_SRC&0x3F);
}
static bool Sensor_isStab(int16_t * data_buf)
{
    static int16_t pre_acc_y;
    static uint32_t timestamp=0x00000000;
    int16_t dif_acc_y;
    SensorData * ptrGyroDate;
    SensorData * ptrAccDate;

    ptrGyroDate = (SensorData *)data_buf;
    ptrAccDate = (SensorData *)&data_buf[3];

    dif_acc_y = ptrGyroDate->Dy - pre_acc_y;
    pre_acc_y = ptrGyroDate->Dy;
    if(uwTick - timestamp > STAB_FREEZE && dif_acc_y > USR.config->ST && 
       abs(ptrAccDate->Dx) < STAB_GYRO && abs(ptrAccDate->Dy) < STAB_GYRO && abs(ptrAccDate->Dz) < STAB_GYRO) {
        timestamp = uwTick;
        return true;
    }
    else {
        return false;
    }
}
static uint8_t Sensor_isSpin(int16_t *data_buf)
{
    static eSpStatus CurrentSpStatus=SP_NONE;
    SensorData * ptrGyroDate;
    SensorData * ptrAccDate;
    uint32_t SpLevel;

    ptrGyroDate = (SensorData *)data_buf;
    ptrAccDate = (SensorData *)&data_buf[3];
    
    SpLevel = ptrGyroDate->Dx * ptrGyroDate->Dx + ptrGyroDate->Dz * ptrGyroDate->Dz;

    switch(SpLevel) {
        case SP_L1:
            break;
        case SP_L2:
            break;
        case SP_L3:
            break;
        case SP_L4:
            break;
        default:
            break; 
    }
    
    return 0x00;
}


uint8_t Sensor_RD(uint8_t addr)
{
  uint16_t tx_buf;
  uint16_t rx_buf;
  
  Sensor_CS(Select);
  tx_buf=((uint16_t)(addr | 0x80)) << 8 | 0x00FF;

  *((__IO uint16_t*)&hspi2.Instance->DR) = tx_buf;

  while(!(hspi2.Instance->SR & SPI_FLAG_TXE));

  while(hspi2.Instance->SR & SPI_FLAG_BSY);

  while(!(__HAL_SPI_GET_FLAG(&hspi2, SPI_FLAG_RXNE)));

  rx_buf = hspi2.Instance->DR;

  __HAL_SPI_CLEAR_OVRFLAG(&hspi2);

  Sensor_CS(Deselect);
  return (uint8_t)(rx_buf & 0xFF);
}

//SPI1写函数
static void Sensor_WR(uint8_t addr, uint8_t wrdata)
{
  uint16_t tx_buf;
  Sensor_CS(Select);
  tx_buf=((uint16_t)(addr & 0x7f)) << 8 | wrdata;

  Sensor_SPI_WR(tx_buf);
  
  Sensor_CS(Deselect);
}

static void Sensor_CS(eOpStatus status)
{
  if(Select==status) {
    HAL_GPIO_WritePin(SENSOR_CSN_GPIO_Port, SENSOR_CSN_Pin, GPIO_PIN_RESET);
  }
  else {
    HAL_GPIO_WritePin(SENSOR_CSN_GPIO_Port, SENSOR_CSN_Pin, GPIO_PIN_SET);
    __IO uint8_t delay;
    for(delay=0;delay<7;delay++);
  }
}

void Sensor_MultiDataRD(uint8_t addr, uint16_t *pRxData, uint8_t Size)
{    
    Sensor_CS(Select);

    /* Variable used to alternate Rx and Tx during transfer */
    bool txallowed = 0U;

    Size++;
    hspi2.TxXferCount = Size;
    hspi2.RxXferCount = Size;

    addr--;
    *(__IO uint16_t *)&hspi2.Instance->DR = ((uint16_t)(addr |= 0x80) << 8) | 0x00FF;

    hspi2.TxXferCount--;

    while((hspi2.TxXferCount > 0U) || (hspi2.RxXferCount > 0U))
    {
        /* check TXE flag */
        if(txallowed && (hspi2.TxXferCount > 0U) && (__HAL_SPI_GET_FLAG(&hspi2, SPI_FLAG_TXE)))
        {
          *(__IO uint16_t *)&hspi2.Instance->DR = 0xFFFF;
          
          hspi2.TxXferCount--;
          /* Next Data is a reception (Rx). Tx not allowed */ 
          txallowed = 0U;
        }

        /* Wait until RXNE flag is reset */
        if((hspi2.RxXferCount > 0U) && (__HAL_SPI_GET_FLAG(&hspi2, SPI_FLAG_RXNE)))
        {
          if(Size==hspi2.RxXferCount) {       
            (*pRxData) = hspi2.Instance->DR;
          }
          else {
            (*pRxData++) = hspi2.Instance->DR;
          }
          hspi2.RxXferCount--;
          /* Next Data is a Transmission (Tx). Tx is allowed */ 
          txallowed = 1U;
        }
    }

    while(!(hspi2.Instance->SR & SPI_FLAG_TXE));

    while(hspi2.Instance->SR & SPI_FLAG_BSY);

    __HAL_SPI_CLEAR_OVRFLAG(&hspi2);
    
    Sensor_CS(Deselect);
}


static void Sensor_SPI_WR(uint16_t Data)
{
    *((__IO uint16_t*)&hspi2.Instance->DR) = Data;

    while(!(hspi2.Instance->SR & SPI_FLAG_TXE));

    while(hspi2.Instance->SR & SPI_FLAG_BSY);

    __HAL_SPI_CLEAR_OVRFLAG(&hspi2);
}
