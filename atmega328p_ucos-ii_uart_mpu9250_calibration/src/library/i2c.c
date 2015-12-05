#include "i2c.h"

/* i2c interrupt disable function is not implemented */

#define i2cIntFlagClr()		TWCR = ex(TWINT) | ex(TWEN) | ex(TWIE)

#define i2cStart()			TWCR = ex(TWINT) | ex(TWSTA) | ex(TWEN) | ex(TWIE)
#define i2cStartClr()		TWCR = ex(TWINT) | ex(TWEN) | ex(TWIE)
#define i2cAck()			TWCR = ex(TWINT) | ex(TWEA) | ex(TWEN) | ex(TWIE)
#define i2cNAck()			TWCR = ex(TWINT) | ex(TWEN) | ex(TWIE)
#define i2cStop()			TWCR = ex(TWINT) | ex(TWSTO) | ex(TWEN) | ex(TWIE)

static uint8 i2cAddress;
static uint8 i2cReg;
static uint8 i2cErrorCode;

static uint8 *i2cRxPtr;
static uint8 i2cRxCnt;
static uint8 i2cRxSize;

static uint8 *i2cTxPtr;
static uint8 i2cTxCnt;
static uint8 i2cTxSize;

static OS_EVENT *i2cReady;

static void (*i2cHandlerPtr)(void);

static void i2cRegRHandler(void);
static void i2cRegWHandler(void);

void i2cInit(void)
{
	/* call enablePullup() in main.c */
	/* enable SCL pull-up */
	PORTC |= ex(5);
	/* enable SDA pull-up */
	PORTC |= ex(4);
	TWSR = 0x00;
	/* 100 kHz, TWBR = 0x48 */
	/* 400 kHz, TWBR = 0x0C */
	TWBR = 0x0C;
	TWCR = 0x04;
	i2cIntFlagClr();
	i2cReady = OSSemCreate(0);
}

INT8U i2cRegW(uint8 address, uint8 reg, uint8 *tx, uint8 txSize)
{
	INT8U err;

	i2cHandlerPtr = i2cRegWHandler;
	i2cAddress = address;
	i2cReg = reg;
	i2cTxPtr = tx;
	i2cTxCnt = 0;
	i2cTxSize = txSize;
	i2cStart();
	OSSemPend(i2cReady, I2C_TIME_OUT, &err);
	return err;
}

INT8U i2cRegR(uint8 address, uint8 reg, uint8 *rx, uint8 rxSize)
{
	INT8U err;

	i2cHandlerPtr = i2cRegRHandler;
	i2cAddress = address;
	i2cReg = reg;
	i2cRxPtr = rx;
	i2cRxCnt = 0;
	i2cRxSize = rxSize;
	i2cStart();
	OSSemPend(i2cReady, I2C_TIME_OUT, &err);
	return err;
}

uint8 i2cReadCount(void)
{
	return i2cRxCnt + 1;
}

uint8 i2cWriteCount(void)
{
	return i2cTxCnt;
}

uint8 i2cReadErrorCode(void)
{
	return i2cErrorCode;
}

static void i2cRegRHandler(void)
{
	i2cErrorCode = TWSR & 0xF8;
	if (i2cErrorCode == 0x08) {
		TWDR = i2cAddress << 1;
		i2cStartClr();
	} else if (i2cErrorCode == 0x10) {
		TWDR = (i2cAddress << 1) | 0x01;
		i2cStartClr();
	} else if (i2cErrorCode == 0x18) {
		TWDR = i2cReg;
		i2cIntFlagClr();
	} else if (i2cErrorCode == 0x20) {
		i2cStop();
		OSIntEnter();
		OSSemPost(i2cReady);
		OSIntExit();
	} else if (i2cErrorCode == 0x28) {
		i2cStart();
	} else if (i2cErrorCode == 0x30) {
		i2cStop();
		OSIntEnter();
		OSSemPost(i2cReady);
		OSIntExit();
	} else if (i2cErrorCode == 0x38) {
		i2cIntFlagClr();
		OSIntEnter();
		OSSemPost(i2cReady);
		OSIntExit();
	} else if (i2cErrorCode == 0x40) {
		if (i2cRxSize < 2) {
			i2cNAck();
		} else {
			i2cAck();
		}
	} else if (i2cErrorCode == 0x48) {
		i2cStop();
		OSIntEnter();
		OSSemPost(i2cReady);
		OSIntExit();
	} else if (i2cErrorCode == 0x50) {
		*(i2cRxPtr + i2cRxCnt) = TWDR;
		i2cRxCnt++;
		if (i2cRxCnt < (i2cRxSize - 1)) {
			i2cAck();
		} else {
			i2cNAck();
		}
	} else if (i2cErrorCode == 0x58) {
		*(i2cRxPtr + i2cRxCnt) = TWDR;
		i2cStop();
		OSIntEnter();
		OSSemPost(i2cReady);
		OSIntExit();
	}
}

static void i2cRegWHandler(void)
{
	i2cErrorCode = TWSR & 0xF8;
	if (i2cErrorCode == 0x08) {
		TWDR = i2cAddress << 1;
		i2cStartClr();
	} else if (i2cErrorCode == 0x10) {
		TWDR = i2cAddress << 1;
		i2cStartClr();
	} else if (i2cErrorCode == 0x18) {
		TWDR = i2cReg;
		i2cIntFlagClr();
	} else if (i2cErrorCode == 0x20) {
		i2cStop();
		OSIntEnter();
		OSSemPost(i2cReady);
		OSIntExit();
	} else if (i2cErrorCode == 0x28) {
		if (i2cTxCnt < i2cTxSize) {
			TWDR = *(i2cTxPtr + i2cTxCnt);
			i2cTxCnt++;
			i2cIntFlagClr();
		} else {
			i2cStop();
			OSIntEnter();
			OSSemPost(i2cReady);
			OSIntExit();
		}
	} else if (i2cErrorCode == 0x30) {
		i2cStop();
		OSIntEnter();
		OSSemPost(i2cReady);
		OSIntExit();
	} else if (i2cErrorCode == 0x38) {
		i2cIntFlagClr();
		OSIntEnter();
		OSSemPost(i2cReady);
		OSIntExit();
	}
}

ISR(TWI_vect)
{
	(*i2cHandlerPtr)();
}
