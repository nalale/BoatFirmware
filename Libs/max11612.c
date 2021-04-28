#include <string.h>

#include "Main.h"
#include "I2cFunc.h"
#include "TimerFunc.h"

#include "max11612.h"

/** Own Slave address in Slave I2C device */
static uint8_t I2CDEV_S_ADDR = typeMAX11612;

typedef union
{
  uint8_t value;
  
  struct
  {
    uint8_t
      
    dummy         :       1,
    RST           :       1,
    BIP_UNI       :       1,
    CLK           :       1,
    SEL0          :       1,
    SEL1          :       1,
    SEL2          :       1,
    REG           :       1;
  };

}MAX11612_SetupByte_t;

typedef union
{
  uint8_t value;
  
  struct
  {
    uint8_t
      
    SGL_DIF     :       1,
    CS0         :       1,
    CS1         :       1,
    CS2         :       1,
    CS3         :       1,
    SCAN0       :       1,
    SCAN1       :       1,
    REG         :       1;
  };

}MAX11612_ConfigByte_t;

MAX11612_SetupByte_t setup_byte;
MAX11612_ConfigByte_t config_byte;

uint8_t tx_buf[2];
uint8_t rx_buf[8];

uint16_t max11612Result[8];
static uint8_t _max11612MsgSend;

void WriteConfig(void);
void WriteSetup(void);

void Max11612_Init(void)
{
    config_byte.SGL_DIF = 1;        // single_ended mode
    config_byte.SCAN0 = 0;
    config_byte.SCAN1 = 0;
    config_byte.CS0 = 1;
    config_byte.CS1 = 1;
    config_byte.REG = 0;
    
    setup_byte.BIP_UNI = 0;         // unipolar mode
    setup_byte.RST = 1;
    setup_byte.SEL2 = 0;
    setup_byte.SEL1 = 0;
    setup_byte.SEL0 = 0;
    setup_byte.CLK = 1;
    setup_byte.REG = 1;
    
    WriteSetup();
    WriteConfig();
}

void adcMax_SetType(MaxTypes_e type)
{
	if(type >= typeMAX11612 && type <= typeMAX11616)
		I2CDEV_S_ADDR = (uint8_t)type;
}

void WriteConfig()
{    
    while(!I2CReadWrite(&config_byte.value, 1, NULL, 0, I2CDEV_S_ADDR))   {   }
}

void WriteSetup()
{
    while(!I2CReadWrite(&setup_byte.value, 1, NULL, 0, I2CDEV_S_ADDR)) { }
}

void Max11612_SetData(void)
{    
	max11612Result[0] = (uint16_t)(((rx_buf[0] & 0x0F) << 8) + rx_buf[1]);
	max11612Result[1] = (uint16_t)(((rx_buf[2] & 0x0F) << 8) + rx_buf[3]);
	max11612Result[2] = (uint16_t)(((rx_buf[4] & 0x0F) << 8) + rx_buf[5]);
	max11612Result[3] = (uint16_t)(((rx_buf[6] & 0x0F) << 8) + rx_buf[7]);
	
//	if(I2CDEV_S_ADDR == typeMAX11614 || I2CDEV_S_ADDR == typeMAX11616)
//	{
//		max11612Result[4] = (uint16_t)(((rx_buf[8] & 0x0F) << 8) + rx_buf[9]);
//		max11612Result[5] = (uint16_t)(((rx_buf[10] & 0x0F) << 8) + rx_buf[11]);
//		max11612Result[6] = (uint16_t)(((rx_buf[12] & 0x0F) << 8) + rx_buf[13]);
//		max11612Result[7] = (uint16_t)(((rx_buf[14] & 0x0F) << 8) + rx_buf[15]);
//	}

	_max11612MsgSend = 0;
}

void Max11612_ClearData(void)
{
	for(int i = 0; i < 8; i++)
		max11612Result[i] = 0;
}

void Max11612_StartConversion(void)
{
	_max11612MsgSend = 1;

	if(I2CDEV_S_ADDR == typeMAX11612)
		I2CReadWrite(NULL, 0, rx_buf, 8, I2CDEV_S_ADDR);
	else if(I2CDEV_S_ADDR == typeMAX11614)
		I2CReadWrite(NULL, 0, rx_buf, 16, I2CDEV_S_ADDR);
}

void Max11612_GetResult(uint8_t *Buf, uint8_t V_ref)
{
	for(uint8_t i = 0; i < 4; i++)
	{
		uint16_t val = max11612Result[i] * V_ref >> 12;
		Buf[i] = val;
	}
}

uint8_t GetMax11612State(void)
{
	return _max11612MsgSend;
}

