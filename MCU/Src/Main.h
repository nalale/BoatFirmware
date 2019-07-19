#ifndef _MAIN_H_
#define _MAIN_H_


#define DIR_IN      0
#define DIR_OUT     1

#include "LPC17xx.h"
#include "lpc_types.h"
#include "FaultsTest.h"
#include "Inverter.h"

#include "../MolniaLib/Config.h"
#include "../MolniaLib/PowerManager.h"

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
	InverterIsOperate			:	1,
	BatteryIsOperate			:	1,
	BatMsgReceived				:	1,
	SkfMsgReceived				:	1,
    InvMsgReceived              :   1;
	
	uint8_t
	cmdDrainPumpOn				:	1,
	cmdMotorPumpCooling			:	1,
	cmdHeatsinkPump				:	1,
	cmdInverterPowerSupply		:	1,
	cmdSteeringPump				:	1,
	cmdTrimUp					:	1,
	cmdTrimDown					:	1,
	cmddummy1					:	1;


	uint8_t
	stWaterSwitch1				:	1,
	stWaterSwitch2				:	1,
	stManualDrainSwitch			:	1,
	stChargingTerminal			:	1,
	stManualDriveSwitch			:	1,
	stManualDriveD				:	1,
	stManuelDriveR				:	1,
	stCoolantLowLevel			:	1;
	
	uint8_t
	PowerOn						:	1,
	Ecu1MeasDataActual			:	1,
	Ecu2MeasDataActual			:	1,
	Ecu4MeasDataActual			:	1,
	dummy1						:	4;
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

typedef enum
{
	// ECU1
	D_IN_KL15_1 = 0,
	D_IN_KL15_CHARGE,
	D_IN_MANUAL_DRAIN,
	D_IN_WATER_SWITCH_1,
	D_IN_THRU_JOY_1,
	D_IN_THRU_JOY_2,
	D_IN_HEATER_SWITCH_1,
	D_IN_HEATER_SWITCH_2,
	D_IN_PADDING_1,
	D_IN_PADDING_2,

	// ECU2
	D_IN_TRIM_JOY_1 = 0,
	D_IN_TRIM_JOY_2,
	D_IN_LIGHT_SWITCH_1,
	D_IN_LIGHT_SWITCH_2,
	D_IN_LIGHT_SWITCH_3,
	D_IN_ROOF_OPEN_SWITCH,
	D_IN_ROOF_CLOSE_SWITCH,
	D_IN_PADDING_3,
	D_IN_PADDING_4,
	D_IN_PADDING_5,

	// ECU4
	D_IN_WATER_SWITCH_2 = 0,
	D_IN_PADDING_6,
	D_IN_PADDING_7,
	D_IN_PADDING_8,
	D_IN_PADDING_9,
	D_IN_PADDING_10,
	D_IN_PADDING_11,
	D_IN_PADDING_12,
	D_IN_PADDING_13,
	D_IN_PADDING_14,
} D_Inputs;

typedef enum
{
	// ECU1
	D_OUT_DRAIN_1 = 0,
	D_OUT_DRAIN_3,
	D_OUT_FAN_1,
	D_OUT_HEATFUN_1,
	D_OUT_HEATFUN_2,
	D_OUT_PADDING_1,

	// ECU2
	D_OUT_PADDING_2,		//6
	D_OUT_LAMP_1,
	D_OUT_LAMP_2,
	D_OUT_LAMP_3,
	D_OUT_LAMP_4,
	D_OUT_LAMP_5,

	// ECU3
	D_OUT_TRIM_RUN,			//12
	D_OUT_TRIM_UpDown,
	D_OUT_INV_CTRL,
	D_OUT_PADDING_3,
	D_OUT_PADDING_4,
	D_OUT_FAN_2,			

	// ECU4
	D_OUT_CPUMP_0,			//18
	D_OUT_CPUMP_1,
	D_OUT_CPUMP_2,
	D_OUT_ROOF_UP,
	D_OUT_ROOF_DOWN,
	D_OUT_DRAIN_2,
} D_Outputs;

#define GET_IN_STATE(x, y) (((x) >> (y)) & 0x01)
#define SET_OUT_STATE(x, y) ((x) << (y))

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

    Traction_Cmd_t TractionData;
    uint8_t BatteryReqState;
	
	BatteryData_t BatteryDataRx;
    Invertor_Data_Rx_t InvertorDataRx;
    Mov_Control_Data_Rx_t MovControlDataRx;

    Helm_Data_Rx_t HelmData;
    SteeringData_t SteeringData;
	TrimData_t TrimDataRx;
	sim100Data_t Sim100DataRx;
		
	uint16_t MaxMotorSpeed;
	uint16_t MaxMotorTorque;
	
	int16_t steeringFeedbackAngle;
	
	DebugData_t DebugData;
	
	uint16_t TargetSteeringAngle;
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
