#include <stdio.h>
#include <stdlib.h>
#include <lpc17xx.h>
#include "lpc17xx_clkpwr.h"

#include "Main.h"
#include "SpiFunc.h"
#include "TimerFunc.h"
#include "../BoardDefinitions/BmsCombi_Board.h"
#include "BMS_Combi_ECU.h"
#include "AdcFunc.h"
#include "CanFunc.h"
#include "WorkStates.h"
#include "Protocol.h"
#include "User.h"
#include "../MolniaLib/DateTime.h"


void CommonState(void);

// Алгоритм работы
void (*Algorithm)(uint8_t *);

// Словарь объектов
ObjectDictionary_t OD;
static EcuConfig_t _config;
static uint8_t Buffer[512];


int main(int argc, char** argv) {
	
	
	// Инициализация контроллера, внутренних переменных словаря
    AppInit(&OD);
    
    SetWorkState(&OD.StateMachine, WORKSTATE_INIT);

    while (1) 
    {
        CommonState();
        
		if(GetTimeFrom(OD.LogicTimers.MainLoopTimer_ms) > OD.DelayValues.MainLoopTime_ms)
		{
			OD.LogicTimers.MainLoopTimer_ms = GetTimeStamp();
            Algorithm(&OD.StateMachine.SubState);
		}		
    }
}



uint8_t GetDataByIndex(uint16_t Index, uint8_t subindex, uint8_t **Buf)
{
	uint8_t _size = 0;
	_config = GetConfigInstance();
	
	switch(Index)
	{
		case didEcuInfo:
			*Buf = (uint8_t*)(&OD.EcuInfo[subindex]);
			_size = (subindex > 3)? 0 : sizeof(OD.EcuInfo[0]);
		break;
		
		// Module Parameters
		case didConfigStructIndex:			
			*Buf = (uint8_t*)&_config;
			_size = CONFIG_SIZE;
		break;
		
		case didDateTime:
		{
			*Buf = (uint8_t*)&secondsTotal;			
			_size = (subindex < 1)? sizeof(secondsTotal) : 0;
		}
		break;
		
		case didEcuVoltage:
		{
			*Buf = (uint8_t*)&OD.ecuPowerSupply_0p1;			
			_size = (subindex < 1)? sizeof(OD.ecuPowerSupply_0p1) : 0;
		}
		break;
		
		case didPowerManagmentState:
		{
			*Buf = (uint8_t*)&OD.PowerMaganerState;			
			_size = (subindex < 1)? sizeof(OD.PowerMaganerState) : 0;
		}
		break;
		
		case didVoltageSensor1:
			*Buf = (uint8_t*)&OD.CurrentSensorVoltage[0];
			_size = (subindex > 0)? 0 : sizeof(OD.CurrentSensorVoltage[0]);
		break;
		
		case didVoltageSensor2:
			*Buf = (uint8_t*)&OD.CurrentSensorVoltage[1];
			_size = (subindex > 0)? 0 : sizeof(OD.CurrentSensorVoltage[1]);
		break;
		
		case didVoltageSensor3:
			*Buf = (uint8_t*)&OD.A_IN[0];
			_size = (subindex > 0)? 0 : sizeof(OD.A_IN[0]);
		break;
		
		case didVoltageSensor4:
			*Buf = (uint8_t*)&OD.A_IN[1];
			_size = (subindex > 0)? 0 : sizeof(OD.A_IN[1]);
		break;
		
		case didModMachineState:
			*Buf = (uint8_t*)&OD.StateMachine.MainState;
			_size = (subindex > 0)? 0 : sizeof(OD.StateMachine.MainState);
		break;
		
		case didModMachineSubState:
			*Buf = (uint8_t*)&OD.StateMachine.SubState;
			_size = (subindex > 0)? 0 : sizeof(OD.StateMachine.SubState);
		break;
		
		case didCellsVoltages:
			*Buf = (uint8_t*)(&OD.CellVoltageArray_mV[subindex]);
			_size = (subindex < _config.CellNumber)? sizeof(OD.CellVoltageArray_mV[0]) : 0;
			break;
		case didModuleTemperatures:
			*Buf = (uint8_t*)(&OD.CellTemperatureArray[subindex]);
			_size = (subindex < TMP_SENSOR_IN_MODULE)? sizeof(OD.CellTemperatureArray[0]) : 0;
		break;	
		case didModMaxCellVoltage:
			*Buf = (uint8_t*)&OD.ModuleData[_config.ModuleIndex].MaxCellVoltage.Voltage_mv;
			_size = (subindex > 0)? 0 : sizeof(OD.ModuleData[_config.ModuleIndex].MaxCellVoltage.Voltage_mv);
		break;
		
		case didModMinCellVoltage:
			*Buf = (uint8_t*)&OD.ModuleData[_config.ModuleIndex].MinCellVoltage.Voltage_mv;
			_size = (subindex > 0)? 0 : sizeof(OD.ModuleData[_config.ModuleIndex].MinCellVoltage.Voltage_mv);
		break;
		
		case didModMaxTemperature:
			*Buf = (uint8_t*)&OD.ModuleData[_config.ModuleIndex].MaxModuleTemperature.Temperature;
			_size = (subindex > 0)? 0 : sizeof(OD.ModuleData[_config.ModuleIndex].MaxModuleTemperature.Temperature);
		break;
		
		case didModMinTemperature:
			*Buf = (uint8_t*)&OD.ModuleData[_config.ModuleIndex].MinModuleTemperature.Temperature;
			_size = (subindex > 0)? 0 : sizeof(OD.ModuleData[_config.ModuleIndex].MinModuleTemperature.Temperature);
		break;
		
		case didModTotalVoltage:
			*Buf = (uint8_t*)&OD.ModuleData[_config.ModuleIndex].TotalVoltage;
			_size = (subindex > 0)? 0 : sizeof(OD.ModuleData[_config.ModuleIndex].TotalVoltage);
		break;
		
		case didModDischargeCellsFlag:
			*Buf = (uint8_t*)&OD.ModuleData[_config.ModuleIndex].DischargingCellsFlag;
			_size = (subindex > 0)? 0 : sizeof(OD.ModuleData[_config.ModuleIndex].DischargingCellsFlag);
		break;
		
		case didModInOutState:
			*Buf = (uint8_t*)&OD.InOutState;
			_size = (subindex > 0)? 0 : sizeof(OD.InOutState);
		break;
		
		// Battery Parameters
		case didBatMaxCellVoltage:
			*Buf = (uint8_t*)&OD.BatteryData[_config.BatteryIndex].MaxCellVoltage.Voltage_mv;
			_size = (subindex > 0)? 0 : sizeof(OD.BatteryData[_config.BatteryIndex].MaxCellVoltage.Voltage_mv);
		break;
		
		case didBatMinCellVoltage:
			*Buf = (uint8_t*)&OD.BatteryData[_config.BatteryIndex].MinCellVoltage.Voltage_mv;
			_size = (subindex > 0)? 0 : sizeof(OD.BatteryData[_config.BatteryIndex].MinCellVoltage.Voltage_mv);
		break;
		
		case didBatMaxTemperature:
			*Buf = (uint8_t*)&OD.BatteryData[_config.BatteryIndex].MaxModuleTemperature.Temperature;
			_size = (subindex > 0)? 0 : sizeof(OD.BatteryData[_config.BatteryIndex].MaxModuleTemperature.Temperature);
		break;
		
		case didBatMinTemperature:
			*Buf = (uint8_t*)&OD.BatteryData[_config.BatteryIndex].MaxModuleTemperature.Temperature;
			_size = (subindex > 0)? 0 : sizeof(OD.BatteryData[_config.BatteryIndex].MaxModuleTemperature.Temperature);
		break;
		
		case didBatMaxVoltage:
			*Buf = (uint8_t*)&OD.BatteryData[_config.BatteryIndex].MaxBatteryVoltage.Voltage;
			_size = (subindex > 0)? 0 : sizeof(OD.BatteryData[_config.BatteryIndex].MaxBatteryVoltage.Voltage);
			break;
		
		case didBatMinVoltage:
			*Buf = (uint8_t*)&OD.BatteryData[_config.BatteryIndex].MinBatteryVoltage.Voltage;
			_size = (subindex > 0)? 0 : sizeof(OD.BatteryData[_config.BatteryIndex].MinBatteryVoltage.Voltage);
		break;
		
		case didBatTotalCurrent:
			*Buf = (uint8_t*)&OD.BatteryData[_config.BatteryIndex].TotalCurrent;
			_size = (subindex > 0)? 0 : sizeof(OD.BatteryData[_config.BatteryIndex].TotalCurrent);
		break;
		
		case didBatTotalVoltage:
			*Buf = (uint8_t*)&OD.BatteryData[_config.BatteryIndex].TotalVoltage;
			_size = (subindex > 0)? 0 : sizeof(OD.BatteryData[_config.BatteryIndex].TotalCurrent);
		break;
		
		case didBatStateOfCharge:
			*Buf = (uint8_t*)&OD.BatteryData[_config.BatteryIndex].SoC;
			_size = (subindex > 0)? 0 : sizeof(OD.BatteryData[_config.BatteryIndex].SoC);
		break;
		
		case didBatEnergy:
			*Buf = (uint8_t*)&OD.BatteryData[_config.BatteryIndex].DischargeEnergy_Ah;
			_size = (subindex > 0)? 0 : sizeof(OD.BatteryData[_config.BatteryIndex].DischargeEnergy_Ah);
		break;	
		
		case didBatCCL:		
			*Buf = (uint8_t*)&OD.BatteryData[_config.BatteryIndex].CCL;
			_size = (subindex > 0)? 0 : sizeof(OD.MasterControl.CCL);
		break;
		
		case didBatDCL:		
			*Buf = (uint8_t*)&OD.BatteryData[_config.BatteryIndex].DCL;
			_size = (subindex > 0)? 0 : sizeof(OD.MasterControl.DCL);
		break;
		
		// Master parameters				
		case didMstTotalCurrent:
			*Buf = (uint8_t*)&OD.MasterData.TotalCurrent;
			_size = (subindex > 0)? 0 : sizeof(OD.MasterData.TotalCurrent);
		break;
		
		case didMstTotalVoltage:
			*Buf = (uint8_t*)&OD.MasterData.TotalVoltage;
			_size = (subindex > 0)? 0 : sizeof(OD.MasterData.TotalVoltage);
		break;
				
		case didMstMaxCurrent:
			*Buf = (uint8_t*)&OD.MasterData.MaxBatteryCurrent.Current;
			_size = (subindex > 0)? 0 : sizeof(OD.MasterData.MaxBatteryCurrent.Current);
			break;
		
		case didMstMinCurrent:
			*Buf = (uint8_t*)&OD.MasterData.MinBatteryCurrent.Current;
			_size = (subindex > 0)? 0 : sizeof(OD.MasterData.MinBatteryCurrent.Current);
		break;
				
		case didMstMaxVoltage:
			*Buf = (uint8_t*)&OD.MasterData.MaxBatteryVoltage.Voltage;
			_size = (subindex > 0)? 0 : sizeof(OD.MasterData.MaxBatteryVoltage.Voltage);
			break;
		
		case didMstMinVoltage:
			*Buf = (uint8_t*)&OD.MasterData.MinBatteryVoltage.Voltage;
			_size = (subindex > 0)? 0 : sizeof(OD.MasterData.MinBatteryVoltage.Voltage);			
		break;
		
		case didMstMaxTemperature:
			*Buf = (uint8_t*)&OD.MasterData.MaxModuleTemperature.Temperature;
			_size = (subindex > 0)? 0 : sizeof(OD.MasterData.MaxModuleTemperature.Temperature);
		break;
		
		case didMstMinTemperature:
			*Buf = (uint8_t*)&OD.MasterData.MinModuleTemperature.Temperature;
			_size = (subindex > 0)? 0 : sizeof(OD.MasterData.MinModuleTemperature.Temperature);			
		break;
		
		case didMstCellMaxVoltage:
			*Buf = (uint8_t*)&OD.MasterData.MaxCellVoltage.Voltage_mv;
			_size = (subindex > 0)? 0 : sizeof(OD.MasterData.MaxCellVoltage.Voltage_mv);
		break;
		
		case didMstCellMinVoltage:
			*Buf = (uint8_t*)&OD.MasterData.MinCellVoltage.Voltage_mv;
			_size = (subindex > 0)? 0 : sizeof(OD.MasterData.MinCellVoltage.Voltage_mv);
			break;
		
		case didMstStateOfCharge:
			*Buf = (uint8_t*)&OD.MasterData.SoC;
			_size = (subindex > 0)? 0 : sizeof(OD.MasterData.SoC);
		break;
		
		case didMstEnergy:		
			*Buf = (uint8_t*)&OD.MasterData.DischargeEnergy_Ah;
			_size = (subindex > 0)? 0 : sizeof(OD.MasterData.DischargeEnergy_Ah);
		break;
		
		case didMstCCL:		
			*Buf = (uint8_t*)&OD.MasterData.CCL;
			_size = (subindex > 0)? 0 : sizeof(OD.MasterData.CCL);
		break;
		
		case didMstDCL:		
			*Buf = (uint8_t*)&OD.MasterData.DCL;
			_size = (subindex > 0)? 0 : sizeof(OD.MasterData.DCL);
		break;
		
		// Faults parameters
		case didFaults_Actual:
		{			
			*Buf = (uint8_t*)(&OD.FaultList[subindex]);
			_size = (subindex >= OD.FaultsNumber)? 0 : sizeof(OD.FaultList[subindex]);
		}
		break;
		
		case didFaults_History:
		{
			*Buf = (uint8_t*)(&OD.OldFaultList[subindex]);
			_size = (subindex >= OD.OldFaultsNumber)? 0 : sizeof(OD.OldFaultList[subindex]);
		}
		break;
		
		case didFaults_FreezeFrame:
		{
			dtcItem_t* dtc;
			uint16_t addr = 0;
			const DiagnosticValueFRZF* f;
			for(int i = 0; i < dtcListSize; i++)
			{
				dtc = dtcList[i];
				uint8_t isConfirmed = dtc->Status.ConfirmedDTC;
				
				*(Buffer + addr++) = dtc->Category;
				*(Buffer + addr++) = isConfirmed;
			}
	
			for(uint8_t i = 0; i < dtcListSize; i++)
			{
				dtc = dtcList[i];
				f = dtc->Property->FreezeFrameItems;
				// Перебираем и сохраняем каждое значение стоп-кадра
				for(uint8_t n = 0; n < dtc->Property->FreezeFrameItemsCount; n++)
				{
					uint8_t* p = (uint8_t*)f->Ref;
					for(uint8_t k = 0; k < f->Length; k++)
					{
						if(dtc->Status.ConfirmedDTC)
							*(Buffer + addr) = p[k];
						addr++;
					}				
					// Если размера буффера не хватит выполнить Realloc			
					
					f++;	// Следующее значение
				}
			}
			_size = addr;
			*Buf = (uint8_t*)Buffer;
		}
			break;
	}
	
//	uint8_t tmp = _size / 4;
	//_size = (_size % 4 == 0)? tmp : tmp + 1;
	
	return _size;
}

