#include "Main.h"
#include "User.h"
#include "../BoardDefinitions/MarineEcu_Board.h"
#include "../MolniaLib/Config.h"
#include "../MolniaLib/PowerManager.h"

#include "FaultTools.h"
#include "Protocol.h"

#include "TimerFunc.h"
#include "PwmFunc.h"
#include "AdcFunc.h"
#include "SpiFunc.h"

#include "../Libs/max11612.h"
#include "../Libs/Btn8982.h"
#include "../Libs/TLE_6368g2.h"

#include "WorkStates.h"
#include "dVCU_ECU.h"

uint32_t timeStamp;

// Массив указателей на функции-режимы
void (*FunArray[])(uint8_t *) =	{ InitializationState, OperatingState, ShutdownState, FaultState, ChargingState };



void SetWorkState(StateMachine_t *state_machine, WorkStates_e new_state)
{
    if(state_machine->MainState == WORKSTATE_SHUTDOWN)
		return;

	state_machine->MainState = new_state;
	state_machine->SubState = 0;
	Algorithm = FunArray[new_state];
}


void InitializationState(uint8_t *SubState)
{
	EcuConfig_t _config = GetConfigInstance();
    switch(*SubState)
    {
        case 0:
			OD.SB.CheckFaults = 0;
			// Инициализация устройства питания
			TLE_GoToPowerSupply();
		
			if(ReadFaults())
				FillFaultsList(OD.OldFaultList, &OD.OldFaultsNumber, 0);

			*SubState = 1;
            break;
		case 1:
			if(OD.SB.PowerOn == 1)
			{
				*SubState = 2;
				timeStamp = GetTimeStamp();
			}
			
		break;
        case 2:
			*SubState = 3;
		break;
			
		case 3:
			OD.SB.CheckFaults = 1;
			SetWorkState(&OD.StateMachine, WORKSTATE_OPERATE);		
		break;
    }
    
    
}

void OperatingState(uint8_t *SubState)
{    
}


void ShutdownState(uint8_t *SubState)
{
	EcuConfig_t _config = GetConfigInstance();
	switch (*SubState)
	{
		// Запомнить время входа в режим, сохранить данные
		case 0:
			OD.LogicTimers.PowerOffTimer_ms = GetTimeStamp();
			SaveFaults();
			*SubState = 1;
		break;
		
		case 1:

		break;
		
		case 2:
			TLE_GoToSleep();
			break;
	}

	if((GetTimeFrom(OD.LogicTimers.PowerOffTimer_ms) >= _config.PowerOffDelay_ms) && (OD.LocalPMState == PM_ShutDown))
		*SubState = 2;
}

void FaultState(uint8_t *SubState)
{
}

void ChargingState(uint8_t *SubState)
{
    
}

void CommonState(void)
{    
	EcuConfig_t ecuConfig = GetConfigInstance();

    Protocol();
    
    if(GetTimeFrom(OD.LogicTimers.Timer_1ms) >= OD.DelayValues.Time1_ms)
    {        
        OD.LogicTimers.Timer_1ms = GetTimeStamp();
		
        // Code   
        ecuProc();
		OD.ecuPowerSupply_0p1 = EcuGetVoltage();
		// Power Manager thread
		PM_Proc(OD.ecuPowerSupply_0p1, 0);

		OD.Out_CSens[0].Out_CSens_Current = btnGetCurrent(0);
		OD.Out_CSens[1].Out_CSens_Current = btnGetCurrent(1);
		OD.Out_CSens[2].Out_CSens_Current = btnGetCurrent(2);
		
		FaultsTest(OD.SB.CheckFaults);
    }
    if(GetTimeFrom(OD.LogicTimers.Timer_10ms) >= OD.DelayValues.Time10_ms)
	{    	
    	OD.LogicTimers.Timer_10ms = GetTimeStamp();

		btnProc();
		TLE_Proc();

		OD.LocalPMState = PM_GetPowerState();

		if(OD.LocalPMState == PM_PowerOn1)
		{
			OD.SB.PowerOn = 1;
		}
		if(OD.LocalPMState == PM_ShutDown || OD.PowerManagerCmd == PM_ShutDown)
			SetWorkState(&OD.StateMachine, WORKSTATE_SHUTDOWN);

	}
    if(GetTimeFrom(OD.LogicTimers.Timer_100ms) >= OD.DelayValues.Time100_ms)
    {       
        OD.LogicTimers.Timer_100ms = GetTimeStamp();
		
		OD.A_Out[0] = btnGetOutputLevel(0);
		OD.A_Out[1] = btnGetOutputLevel(1);
		OD.A_Out[2] = btnGetOutputLevel(2);
		OD.A_Out[3] = btnGetOutputLevel(3);
		
		DisplaySoc(OD.BmuData1.SOC);
		DisplayMotorRpm(OD.MainEcuData1.MotorRpm);
    }
    
    if(GetTimeFrom(OD.LogicTimers.Timer_1s) >= OD.DelayValues.Time1_s)
    {
        OD.LogicTimers.Timer_1s = GetTimeStamp();
        
        // Code
        
    }
}






