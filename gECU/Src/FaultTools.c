#include "Main.h"
#include "../Libs/Btn8982.h"
#include "../Libs/TLE_6368g2.h"
#include "max11612.h"
#include "gVCU_ECU.h"

#include "TimerFunc.h"
#include "MemoryFunc.h"


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


void SetGeneralFRZR(dtcFRZF_General* f);
uint8_t dtcFaultDetection(dtcItem_t* it, dtcEnvironment_t *env, uint8_t isFault);
void dtcSetFault(dtcItem_t* it, dtcEnvironment_t* env);

uint8_t FaultsTest(uint8_t TestIsEnabled)
{	
	if(!TestIsEnabled || OD.StateMachine.MainState == WORKSTATE_FAULT)	
		return 0;
	
	
	/*
	1. Напряжение ECU ниже нормы
	2. Неожиданное отключение питания
	3. Таймаут внешний CAN
	4. Таймаут внутренний CAN
	5. Силовой выход PWM 1
	6. Силовой выход PWM 2
	7. Силовой выход PWM 3
	8. Цепь измерения АЦП
	9. Цепь питания
	*/
	
	dtcItem_t* it;
	dtcEnvironment_t env;
	uint8_t TestFailedThisOperationCycle;
	
	//Таймаут внешний CAN
	it = &dtcExtCanOffline;
	if(++it->SamplePeriodCounter >= it->Property->TestSamplePeriod)
	{
		it->SamplePeriodCounter = 0;
		// Фиксируем ошибку в случае если настроена таблица пересылки
		if(!OD.SB.ExtCanMsgReceived && OD.RepItemCount != 0)
		{
			TestFailedThisOperationCycle = it->Status.TestFailedThisOperationCycle;
			if(dtcFaultDetection(it, &env, 1) == DTC_TEST_RESULT_FAILED)
			{
				if(!TestFailedThisOperationCycle)
				{
					dtcSetFault(it, &env);
					it->Category = 0;
					SetGeneralFRZR(&frzfExtCanOffline);							
				}
				OD.Faults.ExternalCanTimeout = 1;
			}
		}
		else if(dtcFaultDetection(it, &env, 0) == DTC_TEST_RESULT_PASSED)
		{
			OD.Faults.ExternalCanTimeout = 0;
		}
	}
	
	//Таймаут PCAN
	it = &dtcPCanOffline;
	if(++it->SamplePeriodCounter >= it->Property->TestSamplePeriod)
	{
		it->SamplePeriodCounter = 0;
		if(!OD.SB.PCanMsgReceived)
		{
			TestFailedThisOperationCycle = it->Status.TestFailedThisOperationCycle;
			if(dtcFaultDetection(it, &env, 1) == DTC_TEST_RESULT_FAILED)
			{
				if(!TestFailedThisOperationCycle)
				{
					dtcSetFault(it, &env);
					it->Category = 0;
					SetGeneralFRZR(&frzfPCanOffline);							
				}
				OD.Faults.mEcuTimeout = 1;
			}
		}
		else if(dtcFaultDetection(it, &env, 0) == DTC_TEST_RESULT_PASSED)
		{
			OD.Faults.mEcuTimeout = 0;
		}
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
	
	//Цепь питания
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
				OD.Faults.PowerSupplyCircuit = 1;
			}
		}
		else if(dtcFaultDetection(it, &env, 0) == DTC_TEST_RESULT_PASSED)
		{
			OD.Faults.PowerSupplyCircuit = 0;
		}
	}
	
	FillFaultsList(OD.FaultList, &OD.FaultsNumber, 1);
	
    return 0;
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

uint8_t ClearFaults(void)
{
	return MemEcuDtcClear();	
}
