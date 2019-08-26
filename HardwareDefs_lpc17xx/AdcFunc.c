#include "AdcFunc.h"
#include "lpc17xx_pinsel.h"
#include "lpc17xx_adc.h"



#define REF_ADC_VOLTAGE 3300



static void Pin_Configurate(void);

static uint16_t _channelsData[8]; 

void Adc_Init()
{	
     Pin_Configurate();
     /* Configuration for ADC:
	 *  ADC conversion rate = 200KHz
	 */
        
	ADC_Init(LPC_ADC, 200000);
        
    ADC_ChannelCmd(LPC_ADC, ADC_CHANNEL_2, ENABLE);
	ADC_ChannelCmd(LPC_ADC, ADC_CHANNEL_3, ENABLE);  
    ADC_ChannelCmd(LPC_ADC, ADC_CHANNEL_4, ENABLE);
    ADC_ChannelCmd(LPC_ADC, ADC_CHANNEL_5, ENABLE);    
	
//    ADC_IntConfig(LPC_ADC, ADC_ADINTEN2, ENABLE);
//	ADC_IntConfig(LPC_ADC, ADC_ADINTEN3, ENABLE);
//	ADC_IntConfig(LPC_ADC, ADC_ADINTEN4, ENABLE);
//	ADC_IntConfig(LPC_ADC, ADC_ADINTEN5, ENABLE);
//	
//    //ADC_StartCmd(LPC_ADC, ADC_START_NOW);   
	
	ADC_BurstCmd(LPC_ADC, ENABLE);
}



void Pin_Configurate()
{
    PINSEL_CFG_Type PinCfg;
  /*
	 * Init ADC pin connect
	 *	AD0.4 on P1.30
	 */
	PinCfg.Funcnum = 3;
	PinCfg.OpenDrain = 0;
	PinCfg.Pinmode = 0;
	PinCfg.Pinnum = 30;
	PinCfg.Portnum = 1;
	PINSEL_ConfigPin(&PinCfg);
    

	/*
	 * AD0.5 on P1.31
	 *
	 */
	PinCfg.Funcnum = 3; 
	PinCfg.OpenDrain = 0;
	PinCfg.Pinmode = 0;
	PinCfg.Pinnum = 31;
	PinCfg.Portnum = 1;
	PINSEL_ConfigPin(&PinCfg);
    
    /*
	 * Init ADC pin connect
	 *	AD0.2 on P0.25
	 */
	PinCfg.Funcnum = 1;
	PinCfg.OpenDrain = 0;
	PinCfg.Pinmode = 0;
	PinCfg.Pinnum = 25;
	PinCfg.Portnum = 0;
	PINSEL_ConfigPin(&PinCfg);

	/*
	 * AD0.3 on P0.26
	 *
	 */
	PinCfg.Funcnum = 1;
	PinCfg.OpenDrain = 0;
	PinCfg.Pinmode = 0;
	PinCfg.Pinnum = 26;
	PinCfg.Portnum = 0;
	PINSEL_ConfigPin(&PinCfg);
    
    
}


void SetAdcData(uint8_t channel)
{	
	_channelsData[channel] = ADC_ChannelGetData(LPC_ADC, channel);
}

uint16_t GetVoltageValue(uint8_t channel)
{	
	if(ADC_ChannelGetStatus(LPC_ADC, channel, ADC_DATA_DONE))
		_channelsData[channel] =  ADC_ChannelGetData(LPC_ADC, channel);
	
	return ((uint32_t)_channelsData[channel] * REF_ADC_VOLTAGE) >> 12;
	 
}

