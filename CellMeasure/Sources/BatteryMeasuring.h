#ifndef _BATTERY_MEASURING_H_
#define _BATTERY_MEASURING_H_

#include "Main.h"
#include "EcuConfig.h"

typedef enum
{
	type_Master = 0,
	type_Pack,
	type_Module,
} ModuleType_e;

typedef enum
{
	pack_Parallel = 0,
	pack_Serial,
	pack_Unknown,

} PackType_e;

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

// структура данных статистика мастера
typedef struct
{
	ModuleType_e Type;
	PackType_e PackType;
	uint8_t Index;
	// Состояние конечного автомата
    uint8_t MainState;
	uint8_t SubState;
	
	uint8_t Faults;
	uint8_t EstimationDoneInThisCycle;
	uint8_t IsInit;
	uint8_t DataIsReady;

    // Уровень заряда в % * 10
    uint16_t SoC;
	// Для подсчета отданной энергии в Ампер-секундах
    uint32_t ActualEnergy_As;
	uint32_t TotalEnergy_As;
    // Ограничение тока заряда в амперах
    int16_t CCL;
    // Ограничение тока разряда в амперах
    int16_t DCL;

    uint16_t MaxCellVoltageThreshold_mV;
    uint16_t MinCellVoltageThreshold_mV;

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

	int32_t CurrentCycleEnergy_As;
	int32_t EnergyLow;
	int32_t EnergyLowPrev;
	uint32_t CapacityCulcStamp;
} BatteryData_t;


void ModuleStatisticCalculating(BatteryData_t* Handle, const EcuConfig_t *ecuConfig, const int16_t *CellsVoltageArray, const int16_t *CellsTempArray);
void BatteryStatisticCalculating(BatteryData_t *Handle, const BatteryData_t *SourceData, uint8_t SourceItemsNum);
void GetCurrentLimit(const BatteryData_t* handle, const EcuConfig_t *ecuConfig, int16_t* Discharge, int16_t* Charge);

uint8_t sysEnergy_EnergyCounting(BatteryData_t* Handle, int16_t Current_0p1A);
int8_t sysEnergy_Init(BatteryData_t *Handle, uint32_t InitialTotalSystemEnergy_As, uint32_t InitialActualEnergy_As, uint16_t MaxCellVoltageThreshold_mV, uint16_t MinCellVoltageThreshold_mV);
uint32_t sysEnergy_InitEnergyFromMinUcell(BatteryData_t *Handle, int16_t *OcvTable, uint16_t CellVoltage);
uint16_t sysEnergy_CoulombCounting(BatteryData_t *Handle, int16_t Current_0p1);
uint32_t sysEnergy_EnergyEstimation(BatteryData_t *Handle, uint16_t MaxCellVoltage_mV, uint16_t MinCellVoltage_mV);
uint32_t sysEnergy_CapacityInit(BatteryData_t *Handle, uint32_t Capacity);


#endif


