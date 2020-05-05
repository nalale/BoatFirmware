#include "Main.h"
#include "EcuConfig.h"
#include "../MolniaLib/MF_Can_1v1.h"


EcuConfig_t EcuConfig;

void _cfgSetDefaultParams(void)
{
	EcuConfig.DiagnosticID = MAIN_ECU_DIAG_ID;	
	
	EcuConfig.AccPedalFstCh_MaxV = 16;				
	EcuConfig.AccPedalFstCh_0V = 35;
	EcuConfig.AccPedalSndCh_MaxV = 25;
	
	EcuConfig.SteeringKp = 1;		
	EcuConfig.SteeringKd = 0;
	EcuConfig.SteeringKi = 0;	
	
	EcuConfig.InvCoolingOn = 50;
	EcuConfig.MotorCoolingOn = 85;
	
	EcuConfig.MaxMotorSpeedD = 6000;
	EcuConfig.MaxMotorTorque = 300;
	EcuConfig.MaxInverterT = 90;
	EcuConfig.MaxMotorT = 120;	
		
	int16_t speed_table[] = {0, 1000, 2000, 3000, 4000, 5000,
							25, 45, 65, 70, 75, 80};
    
    for(int i = 0; i < 12; i++)
    {
        EcuConfig.SteeringBrakeSpeedTable[i] = speed_table[i];        
    }
	
	EcuConfig.CRC = cfgCRC(&EcuConfig);
}

void cfgApply(void)
{	
	if(cfgRead(&EcuConfig))
	{
		OD.FaultsBits.ConfigCrc = 1;
		_cfgSetDefaultParams();
	}		

	OD.cfgEcu = &EcuConfig;
}

EcuConfig_t GetConfigInstance()
{
	return EcuConfig;
}


