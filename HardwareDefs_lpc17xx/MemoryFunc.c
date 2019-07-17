#include "MemoryFunc.h"



void MemEcuCfgRead(uint8_t *data_pointer)
{
	// адрес в EEPROM
	uint32_t address;
	
	// —читать данные    
    for(address = CONFIG_START_ADDRESS; address < CONFIG_END_ADDRESS; address++)
        *(data_pointer++) = *((uint8_t*)address);
}

uint8_t MemEcuCfgWrite(uint8_t *data_pointer)
{
	return CopyRAM2Flash((uint8_t*)CONFIG_START_ADDRESS, (uint8_t*)data_pointer, (IAP_WRITE_512));
}


uint8_t MemEcuDtcWrite(uint8_t *data_pointer, uint16_t length)
{
	IAP_WRITE_SIZE size = IAP_WRITE_512;
	
	if(length > 512)
		size = IAP_WRITE_1024;
	
	
	return CopyRAM2Flash((uint8_t*)SDATA_START_ADDRESS, (uint8_t*)data_pointer, size);
}

void MemEcuDtcRead(uint8_t *data_pointer, uint16_t DataLength)
{
	// адрес в EEPROM
	uint32_t address;
	uint16_t ptr;
	
	// —читать данные    
    for(address = SDATA_START_ADDRESS, ptr = 0; ptr < DataLength; address++, ptr++)
        *(data_pointer++) = *((uint8_t*)address);
}

uint8_t MemEcuDtcClear(void)
{
	uint32_t sec;
    IAP_STATUS_CODE status;

	// Prepare sectors
    sec = GetSecNum((uint32_t)SDATA_START_ADDRESS);
   	status = PrepareSector(sec, sec);
	if(status != CMD_SUCCESS)
        return status;
    
    status = EraseSector(sec, sec);
	
	return status;
}


