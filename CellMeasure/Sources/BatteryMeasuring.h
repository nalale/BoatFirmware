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

// ��������� ������ ���������� �������
typedef struct
{
	ModuleType_e Type;
	PackType_e PackType;
	uint8_t Index;
	// ��������� ��������� ��������
    uint8_t MainState;
	uint8_t SubState;
	
	uint8_t Faults;
	uint8_t EstimationDoneInThisCycle;
	uint8_t IsInit;
	uint8_t DataIsReady;

    // ������� ������ � % * 10
    uint16_t SoC;
	// ��� �������� �������� ������� � �����-��������
    uint32_t ActualEnergy_As;
	uint32_t TotalEnergy_As;
    // ����������� ���� ������ � �������
    int16_t CCL;
    // ����������� ���� ������� � �������
    int16_t DCL;

    uint16_t MaxCellVoltageThreshold_mV;
    uint16_t MinCellVoltageThreshold_mV;

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


