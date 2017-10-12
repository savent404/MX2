#ifndef _MX_SPI_H_
#define _MX_SPI_H_

#include "mx-config.h"
#include "spi.h"

void MX_SPI_Lis3dh_TxRx(uint8_t *tx, uint8_t *rx, uint8_t cnt);
#endif
