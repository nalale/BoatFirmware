#include "main.h"

#include "LTC6803.h"

#define MAX_CELLS_AMOUNT		24
#define MAX_TEMP_SENSOR_AMOUNT	4


typedef struct
{
	uint16_t CellsVoltageArray[MAX_CELLS_AMOUNT];
	int16_t TempArray[MAX_TEMP_SENSOR_AMOUNT];

	ltc6803_Sensor_t FirstCellSensor;
	ltc6803_Sensor_t SecondCellSensor;

	EcuConfig_t *config;

	uint8_t RequestedState;
	uint8_t stStateMachine;

	BatteryData_t CellStatistics;

	uint32_t PrechargeDuration_ms;
	uint32_t prechTimeStamp;
	uint32_t timeStamp;

} ModuleHandle_t;

static void initVariable(ModuleHandle_t *Handle, const EcuConfig_t *Config);
static void moduleThread(ModuleHandle_t *handle);
static uint8_t waitForPrecharge(ModuleHandle_t *Handle);

void ModuleMainTask(void *param)
{
	// Initialization
	// -Init variable
	// -Wait for PowerManager Cmd

	// Operation
	// -Contactor control
	// -Cells voltage Measuring
	// -Keep track fault conditions

	ModuleHandle_t module;
	const EcuConfig_t *config = (EcuConfig_t*)param;


	initVariable(&module, config);

	do
	{
		CellsSensorThread(&module->FirstCellSensor);
		CellsSensorThread(&module->SecondCellSensor);
		moduleThread(&module);
	}while(1);

}

void moduleThread(ModuleHandle_t *handle)
{
	switch(handle->stStateMachine)
	{
		// Disabled
		case 0:
		{
			BAT_MINUS(BAT_OPEN);
			BAT_PLUS(BAT_OPEN);
			PRECHARGE(BAT_OPEN);

			// Wait for external command
			if(handle->RequestedState == 1)
				handle->stStateMachine = 1;
		}
			break;

		// Precharge
		case 1:
		{
			BAT_MINUS(BAT_CLOSE);
			PRECHARGE(BAT_CLOSE);

			if(waitForPrecharge(handle))
			{
				BAT_PLUS(BAT_CLOSE);
				PRECHARGE(BAT_OPEN);

				handle->stStateMachine = 2;
			}
		}
			break;

		// Operation Enabled
		case 2:
		{
			BAT_MINUS(BAT_CLOSE);
			BAT_PLUS(BAT_CLOSE);
			PRECHARGE(BAT_OPEN);
		}
			break;

		// Fault Handler
		case 3:
		{

		}
			break;
	}
}

void initVariable(ModuleHandle_t *Handle, const EcuConfig_t *Config)
{
	VoltageSensorParams_t p1;
	p1.BalancingMinVoltage = Config->MinVoltageForBalancing;
	p1.CellNumber = (Config->CellNumber > 12)? 12 :  Config->CellNumber;
	p1.CellTargetVoltage = UINT16_MAX;
	p1.ChipAddress = 0;
	p1.ChipEnableOut = CS1_OUT;
	p1.BalancingTime_s = Config->BalancingTime_s;
	p1.MaxVoltageDiff_mV = Config->MaxVoltageDiff;

	VoltageSensorParams_t p2;
	p2.BalancingMinVoltage = Config->MinVoltageForBalancing;
	p2.CellNumber = (Config->CellNumber > 12)? Config->CellNumber - 12 :  0;
	p2.CellTargetVoltage = UINT16_MAX;
	p2.ChipAddress = 1;
	p2.ChipEnableOut = CS2_OUT;
	p2.BalancingTime_s = Config->BalancingTime_s;
	p2.MaxVoltageDiff_mV = Config->MaxVoltageDiff;

	CellsSensorInit(Handle->FirstCellSensor, &p1, Handle->CellsVoltageArray, Handle->TempArray);
	CellsSensorInit(Handle->SecondCellSensor, &p2, &Handle->CellsVoltageArray[12], &Handle->TempArray[2]);

}

// Precharge proccess function
// Parameters:
// Return value: 0 - precharge doesn't complete, 1 - precharge complete
uint8_t waitForPrecharge(ModuleHandle_t *Handle)
{
	if(ModuleIsPackHeader(&Handle->config))
	{
		if(Handle->prechTimeStamp == 0)
		{
			Handle->prechTimeStamp = GetTimeStamp();
		}
		else
		{
			uint32_t ts;
			// Если этап предзаряда не завершается фиксируем время
			if((Handle->CellStatistics.TotalCurrent > ((uint16_t)&Handle->config->PreZeroCurrent * 10)) ||
				(Handle->CellStatistics.TotalCurrent < -((int16_t)&Handle->config->PreZeroCurrent * 10)))
				Handle->timeStamp = GetTimeStamp();

			Handle->PrechargeDuration_ms = GetTimeFrom(Handle->prechTimeStamp);

			if ((ts = GetTimeFrom(Handle->timeStamp)) >= &Handle->config->PreZeroCurrentDuration)
				return 1;
		}

	}
	else
	{
		return 1;
	}


	return 0;
}
