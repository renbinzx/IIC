#include "iic.h"



#define 	I2C_US						500
#define 	IIC_SEND_ACK			0x10
#define 	IIC_SEND_NOACK		0x11

#define 	IIC_STATE_NOACK		0x20
#define 	IIC_STATE_ACK			0x21
#define 	IIC_READ					0X01
#define 	IIC_WRITE					0X00


#define		IIC_SCK_HIGH()		GPIO_SetBits(GPIOB, GPIO_Pin_8)
#define		IIC_SCK_LOW()			GPIO_ResetBits(GPIOB, GPIO_Pin_8)
#define		IIC_SDA_HIGH()		GPIO_SetBits(GPIOB, GPIO_Pin_9)
#define		IIC_SDA_LOW()			GPIO_ResetBits(GPIOB, GPIO_Pin_9)
#define		IIC_SDA_GET()			GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_9)
#define		IIC_SDA_OUT()			GPIOB->MODER |= 1 << 9*2
#define		IIC_SDA_IN()			GPIOB->MODER &= ~(3<<9*2)


void IIC_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);
	
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
		
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
	
}

void DelayUs(uint32_t us)
{
	while(us)
	{
		us--;
	}
}

void IIC_Start()
{
	IIC_SDA_OUT();
	
	IIC_SCK_HIGH();
	IIC_SDA_HIGH();

	DelayUs(I2C_US/2);
	IIC_SDA_LOW();
	
	DelayUs(I2C_US/2);
	IIC_SCK_LOW();
	DelayUs(I2C_US);
}


void IIC_Stop()
{
	//IIC_SCK_OUT();
	//IIC_SDA_OUT();
	IIC_SDA_OUT();
	
	IIC_SDA_LOW();
	DelayUs(I2C_US);
	IIC_SCK_HIGH();		
	DelayUs(I2C_US);
	IIC_SDA_HIGH();
	DelayUs(I2C_US);
}


uint8_t IIC_WriteByte(unsigned char Data)
{
	uint8_t i, IIC_State;
	
	
	IIC_SCK_LOW();
	DelayUs(I2C_US/2);
	for(i=0;i<8;i++)
	{
		IIC_SCK_LOW();
		DelayUs(I2C_US/2);
		if( (Data&0x80)==0x00 )
		{
			IIC_SDA_LOW();
		}
		else
		{
			IIC_SDA_HIGH();
		}
		DelayUs(I2C_US/2);//after level change,wait a few microseconds
		IIC_SCK_HIGH();
		DelayUs(I2C_US);
		Data <<= 1;
	}
	
	IIC_SCK_LOW(); //ACK
	DelayUs(I2C_US);
//	IIC2_SDA_HIGH();
//	I2C2_DelayUs(I2C2_US/2);
	IIC_SCK_HIGH();
	DelayUs(I2C_US);

	i = IIC_SDA_GET();
	
	if( i!=0x00 )
	{
		IIC_State = IIC_STATE_NOACK;
	}
	else
	{
		IIC_State = IIC_STATE_ACK;
	}

	IIC_SCK_LOW();
	DelayUs(I2C_US);
	if(IIC_State == IIC_STATE_NOACK)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}



unsigned char IIC_ReadByte(unsigned char ACK)
{
	unsigned char i,Data,temp;


	Data = 0x00;
	
	IIC_SDA_IN();
	
	for(i=0;i<8;i++)
	{
		Data <<= 1;
		IIC_SCK_LOW();
		DelayUs(I2C_US);	
		IIC_SCK_HIGH();
		DelayUs(I2C_US);
		
		
		temp = IIC_SDA_GET();

		if( temp!=0x00 )
		{
			Data |= 0x01;
		}

	}

	IIC_SDA_OUT();	//主机读数据时，需要根据工作状况发送ACK
	
	if(ACK == IIC_SEND_ACK)
	{
		IIC_SDA_LOW();
	}
	else
	{
		IIC_SDA_HIGH();
	}
	DelayUs(I2C_US);//after level change,wait a few microseconds
	IIC_SCK_HIGH();
	DelayUs(I2C_US);
	IIC_SCK_LOW();
	DelayUs(I2C_US);

	return Data;
}
//****************************************************************************
// @Function      I2C_WriteData 
//
//----------------------------------------------------------------------------
// @Description   
//
//----------------------------------------------------------------------------
// @Returnvalue   0: success
//				  1: faile	
//
//----------------------------------------------------------------------------
// @Parameters    None
//
//----------------------------------------------------------------------------
// @Date          2017/1/6
//
// @author		  renbin	
//****************************************************************************
uint8_t I2C_WriteData(I2C_Type i2cPar)
{
	uint8_t i = 0,temp;
	
	if(i2cPar.D_Addr_Mode == IIC_ADDR7)
	{
		IIC_Start();
		if(IIC_WriteByte((uint8_t)i2cPar.Devices_Addr | IIC_WRITE))
			return 1;
		if(i2cPar.R_Addr_Mode == ADDR_8)
		{
			if(IIC_WriteByte((uint8_t)i2cPar.RAM_Addr))
				return 1;
		}
		else
		{
			temp = (uint8_t)i2cPar.RAM_Addr >> 8;
			IIC_WriteByte((uint8_t)temp);
			IIC_WriteByte((uint8_t)i2cPar.RAM_Addr);
					
		}
		while(i < i2cPar.DataSize)
		{
			
			if(IIC_WriteByte(i2cPar.Data[i++]))
				return 1;
			//Delayms(10);
		}
		IIC_Stop();
	}
	
	return 0;
}//end function I2C_WriteData

//****************************************************************************
// @Function      I2C_ReadData 
//
//----------------------------------------------------------------------------
// @Description   
//
//----------------------------------------------------------------------------
// @Returnvalue   0: success
//				  1: faile	
//
//----------------------------------------------------------------------------
// @Parameters    None
//
//----------------------------------------------------------------------------
// @Date          2017/1/6
//
// @author		  renbin	
//****************************************************************************
uint8_t I2C_ReadData(I2C_Type i2cPar)
{
	uint8_t i = 0,temp;
	
	
	if(i2cPar.D_Addr_Mode == IIC_ADDR7)
	{
		IIC_Start();
		if(IIC_WriteByte(i2cPar.Devices_Addr +IIC_WRITE))
			return 1;
		if(i2cPar.R_Addr_Mode == ADDR_8)
		{
			if(IIC_WriteByte((uint8_t)i2cPar.RAM_Addr))
				return 1;
		}
		else
		{
			temp = (uint8_t)(i2cPar.RAM_Addr >> 8);
			if(IIC_WriteByte((uint8_t)temp))
				return 1;
			if(IIC_WriteByte((uint8_t)i2cPar.RAM_Addr))
				return 1;
			
		}
		IIC_Start();
		if(IIC_WriteByte(i2cPar.Devices_Addr+IIC_READ))
			return 1;
		while(i < i2cPar.DataSize-1)
		{

			i2cPar.Data[i++] = IIC_ReadByte(IIC_SEND_ACK);
			//Delayms(5);
		}
		i2cPar.Data[i] = IIC_ReadByte(IIC_SEND_NOACK);
		IIC_Stop();
		
	}
	return 0;
}

