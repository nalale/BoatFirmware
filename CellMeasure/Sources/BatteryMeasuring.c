#include <string.h>

#include "Main.h"
#include "BatteryMeasuring.h"
#include "../MolniaLib/MF_Tools.h"
#include "TimerFunc.h"



#define SOC_TABLE_SIZE			(CCL_DCL_POINTS_NUM * 2)

int16_t CurrentLimitArray[CCL_DCL_POINTS_NUM * 2];






// Функция получения энергии по минимальному напряжению
uint32_t GetEnergyFromMinUcell(int16_t *OcvTable, uint32_t Voltage, uint32_t TotalCapatity)
{
	uint32_t TempSoC = interpol(OcvTable, SOC_TABLE_SIZE, (uint16_t)Voltage);	
	
	return (TempSoC * (TotalCapatity * 36)) / 10;			// Ah * 3600 [Asec] * SoC / 100;
}



// Расчёт энергии в Aмпер-секундах
// Аргументы:
//  - Ток батареи, в Ампер*10
//  - Указатель на текущую ёмкость
uint16_t CapacityCulc(int16_t Current, uint32_t* CurrentEnergy, uint16_t ModuleCapacity)
{
	static int32_t EnergyLow = 0;
	static int32_t EnergyLowPrev = 0;
    
    static uint32_t CapacityCulcStamp = 0;
	
	uint16_t result = 0;
	result = (10L * (*(CurrentEnergy))) / (36L * ModuleCapacity);
	
	// Каждые 100 мс
	uint32_t tm = GetTimeFrom(CapacityCulcStamp);
	if (tm >= 100)
	{
		EcuConfig_t ecuConfig = GetConfigInstance();
		CapacityCulcStamp = GetTimeStamp() - (tm - 100);
		
		// Положительный ток вытекает из батареи
		// Интегрирование тока. Получаем энергию. Ток в 0.1 Ампера
		EnergyLow -= Current;
	}

	if (EnergyLow - EnergyLowPrev >= 100)			// Заряд
	{
		(*(CurrentEnergy))++;
		EnergyLowPrev += 100;
	}
	else if ((EnergyLow - EnergyLowPrev) <= -100)	// Разряд
	{
		if ((*(CurrentEnergy)) > 0)
			(*(CurrentEnergy))--;
		
		EnergyLowPrev -= 100;
	}
	
	if(result > 1000)
			result = 1000;
	
	return result / 10;
}

uint16_t TargetVoltageCulc(uint16_t MinSystemCellVoltage, uint16_t MinModuleCellVoltage)
{
	EcuConfig_t ecuConfig = GetConfigInstance();
	uint16_t _minVolt_mV = (ecuConfig.IsMaster)? MinSystemCellVoltage : MinModuleCellVoltage;
	
	// Расчёт напряжения балансировки
	uint16_t TargetVoltTemp_mV = _minVolt_mV + ecuConfig.MaxVoltageDiff;
	
	if (TargetVoltTemp_mV < ecuConfig.MinVoltageForBalancing)
		TargetVoltTemp_mV = ecuConfig.MinVoltageForBalancing;
	
	// Если мастер или модуль работает автономно, то возвращаем рассчитанное значение, если нет, то значение принятое по CAN
	if(!(ecuConfig.IsMaster || ecuConfig.IsAutonomic))
		TargetVoltTemp_mV = OD.MasterControl.TargetVoltage_mV;	
	
	return TargetVoltTemp_mV;
}



void GetCurrentLimit(int16_t* Discharge, int16_t* Charge)
{
	int16_t limit;
	int16_t CCLCurrentLimit = 100;
	int16_t DCLCurrentLimit = 100;
	
	EcuConfig_t ecuConfig = GetConfigInstance();
	
	// Только мастер расчитывает ограничения тока
	if(!ecuConfig.IsMaster)
		return;
	
	// Копируем во временный массив данные по ограничению заряда от напряжения
	memcpy(CurrentLimitArray, ecuConfig.VoltageCCLpoint, CCL_DCL_POINTS_NUM * 4);
	// Вычисляем локальное ограничение заряда
	limit = interpol(CurrentLimitArray, CCL_DCL_POINTS_NUM, OD.ModuleData[ecuConfig.ModuleIndex].MaxCellVoltage.Voltage_mv);
	// Если это ограничение жёстче, чем глоабльное, то обновляем глобальное
	if (limit < CCLCurrentLimit)
	{
		CCLCurrentLimit = limit;
	}
	
	// Копируем во временный массив данные об ограничении разряда
	memcpy((uint8_t*)(CurrentLimitArray) + (CCL_DCL_POINTS_NUM * 2), (uint8_t*)(ecuConfig.VoltageCCLpoint) + CCL_DCL_POINTS_NUM * 4, CCL_DCL_POINTS_NUM * 2);
	// Вычисляем локальное ограничение разряда 
	limit = interpol(CurrentLimitArray, CCL_DCL_POINTS_NUM, OD.ModuleData[ecuConfig.ModuleIndex].MinCellVoltage.Voltage_mv);
	if (limit < DCLCurrentLimit)
	{
		DCLCurrentLimit = limit;
	}
	
	
	// Ограничение по температуре.
/*	limit = interpol((int16_t *)EcuConfig.TemperatureCCLpoint, CCL_DCL_POINTS_NUM, OD.MasterData.MaxModuleTemperature.Temperature);
	if (limit < CCLCurrentLimit)
	{
		CCLCurrentLimit = limit;
	}
	if (limit < DCLCurrentLimit)
	{
		DCLCurrentLimit = limit;
	}

	limit = interpol((int16_t *)EcuConfig.TemperatureCCLpoint, CCL_DCL_POINTS_NUM, OD.MasterData.MinModuleTemperature.Temperature);
	if (limit < CCLCurrentLimit)
	{
		CCLCurrentLimit = limit;
	}
	if (limit < DCLCurrentLimit)
	{
		DCLCurrentLimit = limit;
	}
*/	
	// Перевод процентов в амперы
	int32_t v = ((int32_t)DCLCurrentLimit * ecuConfig.Sys_MaxDCL) / 100;
	DCLCurrentLimit = v;
	
	v = ((int32_t)CCLCurrentLimit * ecuConfig.Sys_MaxCCL) / 100;	
	CCLCurrentLimit = (ecuConfig.Sys_MaxCCL > 0)? 0 : v;

	
	*Discharge = DCLCurrentLimit;
	*Charge = CCLCurrentLimit;
	
	if (OD.BatteryData[ecuConfig.BatteryIndex].StateMachine.MainState == WORKSTATE_FAULT)
	{
		*Discharge = 0;
		*Charge = 0;
		return;
	}
}





