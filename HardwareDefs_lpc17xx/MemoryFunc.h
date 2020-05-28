#ifndef _MEMORY_FUNC_H_
#define _MEMORY_FUNC_H_

#include <stdint.h>
#include "lpc17xx_iap.h"
#include "../MolniaLib/Config.h"

#define CONFIG_SIZE sizeof(EcuConfig_t)			// Размер конфигурации в байтах
//#define SDATA_SIZE sizeof(StorableData_t)		// Размер сохраняемых данных в байтах

// Адрес конфигурации в FLASH
#define CONFIG_START_ADDRESS		0x60000
#define CONFIG_FLASH_LENGHT			IAP_WRITE_1024
// Адрес сохраняемых данных в FLASH
#define DTC_START_ADDRESS			CONFIG_START_ADDRESS + CONFIG_FLASH_LENGHT //0x68000
#define DTC_FLASH_LENGHT			IAP_WRITE_1024
#define SDATA_START_ADDRESS			DTC_START_ADDRESS + DTC_FLASH_LENGHT
#define SDATA_FLASH_LENGHT			IAP_WRITE_256
#define SDATA_START_RESERVE_ADDR	SDATA_START_ADDRESS + SDATA_FLASH_LENGHT

//#define SDATA_END_ADDRESS			(SDATA_START_ADDRESS + SDATA_SIZE)

//// Адрес сохранения неисправностей
//#define DTC_START_ADDRESS			SDATA_END_ADDRESS

int8_t MemEcuWriteData(uint8_t *Data, uint16_t Length);


void MemEcuCfgRead(uint8_t *data_pointer);
uint8_t MemEcuCfgWrite(uint8_t *data_pointer);


uint8_t MemEcuDtcWrite(uint8_t *data_pointer, uint16_t DataLength);
void MemEcuDtcRead(uint8_t *data_pointer, uint16_t DataLength);

uint8_t MemEcuSDataWrite(uint8_t *data_p, uint16_t Length, uint8_t BufNum);
void MemEcuSDataRead(uint8_t *data_pointer, uint16_t DataLength, uint8_t BufNum);

#endif
