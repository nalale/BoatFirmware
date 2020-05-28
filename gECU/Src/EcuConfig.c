#include "Main.h"
#include "EcuConfig.h"
#include "../MolniaLib/MF_Can_1v1.h"


EcuConfig_t EcuConfig;

void _cfgSetDefaultParams(void)
{
	EcuConfig.DiagnosticID = GENERAL_ECU_DIAG_ID;	
	EcuConfig.Index = 8;
	EcuConfig.PU_IN1 = 0;
	EcuConfig.PU_IN2 = 0;
	EcuConfig.PU_IN3 = 0;
	EcuConfig.PU_IN4 = 0;
	EcuConfig.KeyOffTime_ms = 250;
	EcuConfig.PowerOffDelay_ms = 1500;
	EcuConfig.IsPowerManager = 1;
	EcuConfig.CRC = cfgCRC(&EcuConfig);
	
	
//	if (!cfgRead(&EcuConfig))
//		SetConfigFault();
	
//	if(!cfgCheck(&EcuConfig))
//		SetConfigFault();		
}

void cfgApply(void)
{	
	if(cfgRead(&EcuConfig))
	{
		OD.Faults.ConfigCrc = 1;
		_cfgSetDefaultParams();
	}		
	
	OD.ConfigData = &EcuConfig;	
	
	EcuConfig.Index = (EcuConfig.Index > 8)? 8 : EcuConfig.Index;
	EcuConfig.DiagnosticID = GENERAL_ECU_DIAG_ID;	
	OD.RepItemCount = 0;
	
	for(uint8_t i = 0; i < REPEATER_TABLE_SIZE; i++)
	{
		if(EcuConfig.RepTable[i].IsActive)
		{
			OD.RepItemCount++;
			
			OD.msgTab[i].SendTime = 0;
			OD.msgTab[i].RepCount = 0;
		}
		else
		{
			//ECU.msgTab[i].NextSendTime = 0xFFFFFFFF;
			continue;
		}
	}	
	
}

EcuConfig_t GetConfigInstance()
{
	return EcuConfig;
}


