#include "lpc17xx_pinsel.h"

#include "WorkStates.h"
#include "Main.h"
#include "User.h"
#include "TimerFunc.h"

#include "UserApplications/SteeringFunc.h"
#include "UserApplications/ObcDriver.h"
#include "UserApplications/HelmDriver.h"
#include "UserApplications/DriveControl.h"


void CommonState(void);

// Алгоритм работы
void (*Algorithm)(uint8_t *);

// Словарь объектов
ObjectDictionary_t OD;

int main(void)
{
    SystemInit();
    
    // Инициализация контроллера, внутренних переменных словаря
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
			*Buf = (uint8_t*)OD.SteeringData.FeedbackVoltage_0p1V;
			_size = (subindex > 0)? 0: sizeof(OD.SteeringData.FeedbackVoltage_0p1V);
		break;
		
		case didSteeringCurent:
			_data = SteeringGetDriveCurrent(&OD.SteeringData);
			*Buf = (uint8_t*)&_data;
			_size = (subindex > 0)? 0: 2;
		break;
		
		case didTrimFB:
			*Buf = (uint8_t*)OD.TrimDataRx.FeedBack_mV;
			_size = (subindex > 0)? 0: sizeof(OD.TrimDataRx.FeedBack_mV);
		break;
		
		case didAccCh1:
			*Buf = (uint8_t*)&OD.AccPedalChannels[0];
			_size = (subindex > 0)? 0: sizeof(OD.AccPedalChannels[0]);
		break;
		
		case didAccCh2:
			*Buf = (uint8_t*)&OD.AccPedalChannels[1];
			_size = (subindex > 0)? 0: sizeof(OD.AccPedalChannels[1]);
		break;
		
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
				
		case didTrimMovCmd:
			*Buf = (uint8_t*)&OD.TrimDataRx.MovCmd;
			_size = (subindex > 0)? 0: sizeof(OD.TrimDataRx.MovCmd);
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
		
		case didAccPosition:
			_data = driveGetDemandAcceleration();
			*Buf = (uint8_t*)&_data;
			_size = (subindex > 0)? 0: 2;
		break;
		
		case didInverterVoltage:
			_data = McuRinegartGetParameter(&OD.mcHandler, mcu_VoltageDC);
			*Buf = (uint8_t*)&_data;
			_size = (subindex > 0)? 0: 2;
			break;
		
		case didBatteryCmd:
			*Buf = (uint8_t*)&OD.BatteryReqState;
			_size = (subindex > 0)? 0: sizeof(OD.BatteryReqState);
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


uint8_t check_failed(uint8_t *file, uint8_t line)
{
	return 0;
}
