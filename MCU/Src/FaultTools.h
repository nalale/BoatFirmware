#ifndef _FAULTS_TEST_H_
#define _FAULTS_TEST_H_

#include <stdint.h>


typedef enum
{
	dtc_General_EcuConfig = 0,
	dtc_General_EcuSupplyOutOfRange,
	dtc_General_UnexpectedPowerOff,
	dtc_General_Interlock,
	dtc_General_EcuDataTimeNotCorrect,	
	
	// Таймаут CAN
	dtc_CAN_Inverter,
	dtc_CAN_Skf,
	dtc_CAN_Battery,
	
	dtc_BatteryFault,	
	dtc_InverterFault,	
	dtc_SteeringPosition,	
	dtc_TrimPosition,	
	dtc_Accelerator	,
	
	dtc_SteeringFeedback,	
	dtc_TrimFeedback,
	
	dtc_PwmCircuit_1,
	dtc_PwmCircuit_2,
	dtc_PwmCircuit_3,
	
	dtc_MeasuringCircuit,
	dtc_PowerSupplyCircuit,
	
} dtCodes_e;


uint8_t FaultsTest(uint8_t FaultsTestIsEnabled);

#endif

