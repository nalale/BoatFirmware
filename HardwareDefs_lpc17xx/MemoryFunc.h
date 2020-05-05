#ifndef _MEMORY_FUNC_H_
#define _MEMORY_FUNC_H_

#include <stdint.h>
#include "lpc17xx_iap.h"
#include "../MolniaLib/Config.h"

#define CONFIG_SIZE sizeof(EcuConfig_t)			// Размер конфигурации в байтах
//#define SDATA_SIZE sizeof(StorableData_t)		// Размер сохраняемых данных в байтах

// Адрес конфигурации в FLASH
#define CONFIG_START_ADDRESS		0x60000
#define CONFIG_END_ADDRESS			(CONFIG_START_ADDRESS + CONFIG_SIZE)
// Адрес сохраняемых данных в FLASH
//#define POWER_STATE_ADDRESS			CONFIG_END_ADDRESS
#define SDATA_START_ADDRESS			0x68000
//#define SDATA_END_ADDRESS			(SDATA_START_ADDRESS + SDATA_SIZE)

//// Адрес сохранения неисправностей
//#define DTC_START_ADDRESS			SDATA_END_ADDRESS

void MemEcuCfgRead(uint8_t *data_pointer);
uint8_t MemEcuCfgWrite(uint8_t *data_pointer);


uint8_t MemEcuDtcWrite(uint8_t *data_pointer, uint16_t DataLength);
void MemEcuDtcRead(uint8_t *data_pointer, uint16_t DataLength);
IAP_STATUS_CODE MemEcuDtcClear(void);


#endif
