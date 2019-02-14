#include "Sensor.h"
#include "gpio.h"
#include "spi.h"

typedef enum {
  Select=0x00,
  Deselect
}eOpStatus;

static unsigned char Sensor_RD(unsigned char addr);
static void Sensor_WR(unsigned char addr, unsigned char wrdata);
static void Sensor_CS(eOpStatus status);

extern SPI_HandleTypeDef hspi2;

void Sensor_Init(void)
{
  //check who I am
  while(0x69!=Sensor_RD(WHO_AM_I));

  /* Configuration Description
   * Disable I2C
   */
  Sensor_WR(CTRL4_C, 0x04);

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
   * Gyroscope ODR: 833Hz
   * Full-scale selecion 2000dps
   */
  Sensor_WR(CTRL2_G, 0x7C);


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
  Sensor_WR(TAP_THS_6D, 0x1F);


  Sensor_WR(WAKE_UP_THS, 0x3F);
  Sensor_WR(WAKE_UP_DUR, 0x60);
  
}

void Sensor_Set(SensorConfig *para)
{
  uint8_t temp;


  //globale sample rate setting
  if(para->GB>0 && para->GB<11) {
    temp=(10-para->GB)<<4;
  }
  else {
    temp=0x90;
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

uint8_t Sensor_isClick(void)
{
  return Sensor_RD(TAP_SRC);
}
uint8_t Sensor_isMove(void)
{
  return Sensor_RD(WAKE_UP_SRC&0x3F);
}

static unsigned char Sensor_RD(unsigned char addr)
{
  unsigned short tx_buf;
  unsigned short rx_buf;
  
  Sensor_CS(Select);
  tx_buf=((unsigned short)(addr | 0x80)) << 8 | 0x00FF;
  HAL_SPI_TransmitReceive(&hspi2, (uint8_t *)(&tx_buf), (uint8_t *)(&rx_buf), 1, 10);
  Sensor_CS(Deselect);
  return (unsigned char)(rx_buf & 0xFF);
}

//SPI1写函数
static void Sensor_WR(unsigned char addr, unsigned char wrdata)
{
  unsigned short tx_buf;
  Sensor_CS(Select);
  tx_buf=((unsigned short)(addr & 0x7f)) << 8 | wrdata;
  HAL_SPI_Transmit(&hspi2, (uint8_t *)(&tx_buf), 1, 10);
  Sensor_CS(Deselect);
}

static void Sensor_CS(eOpStatus status)
{
  if(Select==status) {
    HAL_GPIO_WritePin(SENSOR_CSN_GPIO_Port, SENSOR_CSN_Pin, GPIO_PIN_RESET);
  }
  else {
    HAL_GPIO_WritePin(SENSOR_CSN_GPIO_Port, SENSOR_CSN_Pin, GPIO_PIN_SET);
  }
}


