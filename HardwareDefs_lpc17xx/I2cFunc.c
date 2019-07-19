#include "lpc17xx_i2c.h"
#include "lpc17xx_pinsel.h"
#include "I2cFunc.h"

static void Pin_Configurate(void);
static I2C_M_SETUP_Type transferMCfg;



void I2c_Init(void)
{
	Pin_Configurate();

	// Initialize Slave I2C peripheral
	I2C_Init(LPC_I2C2, 400000);
	/* Enable Slave I2C operation */
	I2C_Cmd(LPC_I2C2, I2C_MASTER_MODE, ENABLE);
}



void Pin_Configurate()
{
  PINSEL_CFG_Type PinCfg;
  
  PinCfg.OpenDrain = PINSEL_PINMODE_OPENDRAIN;
  PinCfg.Pinmode = PINSEL_PINMODE_PULLUP;//PINSEL_PINMODE_TRISTATE;
  PinCfg.Funcnum = 2;
  PinCfg.Pinnum = 10;
  PinCfg.Portnum = 0;
  PINSEL_ConfigPin(&PinCfg);
  
  PinCfg.Pinnum = 11;
  PINSEL_ConfigPin(&PinCfg);
        
}


uint8_t I2CReadWrite(uint8_t *tx_buf, uint8_t tx_lenght, uint8_t *rx_buf, uint8_t rx_lenght, uint16_t slave_address)
{  
  transferMCfg.sl_addr7bit = slave_address;
  transferMCfg.tx_data = tx_buf;
  transferMCfg.tx_length = tx_lenght;
  transferMCfg.rx_data = rx_buf;
  transferMCfg.rx_length = rx_lenght;
  transferMCfg.retransmissions_max = 6;
  return I2C_MasterTransferData(LPC_I2C2, &transferMCfg, I2C_TRANSFER_INTERRUPT); 
}

