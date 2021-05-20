#include <stdint.h>
#include <string.h>
#include "Main.h"
#include "User.h"
#include "BatteryMeasuring.h"

#include "AdcFunc.h"
#include "TimerFunc.h"
#include "CanFunc.h"
#include "PwmFunc.h"
#include "SpiFunc.h"
#include "UartFunc.h"
#include "MemoryFunc.h"

#include "BMS_Combi_ECU.h"

#include "lpc17xx_gpio.h"
#include "lpc17xx_clkpwr.h"



#include "../MolniaLib/DateTime.h"
#include "../MolniaLib/MF_Tools.h"
#include "../Libs/CurrentSens.h"

#define MIN_AVAIL_CELL_VOLTAGE_MV	200
static int16_t CurrentLimitArray[CCL_DCL_POINTS_NUM * 2];

void HardwareInit(void);
void PortInit(void);
void LedBlink(void);

// Precharge proccess function
// Parameters:
// Return value: 0 - precharge doesn't complete, 1 - precharge complete
static uint8_t packWaitForPrecharge(const BatteryData_t* Handle, const EcuConfig_t *config, uint32_t *CompleteConditionTimeStamp);

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
	dateTime_SetCurrentTime(&rtc);
	
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

void TestContactorControl()
{
	static uint32_t ts = 0;
	static uint8_t dsch_flag = 0;

	if(dsch_flag)
	{
		LED(1);
		if(GetTimeFrom(ts) > 5000)
		{
			dsch_flag = 0;
			ts = GetTimeStamp();
		}
	}
	else
	{
		LED(0);
		if(GetTimeFrom(ts) > 15000)
		{
			dsch_flag = 1;
			ts = GetTimeStamp();
		}
	}
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
		if(ModuleIsPackHeader(config))
			return packWaitForPrecharge(Handle, config, CompleteConditionTimeStamp);
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

uint8_t ModuleIsPackHeader(const EcuConfig_t *config)
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
	Handle->MinCellVoltage.ModuleNumber = ecuConfig->ModuleIndex;

	// Рассчёт Max/Min напряжения
	for (uint8_t cellInd = 0; cellInd < cellsCount; cellInd++)
	{
		nv++;
		SumVoltage += CellsVoltageArray[cellInd];

		// Поиск максимльного напряжения
		if (CellsVoltageArray[cellInd] > Handle->MaxCellVoltage.Voltage_mv && CellsVoltageArray[cellInd] > MIN_AVAIL_CELL_VOLTAGE_MV)
		{
			Handle->MaxCellVoltage.Voltage_mv = CellsVoltageArray[cellInd];
			Handle->MaxCellVoltage.CellNumber = cellInd;
		}
		// Поиск минимального напряжения
		if (CellsVoltageArray[cellInd] < Handle->MinCellVoltage.Voltage_mv && CellsVoltageArray[cellInd] > MIN_AVAIL_CELL_VOLTAGE_MV)
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

	Handle->MaxModuleTemperature.Temperature = INT8_MIN;
	Handle->MaxModuleTemperature.SensorNum = TMP_SENSOR_IN_MODULE;
	Handle->MaxModuleTemperature.ModuleNumber = ecuConfig->ModuleIndex;

	Handle->MinModuleTemperature.Temperature = INT8_MAX;
	Handle->MinModuleTemperature.SensorNum = TMP_SENSOR_IN_MODULE;
	Handle->MinModuleTemperature.ModuleNumber = ecuConfig->ModuleIndex;

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
	
	Handle->DataIsReady = ((Handle->MinCellVoltage.Voltage_mv == INT16_MAX) || 
							(Handle->MinBatteryCurrent.Current == INT16_MAX) || 
							(Handle->MinModuleTemperature.Temperature == INT8_MAX))? 0 : 1;

}

/* *********************** Battery Functionality ****************************** */
uint8_t packIsModulesOnline(const BatteryData_t* Handle, const EcuConfig_t *config)
{
	uint8_t tmp = 1;

	if (Handle->MinCellVoltage.Voltage_mv <= config->Sys_MinCellVoltage_mV)
		tmp = 0;

	return tmp;
}

uint8_t packIsReady(const BatteryData_t* Handle, const BatteryData_t *ModulesData, const EcuConfig_t *config, uint32_t *ReadyConditionTimeStamp)
{
	if(ModuleIsPackHeader(config))
	{
		uint8_t tmp = 1;
		for (uint8_t i = config->ModuleIndex + 1; i < config->Sys_ModulesCountS; i++)
		{
			// Ждем когда все модули замкнут контакторы
			if (ModulesData[i].MainState != WORKSTATE_OPERATE)
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

	return 0;
}

// Precharge proccess function
// Parameters:
// Return value: 0 - precharge doesn't complete, 1 - precharge complete
uint8_t packWaitForPrecharge(const BatteryData_t* Handle, const EcuConfig_t *config, uint32_t *CompleteConditionTimeStamp)
{
	if(ModuleIsPackHeader(config))
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

void BatteryCheckModulesOnline(BatteryData_t *Handle, const BatteryData_t *SourceData, uint8_t SourceItemsNum)
{	
	uint8_t packsNum = SourceItemsNum;
	uint8_t mod_index = Handle->Index;
	
	Handle->OnlineFlags = 1 << mod_index;
	Handle->OnlineNumber = 1;
	
	for (uint8_t j = mod_index + 1; j < mod_index + packsNum; j++)
	{
		// If module or pack online
		uint32_t bat_delay = GetTimeFrom(SourceData[j].TimeOutCnts);
		if (bat_delay < 1000)
		{
			// Зафиксировать наличие батарей
			Handle->OnlineFlags |= 1L << j;
			Handle->OnlineNumber = Handle->OnlineNumber + 1;
		}
	}
}


void BatteryStatisticCalculating(BatteryData_t *Handle, const BatteryData_t *SourceData, uint8_t SourceItemsNum)
{
	if(Handle == 0 || SourceData == 0)
		return;

    uint32_t SumVoltageTemp = 0;
	uint32_t SumAvgCellVoltage = 0;
	uint32_t SumCurrentTmp = 0;
	uint32_t SumEnergyTmp = 0;
	uint32_t SumTotalEnergyTmp = 0;
	uint32_t MinEnergyTmp = UINT32_MAX;
	uint32_t MinTotalEnergyTmp = UINT32_MAX;
	uint16_t minSoc = UINT16_MAX;
	uint8_t n = 0;
	uint8_t packsNum = SourceItemsNum;
	uint8_t mod_index = Handle->Index;

	Handle->MinCellVoltage.Voltage_mv = INT16_MAX;
	Handle->MaxCellVoltage.Voltage_mv = INT16_MIN;
	Handle->MinModuleTemperature.Temperature = INT8_MAX;
	Handle->MaxModuleTemperature.Temperature = INT8_MIN;
	Handle->MaxBatteryCurrent.Current = INT16_MIN;
	Handle->MinBatteryCurrent.Current = INT16_MAX;
	Handle->MaxBatteryVoltage.Voltage = 0;
	Handle->MinBatteryVoltage.Voltage = UINT16_MAX;
	Handle->DataIsReady = 1;

	for (uint8_t i = mod_index; i < mod_index + packsNum; i++)
	{
		// Если батареи нет в сети - проверить следующую
		if (!(Handle->OnlineFlags & (1L << i)))
			continue;

		// Среднее значение напряжения на ячейках
		SumAvgCellVoltage += SourceData[i].AvgCellVoltage;
		n++;

		// Рассчёт Max/Min напряжения
		SumVoltageTemp += SourceData[i].TotalVoltage;
		SumCurrentTmp += SourceData[i].TotalCurrent;
		SumEnergyTmp += SourceData[i].ActualEnergy_As;
		SumTotalEnergyTmp += SourceData[i].TotalEnergy_As;

		// Reset Ready flag if uncorrent condition
		if(SourceData[i].MaxCellVoltage.Voltage_mv >= INT16_MAX /*ecuConfig->Sys_MaxCellVoltage_mV*/ || SourceData[i].MinCellVoltage.Voltage_mv <= 0 /*ecuConfig->Sys_MinCellVoltage_mV*/)
			Handle->DataIsReady = 0;

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

		if(SourceData[i].SoC < minSoc)
			minSoc = SourceData[i].SoC;

		if(SourceData[i].ActualEnergy_As < MinEnergyTmp)
			MinEnergyTmp = SourceData[i].ActualEnergy_As;

		if(SourceData[i].TotalEnergy_As < MinTotalEnergyTmp)
			MinTotalEnergyTmp = SourceData[i].TotalEnergy_As;
	}

	if(n > 0)
		Handle->AvgCellVoltage = SumAvgCellVoltage / n;

	Handle->SoC = (Handle->PackType == pack_Serial)?	Handle->SoC : minSoc;
	Handle->TotalVoltage = (Handle->PackType == pack_Serial)? SumVoltageTemp : SumVoltageTemp / n;
	Handle->TotalCurrent = (Handle->PackType == pack_Serial)? Handle->TotalCurrent : SumCurrentTmp;
	Handle->ActualEnergy_As = (Handle->PackType == pack_Serial)? Handle->ActualEnergy_As : SumEnergyTmp;
	Handle->TotalEnergy_As = (Handle->PackType == pack_Serial)? Handle->TotalEnergy_As : SumTotalEnergyTmp;
}



uint8_t packGetBalancingPermission(const BatteryData_t *PackHandle, const PackControl_t *PackControl, const EcuConfig_t *ecuConfig)
{
	uint8_t res = 0;

	if(PackHandle->Type == type_Pack)
	{
		// Разрешаем балансировку при токе меньше 1 Ампера
		if(PackHandle->TotalCurrent > 10 || PackHandle->TotalCurrent < -10)
			res = 0;
		else if(PackHandle->MaxCellVoltage.Voltage_mv > PackControl->TargetVoltage_mV)
			res = 1;
	}
	else
		res = PackControl->BalancingEnabled;

	return res;
}

uint16_t packGetBalancingVoltage(const BatteryData_t *PackHandle, const PackControl_t *PackControl, const EcuConfig_t *ecuConfig)
{
	if(PackHandle->Type != type_Pack)
		return PackControl->TargetVoltage_mV;

	uint16_t _minVolt_mV = PackHandle->MinCellVoltage.Voltage_mv;

	// Расчёт напряжения балансировки
	uint16_t TargetVoltTemp_mV = _minVolt_mV + ecuConfig->MaxVoltageDiff;

	if (TargetVoltTemp_mV < ecuConfig->MinVoltageForBalancing)
		TargetVoltTemp_mV = ecuConfig->MinVoltageForBalancing;

	return TargetVoltTemp_mV;
}

uint32_t packGetMinEnergy(const BatteryData_t *SourceHandle, const EcuConfig_t *ecuConfig)
{
	uint32_t result = UINT32_MAX;

	if(SourceHandle == 0 || ecuConfig == 0 || SourceHandle->Type != type_Module)
		return UINT32_MAX;

	for(int i = 0; i < ecuConfig->Sys_ModulesCountS; i++)
	{
		if(result < SourceHandle[i].ActualEnergy_As)
			result = SourceHandle[i].ActualEnergy_As;
	}

	return result;
}

int16_t packGetChargeCurrentLimit(const BatteryData_t* handle, const EcuConfig_t *ecuConfig, int16_t FullCCL_A)
{
	int16_t limit;
	int16_t Charge = handle->CCL;
	int16_t CCLCurrentLimit = 100;

	if(handle->Type == type_Module)
		return 0;

	if(handle->Type == type_Master)
		FullCCL_A = FullCCL_A * ecuConfig->Sys_ModulesCountP;

	// Копируем во временный массив данные по ограничению заряда от напряжения
	memcpy(CurrentLimitArray, ecuConfig->VoltageCCLpoint, CCL_DCL_POINTS_NUM * 4);
	// Вычисляем локальное ограничение заряда
	limit = interpol(CurrentLimitArray, CCL_DCL_POINTS_NUM, handle->MaxCellVoltage.Voltage_mv);
	CCLCurrentLimit = (limit < CCLCurrentLimit)? limit : CCLCurrentLimit;


	// Ограничение по температуре.
/*	limit = interpol((int16_t *)EcuConfig.TemperatureCCLpoint, CCL_DCL_POINTS_NUM, OD.MasterData.MaxModuleTemperature.Temperature);
	if (limit < CCLCurrentLimit)
	{
		CCLCurrentLimit = limit;
	}

	limit = interpol((int16_t *)EcuConfig.TemperatureCCLpoint, CCL_DCL_POINTS_NUM, OD.MasterData.MinModuleTemperature.Temperature);
	if (limit < CCLCurrentLimit)
	{
		CCLCurrentLimit = limit;
	}
*/

	// Перевод процентов в амперы
	int32_t v =  ((int32_t)CCLCurrentLimit * FullCCL_A) / 100;
	CCLCurrentLimit = (FullCCL_A > 0)? 0 : v;

	if (handle->MainState != WORKSTATE_OPERATE)
	{
		Charge = 0;
	}
	else
	{
		// if less or max value
		if((CCLCurrentLimit >= Charge))		//CCL < 0
			Charge = CCLCurrentLimit;	// Снижаем сразу
		else
			Charge -= 1;	// Повышаем постепенно
	}

	return Charge;
}

int16_t packGetDischargeCurrentLimit(const BatteryData_t* handle, const EcuConfig_t *ecuConfig, int16_t FullDCL_A)
{
	int16_t limit;
	int16_t Discharge = handle->DCL;
	int16_t DCLCurrentLimit = 100;

	if(ecuConfig->PacksNumber <= 1 && !ecuConfig->IsMaster)
		return 0;

	if(ecuConfig->IsMaster)
		FullDCL_A = FullDCL_A * ecuConfig->Sys_ModulesCountP;

	// Копируем во временный массив данные по ограничению заряда от напряжения
	memcpy(CurrentLimitArray, (uint8_t*)(ecuConfig->VoltageCCLpoint), CCL_DCL_POINTS_NUM * 2);
	memcpy((uint8_t*)(CurrentLimitArray) + (CCL_DCL_POINTS_NUM * 2), (uint8_t*)(ecuConfig->VoltageCCLpoint) + CCL_DCL_POINTS_NUM * 4, CCL_DCL_POINTS_NUM * 2);
	limit = interpol(CurrentLimitArray, CCL_DCL_POINTS_NUM, handle->MinCellVoltage.Voltage_mv);
	DCLCurrentLimit = (limit < DCLCurrentLimit)? limit : DCLCurrentLimit;


	// Ограничение по температуре.
/*	limit = interpol((int16_t *)EcuConfig.TemperatureCCLpoint, CCL_DCL_POINTS_NUM, OD.MasterData.MaxModuleTemperature.Temperature);
	if (limit < DCLCurrentLimit)
	{
		DCLCurrentLimit = limit;
	}

	limit = interpol((int16_t *)EcuConfig.TemperatureCCLpoint, CCL_DCL_POINTS_NUM, OD.MasterData.MinModuleTemperature.Temperature);
	if (limit < DCLCurrentLimit)
	{
		DCLCurrentLimit = limit;
	}
*/

	// Перевод процентов в амперы
	int32_t v = ((int32_t)DCLCurrentLimit * FullDCL_A) / 100;
	DCLCurrentLimit = v;

	if (handle->MainState != WORKSTATE_OPERATE)
	{
		Discharge = 0;
	}
	else
	{
		// if less or max value
		if(DCLCurrentLimit <= Discharge)
			Discharge = DCLCurrentLimit;	// Снижаем сразу
		else
			Discharge += 1;				// Повышаем постепенно
	}

	return Discharge;
}

/* *********************** Master Functionality ******************************* */

uint8_t ModuleIsPackMaster(const EcuConfig_t *config)
{
	return config->IsMaster == 1;
}

uint8_t MasterIsReady(const BatteryData_t* Handle, const BatteryData_t *BatteriesData, const EcuConfig_t *config, uint32_t *ReadyConditionTimeStamp)
{
	if(*ReadyConditionTimeStamp == 0)
		*ReadyConditionTimeStamp = GetTimeStamp();

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


// --- Flash

int8_t flashWriteSData(const StorageData_t *sdata)
{	
	StorageData_t tmp_sdata;
	
	// write 1st data to 1st and 2nd buffer
	memcpy(&tmp_sdata.Buf_1st, &sdata->Buf_1st, sizeof(sdata->Buf_1st));
	
	tmp_sdata.Buf_1st.Crc = 0;
	
	for(uint8_t i = 0; i < sizeof(sdata->Buf_1st) - sizeof(sdata->Buf_1st.Crc); i++)
		tmp_sdata.Buf_1st.Crc += *((uint8_t*)&tmp_sdata.Buf_1st + i);

	tmp_sdata.Buf_1st.Crc++;
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
	if(crc0 == sdata->Buf_1st.Crc - 1)
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

