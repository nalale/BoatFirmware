#include "dVCU_ECU.h"

#include "Main.h"
#include "PwmFunc.h"
#include "FaultTools.h"

#include "../MolniaLib/MF_Tools.h"
#include "../Libs/max11612.h"
#include "../Libs/LTC6803.h"
#include "../Libs/Btn8982.h"
#include "../Libs/TLE_6368g2.h"
#include "../Libs/filter.h"

#include "EcuConfig.h"


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
dtcProperty_t dtcProp_UnexpectedPowerOff = { dtc_General_UnexpectedPowerOff, DTC_BIT_NONE, 0, 1, -1, 50, DIAG_ITEM(dVal_frzfUnexpectedPowerOff) };
// Все о неисправности
dtcItem_t dtcUnexpectedPowerOff = {&dtcProp_UnexpectedPowerOff};


// ************************************************************************************************
// DTC: Нет связи с  батареей
// ************************************************************************************************
// Стоп-кадр
dtcFRZF_General frzfBatteryOffline;
// Указатели на данные стоп-кадра (аналогично DiagnosticValue)
const DiagnosticValueFRZF dVal_frzfBatteryOffline[] =
{
	{ didFaults_FreezeFrame, DV_FRZF(frzfBatteryOffline) },
};
// Статические параметры неисправности
dtcProperty_t dtcProp_BatteryOffline = { dtc_CAN_Battery, DTC_BIT_WARNING_ENABLE, 0, 4, -4, 500, DIAG_ITEM(dVal_frzfBatteryOffline) };
// Все о неисправности
dtcItem_t dtcBatteryOffline = {&dtcProp_BatteryOffline};

// ************************************************************************************************
// DTC: Нет связи с  main Ecu
// ************************************************************************************************
// Стоп-кадр
dtcFRZF_General frzfmEcuOffline;
// Указатели на данные стоп-кадра (аналогично DiagnosticValue)
const DiagnosticValueFRZF dVal_frzfmEcuCanOffline[] =
{
	{ didFaults_FreezeFrame, DV_FRZF(frzfmEcuOffline) },
};
// Статические параметры неисправности
dtcProperty_t dtcProp_mEcuOffline = { dtc_CAN_mEcu, DTC_BIT_WARNING_ENABLE, 0, 4, -4, 500, DIAG_ITEM(dVal_frzfmEcuCanOffline) };
// Все о неисправности
dtcItem_t dtcmEcuOffline = {&dtcProp_mEcuOffline};


// ************************************************************************************************
// DTC: PWM канал 1
// ************************************************************************************************
// Стоп-кадр
dtcFRZF_General frzfPwmCircuit_1;
// Указатели на данные стоп-кадра (аналогично DiagnosticValue)
const DiagnosticValueFRZF dVal_frzfPwmCircuit_1[] = 
{
	{ didFaults_FreezeFrame, DV_FRZF(frzfPwmCircuit_1) },
};
// Статические параметры неисправности
dtcProperty_t dtcProp_PwmCircuit_1 = { dtc_PwmCircuit_1, DTC_BIT_WARNING_ENABLE, 0, 5, -5, 10, DIAG_ITEM(dVal_frzfPwmCircuit_1) };
// Все о неисправности
dtcItem_t dtcPwmCircuit_1 = {&dtcProp_PwmCircuit_1};


// ************************************************************************************************
// DTC: PWM канал 2
// ************************************************************************************************
// Стоп-кадр
dtcFRZF_General frzfPwmCircuit_2;
// Указатели на данные стоп-кадра (аналогично DiagnosticValue)
const DiagnosticValueFRZF dVal_frzfPwmCircuit_2[] = 
{
	{ didFaults_FreezeFrame, DV_FRZF(frzfPwmCircuit_2) },
};
// Статические параметры неисправности
dtcProperty_t dtcProp_PwmCircuit_2 = { dtc_PwmCircuit_2, DTC_BIT_WARNING_ENABLE, 0, 5, -5, 10, DIAG_ITEM(dVal_frzfPwmCircuit_2) };
// Все о неисправности
dtcItem_t dtcPwmCircuit_2 = {&dtcProp_PwmCircuit_2};


// ************************************************************************************************
// DTC: PWM канал 3
// ************************************************************************************************
// Стоп-кадр
dtcFRZF_General frzfPwmCircuit_3;
// Указатели на данные стоп-кадра (аналогично DiagnosticValue)
const DiagnosticValueFRZF dVal_frzfPwmCircuit_3[] = 
{
	{ didFaults_FreezeFrame, DV_FRZF(frzfPwmCircuit_3) },
};
// Статические параметры неисправности
dtcProperty_t dtcProp_PwmCircuit_3 = { dtc_PwmCircuit_3, DTC_BIT_WARNING_ENABLE, 0, 5, -5, 10, DIAG_ITEM(dVal_frzfPwmCircuit_3) };
// Все о неисправности
dtcItem_t dtcPwmCircuit_3 = {&dtcProp_PwmCircuit_3};


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
dtcProperty_t dtcProp_MeasuringCircuit = { dtc_MeasuringCircuit, DTC_BIT_WARNING_ENABLE, 0, 5, -5, 10, DIAG_ITEM(dVal_frzfMeasuringCircuit) };
// Все о неисправности
dtcItem_t dtcMeasuringCircuit = {&dtcProp_MeasuringCircuit};

// ************************************************************************************************
// DTC: Обрыв канала измерения
// ************************************************************************************************
// Стоп-кадр
dtcFRZF_General frzfPowerSupplyCircuit;
// Указатели на данные стоп-кадра (аналогично DiagnosticValue)
const DiagnosticValueFRZF dVal_frzfPowerSupplyCircuit[] =
{
	{ didFaults_FreezeFrame, DV_FRZF(frzfPowerSupplyCircuit) },
};
// Статические параметры неисправности
dtcProperty_t dtcProp_PowerSupplyCircuit = { dtc_PowerSupplyCircuit, DTC_BIT_WARNING_ENABLE, 0, 5, -1, 100, DIAG_ITEM(dVal_frzfPowerSupplyCircuit) };
// Все о неисправности
dtcItem_t dtcPowerSupplyCircuit = {&dtcProp_PowerSupplyCircuit};

// Итоговый список неисправностей
dtcItem_t* dtcList[] =
{
	&dtcEcuConfigMemory, &dtcUnexpectedPowerOff, &dtcmEcuOffline, &dtcBatteryOffline, &dtcPwmCircuit_1, &dtcPwmCircuit_2, &dtcPwmCircuit_3,
	&dtcMeasuringCircuit,
};

// Количество элементов списка неисправностей
int dtcListSize = ARRAY_LEN(dtcList);



void ecuInit(ObjectDictionary_t *dictionary)
{
	cfgApply(&dictionary->cfg);

    dictionary->DelayValues.Time1_ms = 1;
    dictionary->DelayValues.Time10_ms = 10;
    dictionary->DelayValues.Time100_ms = 100;
    dictionary->DelayValues.Time1_s = 1000;
    dictionary->DelayValues.MainLoopTime_ms = 10;
	
	dictionary->ecuIndex = dictionary->cfg->DiagnosticID + dictionary->cfg->Index;
    
	Max11612_Init();
	
	btnInit(PWM_Channel1, A_OUT1_CSENS, 20);
	btnInit(PWM_Channel2, A_OUT2_CSENS, 20);
	btnInit(PWM_Channel3, A_OUT3_CSENS, 20);
	btnInit(PWM_Channel4, 0xff, 0xffff);
	
	dictionary->EcuInfo[0] = CLASS_MODEL_ID;
	dictionary->EcuInfo[1] = HARDWARE;
	dictionary->EcuInfo[2] = FW_VERSION;
	dictionary->EcuInfo[3] = HW_VERSION;
}




