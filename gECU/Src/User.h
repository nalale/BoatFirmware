#ifndef _USER_H_
#define _USER_H_

#include "lpc17xx_gpio.h"
#include "Main.h"
#include "../BoardDefinitions/MarineEcu_Board.h"
#include "max11612.h"

void AppInit(ObjectDictionary_t *dictionary);
uint16_t GetOutLoadCurrent_0p1A(uint16_t Voltage_mV, uint16_t VoltageOffset_mV);
void CalibrateOutCurrentSens(A_OUT_CSENS_t *csens);



#endif
