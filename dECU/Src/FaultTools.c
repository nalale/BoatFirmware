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
	1. ���������� ECU ���� �����
	3. ������� mEcu
	4. ������� �������
	5. ������� ����� PWM 1
	6. ������� ����� PWM 2
	7. ������� ����� PWM 3
	8. ���� ��������� ���
	*/
	
	dtcItem_t* it;
	dtcEnvironment_t env;
	uint8_t TestFailedThisOperationCycle;
	
	//������������ ���������� �������
	it = &dtcEcuConfigMemory;
	if(++it->SamplePeriodCounter >= it->Property->TestSamplePeriod)
	{
		it->SamplePeriodCounter = 0;
		// ��������� ������ � ������ ���� ��������� ������� ���������
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

	//������������ ���������� �������
	it = &dtcUnexpectedPowerOff;
	if(++it->SamplePeriodCounter >= it->Property->TestSamplePeriod)
	{
		it->SamplePeriodCounter = 0;
		// ��������� ������ � ������ ���� ��������� ������� ���������
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

	//������� main Ecu
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
	
	//������� �������
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

	//������� ����� PWM 1
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
	
	//������� ����� PWM 2
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
	
	//������� ����� PWM 3
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
	
	//���� ��������� ���
	it = &dtcMeasuringCircuit;
	if(++it->SamplePeriodCounter >= it->Property->TestSamplePeriod)
	{
		it->SamplePeriodCounter = 0;
		uint8_t _receiveMsg = GetMax11612State();
		if(!_receiveMsg)
		{
			// ���� ������ �� ��������� ���������, �� ������ �� ��������
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

