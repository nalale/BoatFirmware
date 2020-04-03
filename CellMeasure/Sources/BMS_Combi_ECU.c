#include "Main.h"
#include "FaultTools.h"
#include "BMS_Combi_ECU.h"

#include "../MolniaLib/MF_Tools.h"
#include "../Libs/LTC6803.h"
#include "../Libs/filter.h"

#define DV_FRZF(val)			sizeof(val), &val
#define DIAG_ITEM(val)		ARRAY_LEN(val),  (void*)&val

// ************************************************************************************************
// DTC: ������ ������ ����������
// ************************************************************************************************
// ����-����
dtcFRZF_General frzfEcuConfigMemory;
// ��������� �� ������ ����-����� (���������� DiagnosticValue)
const DiagnosticValueFRZF dVal_frzfEcuConfigMemory[] = 
{
	{ didFaults_FreezeFrame, DV_FRZF(frzfEcuConfigMemory) },
};
// ����������� ��������� �������������
dtcProperty_t dtcProp_EcuConfigMemory = { dtc_General_EcuConfig, DTC_BIT_WARNING_ENABLE + DTC_BIT_OPERATION_DISABLE, 0, 50, -50, 1, DIAG_ITEM(dVal_frzfEcuConfigMemory) };
// ��� � �������������
dtcItem_t dtcEcuConfigMemory = {&dtcProp_EcuConfigMemory};


// ************************************************************************************************
// DTC: ����������� ���������� �������
// ************************************************************************************************
// ����-����
dtcFRZF_General frzfUnexpectedPowerOff;
// ��������� �� ������ ����-����� (���������� DiagnosticValue)
const DiagnosticValueFRZF dVal_frzfUnexpectedPowerOff[] = 
{
	{ didFaults_FreezeFrame, DV_FRZF(frzfUnexpectedPowerOff) },
};
// ����������� ��������� �������������
dtcProperty_t dtcProp_UnexpectedPowerOff = { dtc_General_UnexpectedPowerOff, DTC_BIT_NONE, 0, 50, -50, 1, DIAG_ITEM(dVal_frzfUnexpectedPowerOff) };
// ��� � �������������
dtcItem_t dtcUnexpectedPowerOff = {&dtcProp_UnexpectedPowerOff};


// ************************************************************************************************
// DTC: ��������
// ************************************************************************************************
// ����-����
dtcFRZF_General frzfInterlock;
// ��������� �� ������ ����-����� (���������� DiagnosticValue)
const DiagnosticValueFRZF dVal_frzfInterlock[] = 
{
	{ didFaults_FreezeFrame, DV_FRZF(frzfInterlock) },
};
// ����������� ��������� �������������
dtcProperty_t dtcProp_Interlock = { dtc_General_Interlock, DTC_BIT_NONE, 0, 50, -50, 1, DIAG_ITEM(dVal_frzfInterlock) };
// ��� � �������������
dtcItem_t dtcInterlock = {&dtcProp_Interlock};

// ************************************************************************************************
// DTC: ��� ����� � PM
// ************************************************************************************************
// ����-����
dtcFRZF_General frzfPMOffline;
// ��������� �� ������ ����-����� (���������� DiagnosticValue)
const DiagnosticValueFRZF dVal_frzfPMOffline[] = 
{
	{ didFaults_FreezeFrame, DV_FRZF(frzfPMOffline) },
};
// ����������� ��������� �������������
dtcProperty_t dtcProp_PMOffline = { dtc_CAN_PM, DTC_BIT_WARNING_ENABLE, 0, 10, -10, 100, DIAG_ITEM(dVal_frzfPMOffline) };
// ��� � �������������
dtcItem_t dtcPMOffline = {&dtcProp_PMOffline};

// ************************************************************************************************
// DTC: ��� ����� � VCU
// ************************************************************************************************
// ����-����
dtcFRZF_General frzfVcuOffline;
// ��������� �� ������ ����-����� (���������� DiagnosticValue)
const DiagnosticValueFRZF dVal_frzfVcuOffline[] = 
{
	{ didFaults_FreezeFrame, DV_FRZF(frzfVcuOffline) },
};
// ����������� ��������� �������������
dtcProperty_t dtcProp_VcuOffline = { dtc_CAN_mainVcu, DTC_BIT_WARNING_ENABLE, 0, 10, -10, 100, DIAG_ITEM(dVal_frzfVcuOffline) };
// ��� � �������������
dtcItem_t dtcVcuOffline = {&dtcProp_VcuOffline};


// ************************************************************************************************
// DTC: ������� ����������
// ************************************************************************************************
// ����-����
dtcFRZF_General frzfBatBmsTimeout;
// ��������� �� ������ ����-����� (���������� DiagnosticValue)
const DiagnosticValueFRZF dVal_frzfBmsTimeout[] = 
{
	{ didFaults_FreezeFrame, DV_FRZF(frzfBatBmsTimeout) },
};
// ����������� ��������� �������������
static dtcProperty_t dtcProp_BmsTimeout = { dtc_CAN_Bms, DTC_BIT_WARNING_ENABLE + DTC_BIT_OPERATION_DISABLE, 0, 10, -10, 200, DIAG_ITEM(dVal_frzfBmsTimeout) };
// ��� � �������������
dtcItem_t dtcBat_BmsTimeout = {&dtcProp_BmsTimeout};

// ************************************************************************************************
// DTC: ����������
// ************************************************************************************************
// ����-����
dtcFRZF_General frzfContactor;
// ��������� �� ������ ����-����� (���������� DiagnosticValue)
const DiagnosticValueFRZF dVal_frzfContactor[] = 
{
	{ didFaults_FreezeFrame, DV_FRZF(frzfContactor) },
};
// ����������� ��������� �������������
dtcProperty_t dtcProp_Contactor = { dtc_Mod_Contactor, DTC_BIT_WARNING_ENABLE + DTC_BIT_OPERATION_DISABLE, 0, 10, -10, 10, DIAG_ITEM(dVal_frzfContactor) };
// ��� � �������������
dtcItem_t dtcContactor = {&dtcProp_Contactor};


// ************************************************************************************************
// DTC: ���������� ������ ��� ���������
// ************************************************************************************************
// ����-����
dtcFRZF_General frzfCellVoltage;
// ��������� �� ������ ����-����� (���������� DiagnosticValue)
const DiagnosticValueFRZF dVal_frzfCellVoltage[] = 
{
	{ didFaults_FreezeFrame, DV_FRZF(frzfCellVoltage) },
};
// ����������� ��������� �������������
dtcProperty_t dtcProp_CellVoltage = { dtc_Mst_CellVoltageOutOfRange, DTC_BIT_WARNING_ENABLE + DTC_BIT_OPERATION_DISABLE, 0, 6, -6, 250, DIAG_ITEM(dVal_frzfCellVoltage) };
// ��� � �������������
dtcItem_t dtcCellVoltage = {&dtcProp_CellVoltage};


// ************************************************************************************************
// DTC: ����������� ������ ��� ���������
// ************************************************************************************************
// ����-����
dtcFRZF_General frzfCellTemperature;
// ��������� �� ������ ����-����� (���������� DiagnosticValue)
const DiagnosticValueFRZF dVal_frzfCellTemperature[] = 
{
	{ didFaults_FreezeFrame, DV_FRZF(frzfCellTemperature) },
};
// ����������� ��������� �������������
dtcProperty_t dtcProp_CellTemperature = { dtc_Mod_CellTempOutOfRange, DTC_BIT_WARNING_ENABLE, 0, 10, -10, 100, DIAG_ITEM(dVal_frzfCellTemperature) };
// ��� � �������������
dtcItem_t dtcCellTemperature = {&dtcProp_CellTemperature};


// ************************************************************************************************
// DTC: ����� ������ ���������
// ************************************************************************************************
// ����-����
dtcFRZF_General frzfMeasuringCircuit;
// ��������� �� ������ ����-����� (���������� DiagnosticValue)
const DiagnosticValueFRZF dVal_frzfMeasuringCircuit[] =
{
	{ didFaults_FreezeFrame, DV_FRZF(frzfMeasuringCircuit) },
};
// ����������� ��������� �������������
dtcProperty_t dtcProp_MeasuringCircuit = { dtc_Mod_MeasuringCircuit, DTC_BIT_WARNING_ENABLE, 0, 1, -1, 10, DIAG_ITEM(dVal_frzfMeasuringCircuit) };
// ��� � �������������
dtcItem_t dtcMeasuringCircuit = {&dtcProp_MeasuringCircuit};




// ************************************************************************************************
// DTC: �������� ���������� ������� �������
// ************************************************************************************************
// ����-����
dtcFRZF_General frzfModuleNumber;
// ��������� �� ������ ����-����� (���������� DiagnosticValue)
const DiagnosticValueFRZF dVal_frzfModuleNumber[] = 
{
	{ didFaults_FreezeFrame, DV_FRZF(frzfModuleNumber) },
};
// ����������� ��������� �������������
dtcProperty_t dtcProp_ModuleNumber = { dtc_Bat_WrongModNumber, DTC_BIT_WARNING_ENABLE + DTC_BIT_OPERATION_DISABLE, 0, 5, -5, 250, DIAG_ITEM(dVal_frzfModuleNumber) };
// ��� � �������������
dtcItem_t dtcBat_WrongModNumber = {&dtcProp_ModuleNumber};


// ************************************************************************************************
// DTC: �������� ��������� ������ �������
// ************************************************************************************************
// ����-����
dtcFRZF_General frzfModuleState;
// ��������� �� ������ ����-����� (���������� DiagnosticValue)
const DiagnosticValueFRZF dVal_frzfModuleState[] = 
{
	{ didFaults_FreezeFrame, DV_FRZF(frzfModuleState) },
};
// ����������� ��������� �������������
dtcProperty_t dtcProp_ModuleState = { dtc_Bat_WrongModState, DTC_BIT_WARNING_ENABLE + DTC_BIT_OPERATION_DISABLE, 0, 5, -5, 250, DIAG_ITEM(dVal_frzfModuleState) };
// ��� � �������������
dtcItem_t dtcBat_WrongModState = {&dtcProp_ModuleState};


// ************************************************************************************************
// DTC: ���������� ������� �� ����
// ************************************************************************************************
// ����-����
dtcFRZF_General frzfOverCurrent;
// ��������� �� ������ ����-����� (���������� DiagnosticValue)
const DiagnosticValueFRZF dVal_frzfOverCurrent[] = 
{
	{ didFaults_FreezeFrame, DV_FRZF(frzfOverCurrent) },
};
// ����������� ��������� �������������
dtcProperty_t dtcProp_OverCurrent = { dtc_Bat_OverCurrent, DTC_BIT_WARNING_ENABLE, 0, 10, -10, 250, DIAG_ITEM(dVal_frzfOverCurrent) };
// ��� � �������������
dtcItem_t dtcBat_OverCurrent = {&dtcProp_OverCurrent};


// ************************************************************************************************
// DTC: ������� ���������� ����� ��������
// ************************************************************************************************
// ����-����
dtcFRZF_General frzfModuleVoltageDiff;
// ��������� �� ������ ����-����� (���������� DiagnosticValue)
const DiagnosticValueFRZF dVal_frzfModVoltageDiff[] = 
{
	{ didFaults_FreezeFrame, DV_FRZF(frzfModuleVoltageDiff) },
};
// ����������� ��������� �������������
dtcProperty_t dtcProp_ModVoltageDiff = { dtc_Bat_ModVoltageDiff, DTC_BIT_WARNING_ENABLE, 0, 10, -10, 250, DIAG_ITEM(dVal_frzfModVoltageDiff) };
// ��� � �������������
dtcItem_t dtcBat_VoltageDiff = {&dtcProp_ModVoltageDiff};


// ************************************************************************************************
// DTC: ������ ����������
// ************************************************************************************************
// ����-����
dtcFRZF_General frzfPrecharge;
// ��������� �� ������ ����-����� (���������� DiagnosticValue)
const DiagnosticValueFRZF dVal_frzfPrecharge[] = 
{
	{ didFaults_FreezeFrame, DV_FRZF(frzfPrecharge) },
};
// ����������� ��������� �������������
static dtcProperty_t dtcProp_Precharge = { dtc_Bat_Precharge, DTC_BIT_WARNING_ENABLE + DTC_BIT_OPERATION_DISABLE, 0, 15, -15, 100, DIAG_ITEM(dVal_frzfPrecharge) };
// ��� � �������������
dtcItem_t dtcBat_Precharge = {&dtcProp_Precharge};



// ************************************************************************************************
// DTC: ������ ����������� ������� ����
// ************************************************************************************************
// ����-����
dtcFRZF_General frzfBatCurrentSensor;
// ��������� �� ������ ����-����� (���������� DiagnosticValue)
const DiagnosticValueFRZF dVal_frzfCurrentSensor[] = 
{
	{ didFaults_FreezeFrame, DV_FRZF(frzfBatCurrentSensor) },
};
// ����������� ��������� �������������
static dtcProperty_t dtcProp_CurrentSensor = { dtc_Bat_CurrentSensor, DTC_BIT_WARNING_ENABLE, 0, 10, -10, 50, DIAG_ITEM(dVal_frzfCurrentSensor) };
// ��� � �������������
dtcItem_t dtcBat_CurrentSensor = {&dtcProp_CurrentSensor};

// ************************************************************************************************
// DTC: �������� ���������� �������
// ************************************************************************************************
// ����-����
dtcFRZF_General frzfBatNumber;
// ��������� �� ������ ����-����� (���������� DiagnosticValue)
const DiagnosticValueFRZF dVal_frzfBatNumber[] = 
{
	{ didFaults_FreezeFrame, DV_FRZF(frzfBatNumber) },
};
// ����������� ��������� �������������
static dtcProperty_t dtcProp_BatNumber = { dtc_Mst_WrongBatNumber, DTC_BIT_WARNING_ENABLE + DTC_BIT_OPERATION_DISABLE, 0, 5, -5, 250, DIAG_ITEM(dVal_frzfBatNumber) };
// ��� � �������������
dtcItem_t dtcMst_BatNumber = {&dtcProp_BatNumber};


// ************************************************************************************************
// DTC: �������� ��������� ����� �� �������
// ************************************************************************************************
// ����-����
dtcFRZF_General frzfBatState;
// ��������� �� ������ ����-����� (���������� DiagnosticValue)
const DiagnosticValueFRZF dVal_frzfBatState[] = 
{
	{ didFaults_FreezeFrame, DV_FRZF(frzfBatNumber) },
};
// ����������� ��������� �������������
static dtcProperty_t dtcProp_BatState = { dtc_Mst_WrongBatState, DTC_BIT_WARNING_ENABLE + DTC_BIT_OPERATION_DISABLE, 0, 3, -3, 250, DIAG_ITEM(dVal_frzfBatState) };
// ��� � �������������
dtcItem_t dtcMst_BatState = {&dtcProp_BatState};


// ************************************************************************************************
// DTC: ������� ���������� �������
// ************************************************************************************************
// ����-����
dtcFRZF_General frzfBatVoltageDiff;
// ��������� �� ������ ����-����� (���������� DiagnosticValue)
const DiagnosticValueFRZF dVal_frzfBatVoltageDiff[] = 
{
	{ didFaults_FreezeFrame, DV_FRZF(frzfBatVoltageDiff) },
};
// ����������� ��������� �������������
static dtcProperty_t dtcProp_BatVoltageDiff = { dtc_Mst_BatVoltageDiff, DTC_BIT_WARNING_ENABLE, 0, 5, -5, 100, DIAG_ITEM(dVal_frzfBatVoltageDiff) };
// ��� � �������������
dtcItem_t dtcMst_BatVoltageDiff = {&dtcProp_BatVoltageDiff};



// �������� ������ ��������������
dtcItem_t* dtcList[] =
{
	&dtcEcuConfigMemory, &dtcUnexpectedPowerOff, &dtcVcuOffline, &dtcInterlock, &dtcContactor, &dtcCellTemperature, &dtcMeasuringCircuit,
	&dtcBat_WrongModNumber, &dtcBat_WrongModState, &dtcBat_OverCurrent, &dtcBat_VoltageDiff, &dtcBat_Precharge, &dtcBat_CurrentSensor,
	&dtcMst_BatNumber, &dtcMst_BatState, &dtcMst_BatVoltageDiff, &dtcCellVoltage, &dtcPMOffline,
};

// ���������� ��������� ������ ��������������
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
	
	// ���������� ������
	if(!_config.CheckContactor)
		dtcContactor.Property->Bits.IsCritical = 0;
	else
		dtcContactor.Property->Bits.IsCritical = 1;
	
	dictionary->ecuIndex = _config.DiagnosticID + _config.ModuleIndex + (MAX_MODULE_NUM * _config.BatteryIndex);	
	
	// ��������� ������ ����������
	dtcBat_Precharge.Property->TestSamplePeriod = _config.PreMaxDuration / dtcBat_Precharge.Property->TestFailedThreshold;
	
	// ������������� ��������� ���������� �����
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

	// ������ ������� ECU
	Filter_init(50, 1, &fltVoltage);
}

