#include "Main.h"
#include "Config.h"
#include "MemoryFunc.h"
#include "../MolniaLib/MF_Tools.h"






// Проверка конфигурации
uint8_t cfgCheck(EcuConfig_t *_ecuConfig)
{
	uint16_t CRC_loc = cfgCRC(_ecuConfig);
	// Если не совпдают, то возвращаем ошибку
	if (_ecuConfig->CRC != CRC_loc)
		return 0;   
	
	return 1;
}

uint16_t cfgCRC(const EcuConfig_t *_ecuConfig)
{
	// Количество 16-битных записей в конфигурации, без учёта CRC
	uint16_t cfgLen = CONFIG_SIZE;
	cfgLen /= 4;
	cfgLen--;
	
	return CRC16((uint16_t*)_ecuConfig, cfgLen);
}

/*
 * Считать профайл из EEPROM
 * аргументы:
 ** указатель на профайл
 * возвращает:
 ** 0 - CRC совпало
 ** 1 - CRC не совпало
 */
uint8_t cfgRead(EcuConfig_t *EcuConfig)
{	
	// Указатель на профайл
	uint8_t* p = (uint8_t*)EcuConfig;

	MemEcuCfgRead(p);
		
	// Сравнить CRC сохранённые и расчётное
	if (EcuConfig->CRC == cfgCRC(EcuConfig))
	{
		return 0;
	}
	else
	{
		return 1;
	}
}
/*
 * Функция записать профайлв EEPROM
 * Аргумент:
 ** указатель на профайл
 * Возвращает:
 **  FALSE - CRC не совпало и запись не состоялась
 **  TRUE - CRC совпало и данные записали
 */
uint8_t cfgWrite(const EcuConfig_t *_ecuConfig)
{
    IAP_STATUS_CODE Status;
	// Указатель на профайл
	uint16_t * ptr = (uint16_t *)_ecuConfig;
	// CRC расчётное 
	uint16_t CRC_loc = cfgCRC(_ecuConfig);
	// Если не совпдают, то возвращаем ошибку и ни чего не сохраняем
	if (_ecuConfig->CRC != CRC_loc)
		return 0xFF;   
    
    Status = (IAP_STATUS_CODE)MemEcuCfgWrite((uint8_t*)ptr);

	return Status;	
}



