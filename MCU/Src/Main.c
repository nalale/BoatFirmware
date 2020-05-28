#include "lpc17xx_pinsel.h"

#include "WorkStates.h"
#include "User.h"
#include "TimerFunc.h"

#include "Main.h"

void CommonState(void);

// �������� ������
void (*Algorithm)(uint8_t *);

// ������� ��������
ObjectDictionary_t OD;

int main(void)
{
    SystemInit();
    
    // ������������� �����������, ���������� ���������� �������
    AppInit(&OD);
    
    SET_D_OUT1_EN(1);
    SET_D_OUT2_EN(1);
    
    SetWorkState(&OD.StateMachine, WORKSTATE_INIT);

    while(1)
    {       
        CommonState();
        if(GetTimeFrom(OD.LogicTimers.MainLoopTimer_ms) > OD.DelayValues.MainLoopTime_ms)
        {
			OD.LogicTimers.MainLoopTimer_ms = GetTimeStamp();
            Algorithm(&OD.StateMachine.SubState);
            
        }		
    }

}

uint8_t GetDataByIndex(uint16_t Index, uint8_t subindex, uint8_t *Buf[])
{
	uint8_t _size = 0;
	static uint32_t _data = 0;
	
	switch(Index)
	{
	// Common Data
		case didEcuInfo:
			*Buf = (uint8_t*)(&OD.EcuInfo[subindex]);
			_size = (subindex > 3)? 0 : sizeof(OD.EcuInfo[0]);
		break;
		
		case didConfigStructIndex:
			*Buf = (uint8_t*)OD.cfgEcu;
			_size = (subindex > CONFIG_SIZE)? 0 : CONFIG_SIZE;
		break;

		case didDateTime:
			*Buf = (uint8_t*)&OD.SystemTime;
			_size = (subindex > 0)? 0 : sizeof(OD.SystemTime);
			break;
		
		case didEcuVoltage:
			*Buf = (uint8_t*)&OD.ecuPowerSupply_0p1;
			_size = (subindex > 0)? 0 : sizeof(OD.ecuPowerSupply_0p1);
		break;
		
		case didPowerManagmentState:
			*Buf = (uint8_t*)&OD.LocalPMState;
			_size = (subindex > 0)? 0 : sizeof(OD.LocalPMState);
		break;
		
		// Voltage Data
		case didVoltageSensor1:
			*Buf = (uint8_t*)&OD.A_CH_Voltage_0p1[0];
			_size = (subindex > 0)? 0 : sizeof(OD.A_CH_Voltage_0p1[0]);
		break;
		
		case didVoltageSensor2:
			*Buf = (uint8_t*)&OD.A_CH_Voltage_0p1[1];
			_size = (subindex > 0)? 0 : sizeof(OD.A_CH_Voltage_0p1[1]);
		break;
		
		case didVoltageSensor3:
			*Buf = (uint8_t*)&OD.A_CH_Voltage_0p1[2];
			_size = (subindex > 0)? 0 : sizeof(OD.A_CH_Voltage_0p1[2]);
		break;
		
		case didVoltageSensor4:
			*Buf = (uint8_t*)&OD.A_CH_Voltage_0p1[3];
			_size = (subindex > 0)? 0 : sizeof(OD.A_CH_Voltage_0p1[3]);
		break;
		
		case didCurrentSensor1:
			*Buf = (uint8_t*)&OD.PwmLoadCurrent[0];
			_size = (subindex > 0)? 0 : sizeof(OD.PwmLoadCurrent[0]);
		break;
		
		case didCurrentSensor2:
			*Buf = (uint8_t*)&OD.PwmLoadCurrent[1];
			_size = (subindex > 0)? 0 : sizeof(OD.PwmLoadCurrent[1]);
		break;
		
		case didCurrentSensor3:
			*Buf = (uint8_t*)&OD.PwmLoadCurrent[2];
			_size = (subindex > 0)? 0 : sizeof(OD.PwmLoadCurrent[2]);
		break;
		
		case didMachineState:
			*Buf = (uint8_t*)&OD.StateMachine.MainState;
			_size = (subindex > 0)? 0: sizeof(OD.StateMachine.MainState);
		break;

		// Internal State
		case didMachineSubState:
			*Buf = (uint8_t*)&OD.StateMachine.SubState;
			_size = (subindex > 0)? 0: sizeof(OD.StateMachine.SubState);
		break;

		case didInOutState:
			*Buf = (uint8_t*)&OD.IO;
			_size = (subindex > 0)? 0 : sizeof(OD.IO);
		break;
		
		case didPwmOutState:
			*Buf = (uint8_t*)&OD.PwmTargetLevel[subindex];
			_size = (subindex > 3)? 0 : sizeof(OD.PwmTargetLevel[subindex]);		
		break;
		
		case didHelmDemandAngle:
			_data = HelmGetTargetAngle();
			*Buf = (uint8_t*)&_data;
			_size = (subindex > 0)? 0 : 2;
		break;
		
		case didHelmStatus:
			_data = HelmGetStatus();
			*Buf = (uint8_t*)&_data;
			_size = (subindex > 0)? 0 : 1;
			break;

		case didSteeringFBAngle:
			_data = SteeringGetFeedBackAngle(&OD.SteeringData);
			*Buf = (uint8_t*)&_data;	
			_size = (subindex > 0)? 0: 2;
		break;
		
		case didSteeringFB:
			_data = SteeringGetFeedBackVoltage(&OD.SteeringData);
			*Buf = (uint8_t*)&_data;
			_size = (subindex > 0)? 0: 2;
		break;
		
		case didSteeringCurent:
			_data = SteeringGetDriveCurrent(&OD.SteeringData);
			*Buf = (uint8_t*)&_data;
			_size = (subindex > 0)? 0: 2;
		break;
		
		case didSteeringStatus:
			_data = SteeringGetStatus(&OD.SteeringData);
			*Buf = (uint8_t*)&_data;
			_size = (subindex > 0)? 0: 2;
		break;
		
		case didTrimFB:
			_data = TrimGetParameter(&OD.TrimDataRx, paramTrim_VoltageFB_0p1V);
			*Buf = (uint8_t*)&_data;
			_size = (subindex > 0)? 0: 2;
		break;
		
		case didTrimMovCmd:
			_data = TrimGetParameter(&OD.TrimDataRx, paramTrim_Cmd);
			*Buf = (uint8_t*)&_data;
			_size = (subindex > 0)? 0: 2;
		break;

		case didTrimPosition:
			_data = TrimGetParameter(&OD.TrimDataRx, paramTrim_Position);
			*Buf = (uint8_t*)&_data;
			_size = (subindex > 0)? 0: 2;
		break;

		case didTrimStatus:
			_data = TrimGetParameter(&OD.TrimDataRx, paramTrim_Status);
			*Buf = (uint8_t*)&_data;
			_size = (subindex > 0)? 0: 2;
		break;

		// Acceleration demand unit
		case didAccCh1:
			*Buf = (uint8_t*)&OD.AccPedalChannels[0];
			_size = (subindex > 0)? 0: sizeof(OD.AccPedalChannels[0]);
		break;
		
		case didAccCh2:
			*Buf = (uint8_t*)&OD.AccPedalChannels[1];
			_size = (subindex > 0)? 0: sizeof(OD.AccPedalChannels[1]);
		break;
		
		case didAccPosition:
			_data = thrHandleGetDemandAcceleration();
			*Buf = (uint8_t*)&_data;
			_size = (subindex > 0)? 0: 2;
		break;

		case didAccDemandDirection:
			_data = thrHandleGetDirection();
			*Buf = (uint8_t*)&_data;
			_size = (subindex > 0)? 0: 2;
			break;

		// Inverter
		case didMotorTemp:
			_data = McuRinegartGetParameter(&OD.mcHandler, mcu_MotorTemperature);
			*Buf = (uint8_t*)&_data;
			_size = (subindex > 0)? 0 : 2;
		break;
		
		case didInvTemp:
			_data = McuRinegartGetParameter(&OD.mcHandler, mcu_BoardTemperature);
			*Buf = (uint8_t*)&_data;
			_size = (subindex > 0)? 0: 2;
			break;
		
		case didInverterCurrent:
			_data = McuRinegartGetParameter(&OD.mcHandler, mcu_CurrentDC);
			*Buf = (uint8_t*)&_data;
			_size = (subindex > 0)? 0: 2;
			break;
		
		case didActualRpm:
			_data = McuRinegartGetParameter(&OD.mcHandler, mcu_ActualSpeed);
			*Buf = (uint8_t*)&_data;
			_size = (subindex > 0)? 0: 2;
			break;
		
		case didBatteryCurrent:
			*Buf = (uint8_t*)&OD.BatteryDataRx.TotalCurrent;
			_size = (subindex > 0)? 0: sizeof(OD.BatteryDataRx.TotalCurrent);
			break;
		
		case didActualGear:
			_data = McuRinegartGetParameter(&OD.mcHandler, mcu_Direction);
			*Buf = (uint8_t*)&_data;
			_size = (subindex > 0)? 0: sizeof(_data);
		break;
		
		case didTargetSpeed:
			_data = McuRinegartGetParameter(&OD.mcHandler, mcu_TargetTorque);
			*Buf = (uint8_t*)&_data;
			_size = (subindex > 0)? 0: 2;
		break;
		
		case didInverterVoltage:
			_data = McuRinegartGetParameter(&OD.mcHandler, mcu_VoltageDC);
			*Buf = (uint8_t*)&_data;
			_size = (subindex > 0)? 0: 2;
			break;
		
		case didInverterEnable:
			_data = McuRinegartGetParameter(&OD.mcHandler, mcu_EnableCmd);
			*Buf = (uint8_t*)&_data;
			_size = (subindex > 0)? 0: 2;
		break;
				
		case didInverterState:
			_data = McuRinegartGetParameter(&OD.mcHandler, mcu_Status);
			*Buf = (uint8_t*)&_data;
			_size = (subindex > 0)? 0: 2;
		break;
		
		// Battery system
		case didBatteryCmd:
			*Buf = (uint8_t*)&OD.BatteryReqState;
			_size = (subindex > 0)? 0: sizeof(OD.BatteryReqState);
		break;

		case didWaterSwitches:
			_data = OD.SB.stWaterSwitch1 | (OD.SB.stWaterSwitch2 << 1) | (OD.SB.stManualDrainSwitch << 2);
			*Buf = (uint8_t*)&_data;	
			_size = (subindex > 0)? 0: sizeof(uint8_t);
		break;
		
		case didIsoIsolation:
			_data = OD.Sim100DataRx.Isolation;
			*Buf = (uint8_t*)&_data;	
			_size = (subindex > 0)? 0: sizeof(uint16_t);
		break;
		
		case didObcStatus:
			_data = obcGetStatus();
			*Buf = (uint8_t*)&_data;	
			_size = (subindex > 0)? 0: sizeof(uint8_t);
		break;
		
		case didObcOnlineNumber:
			_data = obcGetOnlineUnitsNumber();
			*Buf = (uint8_t*)&_data;	
			_size = (subindex > 0)? 0: sizeof(uint8_t);
		break;
		
		case didObcTotalCurrent:
			_data = obcGetOutputCurrent();
			*Buf = (uint8_t*)&_data;	
			_size = (subindex > 0)? 0: sizeof(uint16_t);
		break;
		
		case didObcDemandTotalCurrent:
			_data = obcGetDemandCurrent();
			*Buf = (uint8_t*)&_data;	
			_size = (subindex > 0)? 0: sizeof(uint16_t);
			break;

		case didGearActualDirection:
			_data = transmissionGetActualGear();
			*Buf = (uint8_t*)&_data;
			_size = (subindex > 0)? 0: sizeof(uint16_t);
			break;

		case didGearIsShifting:
			_data = transmissionShiftInProcess();
			*Buf = (uint8_t*)&_data;
			_size = (subindex > 0)? 0: sizeof(uint16_t);
			break;

		case didGearActuatorState:
			_data = transmissionMovementPermission();
			*Buf = (uint8_t*)&_data;
			_size = (subindex > 0)? 0: sizeof(uint16_t);
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
	}
	return _size;
}


//uint8_t check_failed(uint8_t *file, uint8_t line)
//{
//	return 0;
//}
