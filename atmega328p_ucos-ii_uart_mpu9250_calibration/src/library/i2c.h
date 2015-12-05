#ifndef __I2C_H__
#define __I2C_H__

#include "define.h"

#define I2C_TIME_OUT		0xFFFFFFFF

void i2cInit(void);
INT8U i2cRegR(uint8 address, uint8 reg, uint8 *rx, uint8 rxSize);
INT8U i2cRegW(uint8 address, uint8 reg, uint8 *tx, uint8 txSize);
uint8 i2cReadCount(void);
uint8 i2cWriteCount(void);
uint8 i2cReadErrorCode(void);

#endif
