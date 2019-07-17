#include "Main.h"
#include "BatteryMeasuring.h"
#include "../MolniaLib/MF_Tools.h"
#include "TimerFunc.h"
#include <string.h>


#define SOC_TABLE_SIZE			(CCL_DCL_POINTS_NUM * 2)

int16_t CurrentLimitArray[CCL_DCL_POINTS_NUM * 2];

void GetCellStatistics(BatteryData_t* bat)
{
	uint16_t nv = 0;
	uint32_t SumVoltage = 0;
	
	EcuConfig_t ecuConfig = GetConfigInstance();
	
	int16_t nt = 0;
	int32_t SumTemperature = 0;
	uint8_t cellsCount = ecuConfig.CellNumber;
	
	
	
	bat->MaxCellVoltage.Voltage_mv = INT16_MIN;	
	bat->MaxCellVoltage.CellNumber = cellsCount;
	bat->MaxCellVoltage.ModuleNumber = ecuConfig.ModuleIndex;
	
	bat->MinCellVoltage.Voltage_mv = INT16_MAX;
	bat->MinCellVoltage.CellNumber = cellsCount;	
	bat->MaxCellVoltage.ModuleNumber = ecuConfig.ModuleIndex;	
	
	bat->MaxModuleTemperature.Temperature = INT8_MIN;	
	bat->MaxModuleTemperature.SensorNum = TMP_SENSOR_IN_MODULE;
	bat->MaxModuleTemperature.ModuleNumber = ecuConfig.ModuleIndex;
    
	bat->MinModuleTemperature.Temperature = INT8_MAX;
	bat->MinModuleTemperature.SensorNum = TMP_SENSOR_IN_MODULE;
	bat->MinModuleTemperature.ModuleNumber = ecuConfig.ModuleIndex;	    

    // ������� Max/Min ����������
    for (uint8_t cellInd = 0; cellInd < cellsCount; cellInd++)
    {
        nv++;
        SumVoltage += OD.CellVoltageArray_mV[cellInd];

        // ����� ������������ ����������
        if (OD.CellVoltageArray_mV[cellInd] > bat->MaxCellVoltage.Voltage_mv)
        {
            bat->MaxCellVoltage.Voltage_mv = OD.CellVoltageArray_mV[cellInd];
            bat->MaxCellVoltage.CellNumber = cellInd;
        }
        // ����� ������������ ����������
        if (OD.CellVoltageArray_mV[cellInd] < bat->MinCellVoltage.Voltage_mv)
        {
            bat->MinCellVoltage.Voltage_mv = OD.CellVoltageArray_mV[cellInd];
            bat->MinCellVoltage.CellNumber = cellInd;
        }
    }    
    
    bat->TotalVoltage = SumVoltage / 100;
    
    // ������� �������� ���������� �� �������
	if(nv > 0)
		bat->AvgCellVoltage = SumVoltage / nv;	

    // ������� Max/Min �����������
	for (uint8_t TempSensCnt = 0; TempSensCnt < TMP_SENSOR_IN_MODULE; TempSensCnt++)
    {
        int8_t t = OD.CellTemperatureArray[TempSensCnt];
        
        if (t > 100 || t < -100)
            continue;

        nt++;
        SumTemperature += t;
        
        if (t > bat->MaxModuleTemperature.Temperature)
        {
            bat->MaxModuleTemperature.Temperature = t;    
			bat->MaxModuleTemperature.SensorNum = TempSensCnt;
        }
        if (t < bat->MinModuleTemperature.Temperature)
        {
            bat->MinModuleTemperature.Temperature = t;   
			bat->MinModuleTemperature.SensorNum = TempSensCnt;					
        }
    }
        
    if(nv > 0)
		bat->AvgModuleTemperature = SumTemperature / nt;	
	
}


void GetBatteriesStatistic(BatteryData_t *Data, BatteryData_t *SourceData, uint8_t BatteryCount, uint8_t IsModulesData)
{
    uint32_t SumVoltageTemp = 0;	
	uint32_t SumAvgCellVoltage = 0;
	uint32_t SumCurrentTmp = 0;
	uint32_t SumEnergyTmp = 0;
	uint8_t n = 0;
	
	EcuConfig_t ecuConfig = GetConfigInstance();

	Data->TotalCurrent = 0;
	Data->MinCellVoltage.Voltage_mv = INT16_MAX;
	Data->MaxCellVoltage.Voltage_mv = INT16_MIN;
	Data->MinModuleTemperature.Temperature = INT8_MAX;
	Data->MaxModuleTemperature.Temperature = INT8_MIN;
	Data->MaxBatteryCurrent.Current = INT16_MIN;
	Data->MinBatteryCurrent.Current = INT16_MAX;
	Data->MaxBatteryVoltage.Voltage = 0;
	Data->MinBatteryVoltage.Voltage = UINT16_MAX;
	Data->CCL = INT16_MIN;
	Data->DCL = INT16_MAX;
		
	for (uint8_t i = 0; i < BatteryCount; i++)
	{
        // �������� ��������
		Data->OnlineFlags = 1;
		Data->OnlineNumber = 1;
        
        for (uint8_t i = 1; i < BatteryCount; i++)
		{
			// ���� �������/������ � ����
            uint32_t bat_delay = GetTimeFrom(SourceData[i].TimeOutCnts);
			if (bat_delay < 1000)
			{              
				// ������������� ������� �������
				Data->OnlineFlags |= 1L << i;
				Data->OnlineNumber = Data->OnlineNumber + 1;
			}
		}
        
        
		// ���� ������� ��� � ���� - ��������� ���������
		if (!(Data->OnlineFlags & (1L << i)))
			continue;	
		
		// ������� �������� ���������� �� �������
		SumAvgCellVoltage += SourceData[i].AvgCellVoltage;
		n++;
		
		// ������� Max/Min ����������
		SumVoltageTemp += SourceData[i].TotalVoltage;
		SumCurrentTmp += SourceData[i].TotalCurrent;
		SumEnergyTmp += SourceData[i].DischargeEnergy_Ah;

		// ����� ������������ �����������
		if (SourceData[i].MaxCellVoltage.Voltage_mv > Data->MaxCellVoltage.Voltage_mv)
		{
			Data->MaxCellVoltage.Voltage_mv = SourceData[i].MaxCellVoltage.Voltage_mv;
			Data->MaxCellVoltage.CellNumber = SourceData[i].MaxCellVoltage.CellNumber;
			Data->MaxCellVoltage.ModuleNumber = i;
			Data->MaxCellVoltage.BatteryNumber = i;
		}
		// ����� ������������ �����������
		if (SourceData[i].MinCellVoltage.Voltage_mv < Data->MinCellVoltage.Voltage_mv)
		{
			Data->MinCellVoltage.Voltage_mv = SourceData[i].MinCellVoltage.Voltage_mv;
			Data->MinCellVoltage.CellNumber = SourceData[i].MinCellVoltage.CellNumber;
			Data->MinCellVoltage.ModuleNumber = i;
			Data->MinCellVoltage.BatteryNumber = i;
		}
		// ����� ����������� �����������
		if (SourceData[i].MaxModuleTemperature.Temperature > Data->MaxModuleTemperature.Temperature)
		{
			Data->MaxModuleTemperature.Temperature = SourceData[i].MaxModuleTemperature.Temperature;
			Data->MaxModuleTemperature.SensorNum = SourceData[i].MaxModuleTemperature.SensorNum;
			Data->MaxModuleTemperature.ModuleNumber = i;
			Data->MaxModuleTemperature.BatteryNumber = i;
		}
		// ����� ����������� �����������
		if (SourceData[i].MinModuleTemperature.Temperature < Data->MinModuleTemperature.Temperature)
		{
			Data->MinModuleTemperature.Temperature = SourceData[i].MinModuleTemperature.Temperature;
			Data->MinModuleTemperature.SensorNum = SourceData[i].MinModuleTemperature.SensorNum;
			Data->MinModuleTemperature.ModuleNumber = i;
			Data->MinModuleTemperature.BatteryNumber = i;
		}
		// ����� ������������ � ������������� ����
		if (SourceData[i].TotalCurrent > Data->MaxBatteryCurrent.Current)
		{
			Data->MaxBatteryCurrent.Current = SourceData[i].TotalCurrent;
			Data->MaxBatteryCurrent.BatteryNumber = i;
		}
		if (SourceData[i].TotalCurrent < Data->MinBatteryCurrent.Current)
		{
			Data->MinBatteryCurrent.Current = SourceData[i].TotalCurrent;
			Data->MinBatteryCurrent.BatteryNumber = i;
		}
		
		// ����� ������������ � ������������� ����������
		if (SourceData[i].TotalVoltage > Data->MaxBatteryVoltage.Voltage)
		{
			Data->MaxBatteryVoltage.Voltage = SourceData[i].TotalVoltage;
			Data->MaxBatteryVoltage.BatteryNumber = i;
		}
		if (SourceData[i].TotalVoltage < Data->MinBatteryVoltage.Voltage)
		{
			Data->MinBatteryVoltage.Voltage = SourceData[i].TotalVoltage;
			Data->MinBatteryVoltage.BatteryNumber = i;
		}
	}
	
	
	
	if(n > 0)
		Data->AvgCellVoltage = SumAvgCellVoltage / n;
		
	Data->DischargeEnergy_Ah = SumEnergyTmp;
	Data->TotalVoltage = (IsModulesData)? SumVoltageTemp : SumVoltageTemp / n;
	Data->TotalCurrent = SumCurrentTmp;
	
	// ������ �������� ����������� CCL ������� �� ���������� �������
	Data->CCL = (!IsModulesData)?  OD.MasterControl.CCL * BatteryCount : OD.MasterControl.CCL;
	Data->DCL = (!IsModulesData)?  OD.MasterControl.DCL * BatteryCount : OD.MasterControl.DCL;
	
}

// ������� ��������� ������� �� ������������ ����������
uint32_t GetEnergyFromMinUcell(int16_t *OcvTable, uint16_t Voltage, uint16_t TotalCapatity)
{
	uint32_t TempSoC = interpol(OcvTable, SOC_TABLE_SIZE, Voltage);	
	
	return (TempSoC * (TotalCapatity * 36)) / 10;			// Ah * 3600 [Asec] * SoC / 100;
}



// ������ ������� � A����-��������
// ���������:
//  - ��� �������, � �����*10
//  - ��������� �� ������� �������
uint16_t CapacityCulc(int16_t Current, uint32_t* CurrentEnergy, uint16_t ModuleCapacity)
{
	static int32_t EnergyLow = 0;
	static int32_t EnergyLowPrev = 0;
    
    static uint32_t CapacityCulcStamp = 0;
	
	uint16_t result = 0;
	result = (10L * (*(CurrentEnergy))) / (36L * ModuleCapacity);
	
	// ������ 100 ��
	uint32_t tm = GetTimeFrom(CapacityCulcStamp);
	if (tm >= 100)
	{
		EcuConfig_t ecuConfig = GetConfigInstance();
		CapacityCulcStamp = GetTimeStamp() - (tm - 100);
		
		// ������������� ��� �������� �� �������
		// �������������� ����. �������� �������. ��� � 0.1 ������
		EnergyLow -= Current;
	}

	if (EnergyLow - EnergyLowPrev >= 100)			// �����
	{
		(*(CurrentEnergy))++;
		EnergyLowPrev += 100;
	}
	else if ((EnergyLow - EnergyLowPrev) <= -100)	// ������
	{
		if ((*(CurrentEnergy)) > 0)
			(*(CurrentEnergy))--;
		
		EnergyLowPrev -= 100;
	}
	
	if(result > 1000)
			result = 1000;
	
	return result;
}

uint16_t TargetVoltageCulc(uint16_t MinSystemCellVoltage, uint16_t MinModuleCellVoltage)
{
	EcuConfig_t ecuConfig = GetConfigInstance();
	uint16_t _minVolt_mV = (ecuConfig.IsMaster)? MinSystemCellVoltage : MinModuleCellVoltage;
	
	// ������ ���������� ������������
	uint16_t TargetVoltTemp_mV = _minVolt_mV + ecuConfig.MaxVoltageDiff;
	
	if (TargetVoltTemp_mV < ecuConfig.MinVoltageForBalancing)
		TargetVoltTemp_mV = ecuConfig.MinVoltageForBalancing;
	
	// ���� ������ ��� ������ �������� ���������, �� ���������� ������������ ��������, ���� ���, �� �������� �������� �� CAN
	if(ecuConfig.IsMaster || ecuConfig.IsAutonomic)
		TargetVoltTemp_mV = TargetVoltTemp_mV;
	else
		TargetVoltTemp_mV = OD.MasterControl.TargetVoltage_mV;	
	
	return TargetVoltTemp_mV;
}



void GetCurrentLimit(int16_t* Discharge, int16_t* Charge)
{
	int16_t limit;
	int16_t CCLCurrentLimit = 100;
	int16_t DCLCurrentLimit = 100;
	
	EcuConfig_t ecuConfig = GetConfigInstance();
	
	// ������ ������ ����������� ����������� ����
	if(!ecuConfig.IsMaster)
		return;
	
	// �������� �� ��������� ������ ������ �� ����������� ������ �� ����������
	memcpy(CurrentLimitArray, ecuConfig.VoltageCCLpoint, CCL_DCL_POINTS_NUM * 4);
	// ��������� ��������� ����������� ������
	limit = interpol(CurrentLimitArray, CCL_DCL_POINTS_NUM, OD.ModuleData[ecuConfig.ModuleIndex].MaxCellVoltage.Voltage_mv);
	// ���� ��� ����������� �����, ��� ����������, �� ��������� ����������
	if (limit < CCLCurrentLimit)
	{
		CCLCurrentLimit = limit;
	}
	
	// �������� �� ��������� ������ ������ �� ����������� �������
	memcpy((uint8_t*)(CurrentLimitArray) + (CCL_DCL_POINTS_NUM * 2), (uint8_t*)(ecuConfig.VoltageCCLpoint) + CCL_DCL_POINTS_NUM * 4, CCL_DCL_POINTS_NUM * 2);
	// ��������� ��������� ����������� ������� 
	limit = interpol(CurrentLimitArray, CCL_DCL_POINTS_NUM, OD.ModuleData[ecuConfig.ModuleIndex].MinCellVoltage.Voltage_mv);
	if (limit < DCLCurrentLimit)
	{
		DCLCurrentLimit = limit;
	}
	
	
	// ����������� �� �����������.
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
	// ������� ��������� � ������
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





