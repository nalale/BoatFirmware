#include <string.h>

#include "User.h"
#include "mVcu_Ecu.h"

#include "TimerFunc.h"
#include "CanFunc.h"
#include "PwmFunc.h"
#include "I2cFunc.h"
#include "AdcFunc.h"
#include "UartFunc.h"
#include "SpiFunc.h"
#include "MemoryFunc.h"

#include "lpc17xx_gpio.h"
#include "lpc17xx_clkpwr.h"
#include "lpc17xx_can.h"

#include "../Libs/Btn8982.h"
#include "../Libs/filter.h"

#include "../BoardDefinitions/MarineEcu_Board.h"

#include "../MolniaLib/MF_Tools.h"
#include "../MolniaLib/FaultsServices.h"


void ecuInit(ObjectDictionary_t *dictionary);
void HardwareInit(void);
void PortInit(void);
uint8_t DrainSupply(void);
uint8_t TemperatureControl(void);

void AppInit(ObjectDictionary_t *dictionary)
{
    HardwareInit();
    ecuInit(dictionary);
}

void HardwareInit(void)
{
    CLKPWR_ConfigPPWR(CLKPWR_PCONP_PCTIM0 | CLKPWR_PCONP_PCPWM1 | CLKPWR_PCONP_PCSSP1 |
                        CLKPWR_PCONP_PCAN1 | CLKPWR_PCONP_PCAN2 | CLKPWR_PCONP_PCGPIO | CLKPWR_PCONP_PCI2C2 |
						CLKPWR_PCONP_PCUART0,
                        ENABLE);
    PortInit();
    Tmr_Init(1000);
    Can_Init(1, 1);
    Pwm_Init(10000);
    Adc_Init();
    I2c_Init();
	Uart_Init(0, 9600);

	Spi_Init(DATABIT_16);

	NVIC_EnableIRQ(CAN_IRQn);
}


void PortInit(void)
{
	FIO_SetDir(0, 0x40, DIR_OUT);
	SET_CS_OUT(1);

    FIO_SetDir(1, 0xFFFFFFFF, DIR_OUT);
    //FIO_SetValue(1, 0xFFFFFFFF);
	SET_C_OUT5(1);
	SET_C_OUT6(1);
	SET_C_OUT7(1);
	SET_C_OUT8(1);
	SET_C_OUT9(1);
	SET_C_OUT10(1);

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

void InverterPower(uint8_t Cmd)
{
	SetDOutput(6, Cmd);
}

void SteeringPumpPower(uint8_t Cmd)
{
	Cmd = (Cmd >= 1)? 255 : 0;

	btnSetOutputLevel(2, Cmd);
}

uint8_t ChargersCircuitOn(uint8_t Cmd)
{
	if(Cmd == 1)
		btnSetOutputLevel(3, 255);
	else if(Cmd == 0)
		btnSetOutputLevel(3, 0);

	return 0;
}

void TransmissionSetGear(TransmissionGear_e Cmd)
{
	switch(Cmd)
	{
	case GEAR_NEU:
		OD.SB.cmdETBackward = 0;
		OD.SB.cmdETForward = 0;
		break;

	case GEAR_FW:
		OD.SB.cmdETBackward = 0;
		OD.SB.cmdETForward = 1;
		break;

	case GEAR_BW:
		OD.SB.cmdETBackward = 1;
		OD.SB.cmdETForward = 0;
		break;
	}
}

uint8_t SystemThreat(ObjectDictionary_t* OD)
{
	// Steering Functionality
	helmThread(McuRinegartGetParameter(&(OD->mcHandler), mcu_ActualSpeed));
	steeringThread(&(OD->SteeringData));

	// Traction Functionality
	McuRinehartThread(&OD->mcHandler);
	thrHandleControlThread(McuRinegartGetParameter(&OD->mcHandler, mcu_ActualSpeed));
	obcThread();
	TrimProc(&(OD->TrimDataRx), McuRinegartGetParameter(&(OD->mcHandler), mcu_ActualSpeed));		// Trim control
	transmissionThread(McuRinegartGetParameter(&(OD->mcHandler), mcu_ActualSpeed));

	TemperatureControl();

	return 0;
}

uint8_t TemperatureControl()
{
	static uint32_t _coolingOffDelay = 0;

	if(OD.SB.InverterIsOperate)
	{
		EcuConfig_t cfgEcu = GetConfigInstance();

		OD.SB.cmdMotorPumpCooling = 1;

		if((McuRinegartGetParameter(&OD.mcHandler, mcu_BoardTemperature) > OD.cfgEcu->InvCoolingOn) ||
				(McuRinegartGetParameter(&OD.mcHandler, mcu_MotorTemperature) > OD.cfgEcu->MotorCoolingOn))
			OD.SB.cmdHeatsinkPump = 1;
		else if((McuRinegartGetParameter(&OD.mcHandler, mcu_BoardTemperature) <= OD.cfgEcu->InvCoolingOn - 15) &&
				(McuRinegartGetParameter(&OD.mcHandler, mcu_MotorTemperature) <= OD.cfgEcu->MotorCoolingOn - 15))
			OD.SB.cmdHeatsinkPump = 0;

		_coolingOffDelay = GetTimeStamp();

	}
	else
	{
		if(GetTimeFrom(_coolingOffDelay) > 15000)
		{
			OD.SB.cmdMotorPumpCooling = 0;
			OD.SB.cmdHeatsinkPump = 0;

			return 1;
		}
	}

	return 0;
}

uint8_t DrainSupply()
{
	static uint32_t _waterSwitch1OnDelay = 0;

	if(OD.SB.stWaterSwitch1 || OD.SB.stWaterSwitch2 || OD.SB.stManualDrainSwitch)
	{
		if(GetTimeFrom(_waterSwitch1OnDelay) > 2000)
		{
			return 1;
		}
	}
	else
	{
		_waterSwitch1OnDelay = GetTimeStamp();
		return 0;
	}

	return 0;
}

void LedBlink()
{
	uint8_t x = LED_STATE;
	SET_LED(x);
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
