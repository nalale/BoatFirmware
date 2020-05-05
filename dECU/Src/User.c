#include "User.h"
#include "WorkStates.h"
#include "dVCU_ECU.h"

#include "TimerFunc.h"
#include "CanFunc.h"
#include "PwmFunc.h"
#include "I2cFunc.h"
#include "AdcFunc.h"
#include "UartFunc.h"
#include "SpiFunc.h"

#include "lpc17xx_pinsel.h"
#include "lpc17xx_gpio.h"
#include "lpc17xx_clkpwr.h"
#include "lpc17xx_timer.h"

#include "../MolniaLib/MF_Tools.h"

#include "../Libs/Btn8982.h"



void HardwareInit(void);
void DisplayMotorRpmInit(void);

void AppInit(ObjectDictionary_t *dictionary)
{
    HardwareInit();
    ecuInit(dictionary);    
      
	DisplayMotorRpmInit();

	SetWorkState(&dictionary->StateMachine, WORKSTATE_INIT);
}

void HardwareInit(void)
{
    CLKPWR_ConfigPPWR(CLKPWR_PCONP_PCTIM0 | CLKPWR_PCONP_PCPWM1 | CLKPWR_PCONP_PCSSP1 | CLKPWR_PCONP_PCAD |
                        CLKPWR_PCONP_PCAN1 | CLKPWR_PCONP_PCAN2 | CLKPWR_PCONP_PCGPIO,
                        ENABLE);
	
    Tmr_Init(1000);		// resolution 1mS
    Can_Init(1, 1);
    Pwm_Init(1000);
    Adc_Init();
    I2c_Init();
	Spi_Init(DATABIT_16);

	NVIC_EnableIRQ(CAN_IRQn);
}


void DisplaySoc(uint16_t Value)
{
	const EcuConfig_t *config = OD.cfg;
	uint8_t dutycycle = (uint8_t)interpol((int16_t*)config->SoC, 6, Value);

	btnSetOutputLevel(DISPLAY_SOC_CH, dutycycle);
}

void DisplayTrim(uint16_t Value)
{
	const EcuConfig_t *config = OD.cfg;
	uint8_t dutycycle = (uint8_t)interpol((int16_t*)config->TrimPosition, 6, Value);

	btnSetOutputLevel(DISPLAY_TRIM_CH, dutycycle);
}

void DisplayEnergy(uint16_t Value)
{
	const EcuConfig_t *config = OD.cfg;
	uint8_t dutycycle = (uint8_t)interpol((int16_t*)config->SpecPower, 6, Value);

	btnSetOutputLevel(DISPLAY_CONS_CH, dutycycle);
}

void DisplayMotorRpm(uint16_t Value)
{
	const EcuConfig_t *config = OD.cfg;
	uint16_t frequency = (uint16_t)interpol((int16_t*)config->MotorRpm, 6, Value);
	frequency = (frequency == 0)? 1 : frequency << 1;	// pin toggle mode => freq * 2
	uint32_t _period_us = (1000000 / frequency); 
	
	TIM_UpdateMatchValue(LPC_TIM0, 1, _period_us);
}

void DisplayMotorRpmInit(void)
{		
	// Init output for Speed Viewer
	PINSEL_CFG_Type pinCfg;
	pinCfg.Portnum = 1;
	pinCfg.Pinnum = 29;	// C_OUT_5
	pinCfg.Funcnum = PINSEL_FUNC_3;
	PINSEL_ConfigPin(&pinCfg);
	
	TIM_TIMERCFG_Type tmrCfg;
	TIM_ConfigStructInit(TIM_TIMER_MODE, &tmrCfg);
	
	TIM_Init(LPC_TIM0, TIM_TIMER_MODE, &tmrCfg);
	
	TIM_MATCHCFG_Type matchCfg;
	matchCfg.MatchChannel = 1;
	matchCfg.MatchValue = 5000;
	matchCfg.ExtMatchOutputType = TIM_EXTMATCH_TOGGLE;	
	matchCfg.IntOnMatch = DISABLE;
	matchCfg.StopOnMatch = DISABLE;
	matchCfg.ResetOnMatch = ENABLE;
	
	TIM_ConfigMatch(LPC_TIM0, &matchCfg);
	
	TIM_Cmd(LPC_TIM0, ENABLE);
}

