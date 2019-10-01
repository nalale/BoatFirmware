#ifndef _PWM_FUNC_H_
#define _PWM_FUNC_H_

#include <lpc17xx_pwm.h>

typedef enum 
{
	PWM_Channel1 = 0,
	PWM_Channel2,
	PWM_Channel3,
	PWM_Channel4,
}PwmChannels_e;

void Pwm_Init(uint16_t Freq);
void Pwm_Ch_Init(uint8_t ChNum, uint16_t Freq, uint8_t DutyCycle);
void PwmUpdate(uint8_t ChNum, uint8_t DutyCycle);
void PwmFreqUpdate(uint16_t Freq);

#endif
