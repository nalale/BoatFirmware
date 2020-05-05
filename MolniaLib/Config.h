#ifndef _CONFIG_H_
#define _CONFIG_H_

#include "EcuConfig.h"


#define CONFIG_SIZE sizeof(EcuConfig_t)

// �������� CRC �������
uint8_t cfgCheck(EcuConfig_t *EcuConfig);
// ������ ������� �� ������
uint8_t cfgRead(EcuConfig_t *EcuConfig);
// ���������� ������� � ������
uint8_t cfgWrite(const EcuConfig_t *EcuConfig);
uint16_t cfgCRC(const EcuConfig_t *ecuConfig);







#endif
