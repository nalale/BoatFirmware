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

// Максимальное число батарей, которые могут работать вместе
#define MAX_BATTERY_AMOUNT			2
// Количество аккумуляторов в модуле
#define CELLS_IN_MODULE				24
// КОличество модулей в ветке
#define MAX_MODULE_NUM				4
// Количество термодатчиков в модуле
#define TMP_SENSOR_IN_MODULE		4
// Максимальное количество в списке ошибок
#define MAX_FAULTS_NUM				10


// Состояния конечного автомата
typedef enum {
    WORKSTATE_INIT = 0,
    WORKSTATE_PREOP,
    WORKSTATE_OPERATE,
    WORKSTATE_SHUTDOWN,
    WORKSTATE_FAULT,    
    WORKSTATE_TEST
} WorkStates_e ;

// Конечный автомат имеет двухуровневую структуру
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

// Напряжение на ячейке
typedef struct
{
	// Напряжение ячейки в мВ
	int16_t Voltage_mv;
	// Номер ячейки
	uint8_t  CellNumber;
	// Номер модуля
	uint8_t ModuleNumber;
	// Номер батареи
	uint8_t  BatteryNumber;
} CellVoltage_t;

// Температура модуля
typedef struct
{
    // Температура модуля
    int8_t Temperature;
	// Номер датчика
	uint8_t SensorNum;
    // Номер модуля
    uint8_t  ModuleNumber;
    // Номер батареи
    uint8_t  BatteryNumber;
} ModuleTemperature_t;

// Ток батареи
typedef struct
{
    // Ток батареи в амперах * 10
    int16_t Current;
    // Номер батареи
    int8_t BatteryNumber;
} BatteryCurrent_t;

// Напряжение батареи
typedef struct
{
    // Напряжение батареи в вольтах * 10
    uint16_t Voltage;
    // Номер батареи
    int8_t BatteryNumber;
} BatteryVoltage_t;

//typedef struct
//{
//	// Состояние конечного автомата
//    StateMachine_t StateMachine;

//	// Ток батареи в амперах * 10
//    int16_t BatteryCurrent;
//    // Напряжение модуля в вольтах * 10
//    uint16_t BatteryVoltage_0p1V;
//	
//    // Напряжение наиболее заряженной ячейки в мВ
//	CellVoltage_t MaxCellVoltage;
//    // Напряжение наименее заряженной ячейки в мВ
//	CellVoltage_t MinCellVoltage;

//	// Среднее напряжение на ячейках в мВ
//	uint16_t AvgCellVoltage;
//		
//    // Максимальная температура модуля
//    ModuleTemperature_t MaxModuleTemperature;
//    // Минимальная температура модуля
//    ModuleTemperature_t MinModuleTemperature;
//} BatteryData_t;

// структура данных статистика мастера
typedef struct
{
	// Состояние конечного автомата
    StateMachine_t StateMachine;
	
	uint8_t Faults;
	
    // Уровень заряда в % * 10
    uint16_t SoC;
	// Для подсчета отданной энергии в Ампер-секундах
    uint16_t DischargeEnergy_Ah;
    // Ограничение тока заряда в амперах
    int16_t CCL;
    // Ограничение тока разряда в амперах
    int16_t DCL;
	
	// Флаги батарей в сети
    uint32_t OnlineFlags;
	// Сколько всего батарей в сети
    uint8_t OnlineNumber;
	// Флаги балансируемых ячеек
	uint32_t DischargingCellsFlag;
	
    // Ток всей системы * 10
    int16_t TotalCurrent;
    // Суммарное напряжение на аккумуляторах * 10
    uint16_t TotalVoltage;
	
    // Максимальный ток батареи в амперах * 10
    BatteryCurrent_t MaxBatteryCurrent;
    // Минимальный ток батареи в амперах * 10
    BatteryCurrent_t MinBatteryCurrent;
	
    // Максимальное напряжение из всех батарей в вольтах
    BatteryVoltage_t MaxBatteryVoltage;
    // Минимальное напряжение из всех батарей
    BatteryVoltage_t MinBatteryVoltage;
	
	uint32_t TimeOutCnts;
	
	// ********************** Напряжение и температура ячеек **************************************	
    // Напряжение наиболее заряженной ячейки в мВ
	CellVoltage_t MaxCellVoltage;
    // Напряжение наименее заряженной ячейки в мВ
	CellVoltage_t MinCellVoltage;
	// Среднее напряжение на ячейках в мВ
	uint16_t AvgCellVoltage;

	
    // Максимальная температура модуля
    ModuleTemperature_t MaxModuleTemperature;
    // Минимальная температура модуля
    ModuleTemperature_t MinModuleTemperature;
	// Средняя температура модуля
	int8_t AvgModuleTemperature;	
} BatteryData_t;

typedef struct
{
    // Запрос от мастера на определённое состояние
    WorkStates_e RequestState;
    // Напряжение балансировки
    uint16_t TargetVoltage_mV;
    // Лимит зарядного тока
    int16_t CCL;
    // Лимит разрядного тока
    int16_t DCL;
    // Максимально допустимое напряжение аккумулятора
    uint16_t CellOverVoltageLevel_mV;
    // Минимально допустимое напряжение аккумулятора
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

// Словарь объектов
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
    // Данные батареи
	BatteryData_t BatteryData[MAX_BATTERY_AMOUNT];
    // Данные мастера
    BatteryData_t MasterData;
	
    // Управляющие данные мастера
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

	// Неисправности
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
