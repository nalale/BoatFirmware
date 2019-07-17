/******************************************************************************************
              Фугнкции, отвечаеющие за работу с ПЗУ
******************************************************************************************/

#include "memory.h" 


//-----program-----
  
/******************************************************************************************
						Сохранение данных в ПЗУ
******************************************************************************************/
uint8_t SaveProfile(const PROFILE * param)
{
	uint16_t crc_value;

	crc_value = CRC16((uint8_t *) param, (PROFILE_SIZE / 2) - 1);
	
	if (param->CRC == crc_value)
	{
		WriteToFram(CONFIG_START_ADDRESS, (void*)param, PROFILE_SIZE);
		return 1;
	}
	else
	{
		return 0;
	}
}

/******************************************************************************************
						Восстановление данных из ПЗУ
******************************************************************************************/
uint8_t RecallProfile(PROFILE* param)
{
	uint16_t crc_value;

	ReadFromFram(CONFIG_START_ADDRESS, param, PROFILE_SIZE);
	crc_value = CRC16((uint8_t *) param, (PROFILE_SIZE / 2) - 1);
	CheckProfile(param);
	if (param->CRC == crc_value)
	{
		//WriteToFram(CONFIG_START_ADDRESS, param, PROFILE_SIZE);
		return 1;
	}
	else
	{
		return 0;
	}
}

/***************************************************************************************
                    Проверка настроек на адекватность
***************************************************************************************/
void CheckProfile(PROFILE* param)
{
	if (param->ObdCanID > 0x17)
	{
		param->ObdCanID = 9;
	}
}

/***************************************************************************************
                    Проверка CRC
***************************************************************************************/
uint16_t CRC16(const uint8_t * buf, uint16_t len)
{
	uint16_t crc = 0xFFFF;
	uint16_t DummyInt;
	uint16_t pos, i;
	for (pos = 0; pos < len; pos++)
	{
		DummyInt = (uint16_t)((buf[pos * 2 + 1] << 8) + buf[pos * 2]);   //(UInt16)((TempData[pos * 2 + 1] << 8) + TempData[pos * 2]);
//		buf += 2;
		crc ^= DummyInt;
		for (i = 8; i != 0; i--)
		{
			if ((crc & 0x0001) != 0)
			{
				crc >>= 1;
				crc ^= 0xA001;
			}
			else
				crc >>= 1;
		}
	}
	// Помните, что младший и старший байты поменяны местами, используйте соответственно (или переворачивайте)
	return crc;
}
 
void SaveDTC()
{
	// Структура данных DTC в EEPROM:
	// - Сначала идет список всех возможных неисправностей (см. dtcList), каждый элемент которого содержит только Category и Status.ConfirmedDTC
	// - Далее идет контрольная сумма (НЕ CRC16)
	// - За ней в той же последовательности идут стоп-кадры (сохраняем только если ConfirmedDTC)

	dtcItem_t* dtc;
	uint16_t sum = 0;
	uint16_t addr = DTC_START_ADDRESS;

	for(int i = 0; i < dtcListSize; i++)
	{
		dtc = dtcList[i];
		uint8_t isConfirmed = dtc->Status.ConfirmedDTC;
		sum += '0' + (isConfirmed << 1);		
		WriteToFram(addr++, (void*)&isConfirmed, 1);
	}

	WriteToFram(addr++, (void*)&sum, 1);
	sum >>= 8;
	WriteToFram(addr++, (void*)&sum, 1);
	
	// Стоп-кадры
	const DiagnosticValueFRZF* f;

	for(uint8_t i = 0; i < dtcListSize; i++)
	{
		dtc = dtcList[i];
		f = dtc->Property->FreezeFrameItems;
		// Перебираем и сохраняем каждое значение стоп-кадра
		for(uint8_t n = 0; n < dtc->Property->FreezeFrameItemsCount; n++)
		{
			uint8_t* p = (uint8_t*)f->Ref;
			for(uint8_t k = 0; k < f->Length; k++)
			{				
				if(dtc->Status.ConfirmedDTC)
					WriteToFram(addr, (void*)&p[k], 1);
				
				addr++;
			}

			f++;	// Следующее значение
		}
	}
	
}

uint8_t ReadDTC()
{
	dtcItem_t* dtc;
	uint16_t sum1 = 0, sum2, sum3;
	uint16_t addr = DTC_START_ADDRESS;

	for(int i = 0; i < dtcListSize; i++)
	{
		dtc = dtcList[i];		
		uint8_t isConfirmed = 0;
		ReadFromFram(addr++, (void*)&isConfirmed, 1);
		dtc->Status.ConfirmedDTC = isConfirmed;
		sum1 += '0' + (isConfirmed << 1);
	}
	ReadFromFram(addr++, (void*)&sum2, 1);
	ReadFromFram(addr++, (void*)&sum3, 1);
	sum2 += (sum3 << 8);

	if(sum1 != sum2)
	{
		ecuClearAllDTC();
		return 0;
	}
	
	// Стоп-кадры
	const DiagnosticValueFRZF* f;

	for(int i = 0; i < dtcListSize; i++)
	{
		dtc = dtcList[i];
		f = dtc->Property->FreezeFrameItems;
		// Перебираем и сохраняем каждое значение стоп-кадра
		for(uint8_t n = 0; n < dtc->Property->FreezeFrameItemsCount; n++)
		{
			uint8_t* p = (uint8_t*)f->Ref;
			for(uint8_t k = 0; k < f->Length; k++)
			{				
				ReadFromFram(addr, (void*)&p[k], 1);
				addr++;
			}

			f++;	// Следующее значение
		}
	}

	return 1;
}

// Удаление всех кодов неисправностей
// Возвращает результат выполнения операции: 1 - успешно, 0 - нет
uint8_t ecuClearAllDTC()
{
	for(int i = 0; i < dtcListSize; i++)
	{
		if(dtcList[i]->Status.Value & DTC_IS_STATUS_FAULT)
		{
			dtcList[i]->Status.Value = DTC_CLEAR_STATUS;
			
			ErrorFrame_t* frzf = (ErrorFrame_t*)((DiagnosticValueFRZF*)(dtcList[i]->Property->FreezeFrameItems))->Ref;
			frzf->OccurrenceCounter = 0;			
		}
	}
	
	SaveDTC();
	SetState(INITIALIZATION_STATE);
	
	return 1;
}
