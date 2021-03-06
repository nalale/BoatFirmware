#include "Main.h"
#include "User.h"
#include "../BoardDefinitions/MarineEcu_Board.h"
#include "../MolniaLib/PowerManager.h"
#include "../MolniaLib/DateTime.h"

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

// ������ ���������� �� �������-������
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
    switch(*SubState)
    {
        case 0:
			OD.SB.CheckFaults = 0;
			// ������������� ���������� �������
			TLE_GoToPowerSupply();
		
			if(ReadFaults(dtcList, dtcListSize))
				OD.OldFaultsNumber = FillFaultsList(dtcList, dtcListSize, OD.OldFaultList, 0);

			flashReadSData(&OD.SData);

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
	const EcuConfig_t *_config = OD.cfg;
	switch (*SubState)
	{
		// ��������� ����� ����� � �����, ��������� ������
		case 0:
			OD.LogicTimers.PowerOffTimer_ms = GetTimeStamp();
			OD.SData.Buf_1st.SystemTime = OD.SystemTime;
			OD.SData.Buf_1st.NormalPowerOff = 1;

			flashStoreData(&OD.SData);
			*SubState = 1;
		break;
		
		case 1:

		break;
		
		case 2:
			TLE_GoToSleep();
			break;
	}

	if((GetTimeFrom(OD.LogicTimers.PowerOffTimer_ms) >= _config->PowerOffDelay_ms) && (OD.LocalPMState == PM_ShutDown))
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
    Protocol();
    
    if(GetTimeFrom(OD.LogicTimers.Timer_1ms) >= OD.DelayValues.Time1_ms)
    {        
        OD.LogicTimers.Timer_1ms = GetTimeStamp();		
		
        // Code   
        boardMarineECU_Thread();
		OD.ecuPowerSupply_0p1 = boardMarineECU_GetVoltage();
		// Power Manager thread
		PM_Proc(OD.ecuPowerSupply_0p1, OD.cfg->IsPowerManager);

		Max11612_GetResult(OD.A_CH_Voltage_0p1, V_AN);
		
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
		Max11612_StartConversion();
		
		OD.LocalPMState = (OD.cfg->IsPowerManager)? PM_GetPowerState() : OD.PowerManagerCmd;

		if(OD.LocalPMState == PM_PowerOn1)
			OD.SB.PowerOn = 1;
		else if((OD.LocalPMState == PM_ShutDown) &&
						(OD.SB.PowerOn == 1 && OD.StateMachine.MainState != WORKSTATE_SHUTDOWN))
		{
			OD.SB.PowerOn = 0;
			SetWorkState(&OD.StateMachine, WORKSTATE_SHUTDOWN);
		}
	}
    if(GetTimeFrom(OD.LogicTimers.Timer_100ms) >= OD.DelayValues.Time100_ms)
    {       
        OD.LogicTimers.Timer_100ms = GetTimeStamp();
		
		OD.A_Out[0] = btnGetOutputLevel(0);
		OD.A_Out[1] = btnGetOutputLevel(1);
		OD.A_Out[2] = btnGetOutputLevel(2);
		OD.A_Out[3] = btnGetOutputLevel(3);
		
		DisplaySoc(OD.BmuData1.SOC);
		DisplayTrim(OD.MainEcuData1.TrimPosition);
		DisplayEnergy(OD.MainEcuData1.SpecPowerCons);
		DisplayMotorRpm(OD.MainEcuData1.MotorRpm);
    }
    
    if(GetTimeFrom(OD.LogicTimers.Timer_1s) >= OD.DelayValues.Time1_s)
    {
        OD.LogicTimers.Timer_1s = GetTimeStamp();
        OD.IO = boardMarineECU_GetDiscreteIO();
        OD.SystemTime = dateTime_GetCurrentTotalSeconds();

        if(OD.SData.DataChanged && OD.SB.PowerOn)
        	flashStoreData(&OD.SData);

        // Code
        FaultHandler();
    }
}






