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
#include "SteeringFunc.h"


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

#include "mVCU_ECU.h"

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
	EcuConfig_t _cfgEcu = GetConfigInstance();
    switch(*SubState)
    {
		case 0:	
			OD.SB.CheckFaults = 0;
			OD.TractionData.Direction = 0;

			// Инициализация параметров ручки акселератора
			AccPedalInit(_cfgEcu.AccPedalFstCh_MaxV, _cfgEcu.AccPedalFstCh_0V);
			// Инициализация параметров трим
			TrimInit(&OD.TrimDataRx, &OD.A_CH_Voltage_0p1[0], _cfgEcu.TrimMaxVal_0p1V, _cfgEcu.TrimMinVal_0p1V);
			// Инициализация параметров рулевой рейки
			SteeringInit(&OD.SteeringData, &OD.A_CH_Voltage_0p1[1], _cfgEcu.SteeringMinVal_0p1V, _cfgEcu.SteeringMaxVal_0p1V, &OD.PwmLoadCurrent[0], &OD.PwmLoadCurrent[1], _cfgEcu.SteeringMaxCurrent_0p1A);

			if(ReadFaults())
				FillFaultsList(OD.OldFaultList, &OD.OldFaultsNumber, 0);

			*SubState = 1;
            break;
		// Waiting power supply
        case 1:
        	if(OD.SB.PowerOn == 1)
			{
        		timeStamp = GetTimeStamp();
				*SubState = 2;
			}

            break;		
		
		// Steering control channels Init
        case 2:
            if(btnCalibrate(0) && btnCalibrate(1) && btnCalibrate(2))
            {                
				timeStamp = GetTimeStamp();
				
				OD.BatteryReqState = 1;				
                *SubState = 3;
            }
            break;
        // Waiting battery init
		case 3:
			if(OD.SB.BatteryIsOperate)
			{
				OD.SB.cmdInverterPowerSupply = 1;
				OD.SB.cmdSteeringPump = 1;
				OD.SB.CheckFaults = 1;
				*SubState = 4;
			}
		break;
            
        // Waiting inverter init
        case 4:
            if(OD.SB.InverterIsOperate)
            {
				SetWorkState(&OD.StateMachine, WORKSTATE_OPERATE);
            }
            break;
    }
    
    
}

void OperatingState(uint8_t *SubState)
{    
    OD.TractionData.TargetTorque = GetTargetTorque(OD.MovControlDataRx.AccPosition, OD.MovControlDataRx.Gear);

	//GetTargetSpeed(OD.MovControlDataRx.AccPosition, OD.MovControlDataRx.Gear);
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
		
		// Idle
		case 1:
			OD.SB.cmdInverterPowerSupply = 0;
			OD.SB.cmdSteeringPump = 0;
		break;
		
		case 2:	
			
			TLE_GoToSleep();
			break;
	}
	
	if(GetTimeFrom(OD.LogicTimers.PowerOffTimer_ms) >= _config.PowerOffDelay_ms)
		*SubState = 2;
}

void FaultState(uint8_t *SubState)
{
	// faults handle
	
	if(OD.FaultsBits.Accelerator)
	{
		// 1. Обнуляем запрашиваемый момент
		// 2. Переводим инвертор в нерабочий режим
		OD.TractionData.TargetTorque = 0;
	}

	if(OD.FaultsBits.SteeringTimeout)
	{
		
	}
	
	if(OD.FaultsBits.InverterTimeout)
	{
		OD.BatteryReqState = 0;
	}
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
		PM_Proc(OD.ecuPowerSupply_0p1, ecuConfig.IsPowerManager);		
		
		Max11612_GetResult(OD.A_CH_Voltage_0p1, V_AN);	
		
		OD.PwmLoadCurrent[0] = btnGetCurrent(0);
		OD.PwmLoadCurrent[1] = btnGetCurrent(1);
		OD.PwmLoadCurrent[2] = btnGetCurrent(2);

		if(FaultsTest(OD.SB.CheckFaults))
			SetWorkState(&OD.StateMachine, WORKSTATE_FAULT);
    }
    
	if(GetTimeFrom(OD.LogicTimers.Timer_10ms) >= OD.DelayValues.Time10_ms)
	{
		OD.LogicTimers.Timer_10ms = GetTimeStamp();
		
		OD.PowerMaganerState = PM_GetPowerState();

		if(OD.PowerMaganerState == PM_PowerOn1 || OD.PowerMaganerState == PM_PowerOn2)
		{
			TLE_GoToPowerSupply();
			OD.SB.PowerOn = 1;
		}
		else if(OD.PowerMaganerState == PM_ShutDown)
			SetWorkState(&OD.StateMachine, WORKSTATE_SHUTDOWN);
		
		SystemThreat();

		Max11612_StartConversion();
		btnProc();
		TLE_Proc();
		
		OD.MovControlDataRx.Gear = GetDriveDirection(OD.AccPedalChannels[0], OD.AccPedalChannels[1]);
		OD.MovControlDataRx.AccPosition = GetAccelerationPosition(OD.AccPedalChannels[0], OD.AccPedalChannels[1]);
	}
	
	
    if(GetTimeFrom(OD.LogicTimers.Timer_100ms) >= OD.DelayValues.Time100_ms)
    {
		OD.LogicTimers.Timer_100ms = GetTimeStamp();
		
		EcuConfig_t cfgEcu = GetConfigInstance();
		
		// Code
		OD.PwmTargetLevel[0] = btnGetOutputLevel(0);
		OD.PwmTargetLevel[1] = btnGetOutputLevel(1);
		OD.PwmTargetLevel[2] = btnGetOutputLevel(2);
		OD.PwmTargetLevel[3] = btnGetOutputLevel(3);
		
//		OD.SteeringData.TargetSteeringBrake = interpol(cfgEcu.SteeringBrakeSpeedTable, 6, OD.MovControlDataRx.ActualSpeed);
		OD.steeringFeedbackAngle = SteeringGetFeedBackAngle(&OD.SteeringData);
		
		OD.SB.BatteryIsOperate = (OD.BatteryDataRx.MainState == 2) && (OD.BatteryDataRx.SubState == 3);		
		OD.SB.InverterIsOperate = OD.InvertorDataRx.InverterIsEnable;
    }
    
    if(GetTimeFrom(OD.LogicTimers.Timer_1s) >= OD.DelayValues.Time1_s)
    {
        OD.LogicTimers.Timer_1s = GetTimeStamp();
		
		OD.TargetChargingCurrent_A = CheckChargingCond(1/*OD.SB.ChargingTerminalConnected*/, OD.SB.BatteryIsOperate);
		
    }
}


