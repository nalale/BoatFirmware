#include <string.h>

#include "Main.h"
#include "../Libs/Btn8982.h"
#include "../Libs/TLE_6368g2.h"
#include "../Libs/max11612.h"
#include "TimerFunc.h"
#include "MemoryFunc.h"
#include "dVCU_ECU.h"


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
	
	FillFaultsList(OD.FaultList, &OD.FaultsNumber, 1);
	
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
		OD.MainEcuData2.SpecPowerCons = 0;
		OD.MainEcuData2.TrimPosition = 0;
	}
	
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


int8_t SaveFaults()
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

int8_t ReadFaults()
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
		
		if(!dtc->Status.ConfirmedDTC)
		{
			dtc->Status.TestFailed = 0;
			dtc->Status.TestNotCompletedThisOperationCycle = 0;
			dtc->Status.WarningIndicatorRequested = 0;			
		}
		
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
