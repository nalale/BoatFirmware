#include "MemoryFunc.h"

int8_t MemEcuWriteData(uint8_t *Data, uint16_t Length)
{
	int8_t res = CopyRAM2Flash((uint8_t*)CONFIG_START_ADDRESS, (uint8_t*)0, 0);
//	if(res == 0)
//		res = CopyRAM2Flash((uint8_t*)CONFIG_START_ADDRESS, (uint8_t*)data_pointer, Length);

	return res;
}

void MemEcuCfgRead(uint8_t *data_pointer)
{
	// адрес в EEPROM
	uint32_t address;
	
	// —читать данные    
    for(address = CONFIG_START_ADDRESS; address < CONFIG_START_ADDRESS + CONFIG_SIZE; address++)
        *(data_pointer++) = *((uint8_t*)address);
}

uint8_t MemEcuCfgWrite(uint8_t *data_pointer)
{
	return CopyRAM2Flash((uint8_t*)CONFIG_START_ADDRESS, (uint8_t*)data_pointer, (CONFIG_FLASH_LENGHT));
}


uint8_t MemEcuDtcWrite(uint8_t *data_pointer, uint16_t length)
{
	return CopyRAM2Flash((uint8_t*)DTC_START_ADDRESS, (uint8_t*)data_pointer, DTC_FLASH_LENGHT);
}

void MemEcuDtcRead(uint8_t *data_pointer, uint16_t DataLength)
{
	// адрес в EEPROM
	uint32_t address;
	uint16_t ptr;
	
	// —читать данные    
    for(address = DTC_START_ADDRESS, ptr = 0; ptr < DataLength; address++, ptr++)
        *(data_pointer++) = *((uint8_t*)address);
}

uint8_t MemEcuSDataWrite(uint8_t *data_p, uint16_t Length, uint8_t BufNum)
{
	if(BufNum == 0)
		return CopyRAM2Flash((uint8_t*)SDATA_START_ADDRESS, (uint8_t*)data_p, (SDATA_FLASH_LENGHT));
	else
		return CopyRAM2Flash((uint8_t*)SDATA_START_RESERVE_ADDR, (uint8_t*)data_p, (SDATA_FLASH_LENGHT));
}

void MemEcuSDataRead(uint8_t *data_pointer, uint16_t DataLength, uint8_t BufNum)
{
	// адрес в EEPROM
	uint32_t address;
	uint16_t ptr;

	if(BufNum == 0)
	{
		// —читать данные
		for(address = SDATA_START_ADDRESS, ptr = 0; ptr < DataLength; address++, ptr++)
			*(data_pointer++) = *((uint8_t*)address);
	}
	else
	{
		for(address = SDATA_START_RESERVE_ADDR, ptr = 0; ptr < DataLength; address++, ptr++)
			*(data_pointer++) = *((uint8_t*)address);
	}
}
