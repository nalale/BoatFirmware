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

#include "../Libs/Btn8982.h"

#include "dVCU_ECU.h"

void HardwareInit(void);
void PortInit(void);

void AppInit(ObjectDictionary_t *dictionary)
{
    HardwareInit();
    ecuInit(dictionary);    
      
}

void HardwareInit(void)
{
    CLKPWR_ConfigPPWR(CLKPWR_PCONP_PCTIM0 | CLKPWR_PCONP_PCPWM1 | CLKPWR_PCONP_PCSSP1 | CLKPWR_PCONP_PCAD |
                        CLKPWR_PCONP_PCAN1 | CLKPWR_PCONP_PCAN2 | CLKPWR_PCONP_PCGPIO,
                        ENABLE);
    PortInit();
	
    Tmr_Init(1000);
    Can_Init(1, 1);
    Pwm_Init(1000);
    Adc_Init();
    I2c_Init();
	Spi_Init(DATABIT_16);

	NVIC_EnableIRQ(CAN_IRQn);
}



void PortInit(void)
{    
	FIO_SetDir(0, 0x40, DIR_OUT);
	SET_CS_OUT(1);
	
    FIO_SetDir(1, 0xFFFFFFFF, DIR_OUT);
	SET_C_OUT5(0);
	SET_C_OUT6(0);
	SET_C_OUT7(0);
	SET_C_OUT8(0);
	SET_C_OUT9(0);
	SET_C_OUT10(0);
	
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


void DisplaySoc(uint16_t Value)
{
	EcuConfig_t config = GetConfigInstance();
	uint8_t dutycycle = (uint8_t)interpol((int16_t*)config.SoC, 6, Value);

	btnSetOutputLevel(DISPLAY_SOC_CH, dutycycle);
}

void DisplayTrim(uint16_t Value)
{
	EcuConfig_t config = GetConfigInstance();
	uint8_t dutycycle = (uint8_t)interpol((int16_t*)config.TrimPosition, 6, Value);

	btnSetOutputLevel(DISPLAY_TRIM_CH, dutycycle);
}

void DisplayEnergy(uint16_t Value)
{
	EcuConfig_t config = GetConfigInstance();
	uint8_t dutycycle = (uint8_t)interpol((int16_t*)config.SpecPower, 6, Value);

	btnSetOutputLevel(DISPLAY_CONS_CH, dutycycle);
}

void DisplayMotorRpm(uint16_t Value)
{
	EcuConfig_t config = GetConfigInstance();
	uint16_t frequency = (uint16_t)interpol((int16_t*)config.MotorRpm, 6, Value);

	// If motor speed < 100 rpm duty cycle should be 0
	if(Value < 100)
	{
		btnSetOutputLevel(DISPLAY_RPM_CH, 0);
		PwmFreqUpdate(1000);
	}
	else
	{
		btnSetOutputLevel(DISPLAY_RPM_CH, 125);
		PwmFreqUpdate(frequency);
	}
}

