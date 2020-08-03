#include "Main.h"
#include "EcuConfig.h"
#include "../MolniaLib/MF_Can_1v1.h"


EcuConfig_t EcuConfig;

void _cfgSetDefaultParams(void)
{
	EcuConfig.DiagnosticID = BMS_ECU_DIAG_ID;	
	
	EcuConfig.BatteryIndex = 0;	
	EcuConfig.ModuleIndex = MAX_MODULE_NUM - 1;
	EcuConfig.IsMaster = 0;
	
	EcuConfig.Sys_MaxDCL = 300;
	EcuConfig.Sys_MaxCCL = -300;
	
	int16_t SOCTable1[2][14] = {
	{3420, 3455, 3494, 3613, 3639, 3678, 3753, 3821, 3908, 4010, 4067, 4131 },
	{ 0, 50, 100, 300, 400, 500, 600, 700, 800, 900, 950, 1000 } };
	
	
	
	for(int i = 0; i < CCL_DCL_LINES_NUM; i++)
	{
		for(int j = 0; j < CCL_DCL_POINTS_NUM; j++)
		{
			EcuConfig.VoltageCCLpoint[i][j] = 0;
		}
		
		for(int j = 0; j < 14; j++)
		{
			EcuConfig.OCVpoint[i][j] = SOCTable1[i][j];
		}
	}
	for(int i = 0; i < CCL_DCL_LINES_NUM - 1; i++)
	{
		for(int j = 0; j < CCL_DCL_POINTS_NUM; j++)
		{
			EcuConfig.TemperatureCCLpoint[i][j] = 0;
		}
	}
	
	EcuConfig.Sys_MaxVoltageDisbalanceP = 50;	
	EcuConfig.Sys_MaxVoltageDisbalanceS = 50;	

	EcuConfig.TestMode = 0;
	EcuConfig.CellNumber = 24;
	EcuConfig.Sys_ModulesCountS = 1;
	EcuConfig.Sys_ModulesCountP = 1;
	EcuConfig.CurrentSensType = 3;
	EcuConfig.ModuleCapacity = 90;
	EcuConfig.PreMaxDuration = 600;
	EcuConfig.PreZeroCurrentDuration = 300;
	EcuConfig.PreZeroCurrent = 1;
	EcuConfig.PreMaxCurrent = 5;	
	EcuConfig.IsPowerManager = 1;
	EcuConfig.KeyOffTime_ms = 150;
	EcuConfig.PowerOffDelay_ms = 1500;
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
		OD.Faults.Mod_ConfigCrc = 1;
		_cfgSetDefaultParams();
	}	
	
	OD.ConfigData = &EcuConfig;
	csSetCurrentSensorType((CurrentSensorType_e)EcuConfig.CurrentSensType, EcuConfig.CurrentSensDirection);		// CurrentSensor init
}

EcuConfig_t GetConfigInstance()
{
	return EcuConfig;
}


