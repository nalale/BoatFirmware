#include <stdint.h>
#include "lpc17xx_pinsel.h"
#include "lpc17xx_mcpwm.h"

// MCPWM Channel configuration data
MCPWM_CHANNEL_CFG_Type channelsetup[3];

void MCPwm_Init(void)
{	
	/* Init MCPWM peripheral */
	MCPWM_Init(LPC_MCPWM);

	channelsetup[0].channelType = MCPWM_CHANNEL_EDGE_MODE;
	channelsetup[0].channelPolarity = MCPWM_CHANNEL_PASSIVE_LO;
	channelsetup[0].channelDeadtimeEnable = DISABLE;
	channelsetup[0].channelDeadtimeValue = 0;
	channelsetup[0].channelUpdateEnable = ENABLE;
	channelsetup[0].channelTimercounterValue = 0;
	channelsetup[0].channelPeriodValue = 300;
	channelsetup[0].channelPulsewidthValue = 0;

	channelsetup[1].channelType = MCPWM_CHANNEL_EDGE_MODE;
	channelsetup[1].channelPolarity = MCPWM_CHANNEL_PASSIVE_LO;
	channelsetup[1].channelDeadtimeEnable = DISABLE;
	channelsetup[1].channelDeadtimeValue = 0;
	channelsetup[1].channelUpdateEnable = ENABLE;
	channelsetup[1].channelTimercounterValue = 0;
	channelsetup[1].channelPeriodValue = 300;
	channelsetup[1].channelPulsewidthValue = 0;

	channelsetup[2].channelType = MCPWM_CHANNEL_EDGE_MODE;
	channelsetup[2].channelPolarity = MCPWM_CHANNEL_PASSIVE_LO;
	channelsetup[2].channelDeadtimeEnable = DISABLE;
	channelsetup[2].channelDeadtimeValue = 0;
	channelsetup[2].channelUpdateEnable = ENABLE;
	channelsetup[2].channelTimercounterValue = 0;
	channelsetup[2].channelPeriodValue = 300;
	channelsetup[2].channelPulsewidthValue = 0;

	MCPWM_ConfigChannel(LPC_MCPWM, 0, &channelsetup[0]);
	MCPWM_ConfigChannel(LPC_MCPWM, 1, &channelsetup[1]);
	MCPWM_ConfigChannel(LPC_MCPWM, 2, &channelsetup[2]);
	
	MCPWM_ACMode(LPC_MCPWM, ENABLE);
}



void Pin_Configurate()
{
	PINSEL_CFG_Type PinCfg;
	/* Pin configuration for MCPWM function:
	 * Assign: 	- P1.19 as MCOA0 - Motor Control Channel 0 Output A
	 * 			- P1.22 as MCOB0 - Motor Control Channel 0 Output B
	 * 			- P1.25 as MCOA1 - Motor Control Channel 1 Output A
	 * 			- P1.26 as MCOB1 - Motor Control Channel 1 Output B
	 * 			- P1.28 as MCOA2 - Motor Control Channel 2 Output A
	 * 			- P1.29 as MCOB2 - Motor Control Channel 2 Output B
	 * 			- P1.20 as MCI0	 - Motor Control Feed Back Channel 0
	 * Warning: According to Errata.lpc1768-18.March.2010: Input pin (MIC0-2)
	 * on the Motor Control PWM peripheral are not functional
	 */
	PinCfg.Funcnum = 1;
	PinCfg.OpenDrain = 0;
	PinCfg.Pinmode = 0;
	PinCfg.Portnum = 1;
	PinCfg.Pinnum = 19;
	PINSEL_ConfigPin(&PinCfg);
	PinCfg.Pinnum = 22;
	PINSEL_ConfigPin(&PinCfg);
	PinCfg.Pinnum = 25;
	PINSEL_ConfigPin(&PinCfg);
	PinCfg.Pinnum = 26;
	PINSEL_ConfigPin(&PinCfg);
	PinCfg.Pinnum = 28;
	PINSEL_ConfigPin(&PinCfg);
	PinCfg.Pinnum = 29;
	PINSEL_ConfigPin(&PinCfg);
	PinCfg.Pinnum = 20;
	PINSEL_ConfigPin(&PinCfg);
}




void MCPwm_Update(uint32_t ChannelA, uint32_t ChannelB, uint32_t ChannelC)
{
	channelsetup[0].channelPulsewidthValue = ChannelA;
	channelsetup[1].channelPulsewidthValue = ChannelB;
	channelsetup[2].channelPulsewidthValue = ChannelC;
	
	MCPWM_WriteToShadow(LPC_MCPWM, 0, &channelsetup[0]);
	MCPWM_WriteToShadow(LPC_MCPWM, 1, &channelsetup[1]);
	MCPWM_WriteToShadow(LPC_MCPWM, 2, &channelsetup[2]);
}

