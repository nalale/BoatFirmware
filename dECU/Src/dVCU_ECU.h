#ifndef _DISPLAY_ECU_H_
#define _DISPLAY_ECU_H_

#include "main.h"
#include "FaultTools.h"
#include "../MolniaLib/MF_CAN_1v1.h"


#define VER(major, minor)		((major << 8) + minor)


#define CLASS_MODEL_ID			DISPLAY_ECU_DIAG_ID		// Класс и модель устройства.
#define HARDWARE				MARINE_ECU			// Контроллер
#define	FW_VERSION				VER(0, 10)			// Версия прошивки. Старший байт - major, младший - minor (0...99)
#define	HW_VERSION				VER(1, 1)			// Версия железа. Старший байт - major, младший - minor (0...99)


extern dtcItem_t dtcEcuConfigMemory;
extern dtcItem_t dtcUnexpectedPowerOff;
extern dtcItem_t dtcBatteryOffline;
extern dtcItem_t dtcmEcuOffline;
extern dtcItem_t dtcPwmCircuit_1;
extern dtcItem_t dtcPwmCircuit_2;
extern dtcItem_t dtcPwmCircuit_3;
extern dtcItem_t dtcMeasuringCircuit;
extern dtcItem_t dtcPowerSupplyCircuit;


extern dtcFRZF_General frzfEcuConfigMemory;
extern dtcFRZF_General frzfUnexpectedPowerOff;
extern dtcFRZF_General frzfBatteryOffline;
extern dtcFRZF_General frzfmEcuOffline;
extern dtcFRZF_General frzfPwmCircuit_1;
extern dtcFRZF_General frzfPwmCircuit_2;
extern dtcFRZF_General frzfPwmCircuit_3;
extern dtcFRZF_General frzfMeasuringCircuit;
extern dtcFRZF_General frzfPowerSupplyCircuit;


extern int dtcListSize;
extern dtcItem_t* dtcList[];


void ecuInit(ObjectDictionary_t *dictionary);
void ecuProc(void);

uint16_t EcuGetVoltage(void);

#endif


