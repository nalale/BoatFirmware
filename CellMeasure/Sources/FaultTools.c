#include <stdlib.h>
#include <string.h>

#include "FaultTools.h"
#include "Main.h"
#include "TimerFunc.h"
#include "MemoryFunc.h"
#include "BMS_Combi_ECU.h"
#include "../Libs/LTC6803.h"
#include "../Libs/CurrentSens.h"

#include "../MolniaLib/FaultCategory.h"
#include "../MolniaLib/FaultsServices.h"









uint8_t MasterFaultTest(EcuConfig_t *ecuConfig);
uint8_t BatteryFaultTest(EcuConfig_t *ecuConfig);
uint8_t ModuleFaultTest(EcuConfig_t *ecuConfig);

// Handlers
static uint8_t ContactorWeldingHandler(uint32_t *timeStamp);



uint8_t FaultsTest()
{
	EcuConfig_t ecuConfig = GetConfigInstance();
	
	// Эту проверку выполняют все модули
	uint8_t is_critical_fault = ModuleFaultTest(&ecuConfig);
	
	if(!OD.SB.CheckFaults || OD.StateMachine.MainState == WORKSTATE_FAULT)
	{
		OD.FaultsNumber = FillFaultsList(dtcList, dtcListSize, OD.FaultList, 1);
		return 0;	
	}

	// Эту проверку выполняют только крайние модули в ветке
	if(ecuConfig.ModuleIndex == 0)
		is_critical_fault |= BatteryFaultTest(&ecuConfig);

	// Эту проверку выполняет только мастер
	if(ecuConfig.IsMaster)
		is_critical_fault |= MasterFaultTest(&ecuConfig);	
	
	OD.FaultsNumber = FillFaultsList(dtcList, dtcListSize, OD.FaultList, 1);
	OD.OldFaultsNumber = FillFaultsList(dtcList, dtcListSize, OD.OldFaultList, 0);
	
    return is_critical_fault;
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
			if(OD.PackData[i].MainState != OD.MasterControl.RequestState)
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
		if(OD.Faults.Mod_Measurenment == 0)
		{
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
		if(OD.PackData[ecuConfig->BatteryIndex].OnlineNumber != ecuConfig->Sys_ModulesCountS)
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
				if(OD.ModuleData[i].MainState != WORKSTATE_OPERATE)
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
				if(OD.ModuleData[i].MainState != WORKSTATE_OPERATE)
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
		
		int16_t _system_current = OD.PackData[ecuConfig->BatteryIndex].TotalCurrent / 10;
		
		if(_system_current > OD.PackData[ecuConfig->BatteryIndex].DCL)
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
		else if(_system_current < OD.PackData[ecuConfig->BatteryIndex].CCL)
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
		if(OD.PackData[ecuConfig->BatteryIndex].MaxBatteryVoltage.Voltage - OD.PackData[ecuConfig->BatteryIndex].MinBatteryVoltage.Voltage > (ecuConfig->Sys_MaxVoltageDisbalanceS * 10))
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
			if ((OD.PackData[ecuConfig->BatteryIndex].TotalCurrent > ((uint16_t)ecuConfig->PreZeroCurrent * 10)) || 
				(OD.PackData[ecuConfig->BatteryIndex].TotalCurrent < -((int16_t)ecuConfig->PreZeroCurrent * 10)))
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
		if (tmp1 != 0 && OD.SB.CurrentSensorReady)
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
	it = &dtcVcuOffline;
	if(++it->SamplePeriodCounter >= it->Property->TestSamplePeriod)
	{
		it->SamplePeriodCounter = 0;
		if(OD.SB.MsgFromSystem == 0 && !(OD.ConfigData->IsMaster && OD.ConfigData->IsAutonomic))
		{
			TestFailedThisOperationCycle = it->Status.TestFailedThisOperationCycle;
			if(dtcFaultDetection(it, &env, 1) == DTC_TEST_RESULT_FAILED)
			{				
				if(TestFailedThisOperationCycle == 0)
				{
					dtcSetFault(it, &env);
					it->Category = 0;
					SetGeneralFRZR(&frzfVcuOffline);				
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
	8. Аварийное выключение питания
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
			if(dtcFaultDetection(it, &env, 0) == DTC_TEST_RESULT_PASSED)
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
			OD.SB.MsgFromPM = 0;
			if(dtcFaultDetection(it, &env, 0) == DTC_TEST_RESULT_PASSED)
				OD.Faults.Mod_PowerManagerOffline = 0;			
		}
	}


	// Ошибка аварийное отключение питания
	it = &dtcUnexpectedPowerOff;
	if(++it->SamplePeriodCounter >= it->Property->TestSamplePeriod)
	{
		it->SamplePeriodCounter = 0;
		if(!OD.SB.PowerOff_SaveParams)
		{
			if(!it->Status.TestFailedThisOperationCycle)
			{
				dtcSetFault(it, &env);
				it->Category = 0;
				SetGeneralFRZR(&frzfUnexpectedPowerOff);
				OD.Faults.Mod_EmergencyPowerOff = 1;
			}
		}
		else
		{			
			OD.Faults.Mod_EmergencyPowerOff = 0;
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






