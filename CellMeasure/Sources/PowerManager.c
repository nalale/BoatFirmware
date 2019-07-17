#include <stdint.h>

#include "PowerManager.h"
#include "AdcFunc.h"
#include "TimerFunc.h"

#include "Main.h"
#include "EcuConfig.h"
#include "../Libs/Btn8982.h"
#include "../Libs/filter.h"

static PowerStates_e _powerOff = PM_PowerOff;

FILTER_STRUCT fltVoltage;

void PM_Proc()
{
	static uint32_t _start_timestamp = 0;
	
	if(!fltVoltage.OK)
		Filter_init(50, 1, &fltVoltage);
	
	uint16_t voltage_mV = Filter((GetVoltageValue(A_CHNL_KEY) * 11) , &fltVoltage);
	OD.ecuPowerSupply_0p1 = voltage_mV / 100;
	
	EcuConfig_t _config = GetConfigInstance();
	
	// ≈сли блок не управл€ет питание выходим
//	if(!_config.IsPowerManager && !OD.Faults.Mod_PowerManagerOffline)
//		return;	
	
	if(OD.ecuPowerSupply_0p1 > 90)
	{
		if(GetTimeFrom(_start_timestamp) > _config.KeyOffTime_ms)
			_powerOff = PM_PowerOn1;
		
		OD.LogicTimers.KeyOffTimer_ms = GetTimeStamp();
	}
	else
		_start_timestamp = GetTimeStamp();
	
	if(_powerOff == PM_PowerOn1 || _powerOff == PM_PowerOn2)
	{
		if(GetTimeFrom(OD.LogicTimers.KeyOffTimer_ms) >= _config.KeyOffTime_ms)		
			_powerOff = PM_ShutDown;		
		
		if(GetTimeFrom(_start_timestamp) > 2000 && (_config.IsPowerManager))		
			_powerOff = PM_PowerOn2;
		
	}
	else if(_powerOff == PM_PowerOff)
	{
		if(GetTimeFrom(OD.LogicTimers.KeyOffTimer_ms) > _config.KeyOffTime_ms + 100)
		{
			_powerOff = PM_ShutDown;
		}
	}
}


PowerStates_e PM_GetPowerState()
{
	return _powerOff;
}

