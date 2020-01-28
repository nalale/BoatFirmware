#include <stdint.h>

#include "Main.h"
#include "User.h"
#include "BatteryMeasuring.h"

#include "AdcFunc.h"
#include "TimerFunc.h"
#include "CanFunc.h"
#include "PwmFunc.h"
#include "SpiFunc.h"
#include "UartFunc.h"
#include "BMS_Combi_ECU.h"

#include "lpc17xx_gpio.h"
#include "lpc17xx_clkpwr.h"



#include "../MolniaLib/DateTime.h"
#include "../MolniaLib/MF_Tools.h"
#include "../Libs/CurrentSens.h"

void HardwareInit(void);
void PortInit(void);
void LedBlink(void);

// Precharge proccess function
// Parameters:
// Return value: 0 - precharge doesn't complete, 1 - precharge complete
static uint8_t BatteryWaitForPrecharge(const BatteryData_t* Handle, const EcuConfig_t *config, uint32_t *CompleteConditionTimeStamp);

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


/* ************************** Module Functionality ************************** */

uint8_t ModuleSetContactorPosition(const BatteryData_t* Handle, stBattery_e StateCmd, const EcuConfig_t *config, uint32_t *CompleteConditionTimeStamp)
{
	switch(StateCmd)
	{
	case stBat_Disabled:
		BAT_MINUS(BAT_OPEN);
		BAT_PLUS(BAT_OPEN);
		PRECHARGE(BAT_OPEN);
		return 1;

	case stBat_Precharging:
	{
		// If current module is terminal with precharge function
		if(ModuleIsTerminal(config))
			return BatteryWaitForPrecharge(Handle, config, CompleteConditionTimeStamp);
		else
			BAT_MINUS(BAT_CLOSE);
			BAT_PLUS(BAT_CLOSE);
			PRECHARGE(BAT_OPEN);
			return 1;
	}

	case stBat_Enabled:
		BAT_MINUS(BAT_CLOSE);
		BAT_PLUS(BAT_CLOSE);
		PRECHARGE(BAT_OPEN);
		return 1;
	}

	return 0;
}

uint8_t ModuleIsTerminal(const EcuConfig_t *config)
{
	return config->ModuleIndex == 0;
}

void ModuleStatisticCalculating(BatteryData_t* Handle, const EcuConfig_t *ecuConfig, const int16_t *CellsVoltageArray, const int16_t *CellsTempArray)
{
	uint16_t nv = 0;
	uint32_t SumVoltage = 0;

	int16_t nt = 0;
	int32_t SumTemperature = 0;
	uint8_t cellsCount;

	if(Handle == 0 || CellsVoltageArray == 0 || CellsTempArray == 0 || ecuConfig == 0)
		return;

	cellsCount = (ecuConfig->CellNumber >= CELLS_IN_MODULE)? CELLS_IN_MODULE : ecuConfig->CellNumber;


	Handle->MaxCellVoltage.Voltage_mv = INT16_MIN;
	Handle->MaxCellVoltage.CellNumber = cellsCount;
	Handle->MaxCellVoltage.ModuleNumber = ecuConfig->ModuleIndex;

	Handle->MinCellVoltage.Voltage_mv = INT16_MAX;
	Handle->MinCellVoltage.CellNumber = cellsCount;
	Handle->MaxCellVoltage.ModuleNumber = ecuConfig->ModuleIndex;

	Handle->MaxModuleTemperature.Temperature = INT8_MIN;
	Handle->MaxModuleTemperature.SensorNum = TMP_SENSOR_IN_MODULE;
	Handle->MaxModuleTemperature.ModuleNumber = ecuConfig->ModuleIndex;

	Handle->MinModuleTemperature.Temperature = INT8_MAX;
	Handle->MinModuleTemperature.SensorNum = TMP_SENSOR_IN_MODULE;
	Handle->MinModuleTemperature.ModuleNumber = ecuConfig->ModuleIndex;

    // Рассчёт Max/Min напряжения
    for (uint8_t cellInd = 0; cellInd < cellsCount; cellInd++)
    {
        nv++;
        SumVoltage += CellsVoltageArray[cellInd];

        // Поиск максимльного напряжения
        if (CellsVoltageArray[cellInd] > Handle->MaxCellVoltage.Voltage_mv)
        {
            Handle->MaxCellVoltage.Voltage_mv = CellsVoltageArray[cellInd];
            Handle->MaxCellVoltage.CellNumber = cellInd;
        }
        // Поиск минимального напряжения
        if (CellsVoltageArray[cellInd] < Handle->MinCellVoltage.Voltage_mv)
        {
            Handle->MinCellVoltage.Voltage_mv = CellsVoltageArray[cellInd];
            Handle->MinCellVoltage.CellNumber = cellInd;
        }
    }

    Handle->TotalVoltage = SumVoltage / 100;

    // Среднее значение напряжений на ячейках
	if(nv > 0)
		Handle->AvgCellVoltage = SumVoltage / nv;

    // Рассчёт Max/Min температуры
	for (uint8_t TempSensCnt = 0; TempSensCnt < TMP_SENSOR_IN_MODULE; TempSensCnt++)
    {
        int8_t t = CellsTempArray[TempSensCnt];

        if (t > 100 || t < -100)
            continue;

        nt++;
        SumTemperature += t;

        if (t > Handle->MaxModuleTemperature.Temperature)
        {
            Handle->MaxModuleTemperature.Temperature = t;
			Handle->MaxModuleTemperature.SensorNum = TempSensCnt;
        }
        if (t < Handle->MinModuleTemperature.Temperature)
        {
            Handle->MinModuleTemperature.Temperature = t;
			Handle->MinModuleTemperature.SensorNum = TempSensCnt;
        }
    }

    if(nv > 0)
    	Handle->AvgModuleTemperature = SumTemperature / nt;

}



/* *********************** Battery Functionality ****************************** */

uint8_t BatteryIsReady(const BatteryData_t* Handle, const BatteryData_t *ModulesData, const EcuConfig_t *config, uint32_t *ReadyConditionTimeStamp)
{
	if(ModuleIsTerminal(config))
	{
		if(Handle[config->BatteryIndex].OnlineNumber == config->Sys_ModulesCountS)
		{
			uint8_t tmp = 1;
			for (uint8_t i = config->ModuleIndex; i < config->Sys_ModulesCountS; i++)
			{
				// Ждем когда все модули замкнут контакторы
				if (ModulesData[i].StateMachine.MainState != WORKSTATE_OPERATE)
					tmp = 0;
			}

			if(tmp == 1)
			{
				if(GetTimeFrom(*ReadyConditionTimeStamp) > 500)
					return tmp;

			}
			else
				*ReadyConditionTimeStamp = GetTimeStamp();
		}
		else
			*ReadyConditionTimeStamp = GetTimeStamp();
	}

	return 0;
}

// Precharge proccess function
// Parameters:
// Return value: 0 - precharge doesn't complete, 1 - precharge complete
uint8_t BatteryWaitForPrecharge(const BatteryData_t* Handle, const EcuConfig_t *config, uint32_t *CompleteConditionTimeStamp)
{
	if(ModuleIsTerminal(config))
	{
		if(GET_BAT_MINUS && GET_BAT_PLUS)
		{
			BAT_MINUS(BAT_CLOSE);
			BAT_PLUS(BAT_CLOSE);
			PRECHARGE(BAT_OPEN);

			return 1;
		}
		else
		{
			if(!GET_BAT_MINUS)
			{
				BAT_MINUS(BAT_CLOSE);
				PRECHARGE(BAT_CLOSE);
				*CompleteConditionTimeStamp = GetTimeStamp();
			}
			else if(!GET_BAT_PLUS)
			{
				// Если этап предзаряда не завершается фиксируем время
				if((Handle[config->BatteryIndex].TotalCurrent > ((uint16_t)config->PreZeroCurrent * 10)) ||
					(Handle[config->BatteryIndex].TotalCurrent < -((int16_t)config->PreZeroCurrent * 10)))
					*CompleteConditionTimeStamp = GetTimeStamp();

				if (GetTimeFrom(*CompleteConditionTimeStamp) > config->PreZeroCurrentDuration)
				{
					BAT_PLUS(BAT_CLOSE);
					PRECHARGE(BAT_OPEN);

					return 1;
				}
			}
		}
	}

	return 0;
}

// Function Defenition:
// SOC is calculated for each battery by themself.
// Master calculates SOC as minimum SOC values of each battery
uint8_t BatteryCapacityCalculating(BatteryData_t* Handle, const EcuConfig_t *config, uint8_t StartMeasuringPermission)
{
	if(Handle == 0 || config == 0)
		return 0;

	if(StartMeasuringPermission && ModuleIsTerminal(config))
	{
		Handle[config->BatteryIndex].TotalCurrent = csGetAverageCurrent();
		Handle[config->BatteryIndex].SoC = CapacityCulc(Handle[config->BatteryIndex].TotalCurrent, &OD.Energy_As, config->ModuleCapacity * config->Sys_ModulesCountS);

		return 1;
	}
	else
	{
		Handle[config->BatteryIndex].TotalCurrent = 0;
		Handle[config->BatteryIndex].SoC = 0;

		return 0;
	}
}

void BatteryStatisticCalculating(BatteryData_t *Handle, const BatteryData_t *SourceData, const EcuConfig_t *ecuConfig)
{
    uint32_t SumVoltageTemp = 0;
	uint32_t SumAvgCellVoltage = 0;
	uint32_t SumCurrentTmp = 0;
	uint32_t SumEnergyTmp = 0;
	uint8_t minSoc = 0;
	uint8_t n = 0;
	uint8_t isModulesData = !ecuConfig->IsMaster;
	uint8_t batteryCount = (isModulesData)? ecuConfig->Sys_ModulesCountP :  ecuConfig->Sys_ModulesCountS;

	if(Handle == 0 || SourceData == 0 || ecuConfig == 0 || ecuConfig->ModuleIndex > 0)
		return;

	Handle->TotalCurrent = 0;
	Handle->MinCellVoltage.Voltage_mv = INT16_MAX;
	Handle->MaxCellVoltage.Voltage_mv = INT16_MIN;
	Handle->MinModuleTemperature.Temperature = INT8_MAX;
	Handle->MaxModuleTemperature.Temperature = INT8_MIN;
	Handle->MaxBatteryCurrent.Current = INT16_MIN;
	Handle->MinBatteryCurrent.Current = INT16_MAX;
	Handle->MaxBatteryVoltage.Voltage = 0;
	Handle->MinBatteryVoltage.Voltage = UINT16_MAX;
	Handle->CCL = INT16_MIN;
	Handle->DCL = INT16_MAX;

	for (uint8_t i = 0; i < batteryCount; i++)
	{
        // Сбросить значения
		Handle->OnlineFlags = 1;
		Handle->OnlineNumber = 1;

        for (uint8_t i = 1; i < batteryCount; i++)
		{
			// Если батарея/модуль в сети
            uint32_t bat_delay = GetTimeFrom(SourceData[i].TimeOutCnts);
			if (bat_delay < 1000)
			{
				// Зафиксировать наличие батарей
				Handle->OnlineFlags |= 1L << i;
				Handle->OnlineNumber = Handle->OnlineNumber + 1;
			}
		}


		// Если батареи нет в сети - проверить следующую
		if (!(Handle->OnlineFlags & (1L << i)))
			continue;

		// Среднее значение напряжения на ячейках
		SumAvgCellVoltage += SourceData[i].AvgCellVoltage;
		n++;

		// Рассчёт Max/Min напряжения
		SumVoltageTemp += SourceData[i].TotalVoltage;
		SumCurrentTmp += SourceData[i].TotalCurrent;
		SumEnergyTmp += SourceData[i].DischargeEnergy_Ah;

		// Поиск максимльного напряжяения
		if (SourceData[i].MaxCellVoltage.Voltage_mv > Handle->MaxCellVoltage.Voltage_mv)
		{
			Handle->MaxCellVoltage.Voltage_mv = SourceData[i].MaxCellVoltage.Voltage_mv;
			Handle->MaxCellVoltage.CellNumber = SourceData[i].MaxCellVoltage.CellNumber;
			Handle->MaxCellVoltage.ModuleNumber = i;
			Handle->MaxCellVoltage.BatteryNumber = i;
		}
		// Поиск минимального напряжяения
		if (SourceData[i].MinCellVoltage.Voltage_mv < Handle->MinCellVoltage.Voltage_mv)
		{
			Handle->MinCellVoltage.Voltage_mv = SourceData[i].MinCellVoltage.Voltage_mv;
			Handle->MinCellVoltage.CellNumber = SourceData[i].MinCellVoltage.CellNumber;
			Handle->MinCellVoltage.ModuleNumber = i;
			Handle->MinCellVoltage.BatteryNumber = i;
		}
		// Поиск максимльной температуры
		if (SourceData[i].MaxModuleTemperature.Temperature > Handle->MaxModuleTemperature.Temperature)
		{
			Handle->MaxModuleTemperature.Temperature = SourceData[i].MaxModuleTemperature.Temperature;
			Handle->MaxModuleTemperature.SensorNum = SourceData[i].MaxModuleTemperature.SensorNum;
			Handle->MaxModuleTemperature.ModuleNumber = i;
			Handle->MaxModuleTemperature.BatteryNumber = i;
		}
		// Поиск максимльной температуры
		if (SourceData[i].MinModuleTemperature.Temperature < Handle->MinModuleTemperature.Temperature)
		{
			Handle->MinModuleTemperature.Temperature = SourceData[i].MinModuleTemperature.Temperature;
			Handle->MinModuleTemperature.SensorNum = SourceData[i].MinModuleTemperature.SensorNum;
			Handle->MinModuleTemperature.ModuleNumber = i;
			Handle->MinModuleTemperature.BatteryNumber = i;
		}
		// Поиск минимального и максимального тока
		if (SourceData[i].TotalCurrent > Handle->MaxBatteryCurrent.Current)
		{
			Handle->MaxBatteryCurrent.Current = SourceData[i].TotalCurrent;
			Handle->MaxBatteryCurrent.BatteryNumber = i;
		}
		if (SourceData[i].TotalCurrent < Handle->MinBatteryCurrent.Current)
		{
			Handle->MinBatteryCurrent.Current = SourceData[i].TotalCurrent;
			Handle->MinBatteryCurrent.BatteryNumber = i;
		}

		// Поиск минимального и максимального напряжения
		if (SourceData[i].TotalVoltage > Handle->MaxBatteryVoltage.Voltage)
		{
			Handle->MaxBatteryVoltage.Voltage = SourceData[i].TotalVoltage;
			Handle->MaxBatteryVoltage.BatteryNumber = i;
		}
		if (SourceData[i].TotalVoltage < Handle->MinBatteryVoltage.Voltage)
		{
			Handle->MinBatteryVoltage.Voltage = SourceData[i].TotalVoltage;
			Handle->MinBatteryVoltage.BatteryNumber = i;
		}

		if(SourceData[i].SoC < Handle->SoC)
			minSoc = SourceData[i].SoC;
	}

	if(n > 0)
		Handle->AvgCellVoltage = SumAvgCellVoltage / n;

	Handle->DischargeEnergy_Ah = SumEnergyTmp;
	Handle->TotalVoltage = (isModulesData)? SumVoltageTemp : SumVoltageTemp / n;
	Handle->TotalCurrent = SumCurrentTmp;

	// Мастер умножает минимальный CCL батарей на количество батарей
	Handle->CCL = (!isModulesData)? OD.MasterControl.CCL * batteryCount : OD.MasterControl.CCL;
	Handle->DCL = (!isModulesData)? OD.MasterControl.DCL * batteryCount : OD.MasterControl.DCL;
	Handle->SoC = (!isModulesData)?	minSoc : Handle->SoC;

}

/* *********************** Master Functionality ******************************* */

uint8_t MasterIsReady(const BatteryData_t* Handle, const BatteryData_t *BatteriesData, const EcuConfig_t *config, uint32_t *ReadyConditionTimeStamp)
{
	if(config->IsMaster && Handle->OnlineNumber == config->Sys_ModulesCountP)
	{
		uint8_t tmp = 1;
		for (uint8_t i = 0; i < config->Sys_ModulesCountP - 1; i++)
		{
			if((BatteriesData[i].TotalVoltage < BatteriesData[i+1].TotalVoltage - config->Sys_MaxVoltageDisbalanceP) ||
				(BatteriesData[i].TotalVoltage > BatteriesData[i+1].TotalVoltage + config->Sys_MaxVoltageDisbalanceP))
				tmp = 0;
		}

		if(tmp)
		{
			if(GetTimeFrom(*ReadyConditionTimeStamp) > 500)
				return tmp;
		}
		else
			*ReadyConditionTimeStamp = GetTimeStamp();
	}
	else
		*ReadyConditionTimeStamp = GetTimeStamp();

	return 0;
}
