#include "Main.h"
#include "Config.h"
#include "MemoryFunc.h"
#include "../MolniaLib/MF_Tools.h"






// �������� ������������
uint8_t cfgCheck(EcuConfig_t *_ecuConfig)
{
	uint16_t CRC_loc = cfgCRC(_ecuConfig);
	// ���� �� ��������, �� ���������� ������
	if (_ecuConfig->CRC != CRC_loc)
		return 0;   
	
	return 1;
}

uint16_t cfgCRC(const EcuConfig_t *_ecuConfig)
{
	// ���������� 16-������ ������� � ������������, ��� ����� CRC
	uint16_t cfgLen = CONFIG_SIZE;
	cfgLen /= 4;
	cfgLen--;
	
	return CRC16((uint16_t*)_ecuConfig, cfgLen);
}

/*
 * ������� ������� �� EEPROM
 * ���������:
 ** ��������� �� �������
 * ����������:
 ** 0 - CRC �������
 ** 1 - CRC �� �������
 */
uint8_t cfgRead(EcuConfig_t *EcuConfig)
{	
	// ��������� �� �������
	uint8_t* p = (uint8_t*)EcuConfig;

	MemEcuCfgRead(p);
		
	// �������� CRC ���������� � ���������
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
 * ������� �������� �������� EEPROM
 * ��������:
 ** ��������� �� �������
 * ����������:
 **  FALSE - CRC �� ������� � ������ �� ����������
 **  TRUE - CRC ������� � ������ ��������
 */
uint8_t cfgWrite(const EcuConfig_t *_ecuConfig)
{
    IAP_STATUS_CODE Status;
	// ��������� �� �������
	uint16_t * ptr = (uint16_t *)_ecuConfig;
	// CRC ��������� 
	uint16_t CRC_loc = cfgCRC(_ecuConfig);
	// ���� �� ��������, �� ���������� ������ � �� ���� �� ���������
	if (_ecuConfig->CRC != CRC_loc)
		return 0xFF;   
    
    Status = (IAP_STATUS_CODE)MemEcuCfgWrite((uint8_t*)ptr);

	return Status;	
}



