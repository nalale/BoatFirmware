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


static uint8_t pointer[512];

typedef struct
{
	struct
	{
		uint8_t
		CriticalFaultExist				: 1,				// �������� �� �������� ������ ��� ��� (��������� ������)
		WarningIndicatorRequested		: 1,				// ��������� �� ������������ � ������������� ��� ���
		PowerOff						: 1,				// ����������� ��������� ��� ������������� �������������

		dummy : 5;
	};
} dtcEnvironment_t;


void SetGeneralFRZR(dtcFRZF_General* f);
uint8_t dtcFaultDetection(dtcItem_t* it, dtcEnvironment_t *env, uint8_t isFault);
void dtcSetFault(dtcItem_t* it, dtcEnvironment_t* env);


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
	
	//������� ��������
	it = &dtcInverterOffline;
	if(++it->SamplePeriodCounter >= it->Property->TestSamplePeriod)
	{
		it->SamplePeriodCounter = 0;
		if(!McuRinehartGetOnlineSign(&OD.mcHandler))
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
		
		if(OD.AccPedalChannels[0] > cfgEcu.AccPedalFstCh_0V + 5 || OD.AccPedalChannels[0] < cfgEcu.AccPedalFstCh_MaxV - 5)
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
		uint8_t f = TrimGetState(&OD.TrimDataRx);
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
	it = &dtcPwmCircuit_2;
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
	
	FillFaultsList(OD.FaultList, &OD.FaultsNumber, 1);
    return 0;
}

uint8_t FillFaultsList(uint16_t *Array, uint8_t *FaultNum, uint8_t IsActualFaults)
{
	if(Array == 0 || FaultNum == 0)
		return 1;

	uint8_t j = 0;
	memset(Array, 0, sizeof(Array[0]) * MAX_FAULTS_NUM);
	
	for(uint8_t i = 0; i < dtcListSize; i++)
	{	
		// �������� ����� �������� �������� � �������
		uint8_t TestFailed = (IsActualFaults)? dtcList[i]->Status.TestFailed : 1;
		uint16_t faultCode = (uint16_t)(dtcList[i]->Property->Code << 8) + (dtcList[i]->Category);
		
		if(TestFailed && dtcList[i]->Status.ConfirmedDTC && j < MAX_FAULTS_NUM)
		{
			Array[j++] = faultCode;
		}
	}

	*FaultNum = j;
	
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
	// ��������� (Failed) �������� ������������ �����
	it->Status.TestFailed = 1;
	// ������������� ������������
	it->Status.ConfirmedDTC = 1;
	// � ���� ������� ����� ������������� ���� ��� ������� 1 ���
	it->Status.TestFailedThisOperationCycle = 1;
	// ���� ��� �������� � ���� ������� ����� ��� ����� ������� ������ ��� ������� 1 ���
	it->Status.TestNotCompletedThisOperationCycle = 0;	
	// ������������ ������������� ��� ���
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
			// ���� ��������: ���������� �������������
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
			if(it->FaultDetectionCounter > 0)
				it->FaultDetectionCounter = -1;
			else
				it->FaultDetectionCounter--;
		}
		else
		{
			// ���� ��������: ������������� ��� ��� �������
			if(it->Status.TestFailed || it->Status.TestNotCompletedThisOperationCycle)
			{
				// ��������� (Passed) �������� ������������ �����
				it->Status.TestFailed = 0;
				// ���� ��� �������� � ���� ������� ����� ��� ����� ������� ������ ��� ������� 1 ���
				it->Status.TestNotCompletedThisOperationCycle = 0;
				
				// ���� ���� ����������� ������������� ��������������� �����������������
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
	// ��������� ������ DTC � EEPROM:
	// - ������� ���� ������ ���� ��������� �������������� (��. dtcList), ������ ������� �������� �������� ������ Category � Status.ConfirmedDTC
	// - ����� ���� ����������� ����� (�� CRC16)
	// - �� ��� � ��� �� ������������������ ���� ����-����� (��������� ������ ���� ConfirmedDTC)
	
	// ��������� �� ����������� ������
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
	
	// ����-�����
//	const DiagnosticValueFRZF* f;
//	
//	for(uint8_t i = 0; i < dtcListSize; i++)
//	{
//		dtc = dtcList[i];
//		f = dtc->Property->FreezeFrameItems;
//		// ���������� � ��������� ������ �������� ����-�����
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
//			// ���� ������� ������� ������������ ����������� �� 256 ����
//			if(addr > buf_size)
//			{
//				buf_size = buf_size + 256;
//				realloc(pointer, buf_size);
//			}			
//			
//			f++;	// ��������� ��������
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
//	// ����-�����
//	const DiagnosticValueFRZF* f;
//	
//	for(int i = 0; i < dtcListSize; i++)
//	{
//		dtc = dtcList[i];
//		f = dtc->Property->FreezeFrameItems;
//		// ���������� � ��������� ������ �������� ����-�����
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
//			f++;	// ��������� ��������
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
	IAP_STATUS_CODE status = MemEcuDtcClear();

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

