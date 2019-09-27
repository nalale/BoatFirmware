#include "Main.h"
#include "EcuConfig.h"
#include "../MolniaLib/MF_Can_1v1.h"


EcuConfig_t EcuConfig;

void _cfgSetDefaultParams(void)
{
	EcuConfig.DiagnosticID = DISPLAY_ECU_DIAG_ID;	
	EcuConfig.Index = 0;
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
	EcuConfig.Index = (EcuConfig.Index > 3)? 3 : EcuConfig.Index;
	EcuConfig.DiagnosticID = DISPLAY_ECU_DIAG_ID;	
	
}

EcuConfig_t GetConfigInstance()
{
	return EcuConfig;
}


