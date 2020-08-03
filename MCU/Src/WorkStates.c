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

// Массив указателей на функции-режимы
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
			obcInit(OD.cfgEcu->ChargersNumber, OD.cfgEcu->MaxChargingCurrent_A, 400);

			// Init accelerator parameters
			thrHandleControlInit(OD.cfgEcu->AccPedalFstCh_0V, OD.cfgEcu->AccPedalFstCh_MaxV, OD.cfgEcu->AccPedalTable,
					&OD.AccPedalChannels[0], &OD.AccPedalChannels[1]);

			// Init trim parameters
			TrimInit(&OD.TrimDataRx, &OD.A_CH_Voltage_0p1[0], OD.cfgEcu->TrimMaxVal_0p1V, OD.cfgEcu->TrimMinVal_0p1V, OD.cfgEcu->DriveUpperLimitFB_0p1V);

			// Init motor control unit
			McuRinehartInit(&OD.mcHandler, OD.cfgEcu->RateMotorTorque, 0);

			// Init helm
			helmInit(OD.cfgEcu->SteeringBrakeSpeedTable, NELEMENTS(OD.cfgEcu->SteeringBrakeSpeedTable));

			transmissionInit(1000, TransmissionSetGear);

			if(ReadFaults(dtcList, dtcListSize))
							OD.OldFaultsNumber = FillFaultsList(dtcList, dtcListSize, OD.OldFaultList, 0);

			flashReadSData(&OD.SData);

			OD.SB.PowerOff_SaveParams = 1;

			*SubState = 1;
		}
            break;
		// Waiting for power supply
        case 1:
        	if(OD.SB.PowerOn == 1)
			{				
				timeStamp = GetTimeStamp();				
				OD.SB.CheckFaults = 1;

				// If flash data isn't actual
				if(OD.SData.Result != 0)
					OD.SB.PowerOff_SaveParams = 0;

				*SubState = 2;
			}

            break;		
			
		// Waiting for battery and helm init 
		case 2:			
			if((HelmGetStatus() != mcu_Disabled) && OD.SB.BatteryIsOperate)
			{				
				McuRinehartSetState(&OD.mcHandler, stateCmd_Enable);
				InverterPower(1);					
				
				*SubState = 3;
			}
		break;
		
		// Steering control channels Init
        case 3:
            if(btnCalibrate(0) && btnCalibrate(1) && btnCalibrate(2))
            {                
				PID_Struct_t steeringPID;
				SteeringDependencies_t steeringDependency;

				steeringPID.Kp = OD.cfgEcu->SteeringKp;
				steeringPID.Ki = OD.cfgEcu->SteeringKi;
				steeringPID.Kd = OD.cfgEcu->SteeringKd;
				
				steeringDependency.PositionVoltageFB_0p1V = &OD.A_CH_Voltage_0p1[1];
				steeringDependency.fstChannelCurrentFB_0p1A = &OD.PwmLoadCurrent[0];
				steeringDependency.sndChannelCurrentFB_0p1A =  &OD.PwmLoadCurrent[1];
				steeringDependency.PumpControlFunc = SteeringPumpPower;

				// Init steering parameters
				SteeringInit(&OD.SteeringData, &steeringPID, &steeringDependency,
						OD.cfgEcu->SteeringMinVal_0p1V, OD.cfgEcu->SteeringMaxVal_0p1V, OD.cfgEcu->SteeringMaxCurrent_0p1A);
				
				//SteeringPumpPower(1);
				
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

			switch(thrHandleGetDirection())
			{
				case dr_NeuDirect: {
					TargetTorque = 0;
					transmissionSetGear(GEAR_NEU);
					// Go to charge substate
					if(OD.SB.stChargingTerminal)
						*SubState = 1;

					if((OD.SB.BoostModeRequest && !OD.SB.stBoostMode) &&
							((McuRinegartGetParameter(&OD.mcHandler, mcu_MotorTemperature) < OD.cfgEcu->MaxMotorT - 10) ||
							(McuRinegartGetParameter(&OD.mcHandler, mcu_MotorTemperature) < OD.cfgEcu->MotorCoolingOn)))
					{
						OD.SB.stBoostMode = 1;
						McuRinehartInit(&OD.mcHandler, OD.cfgEcu->MaxMotorTorque, 0);
						McuRinehartSetState(&OD.mcHandler, stateCmd_Enable);
					}
					else if((!OD.SB.BoostModeRequest && OD.SB.stBoostMode) ||
							(McuRinegartGetParameter(&OD.mcHandler, mcu_MotorTemperature) > OD.cfgEcu->MaxMotorT - 10))
					{
						OD.SB.stBoostMode = 0;
						McuRinehartInit(&OD.mcHandler, OD.cfgEcu->RateMotorTorque, 0);
						McuRinehartSetState(&OD.mcHandler, stateCmd_Enable);
					}
				}
					break;
				case dr_FwDirect: {
					TargetTorque = thrHandleGetDemandAcceleration();
					McuRinehartSetMaxSpeed(&OD.mcHandler, OD.cfgEcu->MaxMotorSpeedD);
					transmissionSetGear(GEAR_FW);
				}
					break;
				case dr_BwDirect: {
					// Max R direct speed,
					// TODO: add to cfg
					TargetTorque = thrHandleGetDemandAcceleration();
					McuRinehartSetMaxSpeed(&OD.mcHandler, 1500);
					transmissionSetGear(GEAR_BW);
				}
					break;
			}

			if((OD.SB.stBoostMode) ||
					(McuRinegartGetParameter(&OD.mcHandler, mcu_MotorTemperature) > OD.cfgEcu->MaxMotorT - 10))
			{
				OD.SB.stBoostMode = 0;
				McuRinehartInit(&OD.mcHandler, OD.cfgEcu->RateMotorTorque, 0);
				McuRinehartSetState(&OD.mcHandler, stateCmd_Enable);
			}


			if((TrimGetParameter(&OD.TrimDataRx, paramTrim_Status) == stTrim_Warning))
				TargetTorque = 0;
			else if(transmissionMovementPermission() || transmissionShiftInProcess())
				TargetTorque = 2;


			McuRinehartSetDirection(&OD.mcHandler, mcu_DirFW);
			McuRinehartSetCmd(&OD.mcHandler, TargetTorque, OD.BatteryDataRx.CCL, OD.BatteryDataRx.DCL);
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
						obcSetCurrentLimit(10);
					}
						break;

					case st_OBC_ENABLE:
					case st_OBC_FAULT:
					{
						uint16_t current_val = (OD.BatteryDataRx.CCL < 0)? -OD.BatteryDataRx.CCL : OD.BatteryDataRx.CCL;
						uint16_t driver_current_limit = ((50 - OD.MaxChargingCurrentRequest) * OD.cfgEcu->MaxChargingCurrent_A / 50) + 15;

						if(driver_current_limit < current_val)
							obcSetCurrentLimit(driver_current_limit);
						else
							obcSetCurrentLimit(current_val);
					}
						break;

					case st_OBC_STOP:
					{
						obcSetCurrentLimit(0);
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
}


void ShutdownState(uint8_t *SubState)
{	
	McuRinehartSetCmd(&OD.mcHandler, 0, 0, 0);

	switch (*SubState)
	{
		// Запомнить время входа в режим, сохранить данные
		case 0:
			OD.LogicTimers.PowerOffTimer_ms = GetTimeStamp();
			OD.SData.Buf_1st.SystemTime = OD.SystemTime;
			OD.SData.Buf_1st.NormalPowerOff = 1;

			flashStoreData(&OD.SData);
			*SubState = 1;
		break;
		
		// Idle
		case 1:
			InverterPower(0);
			//SteeringPumpPower(0);
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
	//SteeringPumpPower(0);

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
		transmissionSetEndSwithPosition(D_IN_ET_ACT_POS);

		btnProc();
		TLE_Proc();
		PM_Proc(OD.ecuPowerSupply_0p1, ecuConfig->IsPowerManager);
		Max11612_StartConversion();

		OD.LocalPMState = (OD.cfgEcu->IsPowerManager)? PM_GetPowerState() : OD.PowerManagerCmd;

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
		
		// Code
		OD.PwmTargetLevel[0] = btnGetOutputLevel(0);
		OD.PwmTargetLevel[1] = btnGetOutputLevel(1);
		OD.PwmTargetLevel[2] = btnGetOutputLevel(2);
		OD.PwmTargetLevel[3] = btnGetOutputLevel(3);
		
		TrimSetCmd(&OD.TrimDataRx, OD.SB.cmdTrimUp, OD.SB.cmdTrimDown);
    }
    
    if(GetTimeFrom(OD.LogicTimers.Timer_1s) >= OD.DelayValues.Time1_s)
    {
        OD.LogicTimers.Timer_1s = GetTimeStamp();

        if(OD.SData.DataChanged && OD.SB.PowerOn)
        	flashStoreData(&OD.SData);
        
		LedBlink();
        OD.SystemTime = dateTime_GetCurrentTotalSeconds();
        OD.SB.cmdDrainPumpOn = DrainSupply();								// Drain pump
		OD.SB.InverterIsOperate = (McuRinehartGetState(&OD.mcHandler) == mcu_Enabled || McuRinehartGetState(&OD.mcHandler) == mcu_Warning);
		OD.BatteryReqState = (OD.FaultsBits.InverterFault || OD.FaultsBits.InverterTimeout || OD.Sim100DataRx.Isolation < 500)? 0 : 1;
		OD.SB.BatteryIsOperate = (OD.BatteryDataRx.MainState == 2);
    }
}


