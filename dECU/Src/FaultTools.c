#include <string.h>

#include "Main.h"
#include "../Libs/Btn8982.h"
#include "../Libs/TLE_6368g2.h"
#include "../Libs/max11612.h"
#include "TimerFunc.h"
#include "MemoryFunc.h"
#include "dVCU_ECU.h"
#include "../MolniaLib/FaultsServices.h"

uint8_t FaultsTest(uint8_t TestIsEnabled)
{	
	if(!TestIsEnabled || OD.StateMachine.MainState == WORKSTATE_FAULT)	
		return 0;
	
	
	/*
	1. Напряжение ECU ниже нормы
	3. Таймаут mEcu
	4. Таймаут батареи
	5. Силовой выход PWM 1
	6. Силовой выход PWM 2
	7. Силовой выход PWM 3
	8. Цепь измерения АЦП
	*/
	
	dtcItem_t* it;
	dtcEnvironment_t env;
	uint8_t TestFailedThisOperationCycle;
	
	//Некорректное отключение питания
	it = &dtcEcuConfigMemory;
	if(++it->SamplePeriodCounter >= it->Property->TestSamplePeriod)
	{
		it->SamplePeriodCounter = 0;
		// Фиксируем ошибку в случае если настроена таблица пересылки
		if(OD.Faults.ConfigCrc)
		{
			TestFailedThisOperationCycle = it->Status.TestFailedThisOperationCycle;
			if(dtcFaultDetection(it, &env, 1) == DTC_TEST_RESULT_FAILED)
			{
				if(!TestFailedThisOperationCycle)
				{
					dtcSetFault(it, &env);
					it->Category = 0;
					SetGeneralFRZR(&frzfEcuConfigMemory);
				}
			}
		}
	}

	//Некорректное отключение питания
	it = &dtcUnexpectedPowerOff;
	if(++it->SamplePeriodCounter >= it->Property->TestSamplePeriod)
	{
		it->SamplePeriodCounter = 0;
		// Фиксируем ошибку в случае если настроена таблица пересылки
		if(!OD.SB.PowerOff_SaveParams)
		{
			TestFailedThisOperationCycle = it->Status.TestFailedThisOperationCycle;
			if(dtcFaultDetection(it, &env, 1) == DTC_TEST_RESULT_FAILED)
			{
				if(!TestFailedThisOperationCycle)
				{
					dtcSetFault(it, &env);
					it->Category = 0;
					SetGeneralFRZR(&frzfUnexpectedPowerOff);
				}
			}
		}
	}

	//Таймаут main Ecu
	it = &dtcmEcuOffline;
	if(++it->SamplePeriodCounter >= it->Property->TestSamplePeriod)
	{
		it->SamplePeriodCounter = 0;
		if(!OD.SB.mEcuMsgReceived)
		{
			TestFailedThisOperationCycle = it->Status.TestFailedThisOperationCycle;
			if(dtcFaultDetection(it, &env, 1) == DTC_TEST_RESULT_FAILED)
			{
				if(!TestFailedThisOperationCycle)
				{
					dtcSetFault(it, &env);
					it->Category = 0;
					SetGeneralFRZR(&frzfmEcuOffline);
				}
				OD.Faults.mEcuTimeout = 1;
			}
		}
		else if(dtcFaultDetection(it, &env, 0) == DTC_TEST_RESULT_PASSED)
		{
			OD.Faults.mEcuTimeout = 0;			
		}		
		OD.SB.mEcuMsgReceived = 0;
	}
	
	//Таймаут Батареи
	it = &dtcBatteryOffline;
	if(++it->SamplePeriodCounter >= it->Property->TestSamplePeriod)
	{
		it->SamplePeriodCounter = 0;
		if(!OD.SB.BatteryMsgReceived)
		{
			TestFailedThisOperationCycle = it->Status.TestFailedThisOperationCycle;
			if(dtcFaultDetection(it, &env, 1) == DTC_TEST_RESULT_FAILED)
			{
				if(!TestFailedThisOperationCycle)
				{
					dtcSetFault(it, &env);
					it->Category = 0;
					SetGeneralFRZR(&frzfBatteryOffline);
				}
				OD.Faults.BatteryTimeout = 1;
			}
		}
		else if(dtcFaultDetection(it, &env, 0) == DTC_TEST_RESULT_PASSED)
		{
			OD.Faults.BatteryTimeout = 0;
		}
		OD.SB.BatteryMsgReceived = 0;
	}

	//Силовой выход PWM 1
	it = &dtcPwmCircuit_1;
	if(++it->SamplePeriodCounter >= it->Property->TestSamplePeriod)
	{
		it->SamplePeriodCounter = 0;
		
		Btn8982FaultList_e btnFaultCode = btnGetCircuitState(0);
		if(btnFaultCode > 0)
		{
			TestFailedThisOperationCycle = it->Status.TestFailedThisOperationCycle;
			if(dtcFaultDetection(it, &env, 1) == DTC_TEST_RESULT_FAILED)
			{
				if(!TestFailedThisOperationCycle)
				{
					dtcSetFault(it, &env);
					it->Category = btnFaultCode;
					SetGeneralFRZR(&frzfPwmCircuit_1);							
				}
				OD.Faults.PwmCircuit = 1;
			}
		}
		else if(dtcFaultDetection(it, &env, 0) == DTC_TEST_RESULT_PASSED)
		{
			OD.Faults.PwmCircuit = 0;
		}
	}
	
	//Силовой выход PWM 2
	it = &dtcPwmCircuit_2;
	if(++it->SamplePeriodCounter >= it->Property->TestSamplePeriod)
	{
		it->SamplePeriodCounter = 0;
		
		Btn8982FaultList_e btnFaultCode = btnGetCircuitState(1);
		if(btnFaultCode > 0)
		{
			TestFailedThisOperationCycle = it->Status.TestFailedThisOperationCycle;
			if(dtcFaultDetection(it, &env, 1) == DTC_TEST_RESULT_FAILED)
			{
				if(!TestFailedThisOperationCycle)
				{
					dtcSetFault(it, &env);
					it->Category = btnFaultCode;
					SetGeneralFRZR(&frzfPwmCircuit_2);							
				}
				OD.Faults.PwmCircuit = 1;
			}
		}
		else if(dtcFaultDetection(it, &env, 0) == DTC_TEST_RESULT_PASSED)
		{
			OD.Faults.PwmCircuit = 0;
		}
	}
	
	//Силовой выход PWM 3
	it = &dtcPwmCircuit_3;
	if(++it->SamplePeriodCounter >= it->Property->TestSamplePeriod)
	{
		it->SamplePeriodCounter = 0;
		
		Btn8982FaultList_e btnFaultCode = btnGetCircuitState(2);
		if(btnFaultCode > 0)
		{
			TestFailedThisOperationCycle = it->Status.TestFailedThisOperationCycle;
			if(dtcFaultDetection(it, &env, 1) == DTC_TEST_RESULT_FAILED)
			{
				if(!TestFailedThisOperationCycle)
				{
					dtcSetFault(it, &env);
					it->Category = btnFaultCode;
					SetGeneralFRZR(&frzfPwmCircuit_3);							
				}
				OD.Faults.PwmCircuit = 1;
			}
		}
		else if(dtcFaultDetection(it, &env, 0) == DTC_TEST_RESULT_PASSED)
		{
			OD.Faults.PwmCircuit = 0;
		}
	}
	
	//Цепь измерения АЦП
	it = &dtcMeasuringCircuit;
	if(++it->SamplePeriodCounter >= it->Property->TestSamplePeriod)
	{
		it->SamplePeriodCounter = 0;
		uint8_t _receiveMsg = GetMax11612State();
		if(!_receiveMsg)
		{
			// Если запрос на измерение отправлен, но данные не получены
			TestFailedThisOperationCycle = it->Status.TestFailedThisOperationCycle;
			if(dtcFaultDetection(it, &env, 1) == DTC_TEST_RESULT_FAILED)
			{
				if(!TestFailedThisOperationCycle)
				{
					dtcSetFault(it, &env);
					it->Category = 0;
					SetGeneralFRZR(&frzfMeasuringCircuit);							
				}
				OD.Faults.MeasuringCircuit = 1;
			}
		}
		else if(dtcFaultDetection(it, &env, 0) == DTC_TEST_RESULT_PASSED)
		{
			OD.Faults.MeasuringCircuit = 0;
		}
	}
	
	OD.FaultsNumber = FillFaultsList(dtcList, dtcListSize, OD.FaultList, 1);
	OD.OldFaultsNumber = FillFaultsList(dtcList, dtcListSize, OD.OldFaultList, 0);
	
    return 0;
}

uint8_t FaultHandler()
{
	if(OD.Faults.BatteryTimeout)
	{
		OD.BmuData1.SOC = 0;
	}

	if(OD.Faults.mEcuTimeout)
	{
		OD.MainEcuData1.MotorRpm = 0;
		OD.MainEcuData1.SpecPowerCons = 0;
		OD.MainEcuData1.TrimPosition = 0;
	}
	
	return 0;
}

