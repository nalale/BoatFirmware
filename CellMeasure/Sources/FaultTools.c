#include <stdlib.h>

#include "FaultTools.h"
#include "Main.h"
#include "TimerFunc.h"
#include "MemoryFunc.h"
#include "BMS_Combi_ECU.h"
#include "../Libs/LTC6803.h"
#include "../Libs/CurrentSens.h"
#include "../MolniaLib/FaultCategory.h"




static uint8_t pointer[512];

typedef struct
{
	struct
	{
		uint8_t
		CriticalFaultExist				: 1,				// Выходить из рабочего режима или нет (серьезная ошибка)
		WarningIndicatorRequested		: 1,				// Требуется ли индицировать о неисправности или нет
		PowerOff						: 1,				// Отключиться полностью при возникновении неисправности

		dummy : 5;
	};
} dtcEnvironment_t;



uint8_t MasterFaultTest(EcuConfig_t *ecuConfig);
uint8_t BatteryFaultTest(EcuConfig_t *ecuConfig);
uint8_t ModuleFaultTest(EcuConfig_t *ecuConfig);

// Handlers
static uint8_t ContactorWeldingHandler(uint32_t *timeStamp);

void SetGeneralFRZR(dtcFRZF_General* f);
void dtcSetFault(dtcItem_t* it, dtcEnvironment_t* env);
uint8_t dtcFaultDetection(dtcItem_t* it, dtcEnvironment_t *env, uint8_t isFault);





uint8_t FaultsTest()
{
	EcuConfig_t ecuConfig = GetConfigInstance();
	
	// Эту проверку выполняют все модули
	uint8_t is_critical_fault = ModuleFaultTest(&ecuConfig);
	
	if(!OD.SB.CheckFaults || OD.StateMachine.MainState == WORKSTATE_FAULT)
	{
		FillFaultsList(OD.FaultList, &OD.FaultsNumber, 1);
		return 0;	
	}

	// Эту проверку выполняют только крайние модули в ветке
	if(ecuConfig.ModuleIndex == 0)
		is_critical_fault |= BatteryFaultTest(&ecuConfig);

	// Эту проверку выполняет только мастер
	if(ecuConfig.IsMaster)
		is_critical_fault |= MasterFaultTest(&ecuConfig);	
	
	FillFaultsList(OD.FaultList, &OD.FaultsNumber, 1);
	
    return is_critical_fault;
}


uint8_t FillFaultsList(uint16_t *Array, uint8_t *FaultNum, uint8_t IsActualFaults)
{
	// Перебор по всем актуальным неисправностям
	// Перебор всех возможных неисправностей
	// Если неисправность подтверждена
	
	for(uint8_t i = 0; i < dtcListSize; i++)
	{	
		// Выбираем между текущими ошибками и старыми
		uint8_t TestFailed = (IsActualFaults)? dtcList[i]->Status.TestFailed : 1;
		
		if(dtcList[i]->Status.ConfirmedDTC & TestFailed)
		{
			uint16_t faultCode = (uint16_t)(dtcList[i]->Property->Code << 8) + (dtcList[i]->Category);
			
			int i = 0;
			for(i = 0; i < *FaultNum; i++)
			{
				// Если в списке уже есть текущий код ошибки - выходим из цикла, 
				if(Array[i] == faultCode || i >= MAX_FAULTS_NUM)
					break;			
			}
			
			if(i == *FaultNum)
			{
				Array[i] = faultCode;
				(*FaultNum)++;
			}
		}
	}
	
	return 0;
}

uint8_t MasterFaultTest(EcuConfig_t *ecuConfig)
{
	/*
	1. Не все батареи в сети
	2. Некорректное состояние одной из батарей
	3. Нет связи с Vcu
	4. Разброс напряжений между батареями
	5. 
	*/
	
	dtcItem_t* it;
	dtcEnvironment_t env;
	uint8_t TestFailedThisOperationCycle;
	
	//Не все батареи в сети
	it = &dtcMst_BatNumber;
	if(++it->SamplePeriodCounter >= it->Property->TestSamplePeriod)
	{
		it->SamplePeriodCounter = 0;
		if(OD.MasterData.OnlineNumber != ecuConfig->Sys_ModulesCountP)
		{
			TestFailedThisOperationCycle = it->Status.TestFailedThisOperationCycle;
			if(dtcFaultDetection(it, &env, 1) == DTC_TEST_RESULT_FAILED)
			{
				if(!TestFailedThisOperationCycle)
				{
					dtcSetFault(it, &env);
					it->Category = 0;
					SetGeneralFRZR(&frzfBatNumber);							
				}
				OD.Faults.Mst_BatteryWrongCount = 1;
			}
		}
		else if(dtcFaultDetection(it, &env, 0) == DTC_TEST_RESULT_PASSED)
		{
			OD.Faults.Mst_BatteryWrongCount = 0;
		}
	}
	
	//Разброс напряжений между батареями
	it = &dtcMst_BatVoltageDiff;
	if(++it->SamplePeriodCounter >= it->Property->TestSamplePeriod)
	{
		it->SamplePeriodCounter = 0;
		if(OD.MasterData.MaxBatteryVoltage.Voltage - OD.MasterData.MinBatteryVoltage.Voltage > ecuConfig->Sys_MaxVoltageDisbalanceP)
		{
			TestFailedThisOperationCycle = it->Status.TestFailedThisOperationCycle;
			if(dtcFaultDetection(it, &env, 1) == DTC_TEST_RESULT_FAILED)
			{				
				if(!TestFailedThisOperationCycle)
				{
					dtcSetFault(it, &env);
					it->Category = 0;
					SetGeneralFRZR(&frzfBatVoltageDiff);							
				}
				OD.Faults.Mst_BatteryVoltageDiff = 1;
			}
		}
		else if(dtcFaultDetection(it, &env, 0) == DTC_TEST_RESULT_PASSED)
		{
			OD.Faults.Mst_BatteryVoltageDiff = 0;
		}
	}
	
	//Некорректное состояние одной из батарей
	it = &dtcMst_BatState;
	if(++it->SamplePeriodCounter >= it->Property->TestSamplePeriod)
	{
		uint8_t flag = 0;
		it->SamplePeriodCounter = 0;
		for(uint8_t i = 0; i < ecuConfig->Sys_ModulesCountP; i++)
		{
			if(OD.BatteryData[i].StateMachine.MainState != OD.MasterControl.RequestState)
			{
				flag = 1;
				TestFailedThisOperationCycle = it->Status.TestFailedThisOperationCycle;
				if(dtcFaultDetection(it, &env, 1) == DTC_TEST_RESULT_FAILED)
				{
					if(!TestFailedThisOperationCycle)
					{
						dtcSetFault(it, &env);
						it->Category = 0;
						SetGeneralFRZR(&frzfBatState);							
					}
					OD.Faults.Mst_BatteryFault = 1;			
				}
			}
		}
		if(flag == 0)
		{
			if(dtcFaultDetection(it, &env, 0) == DTC_TEST_RESULT_PASSED)
				OD.Faults.Mst_BatteryFault = 0;
		}
	}
	
	// Ошибка напряжение ячейки ниже или выше предела
	it = &dtcCellVoltage;
	if(++it->SamplePeriodCounter >= it->Property->TestSamplePeriod)
	{
		it->SamplePeriodCounter = 0;
		if(OD.MasterData.MinCellVoltage.Voltage_mv < ecuConfig->Sys_MinCellVoltage_mV)
		{
			TestFailedThisOperationCycle = it->Status.TestFailedThisOperationCycle;
			if(dtcFaultDetection(it, &env, 1) == DTC_TEST_RESULT_FAILED)
			{
				if(TestFailedThisOperationCycle == 0)
				{
					dtcSetFault(it, &env);
					it->Category = dctCat_SignalStuckLow;
					SetGeneralFRZR(&frzfCellVoltage);
				}
				OD.Faults.Mst_CellVoltageOutOfRange = 1;
			}

		}else if(OD.MasterData.MaxCellVoltage.Voltage_mv > ecuConfig->Sys_MaxCellVoltage_mV)
		{
			TestFailedThisOperationCycle = it->Status.TestFailedThisOperationCycle;
			if(dtcFaultDetection(it, &env, 1) == DTC_TEST_RESULT_FAILED)
			{
				if(TestFailedThisOperationCycle == 0)
				{
					dtcSetFault(it, &env);
					it->Category = dctCat_SignalStuckHigh;
					SetGeneralFRZR(&frzfCellVoltage);
				}
				OD.Faults.Mst_CellVoltageOutOfRange = 1;
			}
		}
		else
		{
			if(dtcFaultDetection(it, &env, 0) == DTC_TEST_RESULT_PASSED)
			{
				OD.Faults.Mst_CellVoltageOutOfRange = 0;
			}
		}
		
	}
	
	return env.CriticalFaultExist;
}

uint8_t BatteryFaultTest(EcuConfig_t *ecuConfig)
{
	/*
	1. Не все модули в сети
	2. Некорректное состояние одного из модулей
	3. Превышение по току нагрузки
	4. Превышение по току заряда
	5. Разброс напряжений между модулями
	6. Длительность предзаряда больше заданной
	7. КЗ в нагрузке предзаряда
	8. Ошибка подключения датчика тока
	*/
	
	dtcItem_t* it;
	dtcEnvironment_t env;
	uint8_t TestFailedThisOperationCycle;
	
	//Не все модули в сети
	it = &dtcBat_WrongModNumber;
	if(++it->SamplePeriodCounter >= it->Property->TestSamplePeriod)
	{
		it->SamplePeriodCounter = 0;
		if(OD.BatteryData[ecuConfig->BatteryIndex].OnlineNumber != ecuConfig->Sys_ModulesCountS)
		{
			TestFailedThisOperationCycle = it->Status.TestFailedThisOperationCycle;
			if(dtcFaultDetection(it, &env, 1) == DTC_TEST_RESULT_FAILED)
			{				
				if(TestFailedThisOperationCycle == 0)
				{
					dtcSetFault(it, &env);
					it->Category = 0;
					SetGeneralFRZR(&frzfModuleNumber);				
					OD.Faults.Bat_ModuleWrongCount = 1;								
				}
			}
		}
		else
		{
			if(dtcFaultDetection(it, &env, 0) == DTC_TEST_RESULT_PASSED)
			{	
				OD.Faults.Bat_ModuleWrongCount = 0;		
			}
		}			
		
	}
	
	//Некорректное состояние одного из модулей
	static uint8_t module_fault = 0;
	it = &dtcBat_WrongModState;
	if(++it->SamplePeriodCounter >= it->Property->TestSamplePeriod)
	{
		uint8_t flag = 0;
		it->SamplePeriodCounter = 0;
		// Счет начинается с индекса 1, чтобы не анализировать состояние себя
		for(uint8_t i = 1; i < ecuConfig->Sys_ModulesCountS; i++)
		{
			// В режиме инициализации ждем включения всех модулей
			if(OD.StateMachine.MainState == WORKSTATE_INIT && OD.StateMachine.SubState > 3)
			{
				if(OD.ModuleData[i].StateMachine.MainState != WORKSTATE_OPERATE)
				{
					flag = 1;
					TestFailedThisOperationCycle = it->Status.TestFailedThisOperationCycle;
					if(dtcFaultDetection(it, &env, 1) == DTC_TEST_RESULT_FAILED)
					{			
						if(TestFailedThisOperationCycle == 0)
						{						
							it->Category = 0;
							dtcSetFault(it, &env);
							SetGeneralFRZR(&frzfModuleNumber);							
						}
						OD.Faults.Bat_ModuleFault = 1;
					}
				}				
			}
			// В режиме работы сразу фиксируем ошибку если один из модулей перешел в некорректное состояние
			else if(OD.StateMachine.MainState == WORKSTATE_OPERATE)
			{
				if(OD.ModuleData[i].StateMachine.MainState != WORKSTATE_OPERATE)
				{
					flag = 1;
					TestFailedThisOperationCycle = it->Status.TestFailedThisOperationCycle;
					if(dtcFaultDetection(it, &env, 1) == DTC_TEST_RESULT_FAILED)
					{
						if(TestFailedThisOperationCycle == 0)
						{		
							it->Category = 0;
							dtcSetFault(it, &env);
							SetGeneralFRZR(&frzfModuleNumber);					
						}
						OD.Faults.Bat_ModuleFault = 1;
					}
				}					
			}
			module_fault = flag;
		}
		if(module_fault == 0)
		{
			if(dtcFaultDetection(it, &env, 0) == DTC_TEST_RESULT_PASSED)
			{			
				OD.Faults.Bat_ModuleFault = 0;
			}
		}
	}
	
	// Превышение по току нагрузки
	it = &dtcBat_OverCurrent;
	if(++it->SamplePeriodCounter >= it->Property->TestSamplePeriod)
	{
		it->SamplePeriodCounter = 0;		
		
		int16_t _system_current = OD.BatteryData[ecuConfig->BatteryIndex].TotalCurrent / 10;
		
		if(_system_current > OD.BatteryData[ecuConfig->BatteryIndex].DCL)
		{
			TestFailedThisOperationCycle = it->Status.TestFailedThisOperationCycle;
			if(dtcFaultDetection(it, &env, 1) == DTC_TEST_RESULT_FAILED)
			{			
				if(TestFailedThisOperationCycle == 0)
				{		
					it->Category = dctCat_CircuitCurrentAboveThreshold;
					dtcSetFault(it, &env);
					SetGeneralFRZR(&frzfOverCurrent);			
					OD.Faults.Bat_OverLoad = 1;
				}
			}			
		}
		else if(_system_current < OD.BatteryData[ecuConfig->BatteryIndex].CCL)
		{
			TestFailedThisOperationCycle = it->Status.TestFailedThisOperationCycle;
			if(dtcFaultDetection(it, &env, 1) == DTC_TEST_RESULT_FAILED)
			{			
				if(TestFailedThisOperationCycle == 0)
				{		
					it->Category = dctCat_CircuitCurrentBelowThreshold;
					dtcSetFault(it, &env);
					SetGeneralFRZR(&frzfOverCurrent);			
					OD.Faults.Bat_OverLoad = 1;
				}
			}			
		}
		else if(dtcFaultDetection(it, &env, 0) == DTC_TEST_RESULT_PASSED)
		{
			OD.Faults.Bat_OverLoad = 0;
		}
	}
	
	//Разброс напряжений между модулями
	it = &dtcBat_VoltageDiff;
	if(++it->SamplePeriodCounter >= it->Property->TestSamplePeriod)
	{
		it->SamplePeriodCounter = 0;		
		if(OD.BatteryData[ecuConfig->BatteryIndex].MaxBatteryVoltage.Voltage - OD.BatteryData[ecuConfig->BatteryIndex].MinBatteryVoltage.Voltage > (ecuConfig->Sys_MaxVoltageDisbalanceS * 10))
		{			
			TestFailedThisOperationCycle = it->Status.TestFailedThisOperationCycle;
			if(dtcFaultDetection(it, &env, 1) == DTC_TEST_RESULT_FAILED)
			{		
				OD.Faults.Bat_ModuleVoltageDiff = 1;
				if(!TestFailedThisOperationCycle)
				{
					it->Category = dctCat_SignalCompareFailure;
					dtcSetFault(it, &env);
					SetGeneralFRZR(&frzfModuleVoltageDiff);			
				}					
			}		
		}
		else if(dtcFaultDetection(it, &env, 0) == DTC_TEST_RESULT_PASSED)
		{
			OD.Faults.Bat_ModuleVoltageDiff = 0;
		}
	}
	
	//Ошибка предзаряд
	it = &dtcBat_Precharge;
	if(GET_PRECHARGE)
	{
		if(++it->SamplePeriodCounter >= it->Property->TestSamplePeriod)
		{
			it->SamplePeriodCounter = 0;
			// Если КЗ в нагрузке
			if(OD.ModuleData[ecuConfig->ModuleIndex].TotalCurrent >= ((int16_t)ecuConfig->PreMaxCurrent * 10))
			{	
				OD.Faults.Bat_ModuleVoltageDiff = 1;
				TestFailedThisOperationCycle = it->Status.TestFailedThisOperationCycle;
				if(!TestFailedThisOperationCycle)
				{
					it->Category = dctCat_CircuitShortToGround;
					dtcSetFault(it, &env);
					SetGeneralFRZR(&frzfPrecharge);			
				}
				PRECHARGE(BAT_OPEN);
				OD.Faults.Bat_Precharge = 1;
			}
			
			// Если длительность предзаряда больше заданной 
			if ((OD.BatteryData[ecuConfig->BatteryIndex].TotalCurrent > ((uint16_t)ecuConfig->PreZeroCurrent * 10)) || 
				(OD.BatteryData[ecuConfig->BatteryIndex].TotalCurrent < -((int16_t)ecuConfig->PreZeroCurrent * 10)))
			{
				TestFailedThisOperationCycle = it->Status.TestFailedThisOperationCycle;
				if(dtcFaultDetection(it, &env, 1) == DTC_TEST_RESULT_FAILED)
				{		
					TestFailedThisOperationCycle = it->Status.TestFailedThisOperationCycle;
					if(!TestFailedThisOperationCycle)
					{
						it->Category = dctCat_SignalStuckHigh;
						dtcSetFault(it, &env);
						SetGeneralFRZR(&frzfPrecharge);			
					}
					PRECHARGE(BAT_OPEN);
					OD.Faults.Bat_Precharge = 1;	
				}					
			}
			else if(dtcFaultDetection(it, &env, 0) == DTC_TEST_RESULT_PASSED)
			{
				OD.SB.PrechargeDone = 1;
				OD.Faults.Bat_Precharge = 0;	
			}
		}
	}
	
	//Ошибка подключения датчика тока
	it = &dtcBat_CurrentSensor;
	if(++it->SamplePeriodCounter >= it->Property->TestSamplePeriod)
	{
		uint8_t tmp1;
		it->SamplePeriodCounter = 0;
		
		tmp1 = csGetCircuitState();
		if (tmp1 != 0)
		{
			TestFailedThisOperationCycle = it->Status.TestFailedThisOperationCycle;
			if(dtcFaultDetection(it, &env, 1) == DTC_TEST_RESULT_FAILED)
			{				
				// Стоп-кадр
				if(TestFailedThisOperationCycle == 0)
				{
					it->Category = tmp1;
					dtcSetFault(it, &env);
					SetGeneralFRZR(&frzfBatCurrentSensor);
				}
				
				OD.Faults.Bat_CurrentSens = 1;
			}						
		}
		else if(dtcFaultDetection(it, &env, 0) == DTC_TEST_RESULT_PASSED)
		{
			OD.Faults.Bat_CurrentSens = 0;
			
		}
	}
	
	//Таймаут надсистемы
	it = &dtcBat_BmsTimeout;
	if(++it->SamplePeriodCounter >= it->Property->TestSamplePeriod)
	{
		it->SamplePeriodCounter = 0;
		if(OD.SB.MsgFromSystem == 0)
		{
			TestFailedThisOperationCycle = it->Status.TestFailedThisOperationCycle;
			if(dtcFaultDetection(it, &env, 1) == DTC_TEST_RESULT_FAILED)
			{				
				if(TestFailedThisOperationCycle == 0)
				{
					dtcSetFault(it, &env);
					it->Category = 0;
					SetGeneralFRZR(&frzfBatBmsTimeout);				
					OD.Faults.Bat_ExtTimeout = 1;								
				}
			}
		}
		else
		{
			dtcFaultDetection(it, &env, 0);
			OD.SB.MsgFromSystem = 0;
		}		
	}
	
	return env.CriticalFaultExist;
}

uint8_t ModuleFaultTest(EcuConfig_t *ecuConfig)
{
	/*
	0. Ошибка CRC конфига
	1. Контактор остается закрытым
	2. Контактор не замыкается
	3. Температура превышает макс. значение
	4. Температура ниже мин. значения
	5. Обрыв цепи измерения ltc6803
	6. Неверное опорное напряжение ltc6803
	7. Таймаут PM
	*/
	
	dtcItem_t* it;
	uint8_t TestFailedThisOperationCycle;
	dtcEnvironment_t env;
	int tmp1, tmp2;
	
	env.CriticalFaultExist = 0;
	env.PowerOff = 0;
	env.WarningIndicatorRequested = 0;
	
	// Ошибка программной памяти
	it = &dtcEcuConfigMemory;
	if(OD.Faults.Mod_ConfigCrc)
	{
		if(!it->Status.TestFailedThisOperationCycle)
		{
			dtcSetFault(it, &env);
			it->Category = dctCat_MissingCalibration;
			SetGeneralFRZR(&frzfEcuConfigMemory);	
			OD.Faults.Mod_ConfigCrc = 1;
		}
	}
	else
	{
		dtcFaultDetection(it, &env, 0);
		OD.Faults.Mod_ConfigCrc = 0;
	}
		
	
	// Проверка контакторов	
	it = &dtcContactor;
	// До включения контакторов проверяем залипание
	if (OD.StateMachine.MainState == WORKSTATE_INIT)
	{
		if ((FB_PLUS && !GET_BAT_PLUS) || (FB_MINUS && !GET_BAT_MINUS))
		{
			if(!it->Status.TestFailedThisOperationCycle && !OD.SB.EmergencyMode)
			{				
				dtcSetFault(it, &env);
				it->Category = dctCat_ActuatorStuckClosed;
				SetGeneralFRZR(&frzfContactor);
			}
			if((FB_PLUS && !GET_BAT_PLUS))
				OD.Faults.Mod_ContactorPlusFB = 1;
			if(FB_MINUS && !GET_BAT_MINUS)
				OD.Faults.Mod_ContactorMinusFB = 1;
		}
		else
		{
			dtcFaultDetection(it, &env, 0);
			OD.Faults.Mod_ContactorPlusFB = 0;
			OD.Faults.Mod_ContactorMinusFB = 0;
		}
	}
	// Не замыкается
	else
	{
		if(++it->SamplePeriodCounter >= it->Property->TestSamplePeriod)
		{
			it->SamplePeriodCounter = 0;
			uint8_t fb_p_st = FB_PLUS;
			uint8_t fb_n_st = FB_MINUS;
			uint8_t p_cmd = GET_BAT_PLUS;
			uint8_t n_cmd = GET_BAT_MINUS;
			
			if ((!fb_p_st && p_cmd) || (!fb_n_st && n_cmd))
			{
				TestFailedThisOperationCycle = it->Status.TestFailedThisOperationCycle;
				if(dtcFaultDetection(it, &env, 1) == DTC_TEST_RESULT_FAILED)
				{
					if(TestFailedThisOperationCycle == 0 && !OD.SB.EmergencyMode)
					{
						dtcSetFault(it, &env);
						it->Category = dctCat_ActuatorStuckOpen;
						SetGeneralFRZR(&frzfContactor);
					}
					if(!fb_p_st && p_cmd)
						OD.Faults.Mod_ContactorPlus = 1;
					if(!fb_n_st && n_cmd)
						OD.Faults.Mod_ContactorMinus = 1;
				}
			}else
			{
				if(dtcFaultDetection(it, &env, 0) == DTC_TEST_RESULT_PASSED)
				{
					OD.Faults.Mod_ContactorPlus = 0;
					OD.Faults.Mod_ContactorMinus = 0;
				}
			}
		}
	}
	
	
	//Ошибка температура модуля ниже или выше предела
	it = &dtcCellTemperature;
	if(it->Status.TestFailed == 0)
	{
		// До перегрева/охлаждения берем заданные параметры,
		tmp1 = ecuConfig->TemperatureCCLpoint[0][CCL_DCL_POINTS_NUM - 1];
		tmp2 = ecuConfig->TemperatureCCLpoint[0][0];
	}
	else
	{
		// а для того, чтобы выйти из ошибки - уменьшаем диапазон
		tmp1 = ecuConfig->TemperatureCCLpoint[0][CCL_DCL_POINTS_NUM - 1] - 3;
		tmp2 = ecuConfig->TemperatureCCLpoint[0][0] + 3;
	}
	
	if(++it->SamplePeriodCounter >= it->Property->TestSamplePeriod)
	{
		it->SamplePeriodCounter = 0;
		if(OD.ModuleData[ecuConfig->ModuleIndex].MinModuleTemperature.Temperature < tmp2)
		{
			TestFailedThisOperationCycle = it->Status.TestFailedThisOperationCycle;
			if(dtcFaultDetection(it, &env, 1) == DTC_TEST_RESULT_FAILED)
			{				
				if(TestFailedThisOperationCycle == 0)
				{
					dtcSetFault(it, &env);
					it->Category = dctCat_SignalStuckLow;
					SetGeneralFRZR(&frzfCellTemperature);							
				}
				OD.Faults.Mod_ModuleTempOutOfRange = 1;
			}			
		}
		else if(OD.ModuleData[ecuConfig->ModuleIndex].MaxModuleTemperature.Temperature > tmp1)
		{
			TestFailedThisOperationCycle = it->Status.TestFailedThisOperationCycle;
			if(dtcFaultDetection(it, &env, 1) == DTC_TEST_RESULT_FAILED)
			{				
				if(TestFailedThisOperationCycle == 0)
				{
					dtcSetFault(it, &env);			
					it->Category = dctCat_SignalStuckHigh;
					SetGeneralFRZR(&frzfCellTemperature);							
				}
				OD.Faults.Mod_ModuleTempOutOfRange = 1;
			}
		}
		else
		{
			dtcFaultDetection(it, &env, 0);
			OD.Faults.Mod_ModuleTempOutOfRange = 0;
		}
	}

	// Канал измерения ячеек
	it = &dtcMeasuringCircuit;
	if(++it->SamplePeriodCounter >= it->Property->TestSamplePeriod)
	{
		it->SamplePeriodCounter = 0;

		uint8_t f = ltc6803_GetError(0);
		if(f == LTC6803_FAULTS_NONE)
			f = ltc6803_GetError(1);

		if(f != LTC6803_FAULTS_NONE)
		{
			TestFailedThisOperationCycle = it->Status.TestFailedThisOperationCycle;
			if(dtcFaultDetection(it, &env, 1) == DTC_TEST_RESULT_FAILED)
			{
				if(TestFailedThisOperationCycle == 0)
				{
					switch(f)
					{
						case LTC6803_F_PEC:
							it->Category = dctCat_GeneralChecksumFailure;
							break;

						case LTC6803_F_REF_VOLT:
							it->Category = dctCat_SignalBiasLevelFailure;
							break;

						case LTC6803_F_OPEN_CONNECTION:
							it->Category = dctCat_CircuitOpen;
							break;
						case LTC6803_F_SIGNAL_INVALID:
							it->Category = dctCat_SignalInvalid;
							break;
						default:
							it->Category = dctCat_NoSubtypeInformation;
							break;
					}

					dtcSetFault(it, &env);
					SetGeneralFRZR(&frzfMeasuringCircuit);
				}
				OD.Faults.Mod_Measurenment = 1;
			}
		}
		else
		{
			if(dtcFaultDetection(it, &env, 0) == DTC_TEST_RESULT_PASSED)
			{
				OD.Faults.Mod_Measurenment = 0;
			}
		}
	}
	
	//Таймаут PM
	it = &dtcPMOffline;
	if(++it->SamplePeriodCounter >= it->Property->TestSamplePeriod)
	{
		it->SamplePeriodCounter = 0;
		if(OD.SB.MsgFromPM == 0 && !ecuConfig->IsPowerManager)
		{
			TestFailedThisOperationCycle = it->Status.TestFailedThisOperationCycle;
			if(dtcFaultDetection(it, &env, 1) == DTC_TEST_RESULT_FAILED)
			{				
				if(TestFailedThisOperationCycle == 0)
				{
					dtcSetFault(it, &env);
					it->Category = 0;
					SetGeneralFRZR(&frzfPMOffline);				
					OD.Faults.Mod_PowerManagerOffline = 1;								
				}
			}
		}
		else
		{
			dtcFaultDetection(it, &env, 0);
			OD.Faults.Mod_PowerManagerOffline = 0;			
		}			
		
	}


	return env.CriticalFaultExist;
}


// Faul Handlers
uint8_t ContactorWeldingHandler(uint32_t *timeStamp)
{
	static uint8_t cnt = 0;

	// Every 100ms check contactor conditions
	if(GetTimeFrom(*timeStamp) >= 100 * cnt)
	{
		if(!(cnt & 0x01))
		{
			// If positive contactor is welded negative contactor
			if((FB_PLUS && !GET_BAT_PLUS) && !FB_MINUS)
			{
				BAT_PLUS(BAT_CLOSE);
			}
		}
		else
			BAT_PLUS(BAT_OPEN);

		if(!(cnt & 0x01))
		{
			if(FB_MINUS && !GET_BAT_MINUS && !FB_PLUS)
			{
				BAT_MINUS(BAT_CLOSE);
			}
		}
		else
			BAT_MINUS(BAT_OPEN);

		cnt++;
	}

	if(GetTimeFrom(*timeStamp) >= 1000)
	{
		BAT_PLUS(BAT_OPEN);
		BAT_MINUS(BAT_OPEN);
		return 1;
	}

	return 0;
}


void SetGeneralFRZR(dtcFRZF_General* f)
{
	f->DateTime = OD.SystemTime;
	if (f->OccurrenceCounter < 255)
		f->OccurrenceCounter++;
	
	f->Voltage = 284;
	f->Load = 80;
	f->AmbientTemperature = 22;
	f->Mileage = 32000;
	
//	f->Voltage = OD.Voltage;
//	f->Load = OD.Load;
//	f->AmbientTemperature = OD.AmbientTemperature;
//	f->Mileage = OD.Mileage;
}


void dtcSetFault(dtcItem_t* it, dtcEnvironment_t* env)
{
	// Результат (Failed) текущего завершенного теста
	it->Status.TestFailed = 1;
	// Неисправность подтверждена
	it->Status.ConfirmedDTC = 1;
	// В этом рабочем цикле неисправность была как минимум 1 раз
	it->Status.TestFailedThisOperationCycle = 1;
	// Тест был завершен в этом рабочем цикле или после очистки ошибок как минимум 1 раз
	it->Status.TestNotCompletedThisOperationCycle = 0;	
	// Индицировать неисправность или нет
	it->Status.WarningIndicatorRequested = it->Property->Bits.WarningIndicatorRequested;

	env->CriticalFaultExist |= it->Property->Bits.IsCritical;
	env->WarningIndicatorRequested |= it->Property->Bits.WarningIndicatorRequested;
}

uint8_t dtcFaultDetection(dtcItem_t* it, dtcEnvironment_t *env, uint8_t isFault)
{
	if(isFault)
	{
		if(it->FaultDetectionCounter < it->Property->TestFailedThreshold)
		{
			if(it->FaultDetectionCounter < 0)
				it->FaultDetectionCounter = 1;
			else
				it->FaultDetectionCounter++;
		}
		else
		{
			// Тест завершен: Обнаружена неисправность
			if(!it->Status.TestFailed)
			{
				dtcSetFault(it, env);
				return DTC_TEST_RESULT_FAILED;
			}
		}		
	}
	else
	{
		if(it->FaultDetectionCounter > it->Property->TestPassedThreshold)
		{
			it->FaultDetectionCounter--;
		}
		else
		{
			// Тест завершен: Неисправности нет или пропала
			if(it->Status.TestFailed || it->Status.TestNotCompletedThisOperationCycle)
			{
				// Результат (Passed) текущего завершенного теста
				it->Status.TestFailed = 0;
				// Тест был завершен в этом рабочем цикле или после очистки ошибок как минимум 1 раз
				it->Status.TestNotCompletedThisOperationCycle = 0;
				
				// Если есть возможность автоматически восстанавливать работоспособность
				if(it->Property->Bits.AutoRestore)
					it->Status.TestFailedThisOperationCycle = 0;
					
				return DTC_TEST_RESULT_PASSED;
			}
		}
	}
	
	return DTC_TEST_RESULT_UNCOMPLITED;
}


uint8_t SaveFaults()
{
	// Структура данных DTC в EEPROM:
	// - Сначала идет список всех возможных неисправностей (см. dtcList), каждый элемент которого содержит только Category и Status.ConfirmedDTC
	// - Далее идет контрольная сумма (НЕ CRC16)
	// - За ней в той же последовательности идут стоп-кадры (сохраняем только если ConfirmedDTC)
	
	// Указатель на сохраняемые данные
	uint8_t res = 0;
	dtcItem_t* dtc;
	uint16_t sum = 0;
	uint16_t addr = 0;
	
	for(int i = 0; i < dtcListSize; i++)
	{
		dtc = dtcList[i];
		uint8_t isConfirmed = dtc->Status.ConfirmedDTC;
		sum += dtc->Category + isConfirmed;
		
		*(pointer + addr++) = dtc->Category;
		*(pointer + addr++) = isConfirmed;
	}
	
	*(pointer + addr++) = sum;
	*(pointer + addr++) = sum >> 8;
	
	// Стоп-кадры
//	const DiagnosticValueFRZF* f;
//	
//	for(uint8_t i = 0; i < dtcListSize; i++)
//	{
//		dtc = dtcList[i];
//		f = dtc->Property->FreezeFrameItems;
//		// Перебираем и сохраняем каждое значение стоп-кадра
//		for(uint8_t n = 0; n < dtc->Property->FreezeFrameItemsCount; n++)
//		{
//			uint8_t* p = (uint8_t*)f->Ref;
//			for(uint8_t k = 0; k < f->Length; k++)
//			{
//				if(dtc->Status.ConfirmedDTC)
//					*(pointer + addr) = p[k];
//				addr++;
//			}
//			
//			// Если размера буффера недостаточно увеличиваем на 256 байт
//			if(addr > buf_size)
//			{
//				buf_size = buf_size + 256;
//				realloc(pointer, buf_size);
//			}			
//			
//			f++;	// Следующее значение
//		}
//	}

	
	res = MemEcuDtcWrite(pointer, addr);
	
//	free(pointer);
	
	return res;
}

uint8_t ReadFaults()
{	
	dtcItem_t* dtc;
	uint16_t sum1 = 0, sum2;
	uint16_t addr = 0;
	
	MemEcuDtcRead(pointer, 512);
	
	for(uint16_t cnt = 0; cnt < 512; cnt++)
	{
		if(pointer[cnt] == 0xFF)
			pointer[cnt] = 0;
	}
	
	for(int i = 0; i < dtcListSize; i++)
	{
		dtc = dtcList[i];
		dtc->Category = *(pointer + addr++);
		uint8_t isConfirmed = *(pointer + addr++);
		dtc->Status.ConfirmedDTC = isConfirmed;
		sum1 += dtc->Category + isConfirmed;
	}
	
	sum2 = *(pointer + addr++);
	sum2 += (*(pointer + addr++) << 8);
	
	if(sum1 != sum2)
	{
		ClearFaults();
		return 0;
	}
//	
//	// Стоп-кадры
//	const DiagnosticValueFRZF* f;
//	
//	for(int i = 0; i < dtcListSize; i++)
//	{
//		dtc = dtcList[i];
//		f = dtc->Property->FreezeFrameItems;
//		// Перебираем и сохраняем каждое значение стоп-кадра
//		for(uint8_t n = 0; n < dtc->Property->FreezeFrameItemsCount; n++)
//		{
//			uint8_t* p = (uint8_t*)f->Ref;
//			for(uint8_t k = 0; k < f->Length; k++)
//			{
//				if(dtc->Status.ConfirmedDTC)
//					p[k] = *(pointer + addr);
//				addr++;
//			}
//			
//			f++;	// Следующее значение
//		}
//	}
//	
//	free(pointer);
	return 1;
}

int8_t ClearFaults(void)
{
	uint16_t dtc_cnt = 0;	
	
	// Clear saved fault
	int8_t status = MemEcuDtcClear();	
	
	// Clear runtime faults
	for(uint8_t i = 0; i < dtcListSize; i++)
	{
		if(dtcList[i]->Status.ConfirmedDTC)
			dtc_cnt++;
		
		dtcList[i]->Status.ConfirmedDTC = 0;
		dtcList[i]->Status.TestFailed = 0;
		dtcList[i]->Status.TestFailedThisOperationCycle = 0;
		dtcList[i]->Status.TestNotCompletedThisOperationCycle = 0;
		dtcList[i]->Status.WarningIndicatorRequested = 0;		
	}	
	
	FillFaultsList(OD.OldFaultList, &OD.OldFaultsNumber, 0);
	FillFaultsList(OD.FaultList, &OD.FaultsNumber, 1);
	
	if(status == CMD_SUCCESS)	
		return dtc_cnt;
	else
		return -1;
}


