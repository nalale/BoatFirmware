#include <string.h>

#include "Main.h"
#include "BatteryMeasuring.h"
#include "../MolniaLib/MF_Tools.h"
#include "TimerFunc.h"



#define SOC_TABLE_SIZE			(CCL_DCL_POINTS_NUM * 2)

int16_t CurrentLimitArray[CCL_DCL_POINTS_NUM * 2];






// Сalculation of module energy by the minimum cell voltage and the OCV table
uint32_t GetEnergyFromMinUcell(int16_t *OcvTable, uint32_t Voltage, uint32_t TotalCapatity)
{
	uint32_t TempSoC = interpol(OcvTable, SOC_TABLE_SIZE, (uint16_t)Voltage);	
	
	return (TempSoC * (TotalCapatity * 36));			// Ah * 3600 [Asec] * SoC / 100;
}



// Calculation of current module capacity by current value (Amp * 10) and current energy (Amps)
uint16_t StateOfChargeCalculation(int16_t Current_0p1, uint32_t* CurrentEnergy_Amps, uint16_t ModuleCapacity)
{
	static int32_t EnergyLow = 0;
	static int32_t EnergyLowPrev = 0;
    
    static uint32_t CapacityCulcStamp = 0;
	
	uint16_t result = 0;
	result = (10L * (*CurrentEnergy_Amps)) / (36L * ModuleCapacity);
	
	// period of calculation is 100ms
	uint32_t tm = GetTimeFrom(CapacityCulcStamp);
	if (tm >= 100)
	{
		CapacityCulcStamp = GetTimeStamp() - (tm - 100);
		EnergyLow -= Current_0p1;
	}

	// Coulomb counting
	if (EnergyLow - EnergyLowPrev >= 100)			// Charging
	{
		(*CurrentEnergy_Amps)++;
		EnergyLowPrev += 100;
	}
	else if ((EnergyLow - EnergyLowPrev) <= -100)	// Discharging
	{
		if ((*CurrentEnergy_Amps) > 0)
			(*CurrentEnergy_Amps)--;
		
		EnergyLowPrev -= 100;
	}
	
	if(result > 1000)
			result = 1000;
	
	return result / 10;
}





void GetCurrentLimit(const BatteryData_t* handle, const EcuConfig_t *ecuConfig, int16_t* Discharge, int16_t* Charge)
{
	int16_t limit;
	int16_t CCLCurrentLimit = 100;
	int16_t DCLCurrentLimit = 100;
	
	// Только мастер расчитывает ограничения тока
	if(!ecuConfig->IsMaster)
		return;
	
	// Копируем во временный массив данные по ограничению заряда от напряжения
	memcpy(CurrentLimitArray, ecuConfig->VoltageCCLpoint, CCL_DCL_POINTS_NUM * 4);
	// Вычисляем локальное ограничение заряда
	limit = interpol(CurrentLimitArray, CCL_DCL_POINTS_NUM, handle->MaxCellVoltage.Voltage_mv);
	CCLCurrentLimit = (limit < CCLCurrentLimit)? limit : CCLCurrentLimit;
	
	// Копируем во временный массив данные об ограничении разряда
	memcpy((uint8_t*)(CurrentLimitArray) + (CCL_DCL_POINTS_NUM * 2), (uint8_t*)(ecuConfig->VoltageCCLpoint) + CCL_DCL_POINTS_NUM * 4, CCL_DCL_POINTS_NUM * 2);
	// Вычисляем локальное ограничение разряда 
	limit = interpol(CurrentLimitArray, CCL_DCL_POINTS_NUM, handle->MinCellVoltage.Voltage_mv);
	DCLCurrentLimit = (limit < DCLCurrentLimit)? limit : DCLCurrentLimit;
	
	
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
	int32_t v = ((int32_t)DCLCurrentLimit * ecuConfig->Sys_MaxDCL) / 100;
	DCLCurrentLimit = v;
	
	v = ((int32_t)CCLCurrentLimit * ecuConfig->Sys_MaxCCL) / 100;
	CCLCurrentLimit = (ecuConfig->Sys_MaxCCL > 0)? 0 : v;
	
	if (handle->StateMachine.MainState == WORKSTATE_FAULT)
	{
		*Discharge = 0;
		*Charge = 0;
	}
	else
	{
		*Discharge = DCLCurrentLimit;
		*Charge = CCLCurrentLimit;
	}

	return;
}





