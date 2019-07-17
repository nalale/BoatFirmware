#ifndef _ADC_FUNC_H_
#define _ADC_FUNC_H_

#include "lpc17xx_adc.h"

/************************** PRIVATE DEFINITIONS ***********************/


void Adc_Init(void);
uint16_t GetVoltageValue(uint8_t channel);
void SetAdcData(uint8_t channel);

#endif
