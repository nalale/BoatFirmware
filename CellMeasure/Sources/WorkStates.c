#include "Main.h"
#include "WorkStates.h"
#include "TimerFunc.h"
#include "User.h"
#include "Protocol.h"
#include "AdcFunc.h"

#include "BatteryMeasuring.h"

#include "FaultTools.h"
#include "BMS_Combi_ECU.h"
#include "EcuConfig.h"
#include "../MolniaLib/PowerManager.h"

#include "../MolniaLib/Config.h"
#include "../MolniaLib/DateTime.h"

#include "../Libs/CurrentSens.h"
#include "../Libs/LTC6803.h"

#include "MemoryFunc.h"
#include "UartFunc.h"


// Description:
// Intermidiate module: battery module is battery line intermidiate. It doesn't include current sensor and precharge circuit;
// Terminal module: battery module terminates batteries line, includes current sensor and collects line's statistic. Can make precharging;

uint32_t timeStamp;

// Массив указателей на функции-режимы
void (*FunArray[])(uint8_t *) =	{ InitializationState, PreoperationState, OperatingState, ShutdownState, FaultState, TestingState };

void LedBlink(void);

void SetWorkState(StateMachine_t *state_machine, WorkStates_e new_state)
{
    if(state_machine->MainState == WORKSTATE_SHUTDOWN 
		|| (state_machine->MainState == WORKSTATE_FAULT && new_state != WORKSTATE_SHUTDOWN))
		return;

	state_machine->MainState = new_state;
	state_machine->SubState = 0;
	Algorithm = FunArray[new_state];
}


void InitializationState(uint8_t *SubState)
{
    switch(*SubState)
    {
		// Read saved faults
    	case 0:
    		ModuleSetContactorPosition(OD.BatteryData, stBat_Disabled, OD.ConfigData, &timeStamp);
    		ControlBatteriesState(&OD.MasterControl.RequestState, WORKSTATE_INIT);

			if(ReadFaults())
				FillFaultsList(OD.OldFaultList, &OD.OldFaultsNumber, 0);

			OD.SB.CheckFaults = 0;          // запрещаем проверку ошибок
			OD.SB.CurrentSensorReady = 0;	// датчик тока не откалиброван
            OD.SB.FaultStateHandled = 1;	// все ошибки обработаны
            OD.SB.PrechargeDone = 0;		// Флаг успешного предзаряда
            
			OD.MasterControl.CCL = 0;
			OD.MasterControl.DCL = 0;

			// Сбрасываем счетчики предзаряда
			OD.LastPrechargeMaxCurrent_0p1A = INT16_MIN;
			OD.LastPrechargeDuration = 0;

			*SubState = 1;
    	break;
			
		// Waiting for power on
        case 1:
            
            if(OD.SB.PowerOn == 1)
			{				
            	OD.SB.CheckFaults = 1;

				// Intermediate module goes to Operating state and closes contactors
            	// Terminal module goes to sensor calibrating proccess
				if(!ModuleIsTerminal(OD.ConfigData))
					SetWorkState(&OD.StateMachine, WORKSTATE_OPERATE);
				else
					*SubState = 2;

				timeStamp = GetTimeStamp();
			}
            break;
        
		// Current sensor calibration
        case 2:
		{
            if(GetTimeFrom(timeStamp) > 1000)
            {
            	OD.SB.CurrentSensorReady = 1;
            	timeStamp = GetTimeStamp();

            	*SubState = 3;
            }
            else
            	csCalibrateCurrentSensor();
		}
            break;
            
        case 3:
			// Terminal module waits for all intermediate modules close contactors
        	if(BatteryIsReady(OD.BatteryData, OD.ModuleData, OD.ConfigData, &timeStamp))
        	{
        		OD.Energy_As = GetEnergyFromMinUcell((int16_t*)OD.ConfigData->OCVpoint, OD.BatteryData[OD.ConfigData->BatteryIndex].MinCellVoltage.Voltage_mv, OD.ConfigData->ModuleCapacity);
				timeStamp = GetTimeStamp();

				ControlBatteriesState(&OD.MasterControl.RequestState, WORKSTATE_PREOP);
        	}
            
            break;
    }       
}

void PreoperationState(uint8_t *SubState)
{ 
	static uint32_t preChargingTmr = 0;

	ModuleSetContactorPosition(OD.BatteryData, stBat_Disabled, OD.ConfigData, &timeStamp);

	// Мастер ждет когда система будет готовы к предзаряду
	if(MasterIsReady(&OD.MasterData, OD.BatteryData, OD.ConfigData, &preChargingTmr))
		ControlBatteriesState(&OD.MasterControl.RequestState, WORKSTATE_OPERATE);
}

void OperatingState(uint8_t *SubState)
{    
	if(ModuleSetContactorPosition(OD.BatteryData, stBat_Precharging, OD.ConfigData, &timeStamp))
	{
		OD.SB.PrechargeDone = 1;

		// Operation loop

	}
	else
	{
		// Считаем максимальный ток предзаряда
		if(OD.LastPrechargeMaxCurrent_0p1A < OD.BatteryData[OD.ConfigData->ModuleIndex].TotalCurrent)
			OD.LastPrechargeMaxCurrent_0p1A = OD.BatteryData[OD.ConfigData->ModuleIndex].TotalCurrent;

		// Считаем длительность предзаряда
		OD.LastPrechargeDuration = (uint16_t)GetTimeFrom(timeStamp);
	}
}


void ShutdownState(uint8_t *SubState)
{
	OD.SB.CheckFaults = 0;     
	
	switch (*SubState)
	{
		// Запомнить время входа в режим, сохранить данные
		case 0:
			OD.LogicTimers.PowerOffTimer_ms = GetTimeStamp();
			SaveFaults();
			*SubState = 1;
		break;

		case 1:
			ModuleSetContactorPosition(OD.BatteryData, stBat_Disabled, OD.ConfigData, &timeStamp);
			if(GetTimeFrom(OD.LogicTimers.PowerOffTimer_ms) >= OD.ConfigData->PowerOffDelay_ms)
				*SubState = 2;
		break;

		case 2:

			ECU_GoToSleep();
			break;
	}
//	if(!OD.SB.PowerOff_SaveParams)
//	{
//		OD.SB.PowerOff_SaveParams = 1;
//
//		if(OD.AfterResetTime > 3)
//		{
//			SaveFaults();
//		}
//	}
}

void FaultState(uint8_t *SubState)
{
	switch(*SubState)
	{
		case 0:
			ModuleSetContactorPosition(OD.BatteryData, stBat_Disabled, OD.ConfigData, &timeStamp);
			timeStamp = GetTimeStamp();
		break;
			
		case 1:
			if(GetTimeFrom(timeStamp) > 5000)
				OD.SB.FaultStateHandled = 1;
		break;
	}
	
}

void TestingState(uint8_t *SubState)
{
    
}

void CommonState(void)
{
    Protocol();
    
    if(GetTimeFrom(OD.LogicTimers.Timer_1ms) >= OD.DelayValues.Time1_ms)
    {        
        OD.LogicTimers.Timer_1ms = GetTimeStamp();
		
        // Code      
        OD.ecuPowerSupply_0p1 = EcuGetVoltage();
		OD.A_IN[0] = GetVoltageValue(4);
		OD.A_IN[1] = GetVoltageValue(5);		

		// Power Manager thread
		PM_Proc(OD.ecuPowerSupply_0p1, OD.ConfigData->IsPowerManager);
		ecuProc();
		vs_thread(OD.CellVoltageArray_mV, OD.CellTemperatureArray);
		BatteryCapacityCalculating(OD.BatteryData, OD.ConfigData, OD.SB.CurrentSensorReady);
		
		if(FaultsTest())
		{
			OD.SB.FaultStateHandled = 0;
			ControlBatteriesState(&OD.MasterControl.RequestState, WORKSTATE_FAULT);
			SetWorkState(&OD.StateMachine, WORKSTATE_FAULT);
		}
        
        // Переход в требуемое состояние для батареи и мастера
		if((OD.ConfigData->ModuleIndex == 0) && (OD.MasterControl.RequestState != OD.StateMachine.MainState && OD.StateMachine.MainState != WORKSTATE_FAULT))
		{			
			SetWorkState(&OD.StateMachine, OD.MasterControl.RequestState);
		}

		OD.BatteryData[OD.ConfigData->ModuleIndex].StateMachine.MainState = OD.StateMachine.MainState;
		OD.BatteryData[OD.ConfigData->ModuleIndex].StateMachine.SubState = OD.StateMachine.SubState;
    }
    
	if(GetTimeFrom(OD.LogicTimers.Timer_10ms) >= OD.DelayValues.Time10_ms)
	{
		OD.LogicTimers.Timer_10ms = GetTimeStamp();
		
		// модуль
        ModuleStatisticCalculating(&OD.ModuleData[OD.ConfigData->ModuleIndex], OD.ConfigData, OD.CellVoltageArray_mV, OD.CellTemperatureArray);
		// батарея
		BatteryStatisticCalculating(&OD.BatteryData[OD.ConfigData->BatteryIndex], OD.ModuleData, OD.ConfigData);
		// master		
		BatteryStatisticCalculating(&OD.MasterData, OD.BatteryData, OD.ConfigData);

		OD.PowerMaganerState = PM_GetPowerState();
		
		if(OD.PowerMaganerState == PM_PowerOn1 || OD.PowerMaganerState == PM_PowerOn2)
		{
			ECU_GoToPowerSupply();
			OD.SB.PowerOn = 1;
		}
		else if(OD.PowerMaganerState == PM_ShutDown)		
			SetWorkState(&OD.StateMachine, WORKSTATE_SHUTDOWN);		
	}
	
    if(GetTimeFrom(OD.LogicTimers.Timer_100ms) >= OD.DelayValues.Time100_ms)
    {
        OD.LogicTimers.Timer_100ms = GetTimeStamp();
		
		// Функции мастера
		OD.MasterControl.TargetVoltage_mV = TargetVoltageCulc(OD.MasterData.MinCellVoltage.Voltage_mv, OD.ModuleData[OD.ConfigData->ModuleIndex].MinCellVoltage.Voltage_mv);
        OD.MasterControl.BalancingEnabled = GetBalancingPermission(OD.ModuleData[OD.ConfigData->ModuleIndex].TotalCurrent);
		GetCurrentLimit(&OD.MasterControl.DCL, &OD.MasterControl.CCL);
		
		vs_ban_balancing(!OD.MasterControl.BalancingEnabled);
		vs_set_min_dis_chars(OD.MasterControl.TargetVoltage_mV);
			
		LedStatus(FB_PLUS & FB_MINUS, OD.ModuleData[OD.ConfigData->ModuleIndex].DischargingCellsFlag);

		OD.SB.MsgFromSystem = (OD.ConfigData->IsAutonomic)? 1 : 0;
    }
    
    if(GetTimeFrom(OD.LogicTimers.Timer_1s) >= OD.DelayValues.Time1_s)
    {
        OD.LogicTimers.Timer_1s = GetTimeStamp();
		OD.AfterResetTime++;
		
		// Code    
		if (secondsToday >= SECONDS_IN_DAY) // 86400 секунд в одном дне
		{
			secondsToday = 0;
			NewDay();
		}
        
        
		uint16_t discharge_mask1 = ltc6803_GetDischargingMask(0);
		uint16_t discharge_mask2 = ltc6803_GetDischargingMask(1);		
        OD.ModuleData[OD.ConfigData->ModuleIndex].DischargingCellsFlag = discharge_mask1 + ((uint32_t)discharge_mask2 << 12);
		OD.ModuleData[OD.ConfigData->ModuleIndex].DischargeEnergy_Ah = OD.Energy_As / 3600;
    }
}




