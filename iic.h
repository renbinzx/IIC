#ifndef __IIC_H__
#define __IIC_H__
#include "stm32l1xx.h"

#define ADDR_8						8
#define	ADDR_16						16

#define 	IIC_ADDR7						0
#define 	IIC_ADDR10					1

typedef struct
{
	uint16_t 	Devices_Addr;
	uint8_t  	D_Addr_Mode;
	uint16_t	RAM_Addr;
	uint8_t		R_Addr_Mode;
	uint8_t		*Data;
	uint8_t		DataSize;
}I2C_Type;

void IIC_Init(void);
uint8_t I2C_ReadData(I2C_Type i2cPar);
uint8_t I2C_WriteData(I2C_Type i2cPar);

#endif