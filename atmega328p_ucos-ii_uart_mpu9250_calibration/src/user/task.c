#include "task.h"

void blink(void *pdata)
{
	(void)pdata;
	while (1) {
		ledOn();
		OSTimeDly(1);
		ledOff();
		OSTimeDly(29);
		ledOn();
		OSTimeDly(1);
		ledOff();
		OSTimeDly(219);
	}
}

void serial(void *pdata)
{
	char *str;

	(void)pdata;
	while (1) {
		usart0Read(&str);
		usart0Print(str);
	}
}

void mpu6050Task(void *pdata)
{
	uint8 mpuIntStatus;
	uint8 fifoBuffer[48];
	int16 q[4];

	(void)pdata;
	while (mpu6050DMPInitFromTask());
	while (1) {
		mpu6050DMPSemPend();
		mpuIntStatus = MPU6050getIntStatus();
		if (mpuIntStatus & 0x02) {
			MPU6050getFIFOBytes(fifoBuffer, 48);
			MPU6050dmpGetMag(q, fifoBuffer);
			/*
			usart0Hex16(q[0]);
			usart0Print(" ");
			usart0Hex16(q[1]);
			usart0Print(" ");
			usart0Hex16(q[2]);
			//usart0Print(" ");
			//usart0Hex16(q[3]);
			usart0Print("\r\n");
			*/
			usart0PrintLen((char *)q, 6);
		} else if (mpuIntStatus & 0x10) {
			MPU6050resetFIFO();
		} else {
			MPU6050resetFIFO();			
		}
	}
}
