#include "User.h"

#include "TimerFunc.h"
#include "CanFunc.h"
#include "PwmFunc.h"
#include "I2cFunc.h"
#include "AdcFunc.h"
#include "UartFunc.h"
#include "SpiFunc.h"

#include "lpc17xx_gpio.h"
#include "lpc17xx_clkpwr.h"

#include "../MolniaLib/MF_Tools.h"

#include "gVCU_ECU.h"

void HardwareInit(void);
void PortInit(void);

void AppInit(ObjectDictionary_t *dictionary)
{
    HardwareInit();
    ecuInit(dictionary);    
      
}

void PllInit()
{
    
//  LPC_SC->PLL0CFG   = PLL0CFG_Val;      /* configure PLL0                     */
//  LPC_SC->PLL0FEED  = 0xAA;
//  LPC_SC->PLL0FEED  = 0x55;

//  LPC_SC->PLL0CON   = 0x01;             /* PLL0 Enable                        */
//  LPC_SC->PLL0FEED  = 0xAA;
//  LPC_SC->PLL0FEED  = 0x55;
//  while (!(LPC_SC->PLL0STAT & (1<<26)));/* Wait for PLOCK0                    */

//  LPC_SC->PLL0CON   = 0x03;             /* PLL0 Enable & Connect              */
//  LPC_SC->PLL0FEED  = 0xAA;
//  LPC_SC->PLL0FEED  = 0x55;
//  while ((LPC_SC->PLL0STAT & ((1<<25) | (1<<24))) != ((1<<25) | (1<<24)));  /* Wait for PLLC0_STAT & PLLE0_STAT */

//  LPC_SC->PLL1CFG   = PLL1CFG_Val;
//  LPC_SC->PLL1FEED  = 0xAA;
//  LPC_SC->PLL1FEED  = 0x55;

//  LPC_SC->PLL1CON   = 0x01;             /* PLL1 Enable                        */
//  LPC_SC->PLL1FEED  = 0xAA;
//  LPC_SC->PLL1FEED  = 0x55;
//  while (!(LPC_SC->PLL1STAT & (1<<10)));/* Wait for PLOCK1                    */

//  LPC_SC->PLL1CON   = 0x03;             /* PLL1 Enable & Connect              */
//  LPC_SC->PLL1FEED  = 0xAA;
//  LPC_SC->PLL1FEED  = 0x55;
//  while ((LPC_SC->PLL1STAT & ((1<< 9) | (1<< 8))) != ((1<< 9) | (1<< 8)));  /* Wait for PLLC1_STAT & PLLE1_STAT */
  
  
}

void HardwareInit(void)
{
    CLKPWR_ConfigPPWR(CLKPWR_PCONP_PCTIM0 | CLKPWR_PCONP_PCPWM1 | CLKPWR_PCONP_PCSSP1 | CLKPWR_PCONP_PCAD |
                        CLKPWR_PCONP_PCAN1 | CLKPWR_PCONP_PCAN2 | CLKPWR_PCONP_PCGPIO | CLKPWR_PCONP_PCI2C2,
                        ENABLE);
    PortInit();
	
    Tmr_Init(1000);
    Can_Init(1, 1);
    Pwm_Init(10000);
    Adc_Init();
    I2c_Init();	
	Spi_Init(DATABIT_16);
	
	NVIC_EnableIRQ(CAN_IRQn);
//	NVIC_EnableIRQ(ADC_IRQn);
//	Spi_Init(DATABIT_16);
	
//	Uart_Init(115200);
}



void PortInit(void)
{    
	FIO_SetDir(0, 0x40, DIR_OUT);
	SET_CS_OUT(1);
	
    FIO_SetDir(1, 0xFFFFFFFF, DIR_OUT);
	SET_C_OUT5(1);
	SET_C_OUT6(1);
	SET_C_OUT7(1);
	SET_C_OUT8(1);
	SET_C_OUT9(1);
	SET_C_OUT10(1);
	
	SET_PU_D_IN1(0);
	SET_PU_D_IN2(0);
	
    
    FIO_SetDir(2, 0xFFFFFFFF, DIR_OUT);
    SET_A_OUT1_EN(0);
	SET_A_OUT2_EN(0);
	SET_A_OUT3_EN(0);
	SET_A_OUT4_EN(0);
    
    FIO_SetDir(4, 0xFFFFFFFF, DIR_OUT);
    //FIO_SetValue(4, 0x0);
	SET_PU_D_IN3(0);
	SET_PU_D_IN4(0);
	
	FIO_SetDir(1, 0xC713, DIR_IN);
	FIO_SetDir(0, (1 << 30) | (1 << 29), DIR_IN);
	
//	FIO_SetDir(0, 0x40, DIR_OUT);
//	
//    FIO_SetDir(1, 0xFFFFFFFF, DIR_OUT);		//(1 << 18) | (1 << 19) | (1 << 23) | (1 << 24) | (1 << 25) | (1 << 26) | (1 << 28) | (1 << 29)/*
//    FIO_SetValue(1, 0xFFFFFFFF);
//    
//    FIO_SetDir(2, 0xFFFFFFFF, DIR_OUT);			//0x27F/*
//    FIO_SetValue(2, 0x00000000);
//    
//    FIO_SetDir(4, 0xFFFFFFFF, DIR_OUT);		//(1 << 28) | (1 << 29)/*
//    FIO_SetValue(4, 0xFFFFFFFF);
//	
//	FIO_SetDir(1, 0xC713, DIR_IN);
//	FIO_SetDir(0, (1 << 30) | (1 << 29), DIR_IN);
}
