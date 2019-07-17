#ifndef _MAIN_H_
#define _MAIN_H_


#define DIR_IN      0
#define DIR_OUT     1

#include "LPC17xx.h"
#include "lpc_types.h"
#include "../MolniaLib/Config.h"
#include "FaultsTest.h"
#include "Inverter.h"
#include "PowerManager.h"

#include "TrimFunc.h"
#include "Sim100.h"
#include "SteeringFunc.h"
#include "ObjectsIndex.h"

// Максимальное количество в списке ошибок
#define MAX_FAULTS_NUM				10

// Состояния конечного автомата
typedef enum {
    WORKSTATE_INIT = 0,
    WORKSTATE_OPERATE,
    WORKSTATE_SHUTDOWN,
    WORKSTATE_FAULT,    
    WORKSTATE_CHARGE
} WorkStates_e ;

// Конечный автомат имеет двухуровневую структуру
typedef struct
{
	WorkStates_e MainState;
	uint8_t SubState;
} StateMachine_t;

typedef struct
{
	uint32_t Timer_1ms;	
    uint32_t Timer_10ms;	
	uint32_t Timer_100ms;
    uint32_t Timer_500ms;
    uint32_t Timer_1s;
	uint32_t MainLoopTimer_ms;
	uint32_t PowerOffTimer_ms;
	uint32_t KeyOffTimer_ms;

    //TimeoutsBlocks timers
    uint32_t InverterTimer_ms;
    uint32_t SteeringTimer_ms;
    uint32_t BatteryTimer_ms;
	uint32_t AcceleratorTimer_ms;
	uint32_t Sim100OfflineTime_ms;
    
	int16_t KeyCnt;
} Timers_t;

typedef struct
{
	uint16_t Time1_ms;
	uint16_t Time10_ms;
	uint16_t Time100_ms;
	uint16_t Time1_s;
	uint16_t MainLoopTime_ms;
    
} TimeValues_t;

typedef struct
{
    uint8_t
    CheckFaults                 :   1,
    PowerOff_SaveParams         :   1,
    ReceiveTestRequest          :   1,
	InvPumpCooling				:	1,
	InverterIsOperate			:	1,
	BatteryIsOperate			:	1,
	ChargingTerminalConnected	:	1,
    InvMsgReceived              :   1;
	
	uint8_t
	BatMsgReceived				:	1,
	SkfMsgReceived				:	1,
	MotorPumpCooling			:	1,
	HeatsinkPump				:	1,
	WaterSwitch1				:	1,
	WaterSwitch2				:	1,
	DrainOn						:	1,
	ManualDrainSwitch			:	1;
	
} StateBits_t;

typedef union
{
	uint16_t Flags;
	struct
	{
		uint8_t
		ConfigCrc			:	1,	
		InverterTimeout     :   1,
		SteeringTimeout     :   1,
		BatteryTimeout		:	1,
		BatteryFault        :   1,
		InverterFault		:	1,
		SteeringPosition	:	1,
		TrimPosition		:	1;
		
		uint8_t
		Accelerator			:	1,
		SteeringFeedback	:	1,
		TrimFeedback		:	1,
		PwmCircuit			:	1,
		MeasuringCircuit	:	1,
		PowerSupplyCircuit	:	1,
		dummy				:	2;
	};
} FaultsBits_t;


typedef struct
{
    uint8_t TargetSteeringBrake;  
    
} Steering_Cmd_t;

typedef struct
{
	uint8_t MainState;
	uint8_t SubState;
	uint8_t SoC;
	int16_t CCL;
	int16_t DCL;
	int16_t TotalCurrent;
	int16_t TotalVoltage;
} BatteryData_t;

typedef struct
{
    int16_t TargetTorque;
    int16_t TorqueLimit;
    uint8_t Direction;
    
    uint8_t
    InverterEnable  :   1,
    dummy1          :   7;
    
} Traction_Cmd_t;

typedef struct
{
    uint8_t AccPosition;
    uint8_t Gear;
    int16_t ActualSpeed;
	uint16_t ActualTorque;
} Mov_Control_Data_Rx_t;

typedef struct
{
    uint8_t
    InverterIsEnable    :   1,
    LockupIsEnable      :   1,
    dummy1              :   6;   
	
	InverterStates_e InverterState;
	int16_t InverterTemperature;
	int16_t MotorTemperature;
    uint16_t FaultCode;
	int16_t VoltageDC;
	int16_t CurrentDC;
} Invertor_Data_Rx_t;

typedef struct
{
	uint32_t CanMod;
	uint32_t CanGlobalStatus;
	uint32_t CanInt;
	uint32_t StartSign;
} DebugData_t;

// Словарь объектов
typedef struct
{
	uint16_t EcuInfo[4];
    StateMachine_t StateMachine;
    Timers_t LogicTimers;
    TimeValues_t DelayValues;
    
    StateBits_t SB;
    FaultsBits_t FaultsBits;
	
	uint8_t ecuIndex;
	uint8_t ecuPowerSupply_0p1;
	
	uint32_t IO;
	uint32_t SystemTime;
	
	uint8_t A_CH_Voltage_0p1[4];
	uint16_t PwmLoadCurrent[4];
	uint8_t PwmTargetLevel[4];
	
	uint8_t AccPedalChannels[2];
    
    Steering_Cmd_t SteeringData;
    Traction_Cmd_t TractionData;
    uint8_t BatteryReqState;
	
	BatteryData_t BatteryDataRx;
    Invertor_Data_Rx_t InvertorDataRx;
    Mov_Control_Data_Rx_t MovControlDataRx;
    Steering_Data_Rx_t SteeringDataRx;
	TrimData_t TrimDataRx;
	sim100Data_t Sim100DataRx;
		
	uint16_t MaxMotorSpeed;
	uint16_t MaxMotorTorque;
	
	int16_t steeringFeedbackAngle;
	
	DebugData_t DebugData;
	
	uint8_t TargetChargingCurrent_A;
	
	PowerStates_e PowerMaganerState;
		
	uint8_t FaultsNumber;		
	uint16_t FaultList[MAX_FAULTS_NUM];
	
	uint8_t OldFaultsNumber;
	uint16_t OldFaultList[MAX_FAULTS_NUM];
    
} ObjectDictionary_t;


extern void (*Algorithm)(uint8_t *);
extern ObjectDictionary_t OD;

uint8_t GetDataByIndex(uint16_t Index, uint8_t subindex, uint8_t *Buf[]);
uint8_t check_failed(uint8_t *file, uint8_t line);



#endif
