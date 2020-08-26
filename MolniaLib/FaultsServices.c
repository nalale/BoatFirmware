#include <string.h>
#include <stdint.h>
#include "FaultsServices.h"

static uint8_t pointer[512];
static uint8_t FaultsNumber;		
static uint16_t FaultList[MAX_FAULTS_NUM];
	
static uint8_t OldFaultsNumber;
static uint16_t OldFaultList[MAX_FAULTS_NUM];

//********************* Common Fault Tools ***************************

uint8_t FillFaultsList(dtcItem_t** dtcList, int16_t dtcListSize, uint16_t *FaultsList, uint8_t IsActualFaults)
{
	uint16_t *Array = (IsActualFaults)? FaultList : OldFaultList;
	uint8_t *FaultNum = (IsActualFaults)? &FaultsNumber : &OldFaultsNumber;

	if(Array == 0 || FaultNum == 0)
			return 1;

	uint8_t j = 0;
	memset(Array, 0, sizeof(Array[0]) * MAX_FAULTS_NUM);

	for(uint8_t i = 0; i < dtcListSize; i++)
	{
		// Выбираем между текущими ошибками и старыми
		uint16_t faultCode = (uint16_t)(dtcList[i]->Property->Code << 8) + (dtcList[i]->Category);

		if(IsActualFaults)
		{
			if(dtcList[i]->Status.TestFailedThisOperationCycle && dtcList[i]->Status.ConfirmedDTC && j < MAX_FAULTS_NUM)
				Array[j++] = faultCode;
		}
		else
		{
			if(!dtcList[i]->Status.TestFailed && dtcList[i]->Status.ConfirmedDTC && j < MAX_FAULTS_NUM)
				Array[j++] = faultCode;
		}
	}

	if(FaultsList > 0)
		memcpy(FaultsList, Array, sizeof(Array[0]) * MAX_FAULTS_NUM);

	*FaultNum = j;

	return *FaultNum;
}

void SetGeneralFRZR(dtcFRZF_General* f)
{
	//f->DateTime = OD.SystemTime;
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
				it->FaultDetectionCounter = 0;
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
			if(it->FaultDetectionCounter > 0)
				it->FaultDetectionCounter = 0;
			else
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
				{
					it->Status.TestFailedThisOperationCycle = 0;
					return DTC_TEST_RESULT_PASSED;
				}
				else if(it->Property->Bits.IsCritical)
				{
					return DTC_TEST_RESULT_UNCOMPLITED;
				}
			}
			else
				return DTC_TEST_RESULT_PASSED;
		}
	}

	return DTC_TEST_RESULT_UNCOMPLITED;
}


uint8_t SaveFaults(dtcItem_t** dtcList, int16_t dtcListSize)
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

uint8_t ReadFaults(dtcItem_t** dtcList, int16_t dtcListSize)
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
		ClearFaults(dtcList, dtcListSize);
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

int8_t ClearFaults(dtcItem_t** dtcList, int16_t dtcListSize)
{
	//int8_t status = 0;
	uint16_t dtc_cnt = 0;

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

	FillFaultsList(dtcList, dtcListSize, 0, 0);
	FillFaultsList(dtcList, dtcListSize, 0, 1);

	return dtc_cnt;
}


