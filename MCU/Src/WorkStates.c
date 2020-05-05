/*
Acc Pedal Channels	- from MUX1.  AN[0], AN[1].

Steering Feedback	- from MCU AN[1]
Steering Drive+		- to MCU PWM[0]
Steering Drive-		- to MCU PWM[1]
Steering Pump		- to MCU PWM[2]

Trim Joy+			- from MUX2 DIN[0]
Trim Joy-			- from MUX2 DIN[1]

Trim Drive+/-		- to MCU DOUT[5]
Trim Drive Cmd		- to MCU DOUT[4]
*/


#include "Main.h"
#include "WorkStates.h"
#include "User.h"
#include "PwmFunc.h"
#include "AdcFunc.h"
#include "TimerFunc.h"
#include "UartFunc.h"

#include "Protocol.h"
#include "ExternalProtocol.h"

#include "../Libs/max11612.h"
#include "../Libs/Btn8982.h"
#include "../Libs/TLE_6368g2.h"

#include "../MolniaLib/MF_Tools.h"
#include "../MolniaLib/DateTime.h"

#include "mVCU_ECU.h"

uint32_t timeStamp;

// ������ ���������� �� �������-������
void (*FunArray[])(uint8_t *) =	{ InitializationState, OperatingState, ShutdownState, FaultState };


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
		{
			OD.SB.CheckFaults = 0;
			
			// Init power source
			TLE_GoToPowerSupply();

			// Init Obc driver
			obcInit(OD.cfgEcu->ChargersNumber, OD.cfgEcu->MaxChargingCurrent_A, 115);

			// Init accelerator parameters
			driveAccelerationUnitInit(OD.cfgEcu->AccPedalFstCh_MaxV, OD.cfgEcu->AccPedalFstCh_0V, OD.cfgEcu->AccPedalSndCh_MaxV,
					&OD.AccPedalChannels[0], &OD.AccPedalChannels[1]);

			// Init trim parameters
			TrimInit(&OD.TrimDataRx, &OD.A_CH_Voltage_0p1[0], OD.cfgEcu->TrimMaxVal_0p1V, OD.cfgEcu->TrimMinVal_0p1V);

			// Init motor control unit
			McuRinehartInit(&OD.mcHandler, OD.MaxMotorTorque);

			// Init helm
			helmInit(50, OD.cfgEcu->SteeringBrakeSpeedTable, sizeof(OD.cfgEcu->SteeringBrakeSpeedTable));

			if(ReadFaults())
				FillFaultsList(OD.OldFaultList, &OD.OldFaultsNumber, 0);

			*SubState = 1;
		}
            break;
		// Waiting for power supply
        case 1:
        	if(OD.SB.PowerOn == 1)
			{
				*SubState = 2;
				timeStamp = GetTimeStamp();
			}

            break;		
			
		// Waiting for battery and helm init 
		case 2:
			if(OD.SB.BatteryIsOperate && (HelmGetStatus() != mcu_Disabled))
			{				
				McuRinehartSetState(&OD.mcHandler, stateCmd_Enable);
				InverterPower(1);							
				
				OD.SB.CheckFaults = 1;
				*SubState = 3;
			}
		break;
		
		// Steering control channels Init
        case 3:
            if(btnCalibrate(0) && btnCalibrate(1) && btnCalibrate(2))
            {                
				PID_Struct_t steeringPID;
				steeringPID.Kp = OD.cfgEcu->SteeringKp;
				steeringPID.Ki = OD.cfgEcu->SteeringKi;
				steeringPID.Kd = OD.cfgEcu->SteeringKd;
				
				// Init steering parameters
				SteeringInit(&OD.SteeringData, &steeringPID,
						&OD.A_CH_Voltage_0p1[1], OD.cfgEcu->SteeringMinVal_0p1V, OD.cfgEcu->SteeringMaxVal_0p1V,
						&OD.PwmLoadCurrent[0], &OD.PwmLoadCurrent[1], OD.cfgEcu->SteeringMaxCurrent_0p1A);
				
				SteeringPumpPower(1);	
				
				OD.BatteryReqState = 1;				
                *SubState = 4;
            }
            break;
            
        // Waiting for inverter init
        case 4:
            if(OD.SB.InverterIsOperate)
            {
				SetWorkState(&OD.StateMachine, WORKSTATE_OPERATE);
            }
            break;
    }

    McuRinehartSetCmd(&OD.mcHandler, 0, 0, 0);
}

void OperatingState(uint8_t *SubState)
{    
	int16_t TargetTorque = 0;	

	switch(*SubState)
	{
		// Drive case
		case 0:
		{
			steeringSetAngle(&OD.SteeringData, HelmGetTargetAngle());

			switch(driveGetDirection())
			{
				case dr_NeuDirect: {
					TargetTorque = 0;

					// Go to charge substate
					if(OD.SB.stChargingTerminal)
						*SubState = 1;
				}
					break;
				case dr_FwDirect: {
					TargetTorque = driveGetDemandAcceleration();
					McuRinehartSetMaxSpeed(&OD.mcHandler, OD.cfgEcu->MaxMotorSpeedD);
					McuRinehartSetDirection(&OD.mcHandler, mcu_DirFW);
				}
					break;
				case dr_BwDirect: {
					// Max R direct speed,
					// TODO: add to cfg
					TargetTorque = -driveGetDemandAcceleration();
					McuRinehartSetMaxSpeed(&OD.mcHandler, 1500);
					McuRinehartSetDirection(&OD.mcHandler, mcu_DirBW);
				}
					break;
			}
		}
		break;

		// Charging case
		case 1:
		{
			if(OD.SB.stChargingTerminal && OD.SB.BatteryIsOperate)
			{
				ChargersCircuitOn(1);
				McuRinehartSetState(&OD.mcHandler, stateCmd_Disable);

				switch(obcGetStatus())
				{
					case st_OBC_DISABLE:
					{
						obcSetState(1);
					}
						break;

					case st_OBC_ENABLE:
					case st_OBC_FAULT:
					{
						uint16_t current_val = (OD.BatteryDataRx.CCL < 0)? -OD.BatteryDataRx.CCL : OD.BatteryDataRx.CCL;
						obcSetCurrentLimit(current_val);
					}
						break;

					case st_OBC_STOP:
					{
						obcSetState(0);
					}
						break;
				}
			}
			else
			{
				obcSetState(0);
				if(!OD.SB.stChargingTerminal)
				{
					ChargersCircuitOn(0);
					McuRinehartSetState(&OD.mcHandler, stateCmd_Enable);
					*SubState = 0;			// Go to drive substate
				}
			}
		}
		break;
	}

	McuRinehartSetCmd(&OD.mcHandler, TargetTorque, OD.BatteryDataRx.CCL, OD.BatteryDataRx.DCL);
}


void ShutdownState(uint8_t *SubState)
{	
	McuRinehartSetCmd(&OD.mcHandler, 0, 0, 0);

	switch (*SubState)
	{
		// ��������� ����� ����� � �����, ��������� ������
		case 0:
			OD.LogicTimers.PowerOffTimer_ms = GetTimeStamp();
			SaveFaults();
			*SubState = 1;
		break;
		
		// Idle
		case 1:
			InverterPower(0);
			SteeringPumpPower(0);
		break;
		
		case 2:
			TLE_GoToSleep();
			break;
	}
	
	if((GetTimeFrom(OD.LogicTimers.PowerOffTimer_ms) >= OD.cfgEcu->PowerOffDelay_ms) && (OD.LocalPMState == PM_ShutDown))
		*SubState = 2;
}

void FaultState(uint8_t *SubState)
{
	InverterPower(0);
	SteeringPumpPower(0);

	McuRinehartSetCmd(&OD.mcHandler, 0, 0, 0);
	// faults handle
	
	if(OD.FaultsBits.Accelerator)
	{
		
	}

	if(OD.FaultsBits.SteeringTimeout)
	{
		
	}
	
	if(OD.FaultsBits.InverterTimeout)
	{
		OD.BatteryReqState = 0;
	}
}

void CommonState(void)
{    
	const EcuConfig_t *ecuConfig = OD.cfgEcu;

    Protocol();
    
    if(GetTimeFrom(OD.LogicTimers.Timer_1ms) >= OD.DelayValues.Time1_ms)
    {
        OD.LogicTimers.Timer_1ms = GetTimeStamp();

        // Code      	
        boardThread();

		OD.PwmLoadCurrent[0] = btnGetCurrent(0);
		OD.PwmLoadCurrent[1] = btnGetCurrent(1);
		OD.PwmLoadCurrent[2] = btnGetCurrent(2);

		if(FaultsTest(OD.SB.CheckFaults))
			SetWorkState(&OD.StateMachine, WORKSTATE_FAULT);
    }
    
	if(GetTimeFrom(OD.LogicTimers.Timer_10ms) >= OD.DelayValues.Time10_ms)
	{
		OD.LogicTimers.Timer_10ms = GetTimeStamp();
		
		OD.ecuPowerSupply_0p1 = EcuGetVoltage();
		Max11612_GetResult(OD.A_CH_Voltage_0p1, V_AN);

		SystemThreat(&OD);

		btnProc();
		TLE_Proc();
		PM_Proc(OD.ecuPowerSupply_0p1, ecuConfig->IsPowerManager);
		Max11612_StartConversion();

		if((OD.LocalPMState = PM_GetPowerState()) == PM_PowerOn1)
			OD.SB.PowerOn = 1;

		if(OD.LocalPMState == PM_ShutDown || OD.PowerManagerCmd == PM_ShutDown)
			SetWorkState(&OD.StateMachine, WORKSTATE_SHUTDOWN);
	}
	
	
    if(GetTimeFrom(OD.LogicTimers.Timer_100ms) >= OD.DelayValues.Time100_ms)
    {
		OD.LogicTimers.Timer_100ms = GetTimeStamp();
		
		// Code
		OD.PwmTargetLevel[0] = btnGetOutputLevel(0);
		OD.PwmTargetLevel[1] = btnGetOutputLevel(1);
		OD.PwmTargetLevel[2] = btnGetOutputLevel(2);
		OD.PwmTargetLevel[3] = btnGetOutputLevel(3);
    }
    
    if(GetTimeFrom(OD.LogicTimers.Timer_1s) >= OD.DelayValues.Time1_s)
    {
        OD.LogicTimers.Timer_1s = GetTimeStamp();
		
        OD.SystemTime = dateTime_GetCurrentTotalSeconds();
        OD.SB.cmdDrainPumpOn = DrainSupply();								// Drain pump
		OD.SB.InverterIsOperate = (McuRinehartGetState(&OD.mcHandler) == mcu_Enabled || McuRinehartGetState(&OD.mcHandler) == mcu_Warning);
		OD.BatteryReqState = (OD.FaultsBits.InverterFault || OD.FaultsBits.InverterTimeout || OD.Sim100DataRx.Isolation < 500)? 0 : 1;
		OD.SB.BatteryIsOperate = (OD.BatteryDataRx.MainState == 2);
    }
}


