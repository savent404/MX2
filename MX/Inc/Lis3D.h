#ifndef _LIS3D_H_
#define _LIS3D_H_
#include "mx-gpio.h"
#include "mx-spi.h"
#include <stdint.h>
typedef struct Source
{
  uint8_t Dx_L;
  uint8_t Dy_L;
  uint8_t Dz_L;
  uint8_t Dx_H;
  uint8_t Dy_H;
  uint8_t Dz_H;
} Lis3dSourceData;

typedef struct Data
{
  int16_t Dx;
  int16_t Dy;
  int16_t Dz;
} Lis3dData;

typedef struct _Lis3dConfig
{
  uint16_t MD;
  uint16_t MT;
  uint16_t CD;
  uint16_t CT;
  uint16_t CL;
  uint16_t CW;
} Lis3dConfig;

void Lis3d_Init(void);
void Lis3d_Set(Lis3dConfig *para);
uint8_t Lis3d_isClick(void);
uint8_t Lis3d_isMove(void);

#endif
