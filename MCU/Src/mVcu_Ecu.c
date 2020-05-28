#include "Main.h"
#include "PwmFunc.h"
#include "../MolniaLib/MF_Tools.h"
#include "../Libs/max11612.h"
#include "../Libs/LTC6803.h"
#include "../Libs/Btn8982.h"
#include "../Libs/TLE_6368g2.h"
#include "../Libs/filter.h"

#include "mVCU_ECU.h"
#include "EcuConfig.h"
#include "FaultTools.h"

#include "../BoardDefinitions/MarineEcu_Board.h"


#define DV_FRZF(val)			sizeof(val), &val
#define DIAG_ITEM(val)		ARRAY_LEN(val),  (void*)&val


FILTER_STRUCT fltVoltage;

// ECU Variables
static uint16_t _ecuPowerSupply = 0;


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
dtcProperty_t dtcProp_EcuConfigMemory = { dtc_General_EcuConfig, DTC_BIT_WARNING_ENABLE + DTC_BIT_OPERATION_DISABLE, 0, 1, -1, 10, DIAG_ITEM(dVal_frzfEcuConfigMemory) };
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
dtcProperty_t dtcProp_UnexpectedPowerOff = { dtc_General_UnexpectedPowerOff, DTC_BIT_NONE, 0, 1, -1, 10, DIAG_ITEM(dVal_frzfUnexpectedPowerOff) };
// Все о неисправности
dtcItem_t dtcUnexpectedPowerOff = {&dtcProp_UnexpectedPowerOff};


// ************************************************************************************************
// DTC: InverterOffline
// ************************************************************************************************
// Стоп-кадр
dtcFRZF_General frzfInverterOffline;
// Указатели на данные стоп-кадра (аналогично DiagnosticValue)
const DiagnosticValueFRZF dVal_frzfInverterOffline[] = 
{
	{ didFaults_FreezeFrame, DV_FRZF(frzfInverterOffline) },
};
// Статические параметры неисправности
dtcProperty_t dtcProp_InverterOffline = { dtc_CAN_Inverter, DTC_BIT_WARNING_ENABLE, 0, 1, -1, 1000, DIAG_ITEM(dVal_frzfInverterOffline) };
// Все о неисправности
dtcItem_t dtcInverterOffline = {&dtcProp_InverterOffline};

// ************************************************************************************************
// DTC: Нет связи с  SKF
// ************************************************************************************************
// Стоп-кадр
dtcFRZF_General frzfSkfOffline;
// Указатели на данные стоп-кадра (аналогично DiagnosticValue)
const DiagnosticValueFRZF dVal_frzfSkfOffline[] = 
{
	{ didFaults_FreezeFrame, DV_FRZF(frzfSkfOffline) },
};
// Статические параметры неисправности
dtcProperty_t dtcProp_SkfOffline = { dtc_CAN_Skf, DTC_BIT_WARNING_ENABLE, 0, 1, -1, 1000, DIAG_ITEM(dVal_frzfSkfOffline) };
// Все о неисправности
dtcItem_t dtcSkfOffline = {&dtcProp_SkfOffline};

// ************************************************************************************************
// DTC: Нет связи с  Battery
// ************************************************************************************************
// Стоп-кадр
dtcFRZF_General frzfBatteryOffline;
// Указатели на данные стоп-кадра (аналогично DiagnosticValue)
const DiagnosticValueFRZF dVal_frzfBatteryOffline[] = 
{
	{ didFaults_FreezeFrame, DV_FRZF(frzfBatteryOffline) },
};
// Статические параметры неисправности
dtcProperty_t dtcProp_BatteryOffline = { dtc_CAN_Battery, DTC_BIT_WARNING_ENABLE, 0, 5, -1, 1000, DIAG_ITEM(dVal_frzfBatteryOffline) };
// Все о неисправности
dtcItem_t dtcBatteryOffline = {&dtcProp_BatteryOffline};


// ************************************************************************************************
// DTC: Battery Fault
// ************************************************************************************************
// Стоп-кадр
dtcFRZF_General frzfBatteryFault;
// Указатели на данные стоп-кадра (аналогично DiagnosticValue)
const DiagnosticValueFRZF dVal_frzfBatteryFault[] = 
{
	{ didFaults_FreezeFrame, DV_FRZF(frzfBatteryFault) },
};
// Статические параметры неисправности
dtcProperty_t dtcProp_BatteryFault = { dtc_BatteryFault, DTC_BIT_WARNING_ENABLE, 0, 1, -1, 200, DIAG_ITEM(dVal_frzfBatteryFault) };
// Все о неисправности
dtcItem_t dtcBatteryFault = {&dtcProp_BatteryFault};

// ************************************************************************************************
// DTC: Inverter Fault
// ************************************************************************************************
// Стоп-кадр
dtcFRZF_General frzfInverterFault;
// Указатели на данные стоп-кадра (аналогично DiagnosticValue)
const DiagnosticValueFRZF dVal_frzfInverterFault[] = 
{
	{ didFaults_FreezeFrame, DV_FRZF(frzfInverterFault) },
};
// Статические параметры неисправности
dtcProperty_t dtcProp_InverterFault = { dtc_InverterFault, DTC_BIT_WARNING_ENABLE, 0, 1, -1, 250, DIAG_ITEM(dVal_frzfInverterFault) };
// Все о неисправности
dtcItem_t dtcInverterFault = {&dtcProp_InverterFault};


// ************************************************************************************************
// DTC: SteeringPosition
// ************************************************************************************************
// Стоп-кадр
dtcFRZF_General frzfSteeringPosition;
// Указатели на данные стоп-кадра (аналогично DiagnosticValue)
const DiagnosticValueFRZF dVal_frzfSteeringPosition[] = 
{
	{ didFaults_FreezeFrame, DV_FRZF(frzfSteeringPosition) },
};
// Статические параметры неисправности
dtcProperty_t dtcProp_SteeringPosition = { dtc_SteeringPosition, DTC_BIT_WARNING_ENABLE, 0, 10, -10, 100, DIAG_ITEM(dVal_frzfSteeringPosition) };
// Все о неисправности
dtcItem_t dtcSteeringPosition = {&dtcProp_SteeringPosition};

// ************************************************************************************************
// DTC: TrimPosition
// ************************************************************************************************
// Стоп-кадр
dtcFRZF_General frzfTrimPosition;
// Указатели на данные стоп-кадра (аналогично DiagnosticValue)
const DiagnosticValueFRZF dVal_frzfTrimPosition[] = 
{
	{ didFaults_FreezeFrame, DV_FRZF(frzfTrimPosition) },
};
// Статические параметры неисправности
dtcProperty_t dtcProp_TrimPosition = { dtc_TrimPosition, DTC_BIT_WARNING_ENABLE, 0, 10, -10, 100, DIAG_ITEM(dVal_frzfTrimPosition) };
// Все о неисправности
dtcItem_t dtcTrimPosition = {&dtcProp_TrimPosition};

// ************************************************************************************************
// DTC: Accelerator
// ************************************************************************************************
// Стоп-кадр
dtcFRZF_General frzfAcceleratorPosition;
// Указатели на данные стоп-кадра (аналогично DiagnosticValue)
const DiagnosticValueFRZF dVal_frzfAccelerator[] = 
{
	{ didFaults_FreezeFrame, DV_FRZF(frzfAcceleratorPosition) },
};
// Статические параметры неисправности
dtcProperty_t dtcProp_Accelerator = { dtc_Accelerator, DTC_BIT_WARNING_ENABLE + DTC_BIT_OPERATION_DISABLE, 0, 10, -10, 50, DIAG_ITEM(dVal_frzfAccelerator) };
// Все о неисправности
dtcItem_t dtcAcceleratorPosition = {&dtcProp_Accelerator};

// ************************************************************************************************
// DTC: SteeringFeedback
// ************************************************************************************************
// Стоп-кадр
dtcFRZF_General frzfSteeringFeedBack;
// Указатели на данные стоп-кадра (аналогично DiagnosticValue)
const DiagnosticValueFRZF dVal_frzfSteeringFeedBack[] = 
{
	{ didFaults_FreezeFrame, DV_FRZF(frzfSteeringFeedBack) },
};
// Статические параметры неисправности
dtcProperty_t dtcProp_SteeringFeedBack = { dtc_SteeringFeedback, DTC_BIT_WARNING_ENABLE, 0, 5, -5, 100, DIAG_ITEM(dVal_frzfSteeringFeedBack) };
// Все о неисправности
dtcItem_t dtcSteeringFeedBack = {&dtcProp_SteeringFeedBack};

// ************************************************************************************************
// DTC: Trim Feedback
// ************************************************************************************************
// Стоп-кадр
dtcFRZF_General frzfTrimFeedBack;
// Указатели на данные стоп-кадра (аналогично DiagnosticValue)
const DiagnosticValueFRZF dVal_frzfTrimFeedBack[] = 
{
	{ didFaults_FreezeFrame, DV_FRZF(frzfTrimFeedBack) },
};
// Статические параметры неисправности
dtcProperty_t dtcProp_TrimFeedBack = { dtc_TrimFeedback, DTC_BIT_WARNING_ENABLE, 0, 5, -5, 100, DIAG_ITEM(dVal_frzfTrimFeedBack) };
// Все о неисправности
dtcItem_t dtcTrimFeedBack = {&dtcProp_TrimFeedBack};





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
	{ didFaults_FreezeFrame, DV_FRZF(frzfMeasuringCircuit) },
};
// Статические параметры неисправности
dtcProperty_t dtcProp_PowerSupplyCircuit = { dtc_PowerSupplyCircuit, DTC_BIT_WARNING_ENABLE, 0, 10, -1, 5, DIAG_ITEM(dVal_frzfPowerSupplyCircuit) };
// Все о неисправности
dtcItem_t dtcPowerSupplyCircuit = {&dtcProp_PowerSupplyCircuit};



// Итоговый список неисправностей
dtcItem_t* dtcList[] =
{
	&dtcEcuConfigMemory, &dtcUnexpectedPowerOff, &dtcInverterOffline, &dtcSkfOffline, &dtcBatteryOffline, 
	&dtcBatteryFault, &dtcInverterFault, 
	&dtcSteeringPosition, &dtcSteeringPosition, &dtcAcceleratorPosition, &dtcSteeringFeedBack, &dtcTrimFeedBack,
	&dtcPwmCircuit_1, &dtcPwmCircuit_2, &dtcPwmCircuit_3, &dtcMeasuringCircuit, &dtcPowerSupplyCircuit,
};

// Количество элементов списка неисправностей
int dtcListSize = ARRAY_LEN(dtcList);



void ecuInit(ObjectDictionary_t *dictionary)
{
	cfgApply();
	
	dictionary->SData.cfgData = OD.cfgEcu;
	OD.ecuIndex = OD.cfgEcu->DiagnosticID;
	OD.MaxMotorSpeed = OD.cfgEcu->MaxMotorSpeedD;
	OD.MaxMotorTorque = OD.cfgEcu->MaxMotorTorque;

	SET_PU_D_IN1(1);
	SET_PU_D_IN2(1);
	SET_PU_D_IN3(1);
	SET_PU_D_IN4(1);

    dictionary->DelayValues.Time1_ms = 1;
    dictionary->DelayValues.Time10_ms = 10;
    dictionary->DelayValues.Time100_ms = 100;
    dictionary->DelayValues.Time1_s = 1000;
    dictionary->DelayValues.MainLoopTime_ms = 10;
    
	dictionary->EcuInfo[0] = CLASS_MODEL_ID;
	dictionary->EcuInfo[1] = HARDWARE;
	dictionary->EcuInfo[2] = FW_VERSION;
	dictionary->EcuInfo[3] = HW_VERSION;

    Max11612_Init();
	
	btnInit(0, A_OUT1_CSENS, 400);
	btnInit(1, A_OUT2_CSENS, 400);
	btnInit(2, A_OUT3_CSENS, 400);
	btnInit(3, 0xff, 0xffff);
	// Фильтр питания ECU
	Filter_init(50, 1, &fltVoltage);
}

void boardThread()
{
	// 5.7 - делитель напряжения.
	uint16_t voltage_mV = Filter((GetVoltageValue(A_CHNL_KEY)) * 57 / 10 , &fltVoltage);
	_ecuPowerSupply = voltage_mV / 100;
	
	OD.IO = boardMarineECU_GetDiscreteIO();    
}

uint16_t EcuGetVoltage()
{
	return _ecuPowerSupply;
}


