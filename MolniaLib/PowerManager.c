#include <stdint.h>

#include "PowerManager.h"
#include "AdcFunc.h"
#include "TimerFunc.h"

#include "Main.h"
#include "EcuConfig.h"
#include "../Libs/Btn8982.h"
#include "../Libs/filter.h"

static PowerStates_e _pm_state = PM_PowerOff;


void PM_Proc(uint16_t voltageEcu, uint8_t IsPowerManager)
{
	static uint32_t _start_timestamp = 0;
	
	EcuConfig_t _config = GetConfigInstance();

	// Если блок не управляет питанием выходим
	if(!_config.IsPowerManager)
		return;
	
	if(OD.ecuPowerSupply_0p1 > 100)
	{
		if(GetTimeFrom(_start_timestamp) > _config.KeyOffTime_ms)
			_pm_state = PM_PowerOn1;
		
		OD.LogicTimers.KeyOffTimer_ms = GetTimeStamp();
	}
	else
	{
		if(OD.ecuPowerSupply_0p1 < 80)
		{
			if(GetTimeFrom(OD.LogicTimers.KeyOffTimer_ms) >= _config.KeyOffTime_ms)
			{
				_pm_state = (_pm_state == PM_PowerOn1)? PM_ShutDown : _pm_state;

				// Защита от импульсного включения/выключения если время реакции на зажигание установлено слишком короткое
				if(GetTimeFrom(OD.LogicTimers.KeyOffTimer_ms) > _config.KeyOffTime_ms + 100)
				{
					_pm_state = PM_ShutDown;
				}
			}
		}
		_start_timestamp = GetTimeStamp();
	}
}


PowerStates_e PM_GetPowerState()
{
	return _pm_state;
}

