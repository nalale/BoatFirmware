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
    int tmp;
	
	EcuConfig_t ecuConfig = GetConfigInstance();
    
    switch(*SubState)
    {
		// Read saved faults
    	case 0:
			OD.SB.CheckFaults = 0;          // запрещаем проверку ошибок
			OD.SB.CurrentSensorReady = 0;	// датчик тока не откалиброван
            OD.SB.FaultStateHandled = 1;	// все ошибки обработаны
            
			OD.MasterControl.CCL = 0;
			OD.MasterControl.DCL = 0;
		
			if(ReadFaults())
				FillFaultsList(OD.OldFaultList, &OD.OldFaultsNumber, 0);
			
			ControlBatteriesState(&OD.MasterControl.RequestState, WORKSTATE_INIT);
		
    		*SubState = 1;
    	break;
			
		// Waiting for power on
        case 1:
            BAT_PLUS(BAT_OPEN);
            BAT_MINUS(BAT_OPEN);
            PRECHARGE(BAT_OPEN);                
            
            if(OD.SB.PowerOn == 1)
			{				
				timeStamp = GetTimeStamp();
				*SubState = 2;
			}
            break;
        
		// Current sensor calibration
        case 2:
		{
        	static uint8_t cnt = 0;

            csCalibrateCurrentSensor();
		
            // Welded contactor handle
            if(GetTimeFrom(timeStamp) >= 100 * cnt)
            {
            	if(!(cnt & 0x01))
            	{
					if((FB_PLUS && !GET_BAT_PLUS))
					{
						BAT_PLUS(BAT_CLOSE);
					}
            	}
            	else
            		BAT_PLUS(BAT_OPEN);

            	if(!(cnt & 0x01))
				{
					if(FB_MINUS && !GET_BAT_MINUS)
					{
						BAT_MINUS(BAT_CLOSE);
					}
				}
            	else
            		BAT_MINUS(BAT_OPEN);

				cnt++;
            }
            
            if(GetTimeFrom(timeStamp) >= 1000)
            {                
				BAT_PLUS(BAT_OPEN);
				BAT_MINUS(BAT_OPEN);				               
                *SubState = 3;
            }
		}
            break;
            
        case 3:
             OD.SB.CheckFaults = 1;        
            
			// Модуль сразу переходит в рабочий режим и замыкает контакторы
			if(ecuConfig.ModuleIndex > 0)
				SetWorkState(&OD.StateMachine, WORKSTATE_OPERATE);
			else		
			{		
				OD.SB.CurrentSensorReady = 1;	
				
				if(ecuConfig.IsMaster)
					OD.Energy_As = GetEnergyFromMinUcell((int16_t*)ecuConfig.OCVpoint, OD.MasterData.MinCellVoltage.Voltage_mv, ecuConfig.ModuleCapacity * ecuConfig.Sys_ModulesCountP);
								
				*SubState = 4;
			}
            
            break;
            
        case 4:
			// Батареи ждут когда модули пройдут инициализацию и замкнут контакторы
			if(OD.BatteryData[ecuConfig.BatteryIndex].OnlineNumber == ecuConfig.Sys_ModulesCountS)
			{
				tmp = 1;
				for (uint8_t i = 1; i < ecuConfig.Sys_ModulesCountS; i++)
				{
					// Ждем когда все модули замкнут контакторы
					if (OD.ModuleData[i].StateMachine.MainState != WORKSTATE_OPERATE || 
						OD.ModuleData[i].StateMachine.SubState != 3)
						tmp = 0;
				}
				
				if(tmp == 1)
				{
					if(GetTimeFrom(timeStamp) > 2000)
					{
						timeStamp = GetTimeStamp();
						*SubState = 5;
					}
				}					
				else
					timeStamp = GetTimeStamp();
			}
			else
				timeStamp = GetTimeStamp();
            
            break;	

		case 5:
		{
			if(ecuConfig.IsMaster)
            {
                // Мастер проверяет что все батареи в сети
                if(OD.MasterData.OnlineNumber == ecuConfig.Sys_ModulesCountP)
                {
                    tmp = 1;
                    for (uint8_t i = 0; i < ecuConfig.Sys_ModulesCountP - 1; i++)
                    {
						if((OD.BatteryData[i].TotalVoltage < OD.BatteryData[i+1].TotalVoltage - ecuConfig.Sys_MaxVoltageDisbalanceP) || 
							(OD.BatteryData[i].TotalVoltage > OD.BatteryData[i+1].TotalVoltage + ecuConfig.Sys_MaxVoltageDisbalanceP))
							tmp = 0;                     
                    }
                    
                    // Ожидаем выполнения определенных условий: CCL/DCL рассчитаны, все модули доступны и т.п.		
                    if(tmp == 1)
					{
						if(GetTimeFrom(timeStamp) > 2000)
						{
							ControlBatteriesState(&OD.MasterControl.RequestState, WORKSTATE_PREOP);
						}
					}
                }
				else
					timeStamp = GetTimeStamp();
            }
		}
		break;		
    }       
}

void PreoperationState(uint8_t *SubState)
{ 
	BAT_MINUS(BAT_OPEN);
	BAT_PLUS(BAT_OPEN);
	PRECHARGE(BAT_OPEN);
	ControlBatteriesState(&OD.MasterControl.RequestState, WORKSTATE_OPERATE);	
}

void OperatingState(uint8_t *SubState)
{    
    static uint32_t ZeroPreChCurrentTimeStamp = 0;
	EcuConfig_t ecuConfig = GetConfigInstance();
            
    // Реализация предзаряда
	// Предусмотреть - если ток предзаряда будет пересекать PreZeroCurrent в обе стороны, то предзаряд может не закончиться никогда
	switch (*SubState)
	{
		case 0:			
			OD.LastPrechargeMaxCurrent_0p1A = INT16_MIN;
			OD.LastPrechargeDuration = 0;			
			
            *SubState = 1;
			break;
		case 1:
			// Замкнуть цепь предзаряда
			BAT_MINUS(BAT_CLOSE);
			PRECHARGE(BAT_CLOSE);
			OD.SB.PrechargeDone = 0;
			*SubState = 2;
			timeStamp = GetTimeStamp();
			ZeroPreChCurrentTimeStamp = GetTimeStamp();
			
			break;
		case 2:
			// Считаем максимальный ток предзаряда
			if(OD.LastPrechargeMaxCurrent_0p1A < OD.BatteryData[ecuConfig.ModuleIndex].TotalCurrent)
				OD.LastPrechargeMaxCurrent_0p1A = OD.BatteryData[ecuConfig.ModuleIndex].TotalCurrent;
            
			// Считаем длительность предзаряда
			OD.LastPrechargeDuration = (uint16_t)GetTimeFrom(timeStamp);
						
			// Если этап предзаряда не завершается фиксируем время
			if((OD.BatteryData[ecuConfig.BatteryIndex].TotalCurrent > ((uint16_t)ecuConfig.PreZeroCurrent * 10)) || 
				(OD.BatteryData[ecuConfig.BatteryIndex].TotalCurrent < -((int16_t)ecuConfig.PreZeroCurrent * 10)))
				ZeroPreChCurrentTimeStamp = GetTimeStamp();
			
			// Если этап предзаряда завершается PreZeroCurrentDuration мс, переходим в рабочий режим
			if (GetTimeFrom(ZeroPreChCurrentTimeStamp) > ecuConfig.PreZeroCurrentDuration)
			{
				*SubState = 3;				
			}
			break;
		case 3:						
			BAT_PLUS(BAT_CLOSE);
			PRECHARGE(BAT_OPEN);
			break;
	}
}


void ShutdownState(uint8_t *SubState)
{
	EcuConfig_t _config = GetConfigInstance();
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
			if(GetTimeFrom(OD.LogicTimers.PowerOffTimer_ms) >= _config.PowerOffDelay_ms)
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
	static uint32_t timeStamp = 0;
	
	switch(*SubState)
	{
		case 0:
			BAT_MINUS(BAT_OPEN);
			BAT_PLUS(BAT_OPEN);
			PRECHARGE(BAT_OPEN);
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
	EcuConfig_t ecuConfig = GetConfigInstance();
	
    Protocol();
	

	// Измеряем ток
	if(OD.SB.CurrentSensorReady)
	{
		OD.ModuleData[ecuConfig.ModuleIndex].TotalCurrent = (csGetAverageCurrent());
		OD.MasterData.SoC = CapacityCulc(OD.MasterData.TotalCurrent, &(OD.Energy_As), ecuConfig.ModuleCapacity * ecuConfig.Sys_ModulesCountP);
	}
	else
	{
		OD.ModuleData[ecuConfig.ModuleIndex].TotalCurrent = 0;
		OD.BatteryData[ecuConfig.BatteryIndex].SoC = 0;
		OD.MasterData.SoC = 0;
	}	
    
    if(GetTimeFrom(OD.LogicTimers.Timer_1ms) >= OD.DelayValues.Time1_ms)
    {        
        OD.LogicTimers.Timer_1ms = GetTimeStamp();
		
        // Code      
        OD.ecuPowerSupply_0p1 = EcuGetVoltage();
		OD.A_IN[0] = GetVoltageValue(4);
		OD.A_IN[1] = GetVoltageValue(5);		

		// Power Manager thread
		PM_Proc(OD.ecuPowerSupply_0p1, ecuConfig.IsPowerManager);		
		ecuProc();
		vs_thread(OD.CellVoltageArray_mV, OD.CellTemperatureArray);
		
		if(FaultsTest())
		{
			OD.SB.FaultStateHandled = 0;
			ControlBatteriesState(&OD.MasterControl.RequestState, WORKSTATE_FAULT);
			SetWorkState(&OD.StateMachine, WORKSTATE_FAULT);
		}
        
        // Переход в требуемое состояние для батареи и мастера
		if((ecuConfig.ModuleIndex == 0) && (OD.MasterControl.RequestState != OD.StateMachine.MainState && OD.StateMachine.MainState != WORKSTATE_FAULT))
		{			
			SetWorkState(&OD.StateMachine, OD.MasterControl.RequestState);
		}
    }
    
	if(GetTimeFrom(OD.LogicTimers.Timer_10ms) >= OD.DelayValues.Time10_ms)
	{
		OD.LogicTimers.Timer_10ms = GetTimeStamp();
		
		// модуль
        GetCellStatistics(&OD.ModuleData[ecuConfig.ModuleIndex]);
		// батарея
		GetBatteriesStatistic(&OD.BatteryData[ecuConfig.BatteryIndex], OD.ModuleData, ecuConfig.Sys_ModulesCountS, 1);
		// master		
		GetBatteriesStatistic(&OD.MasterData, OD.BatteryData, ecuConfig.Sys_ModulesCountP, 0);
		
		
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
		OD.MasterControl.TargetVoltage_mV = TargetVoltageCulc(OD.MasterData.MinCellVoltage.Voltage_mv, OD.ModuleData[ecuConfig.ModuleIndex].MinCellVoltage.Voltage_mv);
        OD.MasterControl.BalancingEnabled = GetBalancingPermission(OD.ModuleData[ecuConfig.ModuleIndex].TotalCurrent);
		GetCurrentLimit(&OD.MasterControl.DCL, &OD.MasterControl.CCL);
		
		vs_ban_balancing(!OD.MasterControl.BalancingEnabled);
		vs_set_min_dis_chars(OD.MasterControl.TargetVoltage_mV);
			
		LedStatus(FB_PLUS & FB_MINUS, OD.ModuleData[ecuConfig.ModuleIndex].DischargingCellsFlag);
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
        OD.ModuleData[ecuConfig.ModuleIndex].DischargingCellsFlag = discharge_mask1 + ((uint32_t)discharge_mask2 << 12);
		OD.ModuleData[ecuConfig.ModuleIndex].DischargeEnergy_Ah = OD.Energy_As / 3600;
		
		OD.DebugData.CanMod = (uint8_t)LPC_CAN1->MOD;
		OD.DebugData.CanGlobalStatus = (uint8_t)LPC_CAN1->GSR;
		OD.DebugData.CanInt = LPC_CAN1->ICR;
		OD.DebugData.StartSign = 's' | ('t' << 8) | ('o' << 16) | ('p' << 24);
		
		Uart_SendData(0, (uint8_t*)&OD.DebugData, sizeof(OD.DebugData));
    }
}




