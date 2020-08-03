#include <string.h>

#include "User.h"
#include "WorkStates.h"
#include "dVCU_ECU.h"

#include "TimerFunc.h"
#include "CanFunc.h"
#include "PwmFunc.h"
#include "I2cFunc.h"
#include "AdcFunc.h"
#include "UartFunc.h"
#include "SpiFunc.h"
#include "MemoryFunc.h"

#include "lpc17xx_pinsel.h"
#include "lpc17xx_gpio.h"
#include "lpc17xx_clkpwr.h"
#include "lpc17xx_timer.h"

#include "../MolniaLib/MF_Tools.h"

#include "../Libs/Btn8982.h"



void HardwareInit(void);
void DisplayMotorRpmInit(void);
void PortInit(void);

void AppInit(ObjectDictionary_t *dictionary)
{
    HardwareInit();
    ecuInit(dictionary);    
      
	DisplayMotorRpmInit();

	SetWorkState(&dictionary->StateMachine, WORKSTATE_INIT);
}

void HardwareInit(void)
{
    CLKPWR_ConfigPPWR(CLKPWR_PCONP_PCTIM0 | CLKPWR_PCONP_PCPWM1 | CLKPWR_PCONP_PCSSP1 | CLKPWR_PCONP_PCAD |
                        CLKPWR_PCONP_PCAN1 | CLKPWR_PCONP_PCAN2 | CLKPWR_PCONP_PCGPIO,
                        ENABLE);
	
    PortInit();
    Tmr_Init(1000);		// resolution 1mS
    Can_Init(1, 1);
    Pwm_Init(10000);
    Adc_Init();
    I2c_Init();
	Spi_Init(DATABIT_16);

	NVIC_EnableIRQ(CAN_IRQn);
}

void PortInit(void)
{
	FIO_SetDir(0, 0x40, DIR_OUT);
	SET_CS_OUT(1);

    FIO_SetDir(1, 0xFFFFFFFF, DIR_OUT);
	SET_C_OUT5(0);
	SET_C_OUT6(0);
	SET_C_OUT7(0);
	SET_C_OUT8(0);
	SET_C_OUT9(0);
	SET_C_OUT10(0);

	SET_PU_D_IN1(0);
	SET_PU_D_IN2(0);

	SET_LED(0);


    FIO_SetDir(2, 0xFFFFFFFF, DIR_OUT);
    SET_D_OUT1_EN(0);
	SET_D_OUT2_EN(0);
	SET_D_OUT3_EN(0);
	SET_D_OUT4_EN(0);

    FIO_SetDir(4, 0xFFFFFFFF, DIR_OUT);
    //FIO_SetValue(4, 0x0);
	SET_PU_D_IN3(0);
	SET_PU_D_IN4(0);

	FIO_SetDir(1, 0xC713, DIR_IN);
	FIO_SetDir(0, (1 << 30) | (1 << 29), DIR_IN);
}


void DisplaySoc(uint16_t Value)
{
	const EcuConfig_t *config = OD.cfg;
	uint8_t dutycycle = (uint8_t)interpol((int16_t*)config->SoC, 6, Value);

	btnSetOutputLevel(DISPLAY_SOC_CH, dutycycle);
}

void DisplayTrim(uint16_t Value)
{
	const EcuConfig_t *config = OD.cfg;
	uint8_t dutycycle = (uint8_t)interpol((int16_t*)config->TrimPosition, 6, Value);

	btnSetOutputLevel(DISPLAY_TRIM_CH, dutycycle);
}

void DisplayEnergy(uint16_t Value)
{
	const EcuConfig_t *config = OD.cfg;
	uint8_t dutycycle = (uint8_t)interpol((int16_t*)config->SpecPower, 6, Value);	
	btnSetOutputLevel(DISPLAY_CONS_CH, dutycycle);
}

void DisplayMotorRpm(int16_t Value)
{
	const EcuConfig_t *config = OD.cfg;
	Value = (Value < 0)? -Value : Value;
	uint16_t frequency = (uint16_t)interpol((int16_t*)config->MotorRpm, 6, Value);
	frequency = (frequency == 0)? 1 : frequency << 1;	// pin toggle mode => freq * 2
	uint32_t _period_us = (1000000 / frequency); 
	
	TIM_UpdateMatchValue(LPC_TIM0, 1, _period_us);
}

void DisplayMotorRpmInit(void)
{		
	// Init output for Speed Viewer
	PINSEL_CFG_Type pinCfg;
	pinCfg.Portnum = 1;
	pinCfg.Pinnum = 29;	// C_OUT_5
	pinCfg.Funcnum = PINSEL_FUNC_3;
	PINSEL_ConfigPin(&pinCfg);
	
	TIM_TIMERCFG_Type tmrCfg;
	TIM_ConfigStructInit(TIM_TIMER_MODE, &tmrCfg);
	
	TIM_Init(LPC_TIM0, TIM_TIMER_MODE, &tmrCfg);
	
	TIM_MATCHCFG_Type matchCfg;
	matchCfg.MatchChannel = 1;
	matchCfg.MatchValue = 5000;
	matchCfg.ExtMatchOutputType = TIM_EXTMATCH_TOGGLE;	
	matchCfg.IntOnMatch = DISABLE;
	matchCfg.StopOnMatch = DISABLE;
	matchCfg.ResetOnMatch = ENABLE;
	
	TIM_ConfigMatch(LPC_TIM0, &matchCfg);
	
	TIM_Cmd(LPC_TIM0, ENABLE);
}

int8_t flashWriteSData(const StorageData_t *sdata)
{
	StorageData_t tmp_sdata;

	// write 1st data to 1st and 2nd buffer
	memcpy(&tmp_sdata.Buf_1st, &sdata->Buf_1st, sizeof(sdata->Buf_1st));

	tmp_sdata.Buf_1st.Crc = 0;

	for(uint8_t i = 0; i < sizeof(sdata->Buf_1st) - sizeof(sdata->Buf_1st.Crc); i++)
		tmp_sdata.Buf_1st.Crc += *((uint8_t*)&tmp_sdata.Buf_1st + i);

	MemEcuSDataWrite((uint8_t*)&tmp_sdata.Buf_1st, sizeof(sdata->Buf_1st), 0);

	return 0;
}

int8_t flashReadSData(StorageData_t *sdata)
{
	int8_t result = 0;
	uint16_t crc0 = 0;//, crc1 = 0;

	sdata->Result = 0;

	MemEcuSDataRead((uint8_t*)&sdata->Buf_1st, sizeof(sdata->Buf_1st), 0);
	for(uint8_t i = 0; i < sizeof(sdata->Buf_1st) - sizeof(sdata->Buf_1st.Crc); i++)
		crc0 += *((uint8_t*)&sdata->Buf_1st + i);

	// if data from buf is correct
	if(crc0 == sdata->Buf_1st.Crc)
	{
		uint8_t save_poweroff = sdata->Buf_1st.NormalPowerOff;
		// Reset normal power off sign
		sdata->Buf_1st.NormalPowerOff = 0;

		if(!save_poweroff)
			result =  -1;
		else
		{
			result = 0;
			sdata->DataChanged = 1;
		}

	}
	// else restore TotalCapacity from another buffer
	else
	{

		memset(&sdata->Buf_1st, 0xff, sizeof(sdata->Buf_1st));
		result =  -1;
	}

	sdata->Result = result;
	return result;
}

int8_t flashClearData(StorageData_t *sdata)
{
	// Prepare for write
	int16_t Length = sizeof(sdata->Buf_1st) + sizeof(EcuConfig_t) + (dtcListSize * sizeof(dtcItem_t));
	
	__disable_irq();
	MemEcuWriteData((uint8_t*)sdata, Length);

	cfgWrite(sdata->cfgData);
	SaveFaults(dtcList, dtcListSize);	
	
	__enable_irq();
	
	sdata->DataChanged = 0;	
	return 0;
}

int8_t flashClearFaults(StorageData_t *sdata)
{
	ClearFaults(dtcList, dtcListSize);
	return 0;
}

int8_t flashStoreData(StorageData_t *sdata)
{
	// Prepare for write
	int16_t Length = sizeof(sdata->Buf_1st) + sizeof(EcuConfig_t) + (dtcListSize * sizeof(dtcItem_t));

	__disable_irq();
	MemEcuWriteData((uint8_t*)sdata, Length);

	cfgWrite(sdata->cfgData);
	SaveFaults(dtcList, dtcListSize);
	flashWriteSData(sdata);

	__enable_irq();

	sdata->DataChanged = 0;
	return 0;
}
