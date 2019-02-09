#include "Lis3D.h"
#include "stm32f1xx_hal.h"
#include "spi.h"
#include "main.h"

static unsigned char LIS3DH_SPI_RD(unsigned char addr);
static void LIS3DH_SPI_WR(unsigned char addr,unsigned char wrdata);
static void SPI1_CS_Low(void);
static void SPI1_CS_High(void);
static uint8_t SPI_LIS3DH_SendByte(uint8_t byte);

void Lis3d_Init(void)
{
  /* Configuration Description
   * Normal Mode
   * High-pass filter set to 1Hz
   * DOR : 400Hz
   * Click Interrupt on INT1
   * Enable All axis Click Detect
	 * Threshold: 1.8Kg
	 * TIME limit 20ms
   */
  LIS3DH_SPI_WR(0x20, 0x77); // CTRL_REG1: ODR:400Hz, Enable All axis

  /** High-Pass filter Configuration
    * Normal mode (reset reading HP_RESET_FILTER)
    * 1Hz cutoff frequency, When ODR:400Hz
    * Enable High-Pass filter on output register
    * Enable High-Pass filter for CLICK function
    * Enable High-Pass filter for AOI on interrupt 1&2
    */
  LIS3DH_SPI_WR(0x21, 0x3F);

  /** Interrupt 1 Configuration
    * Enable CLICK interrupt on INT1
    */
  LIS3DH_SPI_WR(0x22, 0x80);

  /** CTRL_REG4
    * Blacking output data
    * Full scale ±8g
    * SPI interface Enable
    * Big endian data
    * Disable High-resolution mode
    * No self-test
    */
  LIS3DH_SPI_WR(0x23, 0xB0);

  /** CLICK_CFG Configuration
    * Enable x/y/z single click interrupt request
    */
  LIS3DH_SPI_WR(0x38, 0x15);

  /** CLIK_THS Threshold
    * Keep interrupt pin high for the duration fo the latency Window
    * 1 LSB = full scale/128
    */
  LIS3DH_SPI_WR(0x3A, 30); //About ±1.875Kg threshold

  /** CLICK_TIME
    * 1 LSB = 1/ODR, ODR:400Hz
    */
  LIS3DH_SPI_WR(0x3B, 4); //[*]About 10ms

  /** TIME_LATENCY
    * after the first click detection
      \where the click-detection procedure is disabled.
    * 1 LSB = 1/ODR
    */
  LIS3DH_SPI_WR(0x3C, 30); //About 75ms
  /** TIME Window
    * 1 LSB = 1/ODR
    */
  LIS3DH_SPI_WR(0x3D, 0); //About 0ms


	/* Latch AOI INTERRUPT
	 */
	LIS3DH_SPI_WR(0x24, 0x80);

	/* Enable AOI on interrupt 2
	 */
	LIS3DH_SPI_WR(0x25, 0x40);

	/* INT1 CFG
	 * Enable 6D detection
	 * And every aixs
	 */
	LIS3DH_SPI_WR(0x30, 0x7F);

	/* INT1 THS
	 * Never mind
	 */
	LIS3DH_SPI_WR(0x32, 2);

	/* INT1_DURATION
	 * LSB : 2.5ms
	 */
	LIS3DH_SPI_WR(0x33, 30); // 50ms
}

void Lis3d_Set(Lis3dConfig *para) {
  /* Configuration Description
   * Normal Mode
   * High-pass filter set to 1Hz
   * DOR : 400Hz
   * Click Interrupt on INT1
   * Enable All axis Click Detect
	 * Threshold: 1.8Kg
	 * TIME limit 20ms
   */
  LIS3DH_SPI_WR(0x20, 0x77); // CTRL_REG1: ODR:400Hz, Enable All axis

  /** High-Pass filter Configuration
    * Normal mode (reset reading HP_RESET_FILTER)
    * 1Hz cutoff frequency, When ODR:400Hz
    * Enable High-Pass filter on output register
    * Enable High-Pass filter for CLICK function
    * Enable High-Pass filter for AOI on interrupt 1&2
    */
  LIS3DH_SPI_WR(0x21, 0x3F);

  /** Interrupt 1 Configuration
    * Enable CLICK interrupt on INT1
    */
  LIS3DH_SPI_WR(0x22, 0x80);

  /** CTRL_REG4
    * Blacking output data
    * Full scale ±8g
    * SPI interface Enable
    * Big endian data
    * Disable High-resolution mode
    * No self-test
    */
  LIS3DH_SPI_WR(0x23, 0xB0);

  /** CLICK_CFG Configuration
    * Enable x/y/z single click interrupt request
    */
  LIS3DH_SPI_WR(0x38, 0x15);

  /** CLIK_THS Threshold
    * Keep interrupt pin high for the duration fo the latency Window
    * 1 LSB = full scale/128
    */
  LIS3DH_SPI_WR(0x3A, para->CT); //About ±1.875Kg threshold

  /** CLICK_TIME
    * 1 LSB = 1/ODR, ODR:400Hz
    */
  LIS3DH_SPI_WR(0x3B, para->CD); //[*]About 10ms

  /** TIME_LATENCY
    * after the first click detection
      \where the click-detection procedure is disabled.
    * 1 LSB = 1/ODR
    */
  LIS3DH_SPI_WR(0x3C, para->CL); //About 75ms
  /** TIME Window
    * 1 LSB = 1/ODR
    */
  LIS3DH_SPI_WR(0x3D, para->CW); //About 0ms


	/* Latch AOI INTERRUPT
	 */
	LIS3DH_SPI_WR(0x24, 0x80);

	/* Enable AOI on interrupt 2
	 */
	LIS3DH_SPI_WR(0x25, 0x40);

	/* INT1 CFG
	 * Enable 6D detection
	 * And every aixs
	 */
	LIS3DH_SPI_WR(0x30, 0x7F);

	/* INT1 THS
	 * Never mind
	 */
	LIS3DH_SPI_WR(0x32, para->MT);

	/* INT1_DURATION
	 * LSB : 2.5ms
	 */
	LIS3DH_SPI_WR(0x33, para->MD); // 50ms
}

uint8_t Lis3d_isClick(void) {
  return LIS3DH_SPI_RD(0x39);
}
uint8_t Lis3d_isMove(void) {
  return LIS3DH_SPI_RD(0x31);
}

static unsigned char LIS3DH_SPI_RD(unsigned char addr)
{
  unsigned char temp;
  SPI1_CS_Low();
  //Delay_Spi(10);
  SPI_LIS3DH_SendByte((addr | 0x80) & 0xbf);
  temp = SPI_LIS3DH_SendByte(0xff);
  //Delay_Spi(10);
  SPI1_CS_High();
  return temp;
}

//SPI1写函数
static void LIS3DH_SPI_WR(unsigned char addr, unsigned char wrdata)
{
  SPI1_CS_Low();
  SPI_LIS3DH_SendByte(addr & 0x7f);
  SPI_LIS3DH_SendByte(wrdata);
  SPI1_CS_High();
}

static void SPI1_CS_Low(void)
{
    HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_RESET);
}
static void SPI1_CS_High(void)
{
    HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_SET);
}

static uint8_t SPI_LIS3DH_SendByte(uint8_t byte)
{
    uint8_t buf[1];
    HAL_SPI_TransmitReceive(&hspi2, (uint8_t*)&byte, buf, 1, 10);
    return *buf;
}
