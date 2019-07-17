#ifndef _MAIN_H_
#define _MAIN_H_
#include <stdint.h>

#include "LPC17xx.h"
#include "lpc_types.h"

#include "ObjectsIndex.h"


#include "../BoardDefinitions/BmsCombi_Board.h"
#include "../MolniaLib/Config.h"
#include "../Libs/CurrentSens.h"

#include "../MolniaLib/MF_Tools.h"
#include "../MolniaLib/PowerManager.h"

#define MASTER
//#define MODULE
//#define DEBUG_CELL

// ������������ ����� �������, ������� ����� �������� ������
#define MAX_BATTERY_AMOUNT			2
// ���������� ������������� � ������
#define CELLS_IN_MODULE				24
// ���������� ������� � �����
#define MAX_MODULE_NUM				4
// ���������� ������������� � ������
#define TMP_SENSOR_IN_MODULE		4
// ������������ ���������� � ������ ������
#define MAX_FAULTS_NUM				10


// ��������� ��������� ��������
typedef enum {
    WORKSTATE_INIT = 0,
    WORKSTATE_PREOP,
    WORKSTATE_OPERATE,
    WORKSTATE_SHUTDOWN,
    WORKSTATE_FAULT,    
    WORKSTATE_TEST
} WorkStates_e ;

// �������� ������� ����� ������������� ���������
typedef struct
{
	WorkStates_e MainState;
	uint8_t SubState;
} StateMachine_t;

typedef struct
{
    uint8_t
    CheckFaults                 :   1,
    PowerOff_SaveParams         :   1,
    CurrentSensorReady          :   1,
	FaultStateHandled			:	1,
	Debug						:	1,
	EmergencyMode				:	1,
	PrechargeDone				:	1,
    BalancingIsBanned	        :   1;
	
	uint8_t
	PowerOn						:	1,
	MsgFromSystem				:	1,
	MsgFromPM					:	1,
	dummy						:	5;
} StateBits_t;

typedef struct
{
	uint32_t Timer_1ms;	
    uint32_t Timer_10ms;	
	uint32_t Timer_100ms;
    uint32_t Timer_500ms;
    uint32_t Timer_1s;
	uint32_t MainLoopTimer_ms;
	uint32_t ContactorPlusTimer_ms;
	uint32_t ContactorMinusTimer_ms;
	uint32_t ContactorPrechargeTimer_ms;
	uint32_t ModuleOnlineTimer_ms;
	uint32_t BatteryOnlineTimer_ms;
	uint32_t CurrentOverload_ms;
	uint32_t ModuleVoltageDiff_ms;
	uint32_t BatteryVoltageDiff_ms;
	uint32_t PowerOffTimer_ms;
	uint32_t KeyOffTimer_ms;
	uint32_t PowerMaganerTime_ms;
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

// ���������� �� ������
typedef struct
{
	// ���������� ������ � ��
	int16_t Voltage_mv;
	// ����� ������
	uint8_t  CellNumber;
	// ����� ������
	uint8_t ModuleNumber;
	// ����� �������
	uint8_t  BatteryNumber;
} CellVoltage_t;

// ����������� ������
typedef struct
{
    // ����������� ������
    int8_t Temperature;
	// ����� �������
	uint8_t SensorNum;
    // ����� ������
    uint8_t  ModuleNumber;
    // ����� �������
    uint8_t  BatteryNumber;
} ModuleTemperature_t;

// ��� �������
typedef struct
{
    // ��� ������� � ������� * 10
    int16_t Current;
    // ����� �������
    int8_t BatteryNumber;
} BatteryCurrent_t;

// ���������� �������
typedef struct
{
    // ���������� ������� � ������� * 10
    uint16_t Voltage;
    // ����� �������
    int8_t BatteryNumber;
} BatteryVoltage_t;

//typedef struct
//{
//	// ��������� ��������� ��������
//    StateMachine_t StateMachine;

//	// ��� ������� � ������� * 10
//    int16_t BatteryCurrent;
//    // ���������� ������ � ������� * 10
//    uint16_t BatteryVoltage_0p1V;
//	
//    // ���������� �������� ���������� ������ � ��
//	CellVoltage_t MaxCellVoltage;
//    // ���������� �������� ���������� ������ � ��
//	CellVoltage_t MinCellVoltage;

//	// ������� ���������� �� ������� � ��
//	uint16_t AvgCellVoltage;
//		
//    // ������������ ����������� ������
//    ModuleTemperature_t MaxModuleTemperature;
//    // ����������� ����������� ������
//    ModuleTemperature_t MinModuleTemperature;
//} BatteryData_t;

// ��������� ������ ���������� �������
typedef struct
{
	// ��������� ��������� ��������
    StateMachine_t StateMachine;
	
	uint8_t Faults;
	
    // ������� ������ � % * 10
    uint16_t SoC;
	// ��� �������� �������� ������� � �����-��������
    uint16_t DischargeEnergy_Ah;
    // ����������� ���� ������ � �������
    int16_t CCL;
    // ����������� ���� ������� � �������
    int16_t DCL;
	
	// ����� ������� � ����
    uint32_t OnlineFlags;
	// ������� ����� ������� � ����
    uint8_t OnlineNumber;
	// ����� ������������� �����
	uint32_t DischargingCellsFlag;
	
    // ��� ���� ������� * 10
    int16_t TotalCurrent;
    // ��������� ���������� �� ������������� * 10
    uint16_t TotalVoltage;
	
    // ������������ ��� ������� � ������� * 10
    BatteryCurrent_t MaxBatteryCurrent;
    // ����������� ��� ������� � ������� * 10
    BatteryCurrent_t MinBatteryCurrent;
	
    // ������������ ���������� �� ���� ������� � �������
    BatteryVoltage_t MaxBatteryVoltage;
    // ����������� ���������� �� ���� �������
    BatteryVoltage_t MinBatteryVoltage;
	
	uint32_t TimeOutCnts;
	
	// ********************** ���������� � ����������� ����� **************************************	
    // ���������� �������� ���������� ������ � ��
	CellVoltage_t MaxCellVoltage;
    // ���������� �������� ���������� ������ � ��
	CellVoltage_t MinCellVoltage;
	// ������� ���������� �� ������� � ��
	uint16_t AvgCellVoltage;

	
    // ������������ ����������� ������
    ModuleTemperature_t MaxModuleTemperature;
    // ����������� ����������� ������
    ModuleTemperature_t MinModuleTemperature;
	// ������� ����������� ������
	int8_t AvgModuleTemperature;	
} BatteryData_t;

typedef struct
{
    // ������ �� ������� �� ����������� ���������
    WorkStates_e RequestState;
    // ���������� ������������
    uint16_t TargetVoltage_mV;
    // ����� ��������� ����
    int16_t CCL;
    // ����� ���������� ����
    int16_t DCL;
    // ����������� ���������� ���������� ������������
    uint16_t CellOverVoltageLevel_mV;
    // ���������� ���������� ���������� ������������
    uint16_t CellUnderVoltageLevel_mV;
	uint8_t BalancingEnabled;
} MasterControl_t;

typedef union
{
	uint32_t Flags;
	struct
	{
		// Module Faults
		uint8_t
		Mod_ConfigCrc				:	1,
		Mod_ContactorPlus			:	1,
		Mod_ContactorMinus			:	1,
		Mod_ContactorPlusFB			:	1,	
		Mod_ContactorMinusFB		:	1,
		Mod_ModuleTempOutOfRange	:	1,
		Mod_Measurenment			:	1,
		Mod_PowerManagerOffline		:	1;
		
		// Battery Faults
		uint8_t		
		Mod_dummy2					:	4,
		Bat_ModuleWrongCount		:	1,
		Bat_ModuleFault				:	1,
		Bat_OverLoad				:	1,
		Bat_ModuleVoltageDiff		:	1;
		
		// Master Faults
		uint8_t		
		Bat_Precharge				:	1,
		Bat_CurrentSens				:	1,
		Bat_ExtTimeout				:	1,
		Bat_dummy3					:	5;
		
		uint8_t
		Mst_BatteryWrongCount		:	1,
		Mst_BatteryVoltageDiff		:	1,
		Mst_ExternalCANTimeout		:	1,
		Mst_BatteryFault			:	1,
		Mst_CellVoltageOutOfRange	:	1,
		Mst_ModuleTempOutOfRange	:	1,
		Mst_dummy4					:	2;
	};
} FaultsFlag_t;

typedef struct
{
	uint32_t CanMod;
	uint32_t CanGlobalStatus;
	uint32_t CanInt;
	uint32_t StartSign;
} DebugData_t;

// ������� ��������
typedef struct
{	
	uint16_t EcuInfo[4];
	
    StateMachine_t StateMachine;
	uint32_t SystemTime;
	uint32_t AfterResetTime;
	
    Timers_t LogicTimers;
    TimeValues_t DelayValues;
	
	uint8_t ecuIndex;
	
	uint16_t NodesOnline;
	
    StateBits_t SB;   
	uint32_t InOutState;
    
	uint16_t A_IN[2];
    uint16_t CurrentSensorVoltage[2];
	
    int16_t CellVoltageArray_mV[CELLS_IN_MODULE];
	int16_t CellTemperatureArray[TMP_SENSOR_IN_MODULE];
    
	BatteryData_t ModuleData[MAX_MODULE_NUM];
    // ������ �������
	BatteryData_t BatteryData[MAX_BATTERY_AMOUNT];
    // ������ �������
    BatteryData_t MasterData;
	
    // ����������� ������ �������
    MasterControl_t MasterControl;
    
    int16_t LastPrechargeMaxCurrent_0p1A;
    uint32_t LastPrechargeDuration;
    int16_t PreZeroCurrent_0p1A;
    
    uint32_t Energy_As;
	uint32_t BatteryCapacity;
	
	uint32_t SystemOperateEnabled;
	
	DebugData_t DebugData;
	
	PowerStates_e PowerMaganerState;
	uint16_t ecuPowerSupply_0p1;

	// �������������
	FaultsFlag_t Faults;
	
	uint8_t FaultsNumber;		
	uint16_t FaultList[MAX_FAULTS_NUM];
	
	uint8_t OldFaultsNumber;
	uint16_t OldFaultList[MAX_FAULTS_NUM];
} ObjectDictionary_t;

extern void (*Algorithm)(uint8_t *);
extern ObjectDictionary_t OD;
uint8_t GetDataByIndex(uint16_t Index, uint8_t SubIndex, uint8_t *Buf[]);

#endif
