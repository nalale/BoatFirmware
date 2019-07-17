#include <stdint.h>

#include "User.h"

#include "Main.h"
#include "TimerFunc.h"
#include "CanFunc.h"
#include "PwmFunc.h"
#include "SpiFunc.h"
#include "UartFunc.h"
#include "BMS_Combi_ECU.h"

#include "lpc17xx_gpio.h"
#include "lpc17xx_clkpwr.h"

#include "AdcFunc.h"

#include "../MolniaLib/DateTime.h"
#include "../MolniaLib/MF_Tools.h"
#include "../Libs/CurrentSens.h"

void HardwareInit(void);
void PortInit(void);
void LedBlink(void);

void AppInit(ObjectDictionary_t *dictionary)
{
    HardwareInit();
    ecuInit(dictionary);
}

void HardwareInit(void)
{
    CLKPWR_ConfigPPWR(CLKPWR_PCONP_PCTIM0 | CLKPWR_PCONP_PCPWM1 | CLKPWR_PCONP_PCSSP1 |
                    CLKPWR_PCONP_PCAN1 | CLKPWR_PCONP_PCAN2 | CLKPWR_PCONP_PCGPIO | 
					CLKPWR_PCONP_PCSSP0 | CLKPWR_PCONP_PCI2C2 | CLKPWR_PCONP_PCRTC,
                    ENABLE);
	
	RTC_TIME_Type rtc;
	RTC_GetFullTime(LPC_RTC, &rtc);
	SetDateTime(&rtc);
	
    IO_Init();
    
    Tmr_Init(1000);
    Spi_Init(DATABIT_8);
	Adc_Init();
    Can_Init(1, 0);
	Uart_Init(0, 9600);
	
	// RTC
	RTC_Init (LPC_RTC);
	RTC_Cmd(LPC_RTC, ENABLE);
	
	NVIC_EnableIRQ(CAN_IRQn);
}


void ControlBatteriesState(WorkStates_e *CurrentState, WorkStates_e RequestState)
{
	EcuConfig_t ecuConfig = GetConfigInstance();
	// Если модуль - не мастер, брать состояние из CAN сообщения, иначе из передаваемого в функцию параметра
	if(ecuConfig.IsMaster)
	{
		if(RequestState > WORKSTATE_INIT && RequestState != WORKSTATE_FAULT)
			*CurrentState = (OD.SystemOperateEnabled || ecuConfig.IsAutonomic)? RequestState : WORKSTATE_PREOP;
		else
			*CurrentState = RequestState;
	}
}

uint8_t GetBalancingPermission(int16_t TotalCurrent)
{
	EcuConfig_t ecuConfig = GetConfigInstance();
	uint8_t res = 0;
	if(ecuConfig.IsMaster || ecuConfig.IsAutonomic)
	{		
		// Разрешаем балансировку при токе меньше 1 Ампера
		if(TotalCurrent > 10 || TotalCurrent < -10)
			res = 0;
		else
			res = 1;
	}		
	else
		res = OD.MasterControl.BalancingEnabled;
	
	return res;
}

void LedStatus(uint8_t ContFB, uint32_t Balancing)
{
	static uint32_t ts = 0;
		
	if(ContFB != 1)
	{
		if(GetTimeFrom(ts) > 100)
		{
			LedBlink();
			ts = GetTimeStamp();
		}
	}
	else if(Balancing > 0)
	{
		if(GetTimeFrom(ts) > 500)
		{
			LedBlink();
			ts = GetTimeStamp();
		}
	}
	else
		LED(1);
}

void LedBlink(void)
{
	uint8_t v = GET_LED;
	v = (v == 1)? 0 : 1;
	LED(v);
}

void ECU_GoToSleep(void)
{
	MCU_Key(0);
}

void ECU_GoToPowerSupply(void)
{
	MCU_Key(1);
}

uint8_t ContactorClose(uint8_t num)
{
	static uint8_t step = 0;
	static uint32_t timestamp = 0;
	
	switch(step)
	{
		case 0:
			if(num == 0)
				BAT_MINUS(BAT_CLOSE);
			else
				BAT_PLUS(BAT_CLOSE);
			
			step = 1;
			timestamp = GetTimeStamp();
			break;
		case 1:
			if(GetTimeFrom(timestamp) > 100)
			{
				timestamp = GetTimeStamp();
				
				if(num == 0)
					BAT_MINUS(BAT_OPEN);
				else
					BAT_PLUS(BAT_OPEN);
				
				step = 2;
			}
			break;
		case 2:
			if(GetTimeFrom(timestamp) > 100)
			{
				timestamp = GetTimeStamp();
				if(num == 0)
					BAT_MINUS(BAT_CLOSE);
				else
					BAT_PLUS(BAT_CLOSE);
				
				step = 0;
			}
			break;
			
	}
	
	return step;
}
