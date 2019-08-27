#include "PwmFunc.h"
#include <lpc17xx_pinsel.h>

static uint32_t Pwm1_Period_us = 0;

void Pwm_Init(uint16_t Freq_Hz)
{  
    PWM_TIMERCFG_Type PWMCfgDat;
	PWM_MATCHCFG_Type PWMMatchCfgDat;
	PINSEL_CFG_Type PinCfg;
    
    Pwm1_Period_us = 1000000 / Freq_Hz;
        
        
    // Config Pins For Pwm
    PinCfg.Funcnum = 1;
	PinCfg.OpenDrain = 0;
	PinCfg.Pinmode = 0;
	PinCfg.Portnum = 2;
        
    PinCfg.Pinnum = 0;
	PINSEL_ConfigPin(&PinCfg);
    PinCfg.Pinnum = 1;
	PINSEL_ConfigPin(&PinCfg);
    PinCfg.Pinnum = 2;
	PINSEL_ConfigPin(&PinCfg);      
    PinCfg.Pinnum = 3;
	PINSEL_ConfigPin(&PinCfg);   

    // Set Pwm Prescaler
    PWMCfgDat.PrescaleOption = PWM_TIMER_PRESCALE_USVAL;
	PWMCfgDat.PrescaleValue = 1;                            // us
	PWM_Init(LPC_PWM1, PWM_MODE_TIMER, (void *) &PWMCfgDat);           
        
    // Set Pwm Freq
    PWM_MatchUpdate(LPC_PWM1, 0, Pwm1_Period_us, PWM_MATCH_UPDATE_NOW);
	PWMMatchCfgDat.IntOnMatch = DISABLE;
	PWMMatchCfgDat.MatchChannel = 0;
	PWMMatchCfgDat.ResetOnMatch = ENABLE;
	PWMMatchCfgDat.StopOnMatch = DISABLE;
	PWM_ConfigMatch(LPC_PWM1, &PWMMatchCfgDat);       	
        
        /* Reset and Start counter */
	PWM_ResetCounter(LPC_PWM1);
	PWM_CounterCmd(LPC_PWM1, ENABLE);
	
	PWM_Cmd(LPC_PWM1, ENABLE);
}

void Pwm_Ch_Init(uint8_t ChNum, uint16_t Freq, uint8_t DutyCycle)
{
    PWM_MATCHCFG_Type PWMMatchCfgDat;
    
    PwmUpdate(ChNum, DutyCycle);
    
	PWMMatchCfgDat.IntOnMatch = DISABLE;
	PWMMatchCfgDat.MatchChannel = ChNum;
	PWMMatchCfgDat.ResetOnMatch = DISABLE;
	PWMMatchCfgDat.StopOnMatch = DISABLE;
	PWM_ConfigMatch(LPC_PWM1, &PWMMatchCfgDat);
    
    PWM_ChannelCmd(LPC_PWM1, ChNum, ENABLE);
}

void PwmUpdate(uint8_t ChNum, uint8_t DutyCycle)
{    
    uint16_t duty_cycle_loc = Pwm1_Period_us * DutyCycle / UINT8_MAX;   
	
	// Регистр Match на 1 больше соответствующего канала.
    PWM_MatchUpdate(LPC_PWM1, ChNum + 1, duty_cycle_loc, PWM_MATCH_UPDATE_NOW);
}
