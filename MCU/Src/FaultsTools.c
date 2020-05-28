#include <string.h>

#include "Main.h"
#include "../Libs/Btn8982.h"
#include "../Libs/TLE_6368g2.h"
#include "../Libs/max11612.h"
#include "mVCU_ECU.h"
#include "TimerFunc.h"
#include "MemoryFunc.h"

#include "MotionControllers/Inverter_Rinehart.h"
#include "UserApplications/HelmDriver.h"

#include "../MolniaLib/FaultsServices.h"

uint8_t FaultsTest(uint8_t TestIsEnabled)
{	
	/*
		0. ������
		1. ��� ����� � ����������
		2. ��� ����� � �����
		3. ��� ����� � ��������
		4. ������� � ������
		5. �������� � ������
		5. �������� ���������� ��������� ������� �����
		6. �������� ���������� ��������� ����
		7. ����� ������������
		8. �������� ����� ������� �����
		9. �������� ����� ����
		10. ���� ���
		11. ���� ��������� ���
		12. ���� �������
		10. �������� ����� ���������� LV
	
	*/
	
	if(!TestIsEnabled || OD.StateMachine.MainState == WORKSTATE_FAULT)	
		return 0;
	
    dtcItem_t* it;
	dtcEnvironment_t env;
	uint8_t TestFailedThisOperationCycle;
	
	//������������ ���������� �������
	it = &dtcEcuConfigMemory;
	if(++it->SamplePeriodCounter >= it->Property->TestSamplePeriod)
	{
		it->SamplePeriodCounter = 0;
		// ��������� ������ � ������ ���� ��������� ������� ���������
		if(OD.FaultsBits.ConfigCrc)
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

	//Inverter timeout
	it = &dtcInverterOffline;
	if(++it->SamplePeriodCounter >= it->Property->TestSamplePeriod)
	{
		it->SamplePeriodCounter = 0;
		if((McuRinegartGetParameter(&OD.mcHandler, mcu_Status) != stateCmd_Disable) && 
			!McuRinehartGetOnlineSign(&OD.mcHandler))
		{
			TestFailedThisOperationCycle = it->Status.TestFailedThisOperationCycle;
			if(dtcFaultDetection(it, &env, 1) == DTC_TEST_RESULT_FAILED)
			{
				if(!TestFailedThisOperationCycle)
				{
					dtcSetFault(it, &env);
					it->Category = 0;
					SetGeneralFRZR(&frzfInverterOffline);							
				}
				OD.FaultsBits.InverterTimeout = 1;
			}
		}
		else if(dtcFaultDetection(it, &env, 0) == DTC_TEST_RESULT_PASSED)
		{
			OD.FaultsBits.InverterTimeout = 0;
		}
	}
	
	//������� �������
	it = &dtcBatteryOffline;
	if(++it->SamplePeriodCounter >= it->Property->TestSamplePeriod)
	{
		it->SamplePeriodCounter = 0;
		if(GetTimeFrom(OD.LogicTimers.BatteryTimer_ms) > 500)
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
				OD.FaultsBits.BatteryTimeout = 1;
			}
		}
		else if(dtcFaultDetection(it, &env, 0) == DTC_TEST_RESULT_PASSED)
		{
			OD.FaultsBits.BatteryTimeout = 0;
		}
	}
	
	//������� ����
	it = &dtcSkfOffline;
	if(++it->SamplePeriodCounter >= it->Property->TestSamplePeriod)
	{
		it->SamplePeriodCounter = 0;
		if(!helmGetOnlineSign())
		{
			TestFailedThisOperationCycle = it->Status.TestFailedThisOperationCycle;
			if(dtcFaultDetection(it, &env, 1) == DTC_TEST_RESULT_FAILED)
			{
				if(!TestFailedThisOperationCycle)
				{
					dtcSetFault(it, &env);
					it->Category = 0;
					SetGeneralFRZR(&frzfSkfOffline);							
				}
				OD.FaultsBits.SteeringTimeout = 1;
			}
		}
		else if(dtcFaultDetection(it, &env, 0) == DTC_TEST_RESULT_PASSED)
		{
			OD.FaultsBits.SteeringTimeout = 0;
		}
	}
	
	// ������� � ������
	it = &dtcBatteryFault;
	if(++it->SamplePeriodCounter >= it->Property->TestSamplePeriod)
	{
		it->SamplePeriodCounter = 0;
		if(OD.BatteryDataRx.MainState == WORKSTATE_FAULT)
		{
			TestFailedThisOperationCycle = it->Status.TestFailedThisOperationCycle;
			if(dtcFaultDetection(it, &env, 1) == DTC_TEST_RESULT_FAILED)
			{
				if(!TestFailedThisOperationCycle)
				{
					dtcSetFault(it, &env);
					it->Category = 0;
					SetGeneralFRZR(&frzfBatteryFault);							
				}
				OD.FaultsBits.BatteryFault = 1;
			}
		}
		else if(dtcFaultDetection(it, &env, 0) == DTC_TEST_RESULT_PASSED)
		{
			OD.FaultsBits.BatteryFault = 0;
		}
	}
	
	// �������� � ������
	it = &dtcInverterFault;
	if(++it->SamplePeriodCounter >= it->Property->TestSamplePeriod)
	{
		it->SamplePeriodCounter = 0;
		if(McuRinegartGetParameter(&OD.mcHandler, mcu_Status) == mcu_Fault)
		{
			TestFailedThisOperationCycle = it->Status.TestFailedThisOperationCycle;
			if(dtcFaultDetection(it, &env, 1) == DTC_TEST_RESULT_FAILED)
			{
				if(!TestFailedThisOperationCycle)
				{
					dtcSetFault(it, &env);
					it->Category = 0;
					SetGeneralFRZR(&frzfInverterFault);							
				}
				OD.FaultsBits.BatteryFault = 1;
			}
		}
		else if(dtcFaultDetection(it, &env, 0) == DTC_TEST_RESULT_PASSED)
		{
			OD.FaultsBits.InverterFault = 0;
		}
	}
	
	// ������ ���������������� ������� �����
	it = &dtcSteeringPosition;
	if(++it->SamplePeriodCounter >= it->Property->TestSamplePeriod)
	{
		it->SamplePeriodCounter = 0;

		int16_t f = SteeringGetState(&OD.SteeringData);
		
		if(f)
		{
			TestFailedThisOperationCycle = it->Status.TestFailedThisOperationCycle;
			if(dtcFaultDetection(it, &env, 1) == DTC_TEST_RESULT_FAILED)
			{
				if(!TestFailedThisOperationCycle)
				{
					switch(f)
					{
						case STEERING_F_CURRENT1:
							it->Category = dctCat_CircuitCurrentAboveThreshold;
							break;
						case STEERING_F_CURRENT2:
							it->Category = dctCat_CircuitCurrentAboveThreshold;
							break;
						case STEERING_F_FEEDBACK:
							it->Category = dctCat_SignalInvalid;
							break;
						case STEERING_F_POSITION:
							it->Category = dctCat_CommandedPositionNotReachable;
							break;
					}

					dtcSetFault(it, &env);

					SetGeneralFRZR(&frzfSteeringPosition);							
				}
				OD.FaultsBits.SteeringPosition = 1;
			}
		}
		else if(dtcFaultDetection(it, &env, 0) == DTC_TEST_RESULT_PASSED)
		{
			OD.FaultsBits.SteeringPosition = 0;
		}
	}
	
	// ������ ���������������� ����
	it = &dtcSteeringPosition;
	if(++it->SamplePeriodCounter >= it->Property->TestSamplePeriod)
	{
		it->SamplePeriodCounter = 0;
	}
	
	// ������ ������ ����� ������������
	it = &dtcAcceleratorPosition;
	if(++it->SamplePeriodCounter >= it->Property->TestSamplePeriod)
	{
		EcuConfig_t cfgEcu = GetConfigInstance();
		it->SamplePeriodCounter = 0;
		
		if((OD.AccPedalChannels[0] > cfgEcu.AccPedalFstCh_0V + 5 || OD.AccPedalChannels[0] < cfgEcu.AccPedalFstCh_MaxV - 5) || 
			(OD.AccPedalChannels[1] > cfgEcu.AccPedalFstCh_0V + 5 || OD.AccPedalChannels[1] < cfgEcu.AccPedalSndCh_MaxV - 5))
		{
			TestFailedThisOperationCycle = it->Status.TestFailedThisOperationCycle;
			if(dtcFaultDetection(it, &env, 1) == DTC_TEST_RESULT_FAILED)
			{
				if(!TestFailedThisOperationCycle)
				{
					dtcSetFault(it, &env);
					it->Category = dctCat_SignalInvalid;
					SetGeneralFRZR(&frzfAcceleratorPosition);							
				}
				OD.FaultsBits.Accelerator = 1;
			}
		}
		if(!OD.SB.Ecu1MeasDataActual)
		{
			TestFailedThisOperationCycle = it->Status.TestFailedThisOperationCycle;
			if(dtcFaultDetection(it, &env, 1) == DTC_TEST_RESULT_FAILED)
			{
				if(!TestFailedThisOperationCycle)
				{
					dtcSetFault(it, &env);
					it->Category = dctCat_SignalPlausibilityFailure;
					SetGeneralFRZR(&frzfAcceleratorPosition);
				}
				OD.FaultsBits.Accelerator = 1;
			}
		}
	}
	
	// ������ �������� ����� ������� �����
//	it = &dtcSteeringFeedBack;
//	if(++it->SamplePeriodCounter >= it->Property->TestSamplePeriod)
//	{
//		EcuConfig_t cfgEcu = GetConfigInstance();
//		it->SamplePeriodCounter = 0;
//
//		int16_t fb = SteeringGetFeed
//
//		if(OD.SteeringData.Feedback_0p1V > cfgEcu.SteeringMaxVal_0p1V || OD.SteeringData.Feedback_V < cfgEcu.SteeringMinVal_0p1V)
//		{
//			TestFailedThisOperationCycle = it->Status.TestFailedThisOperationCycle;
//			if(dtcFaultDetection(it, &env, 1) == DTC_TEST_RESULT_FAILED)
//			{
//				if(!TestFailedThisOperationCycle)
//				{
//					dtcSetFault(it, &env);
//					it->Category = dctCat_SignalInvalid;
//					SetGeneralFRZR(&frzfSteeringFeedBack);
//				}
//				OD.FaultsBits.SteeringFeedback = 1;
//			}
//		}
//		else if(dtcFaultDetection(it, &env, 0) == DTC_TEST_RESULT_PASSED)
//		{
//			OD.FaultsBits.SteeringFeedback = 0;
//		}
//	}
	
	// ������ �������� ����� ����
	it = &dtcTrimFeedBack;
	if(++it->SamplePeriodCounter >= it->Property->TestSamplePeriod)
	{
		uint8_t f = TrimGetParameter(&OD.TrimDataRx, paramTrim_Fault);
		it->SamplePeriodCounter = 0;
		
		if(f)
		{
			TestFailedThisOperationCycle = it->Status.TestFailedThisOperationCycle;
			if(dtcFaultDetection(it, &env, 1) == DTC_TEST_RESULT_FAILED)
			{
				if(!TestFailedThisOperationCycle)
				{
					switch(f)
					{
						case TRIM_F_POSITION:
							it->Category = dctCat_CommandedPositionNotReachable;
							break;

						case TRIM_F_FEEDBACK:
							it->Category = dctCat_CircuitOpen;
							break;
					}

					dtcSetFault(it, &env);
					SetGeneralFRZR(&frzfTrimFeedBack);							
				}
				OD.FaultsBits.TrimFeedback = 1;
			}
		}
		else if(dtcFaultDetection(it, &env, 0) == DTC_TEST_RESULT_PASSED)
		{
			OD.FaultsBits.TrimFeedback = 0;
		}
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
				OD.FaultsBits.PwmCircuit = 1;
			}
		}
		else if(dtcFaultDetection(it, &env, 0) == DTC_TEST_RESULT_PASSED)
		{
			OD.FaultsBits.PwmCircuit = 0;
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
				OD.FaultsBits.PwmCircuit = 1;
			}
		}
		else if(dtcFaultDetection(it, &env, 0) == DTC_TEST_RESULT_PASSED)
		{
			OD.FaultsBits.PwmCircuit = 0;
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
				OD.FaultsBits.PwmCircuit = 1;
			}
		}
		else if(dtcFaultDetection(it, &env, 0) == DTC_TEST_RESULT_PASSED)
		{
			OD.FaultsBits.PwmCircuit = 0;
		}
	}
	
	//���� ��������� ���
	it = &dtcMeasuringCircuit;
	if(++it->SamplePeriodCounter >= it->Property->TestSamplePeriod)
	{
		it->SamplePeriodCounter = 0;
		uint8_t RequestSend = GetMax11612State();
		if(RequestSend)
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
				OD.FaultsBits.MeasuringCircuit = 1;
			}
		}
		else if(dtcFaultDetection(it, &env, 0) == DTC_TEST_RESULT_PASSED)
		{
			OD.FaultsBits.MeasuringCircuit = 0;
		}
	}
	
	//���� �������
	it = &dtcPowerSupplyCircuit;
	if(++it->SamplePeriodCounter >= it->Property->TestSamplePeriod)
	{
		it->SamplePeriodCounter = 0;
		
		Tle6368FaultList_e f = TLE_GetCircuitState();
		if(f > 0)
		{
			TestFailedThisOperationCycle = it->Status.TestFailedThisOperationCycle;
			if(dtcFaultDetection(it, &env, 1) == DTC_TEST_RESULT_FAILED)
			{
				if(!TestFailedThisOperationCycle)
				{
					dtcSetFault(it, &env);
					it->Category = f;
					SetGeneralFRZR(&frzfPowerSupplyCircuit);							
				}
				OD.FaultsBits.PowerSupplyCircuit = 1;
			}
		}
		else if(dtcFaultDetection(it, &env, 0) == DTC_TEST_RESULT_PASSED)
		{
			OD.FaultsBits.PowerSupplyCircuit = 0;
		}
	}
	
	OD.FaultsNumber = FillFaultsList(dtcList, dtcListSize, OD.FaultList, 1);
	OD.OldFaultsNumber = FillFaultsList(dtcList, dtcListSize, OD.OldFaultList, 0);

    return 0;
}


