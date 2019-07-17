#include "lpc17xx_pinsel.h"

#include "WorkStates.h"
#include "Main.h"
#include "User.h"
#include "TimerFunc.h"


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
    
    SET_A_OUT1_EN(1);
    SET_A_OUT2_EN(1);
    
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
	EcuConfig_t _config = GetConfigInstance();
	
	switch(Index)
	{
		case didEcuInfo:
			*Buf = (uint8_t*)OD.EcuInfo;
			_size = (subindex > 0)? 0 : sizeof(OD.EcuInfo);
		break;
		
		case didMachineState:
			*Buf = (uint8_t*)&OD.StateMachine.MainState;
			_size = (subindex > 0)? 0: sizeof(OD.StateMachine.MainState);
		break;
		
		case didMachineSubState:
			*Buf = (uint8_t*)&OD.StateMachine.SubState;
			_size = (subindex > 0)? 0: sizeof(OD.StateMachine.SubState);
		break;
		
		case didEcuVoltage:
			*Buf = (uint8_t*)&OD.ecuPowerSupply_0p1;
			_size = (subindex > 0)? 0 : sizeof(OD.ecuPowerSupply_0p1);
		break;
		
		case didPowerManagmentState:
			*Buf = (uint8_t*)&OD.PowerMaganerState;
		_size = (subindex > 0)? 0 : sizeof(OD.PowerMaganerState);
		break;
		
		
		case didConfigStructIndex:			
			*Buf = (uint8_t*)&_config;
			_size = (subindex > CONFIG_SIZE)? 0 : CONFIG_SIZE;
		break;
		
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
		
		case didInOutState:
			*Buf = (uint8_t*)&OD.IO;
			_size = (subindex > 0)? 0 : sizeof(OD.IO);
		break;
		
		case didPwmOutState:
			*Buf = (uint8_t*)&OD.PwmTargetLevel[subindex];
			_size = (subindex > 3)? 0 : sizeof(OD.PwmTargetLevel[subindex]);		
		break;
		
		case didSteeringTargetAngle:
			*Buf = (uint8_t*)&OD.SteeringDataRx.TargetAngle;
			_size = (subindex > 0)? 0 : sizeof(OD.SteeringDataRx.TargetAngle);
		break;
		
		case didSteeringFBAngle:
			_data = OD.steeringFeedbackAngle;
			*Buf = (uint8_t*)&_data;	
			_size = (subindex > 0)? 0: sizeof(OD.steeringFeedbackAngle);
		break;
		
		case didSteeringFB:
			*Buf = (uint8_t*)&OD.SteeringDataRx.Feedback_V;
			_size = (subindex > 0)? 0: sizeof(OD.SteeringDataRx.Feedback_V);
		break;
		
		case didSteeringCurent:
			*Buf = (uint8_t*)&OD.SteeringDataRx.SteeringTotalCurrent_0p1;
			_size = (subindex > 0)? 0: sizeof(OD.SteeringDataRx.SteeringTotalCurrent_0p1);
		break;
		
		case didTrimFB:
			*Buf = (uint8_t*)&OD.TrimDataRx.FeedBack_mV;
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
			*Buf = (uint8_t*)&OD.InvertorDataRx.MotorTemperature;
			_size = (subindex > 0)? 0: sizeof(OD.InvertorDataRx.MotorTemperature);
		break;
		
		case didInvTemp:
			*Buf = (uint8_t*)&OD.InvertorDataRx.InverterTemperature;
			_size = (subindex > 0)? 0: sizeof(OD.InvertorDataRx.InverterTemperature);
			break;
		
		case didInverterCurrent:
			*Buf = (uint8_t*)&OD.InvertorDataRx.CurrentDC;
			_size = (subindex > 0)? 0: sizeof(OD.InvertorDataRx.CurrentDC);
			break;
		
		case didActualRpm:
			*Buf = (uint8_t*)&OD.MovControlDataRx.ActualSpeed;
			_size = (subindex > 0)? 0: sizeof(OD.MovControlDataRx.ActualSpeed);
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
			*Buf = (uint8_t*)&OD.MovControlDataRx.Gear;
			_size = (subindex > 0)? 0: sizeof(OD.MovControlDataRx.Gear);
		break;
		
		case didTargetSpeed:
			*Buf = (uint8_t*)&OD.TractionData.TargetTorque;
			_size = (subindex > 0)? 0: sizeof(OD.TractionData.TargetTorque);
		break;
		
		case didAccPosition:
			*Buf = (uint8_t*)&OD.MovControlDataRx.AccPosition;
			_size = (subindex > 0)? 0: sizeof(OD.MovControlDataRx.AccPosition);
		break;
		
		case didInverterVoltage:
			*Buf = (uint8_t*)&OD.InvertorDataRx.VoltageDC;
			_size = (subindex > 0)? 0: sizeof(OD.InvertorDataRx.VoltageDC);
			break;
		
		case didBatteryCmd:
			*Buf = (uint8_t*)&OD.BatteryReqState;
			_size = (subindex > 0)? 0: sizeof(OD.BatteryReqState);
		break;
		
		case didInverterEnable:
			_data = OD.InvertorDataRx.InverterIsEnable;
			*Buf = (uint8_t*)&_data;
			_size = (subindex > 0)? 0: sizeof(uint8_t);
		break;
				
		case didInverterState:
			_data = OD.InvertorDataRx.InverterState;
			*Buf = (uint8_t*)&_data;
			_size = (subindex > 0)? 0: sizeof(OD.InvertorDataRx.InverterState);
		break;
		
		case didWaterSwitches:
			_data = OD.SB.WaterSwitch1 | (OD.SB.WaterSwitch2 << 1);
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
