#include "lpc17xx_ssp.h"
#include "lpc17xx_gpio.h"
#include "lpc17xx_pinsel.h"
#include "SpiFunc.h"


static void Pin_Configurate(void);
void CS_Init(void);
uint8_t spi_buffer_exchange(uint8_t *dataIn, uint8_t bufLen, uint8_t *dataOut);


void Spi_Init(DataBit_e Databit)
{
  SSP_CFG_Type SSP_ConfigStruct;  
  
	Pin_Configurate();
	
	SSP_ConfigStructInit(&SSP_ConfigStruct);
  
//  SSP_ConfigStruct.CPHA = SSP_CPHA_SECOND;
//  SSP_ConfigStruct.CPOL = SSP_CPOL_HI;
//  SSP_ConfigStruct.ClockRate = 500000;
//  SSP_ConfigStruct.FrameFormat = SSP_FRAME_SPI;
//  SSP_ConfigStruct.Databit = SSP_DATABIT_8;
//  SSP_ConfigStruct.Mode = SSP_MASTER_MODE;
	
	SSP_ConfigStruct.ClockRate = 500000;
	SSP_ConfigStruct.Databit = (Databit == DATABIT_8)? SSP_DATABIT_8 : SSP_DATABIT_16;
	SSP_ConfigStruct.FrameFormat = SSP_FRAME_SPI;
	SSP_ConfigStruct.CPHA = SSP_CPHA_SECOND;
	SSP_ConfigStruct.CPOL = SSP_CPOL_HI;
	SSP_ConfigStruct.Mode = SSP_MASTER_MODE;
  
	SSP_Init(LPC_SSP1, &SSP_ConfigStruct);
//	SET_CS_OUT(1);
	SSP_Cmd(LPC_SSP1, ENABLE);
	
}


void Pin_Configurate()
{
  PINSEL_CFG_Type PinCfg;
  
  /*
  * Initialize SPI pin connect
  * P0.7 - SCK;
  * P0.6 - SSEL - used as GPIO
  * P0.8 - MISO
  * P0.9 - MOSI
  */
  PinCfg.Funcnum = 2;
  PinCfg.OpenDrain = 0;
  PinCfg.Pinmode = 0;
  PinCfg.Portnum = 0;
  
  PinCfg.Pinnum = 7;
  PINSEL_ConfigPin(&PinCfg);
  
  PinCfg.Pinnum = 8;
  PINSEL_ConfigPin(&PinCfg);
  
  PinCfg.Pinnum = 9;
  PINSEL_ConfigPin(&PinCfg);
  
  // CS1 
  PinCfg.Pinnum = 1;
  PinCfg.Funcnum = 0;
  PinCfg.Portnum = 1;
  PINSEL_ConfigPin(&PinCfg);
  
    // CS2
  PinCfg.Pinnum = 14;
  PinCfg.Funcnum = 0;
  PinCfg.Portnum = 1;
  PINSEL_ConfigPin(&PinCfg);
  
    // CS1 
  PinCfg.Pinnum = 6;
  PinCfg.Funcnum = 0;
  PinCfg.Portnum = 0;
  PINSEL_ConfigPin(&PinCfg);
}

/*-------------------------PRIVATE FUNCTIONS------------------------------*/
/*********************************************************************//**
 * @brief 		Initialize CS pin as GPIO function to drive /CS pin
 * 				due to definition of CS_PORT_NUM and CS_PORT_NUM
 * @param		None
 * @return		None
 ***********************************************************************/



uint8_t SpiReadWrite(uint8_t *tx_buf, uint8_t lenght, uint8_t *rx_buf)
{
  SSP_DATA_SETUP_Type xferConfig;
  uint8_t rx_length;
  
  xferConfig.tx_data = tx_buf;
  xferConfig.rx_data = rx_buf;
  xferConfig.length = lenght;
  rx_length = SSP_ReadWrite(LPC_SSP1, &xferConfig, SSP_TRANSFER_POLLING); 
	return rx_length;
}
