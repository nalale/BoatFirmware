#ifndef _TLE_6368_H_
#define _TLE_6368_H_

#include <stdint.h>
#include "../MolniaLib/FaultCategory.h"

typedef enum { 
	TLE_F_NO_FAULT = 0,
	TLE_F_GENERAL_ERROR = dctCat_GeneralElectricalFailure, 
	TLE_F_OVER_TEMP = dctCat_OverTemperature, 		
	TLE_F_WD_ERROR = dctCat_WatchdogOrSafetyFailure,
	TLE_F_OUT_SHORTED = dctCat_CircuitShortToGround, 
	TLE_F_COLD_START = dctCat_CircuitVoltageBelowThreshold,
} Tle6368FaultList_e;

void TLE_Init(void);
void TLE_GoToSleep(void);
Tle6368FaultList_e TLE_GetCircuitState(void);
void TLE_Proc(void);
void TLE_GoToPowerSupply(void);

#endif
