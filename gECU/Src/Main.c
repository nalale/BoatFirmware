#include "lpc17xx_pinsel.h"

#include "WorkStates.h"
#include "Main.h"
#include "User.h"
#include "TimerFunc.h"
#include "UartFunc.h"

void CommonState(void);

// Алгоритм работы
void (*Algorithm)(uint8_t *);

// Словарь объектов
ObjectDictionary_t OD;
static EcuConfig_t _config;
int main(void)
{	
    SystemInit();    
    // Инициализация контроллера, внутренних переменных словаря
    AppInit(&OD);
	
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
	_config = GetConfigInstance();
	
	switch(Index)
	{
		case didEcuInfo:
			*Buf = (uint8_t*)OD.EcuInfo;
			_size = (subindex > 0)? 0 : sizeof(OD.EcuInfo);
		break;
		
		// Module Parameters
		case didConfigStructIndex:			
			*Buf = (uint8_t*)&_config;
			_size = (subindex > CONFIG_SIZE)? 0 : CONFIG_SIZE;
		break;
		
		case didMachineState:
			*Buf = (uint8_t*)&OD.StateMachine.MainState;
			_size = (subindex > 0)? 0 : sizeof(OD.StateMachine.MainState);
		break;
		
		case didMachineSubState:
			*Buf = (uint8_t*)&OD.StateMachine.SubState;
			_size = (subindex > 0)? 0 : sizeof(OD.StateMachine.SubState);
		break;
		
		case didEcuVoltage:
			*Buf = (uint8_t*)&OD.ecuPowerSupply_0p1;
			_size = (subindex > 0)? 0 : sizeof(OD.ecuPowerSupply_0p1);
		break;
		
		case didPowerManagmentState:
			*Buf = (uint8_t*)&OD.PowerMaganerState;
		_size = (subindex > 0)? 0 : sizeof(OD.PowerMaganerState);
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
		
		case didVoltageSensor5:
			*Buf = (uint8_t*)&OD.Out_CSens[0].Out_CSens_Voltage;
			_size = (subindex > 0)? 0 : sizeof(OD.Out_CSens[0].Out_CSens_Voltage);
		break;
		
		case didVoltageSensor6:
			*Buf = (uint8_t*)&OD.Out_CSens[1].Out_CSens_Voltage;
			_size = (subindex > 0)? 0 : sizeof(OD.Out_CSens[1].Out_CSens_Voltage);
		break;
		
		case didVoltageSensor7:
			*Buf = (uint8_t*)&OD.Out_CSens[2].Out_CSens_Voltage;
			_size = (subindex > 0)? 0 : sizeof(OD.Out_CSens[2].Out_CSens_Voltage);
		break;
		
		case didCurrentSensor1:
			*Buf = (uint8_t*)&OD.Out_CSens[0].Out_CSens_Current;
			_size = (subindex > 0)? 0 : sizeof(OD.Out_CSens[0].Out_CSens_Current);
		break;
		
		case didCurrentSensor2:
			*Buf = (uint8_t*)&OD.Out_CSens[1].Out_CSens_Current;
			_size = (subindex > 0)? 0 : sizeof(OD.Out_CSens[1].Out_CSens_Current);
		break;
		
		case didCurrentSensor3:
			*Buf = (uint8_t*)&OD.Out_CSens[2].Out_CSens_Current;
			_size = (subindex > 0)? 0 : sizeof(OD.Out_CSens[2].Out_CSens_Current);
		break;
		
		case didInOutState:
			_data = (OD.IO);
			*Buf = (uint8_t*)&_data;
			_size = (subindex > 0)? 0 : sizeof(OD.IO);
		break;
		
		case didPwmOutState:
			*Buf = (uint8_t*)&OD.A_Out[subindex];
			_size = (subindex > 3)? 0 : sizeof(OD.A_Out[subindex]);
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
