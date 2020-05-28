#ifndef _BMS_COMBI_HW_H_
#define _BMS_COMBI_HW_H_

#include <stdint.h>
#include <lpc17xx_gpio.h>
#include "AdcFunc.h"
#include "../Libs/filter.h"


void boardThread(void);
uint16_t boardBMSCombi_GetVoltage(void);
uint32_t boardBMSCombi_GetDiscreteIO(void);

void IO_Init(void);
void gpio_ltc6804_cs_set(uint32_t cs_num, uint8_t state);

extern FILTER_STRUCT fltVoltage;


#define P0_6	(1 << 6)
#define P0_15	(1 << 15)
#define P0_16	(1 << 16)
#define P0_17	(1 << 17)
#define P0_18	(1 << 18)
#define P0_22	(1 << 22)
#define P0_26	(1 << 26)

#define P1_30	(1 << 30)
#define P1_31	(1 << 31)

#define P2_0	(1 << 0)
#define P2_1	(1 << 1)
#define P2_2	(1 << 2)
#define P2_5	(1 << 5)
#define P2_7	(1 << 7)
#define P2_8	(1 << 8)
#define P2_9	(1 << 9)

#define KEY					P0_6
#define D_OUT1				P2_7
#define D_OUT1_STATUS		P2_8
#define D_OUT2 				P2_9
#define D_OUT2_STATUS		P0_16
#define D_OUT3 				P0_15
#define D_OUT3_STATUS		P0_17
#define D_OUT4 				P0_18
#define D_OUT4_STATUS		P0_22

#define D_IN4			 	P2_0
#define D_IN3				P2_1
#define D_IN2			 	P2_2
#define D_IN1				P2_5

#define A_CHNL_KEY			ADC_CHANNEL_3
#define A_CH1				P1_30
#define A_CH2				P1_31

#define MCU_Key(x)				(x > 0)? GPIO_SetValue(0, P0_6) : GPIO_ClearValue(0, P0_6)
#define SET_D_OUT1(x)			(x > 0)? GPIO_SetValue(2, P2_7) : GPIO_ClearValue(2, P2_7)
#define GET_D_OUT1				((GPIO_ReadValue(2) & P2_7) >> 7)
#define GET_D_OUT1_STATUS		((GPIO_ReadValue(2) & P2_8) >> 8)
#define SET_D_OUT2(x)			(x > 0)? GPIO_SetValue(2, P2_9) : GPIO_ClearValue(2, P2_9)
#define GET_D_OUT2				((GPIO_ReadValue(2) & P2_9) >> 9)
#define GET_D_OUT2_STATUS		((GPIO_ReadValue(0) & P0_16) >> 16)
#define SET_D_OUT3(x)			(x > 0)? GPIO_SetValue(0, P0_15) : GPIO_ClearValue(0, P0_15)
#define GET_D_OUT3				((GPIO_ReadValue(0) & P0_15) >> 15)
#define GET_D_OUT3_STATUS		((GPIO_ReadValue(0) & P0_17) >> 17)
#define SET_D_OUT4(x)			(x > 0)? GPIO_SetValue(0, P0_18	) : GPIO_ClearValue(0, P0_18)
#define GET_D_OUT4				((GPIO_ReadValue(0) & P0_18) >> 18)
#define GET_D_OUT4_STATUS		((GPIO_ReadValue(0) & P0_22) >> 22)


#define GET_D_IN4			(!(GPIO_ReadValue(2) & P2_0))
#define GET_D_IN3			(!((GPIO_ReadValue(2) & P2_1) >> 1))
#define GET_D_IN2			(!((GPIO_ReadValue(2) & P2_2) >> 2))
#define GET_D_IN1			(!((GPIO_ReadValue(2) & P2_5) >> 5))

#define CS1_OUT	(1 << 1)
#define CS2_OUT	(1 << 14)

  // 0 - IN, 1 - OUT
#define OUT_DIR 	1
#define IN_DIR		0

#define GPIO_LOW	0
#define GPIO_HIGH	1

#define BAT_OPEN        0
#define BAT_CLOSE       1

#define BAT_PLUS(x)         (SET_D_OUT3(x))
#define GET_BAT_PLUS		(GET_D_OUT3)//(GET_D_OUT3_STATUS)
#define BAT_MINUS(x)        (SET_D_OUT2(x))
#define GET_BAT_MINUS		(GET_D_OUT2)//(GET_D_OUT2_STATUS)
#define PRECHARGE(x)        (SET_D_OUT4(x))
#define GET_PRECHARGE		(GET_D_OUT4)
#define LED(x)              (SET_D_OUT1(x))
#define GET_LED          	(GET_D_OUT1)

#define FB_PLUS             (GET_D_IN1)
#define FB_MINUS            (GET_D_IN2)
#define IL                  (GET_D_IN3)
#define RESERVE             (GET_D_IN4)


#endif
