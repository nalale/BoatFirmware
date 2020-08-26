#include <string.h>

#include "Main.h"
#include "BatteryMeasuring.h"
#include "../MolniaLib/MF_Tools.h"

#include "TimerFunc.h"

#define SOC_TABLE_SIZE			(CCL_DCL_POINTS_NUM * 2)
#define ENERGY_THRESHOLD_AMP_SEC	18000	// 5Ah
#define ENERGY_RAMP_AMP_SEC			360		// 1/10Ah

static int16_t CurrentLimitArray[CCL_DCL_POINTS_NUM * 2];

// Сalculation of module energy by the minimum cell voltage and the OCV table
uint32_t sysEnergy_InitEnergyFromMinUcell(BatteryData_t *Handle, int16_t *OcvTable, uint16_t CellVoltage)
{
	if(Handle == 0 || OcvTable == 0 || CellVoltage == 0)
		return 1;
	
	uint32_t TempSoC = interpol(OcvTable, SOC_TABLE_SIZE, CellVoltage);	
	Handle->ActualEnergy_As = (TempSoC * (Handle->TotalEnergy_As) / 100);			// Asec * SoC / 100;
	
	return 0;
}

// Calculation of current module capacity by current value (Amp * 10) and current energy (Amps)
uint16_t sysEnergy_CoulombCounting(BatteryData_t *Handle, int16_t Current_0p1)
{
    uint32_t* CurrentEnergy_Amps = &Handle->ActualEnergy_As;
    uint32_t ModuleCapacity_As = Handle->TotalEnergy_As;
	
	uint16_t result = 0;
	result = (100L * (*CurrentEnergy_Amps)) / ModuleCapacity_As;
	
	// period of calculation is 100ms
	uint32_t tm = GetTimeFrom(Handle->CapacityCulcStamp);
	if (tm >= 100)
	{
		Handle->CapacityCulcStamp = GetTimeStamp() - (tm - 100);
		Handle->EnergyLow -= Current_0p1;
	}

	// Coulomb counting
	if (Handle->EnergyLow - Handle->EnergyLowPrev >= 100)			// Charging
	{
		Handle->CurrentCycleEnergy_As++;

		(*CurrentEnergy_Amps)++;
		Handle->EnergyLowPrev += 100;
	}
	else if ((Handle->EnergyLow - Handle->EnergyLowPrev) <= -100)	// Discharging
	{
		Handle->CurrentCycleEnergy_As--;

		if ((*CurrentEnergy_Amps) > 0)
			(*CurrentEnergy_Amps)--;
		
		Handle->EnergyLowPrev -= 100;
	}
	
	if(result > 100)
			result = 100;
	
	return result;
}

int8_t sysEnergy_Init(BatteryData_t *Handle, uint32_t InitialTotalSystemEnergy_As, uint32_t InitialActualEnergy_As, uint16_t MaxCellVoltageThreshold_mV, uint16_t MinCellVoltageThreshold_mV)
{
	Handle->TotalEnergy_As = InitialTotalSystemEnergy_As;
	Handle->ActualEnergy_As = InitialActualEnergy_As;
	Handle->MaxCellVoltageThreshold_mV = MaxCellVoltageThreshold_mV - 50;
	Handle->MinCellVoltageThreshold_mV = MinCellVoltageThreshold_mV + 50;

	Handle->IsInit = 1;

	return 0;
}

uint32_t sysEnergy_EnergyEstimation(BatteryData_t *Handle, uint16_t MaxCellVoltage_mV, uint16_t MinCellVoltage_mV)
{
	static uint32_t low_energy_ts = 0, high_energy_ts = 0;
	uint32_t *CurrentEnergy_As = &Handle->ActualEnergy_As;
	uint32_t TotalEstimatedEnergy = Handle->TotalEnergy_As;
	// Charge is done
	if(MaxCellVoltage_mV >= Handle->MaxCellVoltageThreshold_mV)
	{
		if(!Handle->EstimationDoneInThisCycle && GetTimeFrom(high_energy_ts) > 5000)
		{
			Handle->EstimationDoneInThisCycle = 1;

			if(TotalEstimatedEnergy >= *CurrentEnergy_As)
			{
				if((TotalEstimatedEnergy - *CurrentEnergy_As) > ENERGY_THRESHOLD_AMP_SEC)
				{
					TotalEstimatedEnergy -= (TotalEstimatedEnergy > ENERGY_THRESHOLD_AMP_SEC)? ENERGY_RAMP_AMP_SEC : 0;
				}
			}
			else
			{
				if((*CurrentEnergy_As - TotalEstimatedEnergy) > ENERGY_THRESHOLD_AMP_SEC)
				{
					*CurrentEnergy_As = TotalEstimatedEnergy + ENERGY_RAMP_AMP_SEC;
					TotalEstimatedEnergy += ENERGY_RAMP_AMP_SEC;
				}
			}
		}
	}
	else if(MinCellVoltage_mV <= Handle->MinCellVoltageThreshold_mV)
	{
		if(!Handle->EstimationDoneInThisCycle && GetTimeFrom(low_energy_ts) > 5000)
		{
			if(*CurrentEnergy_As > ENERGY_THRESHOLD_AMP_SEC)
			{
				Handle->EstimationDoneInThisCycle = 1;
				*CurrentEnergy_As = ENERGY_THRESHOLD_AMP_SEC;
				TotalEstimatedEnergy += ENERGY_RAMP_AMP_SEC;
			}
			else if(*CurrentEnergy_As == 0)
			{
				Handle->EstimationDoneInThisCycle = 1;
				*CurrentEnergy_As = ENERGY_THRESHOLD_AMP_SEC;
				TotalEstimatedEnergy -= (TotalEstimatedEnergy > ENERGY_THRESHOLD_AMP_SEC)? ENERGY_RAMP_AMP_SEC : 0;
			}
		}
	}
	else
	{
		low_energy_ts = GetTimeStamp();
		high_energy_ts = GetTimeStamp();
		Handle->EstimationDoneInThisCycle = 0;
	}

	return TotalEstimatedEnergy;
}




void GetCurrentLimit(const BatteryData_t* handle, const EcuConfig_t *ecuConfig, int16_t *Discharge, int16_t *Charge)
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

	if (handle->MainState == WORKSTATE_FAULT)
	{
		*Discharge = 0;
		*Charge = 0;
	}
	else
	{
		// if less or max value
		if((CCLCurrentLimit >= *Charge) || (CCLCurrentLimit == ecuConfig->Sys_MaxCCL))		//CCL < 0
			*Charge = CCLCurrentLimit;	// Снижаем сразу
		else
			*Charge -= 1;	// Повышаем постепенно

		// if less or max value
		if((DCLCurrentLimit <= *Discharge) || (DCLCurrentLimit == ecuConfig->Sys_MaxDCL))
			*Discharge = DCLCurrentLimit;	// Снижаем сразу
		else
			*Discharge += 1;				// Повышаем постепенно

//		*Discharge = DCLCurrentLimit;
//		*Charge = CCLCurrentLimit;
	}

	return;
}

// Function Defenition:
// SOC is calculated for each battery by themself.
// Master calculates SOC as minimum SOC values of each battery
uint8_t sysEnergy_EnergyCounting(BatteryData_t* Handle, int16_t Current_0p1A)
{
	if(Handle == 0)
		return 0;

	if(Handle->IsInit && Current_0p1A != INT16_MAX)
	{
		Handle->TotalCurrent = Current_0p1A;
		Handle->TotalEnergy_As = sysEnergy_EnergyEstimation(Handle, Handle->MaxCellVoltage.Voltage_mv, Handle->MinCellVoltage.Voltage_mv);
		Handle->SoC = sysEnergy_CoulombCounting(Handle, Handle->TotalCurrent);
	}
	else if(Current_0p1A == INT16_MAX)
	{
		Handle->TotalCurrent = 0;
		Handle->TotalEnergy_As = UINT32_MAX;
		Handle->SoC = UINT16_MAX;
	}

	return 1;

}
