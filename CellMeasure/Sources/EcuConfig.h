#ifndef _ECU_CONFIG_H_
#define _ECU_CONFIG_H_

#include <stdint.h>


#define CCL_DCL_POINTS_NUM 6
#define CCL_DCL_LINES_NUM 3

#pragma anon_unions

#pragma pack(4)

typedef struct
{
    uint8_t DiagnosticID;
	uint8_t BatteryIndex;
    uint8_t ModuleIndex;
	
	struct {
		uint8_t
		IsMaster			: 1,
		CheckContactor		: 1,
		CheckInterlock		: 1,
		IsTimeServer		: 1,
		IsAutonomic			: 1,
		TestMode			: 1,
		IsPowerManager		: 1,
		HaveCurrentSensor	: 1;
	};
	uint32_t BaseID;
	
	uint8_t CurrentSensType;
	uint8_t CurrentSensDirection;

	uint8_t Sys_MaxVoltageDisbalanceS;
	uint16_t ModuleCapacity;
	uint16_t Sys_MaxVoltageDisbalanceP;

	int16_t Sys_MaxDCL;
	int16_t Sys_MaxCCL;
	
	uint8_t Sys_ModulesCountS;
	uint8_t Sys_ModulesCountP;

	uint8_t CellNumber;
	int16_t Sys_MaxCellVoltage_mV;
	int16_t Sys_MinCellVoltage_mV;
	
    uint16_t MinVoltageForBalancing;

    int16_t VoltageCCLpoint[CCL_DCL_LINES_NUM][CCL_DCL_POINTS_NUM];

    int16_t TemperatureCCLpoint[CCL_DCL_LINES_NUM-1][CCL_DCL_POINTS_NUM];		

	uint16_t PreMaxDuration;
    uint16_t PreZeroCurrentDuration;
	uint16_t PreZeroCurrent;
    int16_t PreMaxCurrent;
	
	uint8_t BalancingTime_s;	
	uint8_t MaxVoltageDiff;

	
    int16_t OCVpoint[CCL_DCL_LINES_NUM-1][CCL_DCL_POINTS_NUM*2];	
	
	uint16_t PowerOffDelay_ms;
	uint16_t KeyOffTime_ms;
	uint8_t PacksNumber;
	uint8_t addition_5;
	uint16_t addition_6;
	uint16_t addition_7;


    uint16_t CRC;

} EcuConfig_t;



void _cfgSetDefaultParams(void);
void cfgApply(void);
EcuConfig_t GetConfigInstance(void);

#endif

