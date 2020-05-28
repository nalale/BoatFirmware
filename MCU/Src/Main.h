#ifndef _MAIN_H_
#define _MAIN_H_


#define DIR_IN      0
#define DIR_OUT     1

#include "LPC17xx.h"
#include "lpc_types.h"
#include "FaultTools.h"

#include "../MolniaLib/Config.h"
#include "../MolniaLib/PowerManager.h"

#include "UserApplications/SteeringFunc.h"
#include "UserApplications/ThrottleControl.h"
#include "UserApplications/ObcDriver.h"
#include "UserApplications/TrimFunc.h"
#include "UserApplications/Sim100.h"
#include "UserApplications/HelmDriver.h"
#include "UserApplications/TransmissionDriver.h"
#include "MotionControllers/Inverter_Rinehart.h"

#include "ObjectsIndex.h"

// Максимальное количество в списке ошибок
#define MAX_FAULTS_NUM				10

// Состояния конечного автомата
typedef enum {
    WORKSTATE_INIT = 0,
    WORKSTATE_OPERATE,
    WORKSTATE_SHUTDOWN,
    WORKSTATE_FAULT,
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
    BoostModeRequest          	:   1,
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
	cmdLightAll					:	1;


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
	cmdLightNavi				:	1,
	cmdLightCockPit				:	1,
	cmdETForward				:	1,
	cmdETBackward				:	1;

	uint8_t
	stETActuatorSwitch			:	1,
	stBoostMode					:	1,
	dummy2						:	6;
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

	// Display ECU
	D_IN_D_ECU_LIGHT_SWITCH_1 = 0,
	D_IN_PADDING_15,
	D_IN_D_ECU_LIGHT_SWITCH_2,
	D_IN_PADDING_16,
	D_IN_D_ECU_LIGHT_SWITCH_3,
	D_IN_PADDING_17,
	D_IN_PADDING_18,
	D_IN_PADDING_19,
	D_IN_D_ECU_BOOST_SWITCH,
	D_IN_PADDING_21,
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
	uint32_t CanMod;
	uint32_t CanGlobalStatus;
	uint32_t CanInt;
	uint32_t StartSign;
} DebugData_t;

typedef struct
{
	uint32_t SystemTime;
	uint32_t TotalActualEnergy_As;
	uint32_t ActualEnergy_As;

	uint32_t dummy;
	uint32_t dumm1;
	uint32_t dummy2;
	uint32_t dummy3;
	uint32_t dummy4;
	uint16_t NormalPowerOff;

	uint16_t Crc;
} sDataBuf_t;

typedef struct
{
	sDataBuf_t Buf_1st;
	//sDataBuf_t Buf_2nd;

	//const dtcItem_t* dtcListData;
	const EcuConfig_t *cfgData;

	//struct Data_st Data;
	uint8_t Result;
	uint8_t DataChanged;
} StorageData_t;

// Словарь объектов
typedef struct
{
	const EcuConfig_t *cfgEcu;
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
    
    uint8_t BatteryReqState;
	
	BatteryData_t BatteryDataRx;
	McuRinehart_t mcHandler;

    SteeringData_t SteeringData;
	TrimData_t TrimDataRx;
	sim100Data_t Sim100DataRx;
		
	uint16_t MaxMotorSpeed;
	uint16_t MaxMotorTorque;
	
	int16_t steeringFeedbackAngle;
	
	DebugData_t DebugData;
	StorageData_t SData;
	
	uint16_t TargetSteeringAngle;
	uint8_t TargetChargingCurrent_A;

	PowerStates_e LocalPMState;
	PowerStates_e PowerManagerCmd;

	uint8_t FaultsNumber;		
	uint16_t FaultList[MAX_FAULTS_NUM];
	
	uint8_t OldFaultsNumber;
	uint16_t OldFaultList[MAX_FAULTS_NUM];
    
} ObjectDictionary_t;


extern void (*Algorithm)(uint8_t *);
extern ObjectDictionary_t OD;

uint8_t GetDataByIndex(uint16_t Index, uint8_t subindex, uint8_t *Buf[]);
//uint8_t check_failed(uint8_t *file, uint8_t line);



#endif
