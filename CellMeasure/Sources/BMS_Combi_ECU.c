#include "Main.h"
#include "FaultTools.h"
#include "BMS_Combi_ECU.h"

#include "../MolniaLib/MF_Tools.h"
#include "../Libs/LTC6803.h"
#include "../Libs/filter.h"

#define DV_FRZF(val)			sizeof(val), &val
#define DIAG_ITEM(val)		ARRAY_LEN(val),  (void*)&val

// ************************************************************************************************
// DTC: Ошибка памяти параметров
// ************************************************************************************************
// Стоп-кадр
dtcFRZF_General frzfEcuConfigMemory;
// Указатели на данные стоп-кадра (аналогично DiagnosticValue)
const DiagnosticValueFRZF dVal_frzfEcuConfigMemory[] = 
{
	{ didFaults_FreezeFrame, DV_FRZF(frzfEcuConfigMemory) },
};
// Статические параметры неисправности
dtcProperty_t dtcProp_EcuConfigMemory = { dtc_General_EcuConfig, DTC_BIT_WARNING_ENABLE + DTC_BIT_OPERATION_DISABLE, 0, 50, -50, 1, DIAG_ITEM(dVal_frzfEcuConfigMemory) };
// Все о неисправности
dtcItem_t dtcEcuConfigMemory = {&dtcProp_EcuConfigMemory};


// ************************************************************************************************
// DTC: Неожиданное отключение питания
// ************************************************************************************************
// Стоп-кадр
dtcFRZF_General frzfUnexpectedPowerOff;
// Указатели на данные стоп-кадра (аналогично DiagnosticValue)
const DiagnosticValueFRZF dVal_frzfUnexpectedPowerOff[] = 
{
	{ didFaults_FreezeFrame, DV_FRZF(frzfUnexpectedPowerOff) },
};
// Статические параметры неисправности
dtcProperty_t dtcProp_UnexpectedPowerOff = { dtc_General_UnexpectedPowerOff, DTC_BIT_NONE, 0, 50, -50, 1, DIAG_ITEM(dVal_frzfUnexpectedPowerOff) };
// Все о неисправности
dtcItem_t dtcUnexpectedPowerOff = {&dtcProp_UnexpectedPowerOff};


// ************************************************************************************************
// DTC: Интерлок
// ************************************************************************************************
// Стоп-кадр
dtcFRZF_General frzfInterlock;
// Указатели на данные стоп-кадра (аналогично DiagnosticValue)
const DiagnosticValueFRZF dVal_frzfInterlock[] = 
{
	{ didFaults_FreezeFrame, DV_FRZF(frzfInterlock) },
};
// Статические параметры неисправности
dtcProperty_t dtcProp_Interlock = { dtc_General_Interlock, DTC_BIT_NONE, 0, 50, -50, 1, DIAG_ITEM(dVal_frzfInterlock) };
// Все о неисправности
dtcItem_t dtcInterlock = {&dtcProp_Interlock};

// ************************************************************************************************
// DTC: Нет связи с PM
// ************************************************************************************************
// Стоп-кадр
dtcFRZF_General frzfPMOffline;
// Указатели на данные стоп-кадра (аналогично DiagnosticValue)
const DiagnosticValueFRZF dVal_frzfPMOffline[] = 
{
	{ didFaults_FreezeFrame, DV_FRZF(frzfPMOffline) },
};
// Статические параметры неисправности
dtcProperty_t dtcProp_PMOffline = { dtc_CAN_PM, DTC_BIT_WARNING_ENABLE, 0, 10, -10, 100, DIAG_ITEM(dVal_frzfPMOffline) };
// Все о неисправности
dtcItem_t dtcPMOffline = {&dtcProp_PMOffline};

// ************************************************************************************************
// DTC: Нет связи с VCU
// ************************************************************************************************
// Стоп-кадр
dtcFRZF_General frzfVcuOffline;
// Указатели на данные стоп-кадра (аналогично DiagnosticValue)
const DiagnosticValueFRZF dVal_frzfVcuOffline[] = 
{
	{ didFaults_FreezeFrame, DV_FRZF(frzfVcuOffline) },
};
// Статические параметры неисправности
dtcProperty_t dtcProp_VcuOffline = { dtc_CAN_mainVcu, DTC_BIT_WARNING_ENABLE, 0, 10, -10, 100, DIAG_ITEM(dVal_frzfVcuOffline) };
// Все о неисправности
dtcItem_t dtcVcuOffline = {&dtcProp_VcuOffline};


// ************************************************************************************************
// DTC: Таймаут надсистемы
// ************************************************************************************************
// Стоп-кадр
dtcFRZF_General frzfBatBmsTimeout;
// Указатели на данные стоп-кадра (аналогично DiagnosticValue)
const DiagnosticValueFRZF dVal_frzfBmsTimeout[] = 
{
	{ didFaults_FreezeFrame, DV_FRZF(frzfBatBmsTimeout) },
};
// Статические параметры неисправности
static dtcProperty_t dtcProp_BmsTimeout = { dtc_CAN_Bms, DTC_BIT_WARNING_ENABLE + DTC_BIT_OPERATION_DISABLE, 0, 10, -10, 200, DIAG_ITEM(dVal_frzfBmsTimeout) };
// Все о неисправности
dtcItem_t dtcBat_BmsTimeout = {&dtcProp_BmsTimeout};

// ************************************************************************************************
// DTC: Контакторы
// ************************************************************************************************
// Стоп-кадр
dtcFRZF_General frzfContactor;
// Указатели на данные стоп-кадра (аналогично DiagnosticValue)
const DiagnosticValueFRZF dVal_frzfContactor[] = 
{
	{ didFaults_FreezeFrame, DV_FRZF(frzfContactor) },
};
// Статические параметры неисправности
dtcProperty_t dtcProp_Contactor = { dtc_Mod_Contactor, DTC_BIT_WARNING_ENABLE + DTC_BIT_OPERATION_DISABLE, 0, 10, -10, 10, DIAG_ITEM(dVal_frzfContactor) };
// Все о неисправности
dtcItem_t dtcContactor = {&dtcProp_Contactor};


// ************************************************************************************************
// DTC: Напряжение ячейки вне диапазона
// ************************************************************************************************
// Стоп-кадр
dtcFRZF_General frzfCellVoltage;
// Указатели на данные стоп-кадра (аналогично DiagnosticValue)
const DiagnosticValueFRZF dVal_frzfCellVoltage[] = 
{
	{ didFaults_FreezeFrame, DV_FRZF(frzfCellVoltage) },
};
// Статические параметры неисправности
dtcProperty_t dtcProp_CellVoltage = { dtc_Mst_CellVoltageOutOfRange, DTC_BIT_WARNING_ENABLE + DTC_BIT_OPERATION_DISABLE, 0, 6, -6, 250, DIAG_ITEM(dVal_frzfCellVoltage) };
// Все о неисправности
dtcItem_t dtcCellVoltage = {&dtcProp_CellVoltage};


// ************************************************************************************************
// DTC: Температура ячейки вне диапазона
// ************************************************************************************************
// Стоп-кадр
dtcFRZF_General frzfCellTemperature;
// Указатели на данные стоп-кадра (аналогично DiagnosticValue)
const DiagnosticValueFRZF dVal_frzfCellTemperature[] = 
{
	{ didFaults_FreezeFrame, DV_FRZF(frzfCellTemperature) },
};
// Статические параметры неисправности
dtcProperty_t dtcProp_CellTemperature = { dtc_Mod_CellTempOutOfRange, DTC_BIT_WARNING_ENABLE, 0, 10, -10, 100, DIAG_ITEM(dVal_frzfCellTemperature) };
// Все о неисправности
dtcItem_t dtcCellTemperature = {&dtcProp_CellTemperature};


// ************************************************************************************************
// DTC: Обрыв канала измерения
// ************************************************************************************************
// Стоп-кадр
dtcFRZF_General frzfMeasuringCircuit;
// Указатели на данные стоп-кадра (аналогично DiagnosticValue)
const DiagnosticValueFRZF dVal_frzfMeasuringCircuit[] =
{
	{ didFaults_FreezeFrame, DV_FRZF(frzfMeasuringCircuit) },
};
// Статические параметры неисправности
dtcProperty_t dtcProp_MeasuringCircuit = { dtc_Mod_MeasuringCircuit, DTC_BIT_WARNING_ENABLE, 0, 1, -1, 10, DIAG_ITEM(dVal_frzfMeasuringCircuit) };
// Все о неисправности
dtcItem_t dtcMeasuringCircuit = {&dtcProp_MeasuringCircuit};




// ************************************************************************************************
// DTC: Неверное количество модулей батареи
// ************************************************************************************************
// Стоп-кадр
dtcFRZF_General frzfModuleNumber;
// Указатели на данные стоп-кадра (аналогично DiagnosticValue)
const DiagnosticValueFRZF dVal_frzfModuleNumber[] = 
{
	{ didFaults_FreezeFrame, DV_FRZF(frzfModuleNumber) },
};
// Статические параметры неисправности
dtcProperty_t dtcProp_ModuleNumber = { dtc_Bat_WrongModNumber, DTC_BIT_WARNING_ENABLE + DTC_BIT_OPERATION_DISABLE, 0, 5, -5, 250, DIAG_ITEM(dVal_frzfModuleNumber) };
// Все о неисправности
dtcItem_t dtcBat_WrongModNumber = {&dtcProp_ModuleNumber};


// ************************************************************************************************
// DTC: Неверное состояние модуля батареи
// ************************************************************************************************
// Стоп-кадр
dtcFRZF_General frzfModuleState;
// Указатели на данные стоп-кадра (аналогично DiagnosticValue)
const DiagnosticValueFRZF dVal_frzfModuleState[] = 
{
	{ didFaults_FreezeFrame, DV_FRZF(frzfModuleState) },
};
// Статические параметры неисправности
dtcProperty_t dtcProp_ModuleState = { dtc_Bat_WrongModState, DTC_BIT_WARNING_ENABLE + DTC_BIT_OPERATION_DISABLE, 0, 5, -5, 250, DIAG_ITEM(dVal_frzfModuleState) };
// Все о неисправности
dtcItem_t dtcBat_WrongModState = {&dtcProp_ModuleState};


// ************************************************************************************************
// DTC: Перегрузка батареи по току
// ************************************************************************************************
// Стоп-кадр
dtcFRZF_General frzfOverCurrent;
// Указатели на данные стоп-кадра (аналогично DiagnosticValue)
const DiagnosticValueFRZF dVal_frzfOverCurrent[] = 
{
	{ didFaults_FreezeFrame, DV_FRZF(frzfOverCurrent) },
};
// Статические параметры неисправности
dtcProperty_t dtcProp_OverCurrent = { dtc_Bat_OverCurrent, DTC_BIT_WARNING_ENABLE, 0, 10, -10, 250, DIAG_ITEM(dVal_frzfOverCurrent) };
// Все о неисправности
dtcItem_t dtcBat_OverCurrent = {&dtcProp_OverCurrent};


// ************************************************************************************************
// DTC: Разброс напряжений между модулями
// ************************************************************************************************
// Стоп-кадр
dtcFRZF_General frzfModuleVoltageDiff;
// Указатели на данные стоп-кадра (аналогично DiagnosticValue)
const DiagnosticValueFRZF dVal_frzfModVoltageDiff[] = 
{
	{ didFaults_FreezeFrame, DV_FRZF(frzfModuleVoltageDiff) },
};
// Статические параметры неисправности
dtcProperty_t dtcProp_ModVoltageDiff = { dtc_Bat_ModVoltageDiff, DTC_BIT_WARNING_ENABLE, 0, 10, -10, 250, DIAG_ITEM(dVal_frzfModVoltageDiff) };
// Все о неисправности
dtcItem_t dtcBat_VoltageDiff = {&dtcProp_ModVoltageDiff};


// ************************************************************************************************
// DTC: Ошибка предзаряда
// ************************************************************************************************
// Стоп-кадр
dtcFRZF_General frzfPrecharge;
// Указатели на данные стоп-кадра (аналогично DiagnosticValue)
const DiagnosticValueFRZF dVal_frzfPrecharge[] = 
{
	{ didFaults_FreezeFrame, DV_FRZF(frzfPrecharge) },
};
// Статические параметры неисправности
static dtcProperty_t dtcProp_Precharge = { dtc_Bat_Precharge, DTC_BIT_WARNING_ENABLE + DTC_BIT_OPERATION_DISABLE, 0, 15, -15, 100, DIAG_ITEM(dVal_frzfPrecharge) };
// Все о неисправности
dtcItem_t dtcBat_Precharge = {&dtcProp_Precharge};



// ************************************************************************************************
// DTC: Ошибка подключения датчика тока
// ************************************************************************************************
// Стоп-кадр
dtcFRZF_General frzfBatCurrentSensor;
// Указатели на данные стоп-кадра (аналогично DiagnosticValue)
const DiagnosticValueFRZF dVal_frzfCurrentSensor[] = 
{
	{ didFaults_FreezeFrame, DV_FRZF(frzfBatCurrentSensor) },
};
// Статические параметры неисправности
static dtcProperty_t dtcProp_CurrentSensor = { dtc_Bat_CurrentSensor, DTC_BIT_WARNING_ENABLE, 0, 10, -10, 50, DIAG_ITEM(dVal_frzfCurrentSensor) };
// Все о неисправности
dtcItem_t dtcBat_CurrentSensor = {&dtcProp_CurrentSensor};

// ************************************************************************************************
// DTC: Неверное количество батарей
// ************************************************************************************************
// Стоп-кадр
dtcFRZF_General frzfBatNumber;
// Указатели на данные стоп-кадра (аналогично DiagnosticValue)
const DiagnosticValueFRZF dVal_frzfBatNumber[] = 
{
	{ didFaults_FreezeFrame, DV_FRZF(frzfBatNumber) },
};
// Статические параметры неисправности
static dtcProperty_t dtcProp_BatNumber = { dtc_Mst_WrongBatNumber, DTC_BIT_WARNING_ENABLE + DTC_BIT_OPERATION_DISABLE, 0, 5, -5, 250, DIAG_ITEM(dVal_frzfBatNumber) };
// Все о неисправности
dtcItem_t dtcMst_BatNumber = {&dtcProp_BatNumber};


// ************************************************************************************************
// DTC: Неверное состояние одной из батарей
// ************************************************************************************************
// Стоп-кадр
dtcFRZF_General frzfBatState;
// Указатели на данные стоп-кадра (аналогично DiagnosticValue)
const DiagnosticValueFRZF dVal_frzfBatState[] = 
{
	{ didFaults_FreezeFrame, DV_FRZF(frzfBatNumber) },
};
// Статические параметры неисправности
static dtcProperty_t dtcProp_BatState = { dtc_Mst_WrongBatState, DTC_BIT_WARNING_ENABLE + DTC_BIT_OPERATION_DISABLE, 0, 3, -3, 250, DIAG_ITEM(dVal_frzfBatState) };
// Все о неисправности
dtcItem_t dtcMst_BatState = {&dtcProp_BatState};


// ************************************************************************************************
// DTC: Разброс напряжений батарей
// ************************************************************************************************
// Стоп-кадр
dtcFRZF_General frzfBatVoltageDiff;
// Указатели на данные стоп-кадра (аналогично DiagnosticValue)
const DiagnosticValueFRZF dVal_frzfBatVoltageDiff[] = 
{
	{ didFaults_FreezeFrame, DV_FRZF(frzfBatVoltageDiff) },
};
// Статические параметры неисправности
static dtcProperty_t dtcProp_BatVoltageDiff = { dtc_Mst_BatVoltageDiff, DTC_BIT_WARNING_ENABLE, 0, 5, -5, 100, DIAG_ITEM(dVal_frzfBatVoltageDiff) };
// Все о неисправности
dtcItem_t dtcMst_BatVoltageDiff = {&dtcProp_BatVoltageDiff};



// Итоговый список неисправностей
dtcItem_t* dtcList[] =
{
	&dtcEcuConfigMemory, &dtcUnexpectedPowerOff, &dtcVcuOffline, &dtcInterlock, &dtcContactor, &dtcCellTemperature, &dtcMeasuringCircuit,
	&dtcBat_WrongModNumber, &dtcBat_WrongModState, &dtcBat_OverCurrent, &dtcBat_VoltageDiff, &dtcBat_Precharge, &dtcBat_CurrentSensor,
	&dtcMst_BatNumber, &dtcMst_BatState, &dtcMst_BatVoltageDiff, &dtcCellVoltage, &dtcPMOffline,
};

// Количество элементов списка неисправностей
int dtcListSize = ARRAY_LEN(dtcList);



void ecuInit(ObjectDictionary_t *dictionary)
{
	//dtcListSize = ARRAY_LEN(dtcList);	
    cfgApply();		
	
	EcuConfig_t _config = GetConfigInstance();
	
    dictionary->DelayValues.Time1_ms = 1;
    dictionary->DelayValues.Time10_ms = 10;
    dictionary->DelayValues.Time100_ms = 100;
    dictionary->DelayValues.Time1_s = 1000;
    dictionary->DelayValues.MainLoopTime_ms = 10;	
	
	dictionary->EcuInfo[0] = CLASS_MODEL_ID;
	dictionary->EcuInfo[1] = HARDWARE;
	dictionary->EcuInfo[2] = FW_VERSION;
	dictionary->EcuInfo[3] = HW_VERSION;
	
	dictionary->BatteryData[_config.BatteryIndex].Type = type_Pack;
	dictionary->MasterData.Type = type_Master;
	
	// Применение ошибок
	if(!_config.CheckContactor)
		dtcContactor.Property->Bits.IsCritical = 0;
	else
		dtcContactor.Property->Bits.IsCritical = 1;
	
	dictionary->ecuIndex = _config.DiagnosticID + _config.ModuleIndex + (MAX_MODULE_NUM * _config.BatteryIndex);	
	
	// Настройка лимита предзаряда
	dtcBat_Precharge.Property->TestSamplePeriod = _config.PreMaxDuration / dtcBat_Precharge.Property->TestFailedThreshold;
	
	// Инициализация измерения напряжения ячеек
	VoltageSensorParams_t p1;
	p1.BalancingMinVoltage = _config.MinVoltageForBalancing;
	p1.CellNumber = (_config.CellNumber > 12)? 12 :  _config.CellNumber;
	p1.CellTargetVoltage = UINT16_MAX;
	p1.ChipAddress = 0;
	p1.ChipEnableOut = CS1_OUT;   
	p1.BalancingTime_s = _config.BalancingTime_s;
	p1.MaxVoltageDiff_mV = _config.MaxVoltageDiff;
	
	VoltageSensorParams_t p2;
	p2.BalancingMinVoltage = _config.MinVoltageForBalancing;
	p2.CellNumber = (_config.CellNumber > 12)? _config.CellNumber - 12 :  0;
	p2.CellTargetVoltage = UINT16_MAX;
	p2.ChipAddress = 1;
	p2.ChipEnableOut = CS2_OUT;		
	p2.BalancingTime_s = _config.BalancingTime_s;
	p1.MaxVoltageDiff_mV = _config.MaxVoltageDiff;
	
	vs_init(0, &p1);	
	vs_init(1, &p2);

	// Фильтр питания ECU
	Filter_init(50, 1, &fltVoltage);
}

