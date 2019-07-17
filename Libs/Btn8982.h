/* 
 * File:   Btn8982.h
 * Author: a.lazko
 *
 * Created on 19 февраля 2019 г., 16:16
 */

#ifndef BTN8982_H
#define	BTN8982_H

#ifdef	__cplusplus
extern "C" {
#endif
	
#define DRIVER_NUM  4
	
#include "../MolniaLib/FaultCategory.h"
	
typedef enum { 
	BTN_F_NO_FAULT = 0,
	BTN_F_SHORT_TO_GROUND = dctCat_CircuitShortToGround, 
	BTN_F_SHORT_TO_BAT = dctCat_CircuitShortToBattery, 		
	BTN_F_CIRCUIT_OPEN = dctCat_CircuitOpen,
	BTN_F_CURRENT_ABOVE_THRESHOLD = dctCat_CircuitCurrentAboveThreshold, 
	BTN_F_CURRENT_OFFSET_NVALID = dctCat_SignalBiasLevelFailure,
} Btn8982FaultList_e;

void btnInit(uint8_t Number, uint8_t MeasuringChannel, uint16_t CurrentTreshold_0p1A);	
void btnProc(void);
uint8_t btnCalibrate(uint8_t Channel);
void btnSetOutputLevel(uint8_t Channel, uint8_t Level);
uint8_t btnGetOutputLevel(uint8_t Channel);
uint16_t btnGetCurrent(uint8_t Channel);    
Btn8982FaultList_e btnGetCircuitState(uint8_t Channel);

void btnInhibit(uint8_t num, uint8_t state);
void btnClearFaults(uint8_t Channel);

#ifdef	__cplusplus
}
#endif

#endif	/* BTN8982_H */

