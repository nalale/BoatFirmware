#ifndef _EVCU_HW_H_
#define _EVCU_HW_H_

#include "lpc17xx_gpio.h"
#include "AdcFunc.h"



#define A_OUT2_CSENS    ADC_CHANNEL_2
#define A_OUT1_CSENS    ADC_CHANNEL_3
#define A_CHNL_KEY		ADC_CHANNEL_4
#define A_OUT3_CSENS	ADC_CHANNEL_5

#define V_AN		50	// V_ref 5.0 V
#define A_IN_NUM	4

// SPI
#define P0_6	(1 << 6)

#define SPI_CS				0, P0_6

#define SET_CS_OUT(x)		((x) == 1)? GPIO_SetValue(SPI_CS) : GPIO_ClearValue(SPI_CS)

// D_OUT 
#define P1_29	(1 << 29)
#define P1_28	(1 << 28)
#define P1_26	(1 << 26)
#define P1_25	(1 << 25)
#define P1_24	(1 << 24)
#define P1_23	(1 << 23)
#define P1_18	(1 << 18)
#define P1_19	(1 << 19)
#define P4_28	(1 << 28)
#define P4_29	(1 << 29)

#define D_OUT5				1, P1_29
#define D_OUT6				1, P1_28
#define D_OUT7				1, P1_24
#define D_OUT8				1, P1_23
#define D_OUT9				1, P1_26
#define D_OUT10				1, P1_25

#define PU_D_IN1			1, P1_19
#define PU_D_IN2			1, P1_18
#define PU_D_IN3			4, P4_28
#define PU_D_IN4			4, P4_29

// »нверси€, потомучто реле управл€ютс€ минусом
#define SET_C_OUT5(x)			((x) == 0)? GPIO_SetValue(D_OUT5) : GPIO_ClearValue(D_OUT5)
#define C_OUT5_STATE            ((GPIO_ReadValue(1) & P1_29) != 0)
#define SET_C_OUT6(x)			((x) == 0)? GPIO_SetValue(D_OUT6) : GPIO_ClearValue(D_OUT6)
#define C_OUT6_STATE            ((GPIO_ReadValue(1) & P1_28) != 0)
#define SET_C_OUT7(x)			((x) == 0)? GPIO_SetValue(D_OUT7) : GPIO_ClearValue(D_OUT7)
#define C_OUT7_STATE            ((GPIO_ReadValue(1) & P1_24) != 0)
#define SET_C_OUT8(x)			((x) == 0)? GPIO_SetValue(D_OUT8) : GPIO_ClearValue(D_OUT8)
#define C_OUT8_STATE            ((GPIO_ReadValue(1) & P1_23) != 0)
#define SET_C_OUT9(x)			((x) == 0)? GPIO_SetValue(D_OUT9) : GPIO_ClearValue(D_OUT9)
#define C_OUT9_STATE            ((GPIO_ReadValue(1) & P1_26) != 0)
#define SET_C_OUT10(x)			((x) == 0)? GPIO_SetValue(D_OUT10) : GPIO_ClearValue(D_OUT10)
#define C_OUT10_STATE            ((GPIO_ReadValue(1) & P1_25) != 0)

#define SET_PU_D_IN1(x)			((x) == 1)? GPIO_SetValue(PU_D_IN1) : GPIO_ClearValue(PU_D_IN1)
#define PU_D_IN1_STATE          ((GPIO_ReadValue(1) & P1_19) != 0)
#define SET_PU_D_IN2(x)			((x) == 1)? GPIO_SetValue(PU_D_IN2) : GPIO_ClearValue(PU_D_IN2)
#define PU_D_IN2_STATE          ((GPIO_ReadValue(1) & P1_18) != 0)
#define SET_PU_D_IN3(x)			((x) == 1)? GPIO_SetValue(PU_D_IN3) : GPIO_ClearValue(PU_D_IN3)
#define PU_D_IN3_STATE          ((GPIO_ReadValue(4) & P4_28) != 0)
#define SET_PU_D_IN4(x)			((x) == 1)? GPIO_SetValue(PU_D_IN4) : GPIO_ClearValue(PU_D_IN4)
#define PU_D_IN4_STATE          ((GPIO_ReadValue(4) & P4_29) != 0)

// ENABLE A_OUT
#define P2_4	(1 << 4)
#define P2_5	(1 << 5)
#define P2_6	(1 << 6)
#define P2_9	(1 << 9)

#define OUT1_EN             2, P2_4
#define OUT2_EN             2, P2_5
#define OUT3_EN             2, P2_6
#define OUT4_EN             2, P2_9

#define SET_A_OUT1_EN(x)		    ((x) > 0)? GPIO_SetValue(OUT1_EN) : GPIO_ClearValue(OUT1_EN)
#define A_OUT1_EN_STATE            ((GPIO_ReadValue(2) & P2_4) != 0)
#define SET_A_OUT2_EN(x)		    ((x) > 0)? GPIO_SetValue(OUT2_EN) : GPIO_ClearValue(OUT2_EN)
#define A_OUT2_EN_STATE            ((GPIO_ReadValue(2) & P2_5) != 0)
#define SET_A_OUT3_EN(x)		    ((x) > 0)? GPIO_SetValue(OUT3_EN) : GPIO_ClearValue(OUT3_EN)
#define A_OUT3_EN_STATE            ((GPIO_ReadValue(2) & P2_6) != 0)
#define SET_A_OUT4_EN(x)		    ((x) > 0)? GPIO_SetValue(OUT4_EN) : GPIO_ClearValue(OUT4_EN)
#define A_OUT4_EN_STATE            ((GPIO_ReadValue(2) & P2_9) != 0)


// INPUT
#define P0_29	(1 << 29)
#define P0_30	(1 << 30)
#define P1_14	(1 << 14)
#define P1_15	(1 << 15)
#define P1_9	(1 << 9)
#define P1_10	(1 << 10)
#define P1_4	(1 << 4)
#define P1_8	(1 << 8)
#define P1_0	(1 << 0)
#define P1_1	(1 << 1)

#define D_IN1            ((PU_D_IN1_STATE)? (GPIO_ReadValue(0) & P0_29) == 0 : (GPIO_ReadValue(0) & P0_29) != 0)
#define D_IN2            ((PU_D_IN2_STATE)? (GPIO_ReadValue(0) & P0_30) == 0 : (GPIO_ReadValue(0) & P0_30) != 0)
#define D_IN3            ((PU_D_IN3_STATE)? (GPIO_ReadValue(1) & P1_14) == 0 : (GPIO_ReadValue(0) & P1_14) != 0)
#define D_IN4            ((PU_D_IN4_STATE)? (GPIO_ReadValue(1) & P1_15) == 0 : (GPIO_ReadValue(0) & P1_15) != 0)
#define D_IN5            ((GPIO_ReadValue(1) & P1_9) == 0)
#define D_IN6            ((GPIO_ReadValue(1) & P1_10) == 0)
#define D_IN7            ((GPIO_ReadValue(1) & P1_4) == 0)
#define D_IN8            ((GPIO_ReadValue(1) & P1_8) == 0)
#define D_IN9            ((GPIO_ReadValue(1) & P1_0) == 0)
#define D_IN10           ((GPIO_ReadValue(1) & P1_1) == 0)




void SetDOutput(uint8_t num, uint8_t state);
uint32_t GetDiscretIO(void);


#endif
