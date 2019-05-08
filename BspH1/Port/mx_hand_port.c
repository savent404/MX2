#include "hand.h"
#include "Sensor.h"
#include "USR_CONFIG.h"
#include "gpio.h"
#include "spi.h"
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include "cmsis_os.h"

#define STAB_FREEZE (1000)
#define STAB_GYRO (200)
#define FIFO_SAMPLE (64)
#define POOLSIZE (FIFO_SAMPLE * 6)

#define u16(reg)  ((uint16_t)(reg))

#define sw_u16(reg) \
    (((u16(reg) >> 8) & 0xFF) | ((u16(reg) << 8) & 0xFF00))

#define ACC_MAXIUM_RANGE (8.0f)
#define GYRO_MAXIUM_RANGE (2000.0f)

#define convert_acc(intergrate) (intergrate / (float)INT16_MAX * ACC_MAXIUM_RANGE)
#define convert_gyro(intergrate) (intergrate / (float)INT16_MAX * GYRO_MAXIUM_RANGE)

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

typedef struct __FifoFrameTypeDef{
//     int16_t gyrox;
//     int16_t gyroy;
//     int16_t gyroz;
//     int16_t accx;
//     int16_t accy;
//     int16_t accz;
    int16_t gyroz;
    int16_t gyrox;
    int16_t gyroy;
    int16_t accz;
    int16_t accx;
    int16_t accy;
}FifoFrameTypeDef, *pFifoFrameTypeDef;

typedef struct __DataPoolTypeDef{
    uint16_t dataNumber;
    uint16_t dataIndex;
    FifoFrameTypeDef dataFrame[FIFO_SAMPLE];
}DataPoolTypeDef, *pDataPoolTypeDef;

extern __IO uint32_t uwTick;

static void Sensor_Init(void);
static void Sensor_Set(SensorConfig *para);
static uint8_t Sensor_isClick(void);
static uint8_t Sensor_isMove(void);
static bool Sensor_isStab(int16_t *data_buf);
static uint8_t Sensor_isSpin(int16_t * data_buf);
static bool MX_Hand_HW_FIFOpollingStart(void);


uint8_t Sensor_RD(uint8_t addr);
static void Sensor_WR(uint8_t addr, uint8_t wrdata);
static void Sensor_CS(eOpStatus status);
static void Sensor_SPI_WR(uint16_t Data);



extern SPI_HandleTypeDef hspi2;
extern TIM_HandleTypeDef htim5;

static osSemaphoreId SensorTx_Start_FlagHandle;
static osSemaphoreId SensorRx_Cplt_FlagHandle;
static pDataPoolTypeDef pDataPool;

void Sensor_MultiDataRD(uint8_t addr, uint16_t *pRxData, uint8_t Size);

extern HAL_StatusTypeDef HAL_Sensor_Polling_IT(SPI_HandleTypeDef *hspi, uint8_t *pTxData, uint8_t *pRxData, uint16_t Size);

void MX_HAND_HW_Init(void)
{
    osSemaphoreDef(SensorRx_Cplt_Flag);
    SensorRx_Cplt_FlagHandle = osSemaphoreCreate(osSemaphore(SensorRx_Cplt_Flag), 1);
    SensorTx_Start_FlagHandle = osSemaphoreCreate(osSemaphore(SensorRx_Cplt_Flag), 1);
    osSemaphoreWait(SensorRx_Cplt_FlagHandle, 0);
    osSemaphoreWait(SensorTx_Start_FlagHandle, 0);

    pDataPool = pvPortMalloc(sizeof(DataPoolTypeDef));

    __HAL_SPI_DISABLE_IT(&hspi2, (SPI_IT_TXE | SPI_IT_RXNE | SPI_IT_ERR));

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

    HAL_NVIC_EnableIRQ(EXTI0_IRQn);
}

void MX_HAND_HW_DeInit(void)
{
}

static void Sensor_Init(void)
{
  __HAL_SPI_ENABLE(&hspi2);
  
  /* Configuration Description
   * Trigger software reset
   */
  Sensor_WR(CTRL3_C, 0x01);

  osDelay(1);
  
  //check who I am
  while(0x69!=Sensor_RD(WHO_AM_I)); 

  /* Configuration Description
   * 
   */
  //Sensor_WR(CTRL4_C, 0x02);
  //Sensor_WR(CTRL4_C, 0x03);
  Sensor_WR(CTRL4_C, 0x01);

  //osDelay(1);
  
  while(0x69!=Sensor_RD(WHO_AM_I));

  /* Configuration Description
   * Enable XYZ accelerometer output
   */
  Sensor_WR(CTRL9_XL, 0x38);
  
  /* Configuration Description
   * DOR : 1.66KHz
   * Full-scale selecion 8G
   * Anti-aliasing filter bandwidth selection 400Hz
   */
  Sensor_WR(CTRL1_XL, 0x8C);


  /* Configuration Description
   * Enable Gyro X, Y, Z
   */
  Sensor_WR(CTRL10_C, 0x38);

  /* Configuration Description
   * Gyroscope ODR: 1.66KHz
   * Full-scale selecion 2000dps
   */
  Sensor_WR(CTRL2_G, 0x8C);


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

  /* Configuration Description
   * enable block data update, interrupt active low, INT push pull, Swap LSB & MSB
   */
  Sensor_WR(CTRL3_C, 0x46);

  /* Configuration Description
   * Enable FIFO full interrupt on INT1
   */
  Sensor_WR(INT1_CTRL, 0x20);
  /* Configuration Description
   * Set watermark to FIFO_SAMPLE samples
   */
  uint16_t sampleNumber = POOLSIZE + 6;
  
  Sensor_WR(FIFO_CONTROL1, (uint8_t)(sampleNumber & 0x00FF));
  Sensor_WR(FIFO_CONTROL2, (uint8_t)((sampleNumber & 0x0F00) >> 8));
  Sensor_WR(FIFO_CONTROL3, 0x09);
  Sensor_WR(FIFO_CONTROL4, 0x00);
  Sensor_WR(FIFO_CONTROL5, 0x00);

  while(0x69!=Sensor_RD(WHO_AM_I));
}

static void Sensor_Set(SensorConfig *para)
{
  uint8_t sensorCfg;
  uint8_t sensorReadback;

  /* Configuration Description
   * Clear FIFO content
   */
  Sensor_WR(FIFO_CONTROL5, 0x00);

  //globale sample rate setting
  if(para->GB>0 && para->GB<11) {
    sensorCfg=(10-para->GB)<<4;
  }
  else {
    sensorCfg=0x80;
  }

  sensorReadback = Sensor_RD(CTRL1_XL);
  Sensor_WR(CTRL1_XL, (sensorCfg & 0xF0) | (sensorReadback & 0x0F));
  sensorReadback = Sensor_RD(CTRL2_G);
  Sensor_WR(CTRL2_G, (sensorCfg & 0xF0) | (sensorReadback & 0x0F));

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

  /* Configuration Description
   * Set the sample rate as GB, enable FIFO
   */
  Sensor_WR(FIFO_CONTROL5, sensorCfg >> 1 | 0x01);
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

bool MX_HAND_HW_getData(float acc[3], float gyro[3])
{
    uint16_t num, index;
    num = pDataPool->dataNumber;
    index = pDataPool->dataIndex;

    if (!num || index >= num)
    {
        do {
            num = pDataPool->dataNumber;
            index = pDataPool->dataIndex;
            // need read hw to get data
            if (!MX_Hand_HW_FIFOpollingStart()) {
                return false;
            }
        } while (!num || index >= num);
    }

    // get from buffer
    pFifoFrameTypeDef pF = pDataPool->dataFrame + index/6;

    acc[0] = convert_acc(pF->accx);
    acc[1] = convert_acc(pF->accy);
    acc[2] = convert_acc(pF->accz);
    gyro[0] = convert_gyro(pF->gyrox);
    gyro[1] = convert_gyro(pF->gyroy);
    gyro[2] = convert_gyro(pF->gyroz);

    index += 6;
    if (index >= num) {
        pDataPool->dataNumber = 0;
        pDataPool->dataIndex = 0;
    } else {
        pDataPool->dataIndex = index;
    }

    return true;
}


bool MX_Hand_HW_FIFOpollingStart(void)
{
    uint8_t temp;
    uint16_t tp[1];
    
    if(osOK == osSemaphoreWait(SensorTx_Start_FlagHandle, 0)) {
        uint16_t spiTxBuffer = 0xFFFF;

        pDataPool->dataIndex = 0x0000;

        Sensor_MultiDataRD(FIFO_STATUS1, tp, 1);

        pDataPool->dataNumber = sw_u16(*tp) & 0x0FFF;

        __HAL_SPI_DISABLE(&hspi2);
        hspi2.Instance->CR1 &= ~SPI_DATASIZE_16BIT;
        __HAL_SPI_ENABLE(&hspi2);

        Sensor_CS(Select);
        *((__IO uint8_t*)&hspi2.Instance->DR) = 0x80 | FIFO_DATA_OUT_L;
        while(!(hspi2.Instance->SR & SPI_FLAG_TXE));
        while(hspi2.Instance->SR & SPI_FLAG_BSY);
        __HAL_SPI_CLEAR_OVRFLAG(&hspi2);

        __HAL_SPI_DISABLE(&hspi2);
        hspi2.Instance->CR1 |= SPI_DATASIZE_16BIT;
        __HAL_SPI_ENABLE(&hspi2);

        if (pDataPool->dataNumber > POOLSIZE) {
            pDataPool->dataNumber = POOLSIZE;
        }
        HAL_Sensor_Polling_IT(&hspi2, (uint8_t *)spiTxBuffer, (uint8_t *)pDataPool->dataFrame, pDataPool->dataNumber);
            
        osSemaphoreWait(SensorRx_Cplt_FlagHandle, osWaitForever);
        Sensor_CS(Deselect);

        temp = Sensor_RD(FIFO_CONTROL5);
        
        Sensor_WR(FIFO_CONTROL5, temp & 0xFE);

        osDelay(1);
        
        Sensor_WR(FIFO_CONTROL5, temp | 0x01);
    
        return true;
    }
    else {
        return false;
    }
}

void Sensor_SPI_RxCpltCallback(void)
{
    osSemaphoreRelease(SensorRx_Cplt_FlagHandle);
}

void Sensor_SPI_Polling_StartCallback(void)
{
    osSemaphoreRelease(SensorTx_Start_FlagHandle);
}

