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
	
	dtc_CAN_ExtCan,
	dtc_CAN_PCAN,

	dtc_PwmCircuit_1,
	dtc_PwmCircuit_2,
	dtc_PwmCircuit_3,
	dtc_MeasuringCircuit,
	dtc_PowerSupplyCircuit,

} dtCodes_e;

uint8_t FaultsTest(uint8_t FaultsTestIsEnabled);






#endif

