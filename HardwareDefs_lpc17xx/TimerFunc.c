#include <system_LPC17xx.h>
#include <lpc17xx.h>
#include "TimerFunc.h"
#include "../MolniaLib/DateTime.h"
#include "lpc17xx_timer.h"


/* SysTick Counter */
volatile unsigned long SysTickCnt;
static unsigned long cnt_1000ms;

/*----------------- INTERRUPT SERVICE ROUTINES --------------------------*/
/*********************************************************************//**
 * @brief		SysTick handler sub-routine (1ms)
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void SysTick_Handler (void) {
	SysTickCnt++;
	cnt_1000ms++;

	if(cnt_1000ms == 1000)
	{
		cnt_1000ms = 0;
		dateTime_newSecond();
	}
}

/*-------------------------PUBLIC FUNCTIONS------------------------------*/
/*********************************************************************//**
 * @brief		Delay function
 * @param[in]	tick - number milisecond of delay time
 * @return 		None
 **********************************************************************/
void Delay (uint32_t tick) {
  unsigned long systickcnt;

  systickcnt = SysTickCnt;
  while ((SysTickCnt - systickcnt) < tick);
}


void Tmr_Init(uint32_t Freq_Hz)
{
  SysTick_Config(SystemCoreClock/Freq_Hz - 1);
	
	TIM_TIMERCFG_Type usTimer;
	TIM_ConfigStructInit(TIM_TIMER_MODE, &usTimer);
	usTimer.PrescaleOption = TIM_PRESCALE_USVAL;
	usTimer.PrescaleValue = 1;

	TIM_Init(LPC_TIM0, TIM_TIMER_MODE, &usTimer);
	TIM_Cmd(LPC_TIM0, ENABLE);
}

uint32_t GetTimeStamp()
{
  return SysTickCnt;
}

uint32_t GetTimeFrom(uint32_t TimeStampLoc)
{
  uint32_t result, Timer0cnt;
  
  Timer0cnt= GetTimeStamp();
  
  if(Timer0cnt >= TimeStampLoc)
    result = Timer0cnt - TimeStampLoc;
  else
    result = (0xFFffFFff -  TimeStampLoc)  + Timer0cnt;
  
  return result;
}

uint32_t uGetTimeStamp()
{
  return LPC_TIM0->TC;
}

uint32_t uGetTimeFrom(uint32_t TimeStampLoc)
{
  uint32_t result, ucnt;
  
  ucnt= uGetTimeStamp();
  
  if(ucnt >= TimeStampLoc)
    result = ucnt - TimeStampLoc;
  else
    result = (0xFFffFFff -  TimeStampLoc)  + ucnt;
  
  return result;
}


