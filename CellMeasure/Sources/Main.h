#ifndef _MAIN_H_
#define _MAIN_H_
#include <stdint.h>

#include "LPC17xx.h"
#include "lpc_types.h"

#include "ObjectsIndex.h"
#include "BatteryMeasuring.h"

#include "../BoardDefinitions/BmsCombi_Board.h"
#include "../MolniaLib/Config.h"
#include "../Libs/CurrentSens.h"

#include "../MolniaLib/MF_Tools.h"
#include "../MolniaLib/PowerManager.h"
#include "../MOlniaLib/FaultsServices.h"

// ������������ ����� �������, ������� ����� �������� ������
#define MAX_BATTERY_AMOUNT			2
// ���������� ������������� � ������
#define CELLS_IN_MODULE				24
// ���������� ������� � �����
#define MAX_MODULE_NUM				12
// ���������� ������������� � ������
#define TMP_SENSOR_IN_MODULE		4

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
	FlashDataRead				:	1,
	dummy						:	4;
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

typedef struct
{
	uint32_t SystemTime;
	uint32_t TotalActualEnergy_As;
	uint32_t ActualEnergy_As;

	uint32_t SystemTotalEnergy_As;
	uint32_t SystemActualEnergy_As;
	uint32_t BmsTotalEnergy_As;
	uint32_t BmsActualEnergy_As;
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


typedef struct
{
    // ������ �� ������� �� ����������� ���������
    WorkStates_e RequestState;
    // ����� ��������� ����
    int16_t CCL;
    // ����� ���������� ����
    int16_t DCL;
    // ����������� ���������� ���������� ������������
    uint16_t CellOverVoltageLevel_mV;
    // ���������� ���������� ���������� ������������
    uint16_t CellUnderVoltageLevel_mV;

} MasterControl_t;

typedef struct
{
	// ���������� ������������
	uint16_t TargetVoltage_mV;
	uint8_t BalancingEnabled;
	uint8_t ModulesInAssembly;
} PackControl_t;

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
		Mod_EmergencyPowerOff		:	1,
		Mod_dummy2					:	3,
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
	
	StorageData_t SData;

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
	BatteryData_t PackData[MAX_BATTERY_AMOUNT];
    // ������ �������
    BatteryData_t MasterData;
	
    MasterControl_t MasterControl;
    PackControl_t PackControl;
    
    int16_t LastPrechargeMaxCurrent_0p1A;
    uint32_t LastPrechargeDuration;
    int16_t PreZeroCurrent_0p1A;
    
	uint32_t BatteryCapacity;
	
	uint32_t SystemOperateEnabled;
	
	DebugData_t DebugData;
	
	PowerStates_e LocalPMState;
	PowerStates_e PowerMaganerCmd;
	uint16_t ecuPowerSupply_0p1;

	const EcuConfig_t *ConfigData;

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
