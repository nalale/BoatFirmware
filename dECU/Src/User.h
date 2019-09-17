#ifndef _USER_H_
#define _USER_H_

#include "lpc17xx_gpio.h"
#include "Main.h"
#include "../BoardDefinitions/MarineEcu_Board.h"
#include "max11612.h"
#include "PwmFunc.h"

#define DISPLAY_SOC_CH 		PWM_Channel1
#define DISPLAY_RPM_CH 		PWM_Channel2
#define DISPLAY_TRIM_CH 	PWM_Channel3
#define DISPLAY_CONS_CH 	PWM_Channel4


void AppInit(ObjectDictionary_t *dictionary);
void CalibrateOutCurrentSens(A_OUT_CSENS_t *csens);



#endif
