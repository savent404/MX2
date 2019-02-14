#ifndef _LSM6DS33_H_
#define _LSM6DS33_H_
#include <stdint.h>

#define FUNC_CFG_ACCESS 0x1
#define SENSOR_SYNC_TIME_FRAME 0x4
#define FIFO_CONTROL1 0x6
#define FIFO_CONTROL2 0x7
#define FIFO_CONTROL3 0x8
#define FIFO_CONTROL4 0x9
#define FIFO_CONTROL5 0xA
#define ORIENT_CFG_G 0xB
#define INT1_CTRL 0xD
#define INT2_CTRL 0xE
#define WHO_AM_I 0xF
#define CTRL1_XL 0x10
#define CTRL2_G 0x11
#define CTRL3_C 0x12
#define CTRL4_C 0x13
#define CTRL5_C 0x14
#define CTRL6_C 0x15
#define CTRL7_G 0x16
#define CTRL8_XL 0x17
#define CTRL9_XL 0x18
#define CTRL10_C 0x19
#define MASTER_CONFIG 0x1A
#define WAKE_UP_SRC 0x1B
#define TAP_SRC 0x1C
#define D6D_SRC 0x1D
#define STATUS_REG 0x1E
#define STATUS_SPIAux 0x1E
#define OUT_TEMP_L 0x20
#define OUT_TEMP_H 0x21
#define OUTX_L_G 0x22
#define OUTX_H_G 0x23
#define OUTY_L_G 0x24
#define OUTY_H_G 0x25
#define OUTZ_L_G 0x26
#define OUTZ_H_G 0x27
#define OUTX_L_XL 0x28
#define OUTX_H_XL 0x29
#define OUTY_L_XL 0x2A
#define OUTY_H_XL 0x2B
#define OUTZ_L_XL 0x2C
#define OUTZ_H_XL 0x2D
#define SENSORHUB1_REG 0x2E
#define SENSORHUB2_REG 0x2F
#define SENSORHUB3_REG 0x30
#define SENSORHUB4_REG 0x31
#define SENSORHUB5_REG 0x32
#define SENSORHUB6_REG 0x33
#define SENSORHUB7_REG 0x34
#define SENSORHUB8_REG 0x35
#define SENSORHUB9_REG 0x36
#define SENSORHUB10_REG 0x37
#define SENSORHUB11_REG 0x38
#define SENSORHUB12_REG 0x39
#define FIFO_STATUS1 0x3A
#define FIFO_STATUS2 0x3B
#define FIFO_STATUS3 0x3C
#define FIFO_STATUS4 0x3D
#define FIFO_DATA_OUT_L 0x3E
#define FIFO_DATA_OUT_H 0x3F
#define TIMESTAMP0_REG 0x40
#define TIMESTAMP1_REG 0x40
#define TIMESTAMP2_REG 0x41
#define STEP_TIMESTAMP_L 0x49
#define STEP_TIMESTAMP_H 0x4A
#define STEP_COUNTER_L 0x4B
#define STEP_COUNTER_H 0x4C
#define SENSORHUB13_REG 0x4D
#define SENSORHUB14_REG 0x4E
#define SENSORHUB15_REG 0x4F
#define SENSORHUB16_REG 0x50
#define SENSORHUB17_REG 0x51
#define SENSORHUB18_REG 0x52
#define FUNC_SRC 0x53
#define TAP_CFG 0x58
#define TAP_THS_6D 0x59
#define INT_DUR2 0x5A
#define WAKE_UP_THS 0x5B
#define WAKE_UP_DUR 0x5C
#define FREE_FALL 0x5D
#define MD1_CFG 0x5E
#define MD2_CFG 0x5F
#define OUT_MAG_RAW_X_L 0x66
#define OUT_MAG_RAW_X_H 0x67
#define OUT_MAG_RAW_Y_L 0x68
#define OUT_MAG_RAW_Y_H 0x69
#define OUT_MAG_RAW_Z_L 0x6A
#define OUT_MAG_RAW_Z_H 0x6B
#define CTRL_SPIAux 0x70

typedef struct _SensorSourceData
{
  uint8_t Dx_L;
  uint8_t Dy_L;
  uint8_t Dz_L;
  uint8_t Dx_H;
  uint8_t Dy_H;
  uint8_t Dz_H;
} SensorSourceData;

typedef struct _SensorData
{
  int16_t Dx;
  int16_t Dy;
  int16_t Dz;
} SensorData;

typedef struct _SensorConfig
{
  uint16_t GB;
  uint16_t MD;
  uint16_t MT;
  uint16_t CD;
  uint16_t CT;
  uint16_t CL;
  uint16_t CW;
} SensorConfig;

void Sensor_Init(void);
void Sensor_Set(SensorConfig *para);
uint8_t Sensor_isClick(void);
uint8_t Sensor_isMove(void);

#endif
