#ifndef _USER_H_
#define _USER_H_

#include "lpc17xx_gpio.h"
#include "Main.h"
#include "../BoardDefinitions/MarineEcu_Board.h"
#include "max11612.h"

void AppInit(ObjectDictionary_t *dictionary);
void CalibrateOutCurrentSens(A_OUT_CSENS_t *csens);



#endif
