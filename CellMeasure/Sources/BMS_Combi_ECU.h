#ifndef _BMS_ECU_H_
#define _BMS_ECU_H_


#include "FaultTools.h"
#include "../MolniaLib/FaultsServices.h"
#include "../MolniaLib/MF_CAN_1v1.h"

#define VER(major, minor)		((major << 8) + minor)


#define CLASS_MODEL_ID			BMS_ECU_DIAG_ID		// Класс и модель устройства.
#define HARDWARE				BMS_COMBI			// Контроллер
#define	FW_VERSION				VER(0, 13)			// Версия прошивки. Старший байт - major, младший - minor (0...99)
#define	HW_VERSION				VER(1, 1)			// Версия железа. Старший байт - major, младший - minor (0...99)


extern dtcItem_t dtcEcuConfigMemory;
extern dtcItem_t dtcUnexpectedPowerOff;
extern dtcItem_t dtcVcuOffline;
extern dtcItem_t dtcInterlock;
extern dtcItem_t dtcContactor;
extern dtcItem_t dtcCellVoltage;
extern dtcItem_t dtcCellTemperature;
extern dtcItem_t dtcMeasuringCircuit;
extern dtcItem_t dtcBat_WrongModNumber; 
extern dtcItem_t dtcBat_WrongModState;
extern dtcItem_t dtcBat_OverCurrent; 
extern dtcItem_t dtcBat_VoltageDiff;
extern dtcItem_t dtcBat_Precharge;
extern dtcItem_t dtcBat_CurrentSensor;
extern dtcItem_t dtcMst_BatNumber;
extern dtcItem_t dtcMst_BatState;
extern dtcItem_t dtcMst_BatVoltageDiff;
extern dtcItem_t dtcPMOffline;


extern dtcFRZF_General frzfEcuConfigMemory;
extern dtcFRZF_General frzfUnexpectedPowerOff;
extern dtcFRZF_General frzfVcuOffline;
extern dtcFRZF_General frzfInterlock;
extern dtcFRZF_General frzfContactor;
extern dtcFRZF_General frzfCellVoltage;
extern dtcFRZF_General frzfCellTemperature;
extern dtcFRZF_General frzfMeasuringCircuit;
extern dtcFRZF_General frzfModuleNumber;
extern dtcFRZF_General frzfModuleState;
extern dtcFRZF_General frzfOverCurrent;
extern dtcFRZF_General frzfModuleVoltageDiff;
extern dtcFRZF_General frzfPrecharge;
extern dtcFRZF_General frzfBatNumber;
extern dtcFRZF_General frzfBatState;
extern dtcFRZF_General frzfBatVoltageDiff;
extern dtcFRZF_General frzfBatCurrentSensor;
extern dtcFRZF_General frzfPMOffline;



extern int dtcListSize;
extern dtcItem_t* dtcList[];




void ecuInit(ObjectDictionary_t *dictionary);







#endif


