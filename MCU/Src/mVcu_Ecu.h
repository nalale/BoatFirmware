#ifndef _M_ECU_H_
#define _M_ECU_H_


#include "../MolniaLib/MF_CAN_1v1.h"
#include "../BoardDefinitions/MarineEcu_Board.h"
#include "FaultTools.h"

#define VER(major, minor)		((major << 8) + minor)


#define CLASS_MODEL_ID			MAIN_ECU_DIAG_ID		// Класс и модель устройства.
#define HARDWARE				MARINE_ECU				// Контроллер
#define	FW_VERSION				VER(0, 10)				// Версия прошивки. Старший байт - major, младший - minor (0...99)
#define	HW_VERSION				VER(1, 1)				// Версия железа. Старший байт - major, младший - minor (0...99)


// Переопределение входов
//#define KL15		D_IN1
//#define KL15_CHARGE	D_IN2
//#define MDRAIN_SW	D_IN3
//#define 


extern dtcItem_t dtcEcuConfigMemory;
extern dtcItem_t dtcUnexpectedPowerOff;
extern dtcItem_t dtcInverterOffline;
extern dtcItem_t dtcSkfOffline;
extern dtcItem_t dtcBatteryOffline;
extern dtcItem_t dtcBatteryFault;
extern dtcItem_t dtcInverterFault;
extern dtcItem_t dtcSteeringPosition;
extern dtcItem_t dtcTrimPosition;
extern dtcItem_t dtcAcceleratorPosition;
extern dtcItem_t dtcSteeringFeedBack;
extern dtcItem_t dtcTrimFeedBack;
extern dtcItem_t dtcPwmCircuit_1;
extern dtcItem_t dtcPwmCircuit_2;
extern dtcItem_t dtcPwmCircuit_3;
extern dtcItem_t dtcMeasuringCircuit;
extern dtcItem_t dtcPowerSupplyCircuit;


extern dtcFRZF_General frzfEcuConfigMemory;
extern dtcFRZF_General frzfUnexpectedPowerOff;
extern dtcFRZF_General frzfInverterOffline;
extern dtcFRZF_General frzfSkfOffline;
extern dtcFRZF_General frzfBatteryOffline;
extern dtcFRZF_General frzfBatteryFault;
extern dtcFRZF_General frzfInverterFault;
extern dtcFRZF_General frzfSteeringPosition;
extern dtcFRZF_General frzfTrimPosition;
extern dtcFRZF_General frzfAcceleratorPosition;
extern dtcFRZF_General frzfSteeringFeedBack;
extern dtcFRZF_General frzfTrimFeedBack;
extern dtcFRZF_General frzfPwmCircuit_1;
extern dtcFRZF_General frzfPwmCircuit_2;
extern dtcFRZF_General frzfPwmCircuit_3;
extern dtcFRZF_General frzfMeasuringCircuit;
extern dtcFRZF_General frzfPowerSupplyCircuit;


extern int dtcListSize;
extern dtcItem_t* dtcList[];


void ecuInit(ObjectDictionary_t *dictionary);
void boardThread(void);

uint16_t EcuGetVoltage(void);

#endif


