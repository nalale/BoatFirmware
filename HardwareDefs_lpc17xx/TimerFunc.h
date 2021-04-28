#ifndef _TIMER_DRV_H_
#define _TIMER_DRV_H_

#include "stdint.h"

void Delay (uint32_t tick);
void Tmr_Init(uint32_t Freq_Hz);

uint32_t GetTimeStamp(void);
uint32_t GetTimeFrom(uint32_t TimeStamp);
uint32_t uGetTimeStamp(void);
uint32_t uGetTimeFrom(uint32_t TimeStampLoc);


#endif
