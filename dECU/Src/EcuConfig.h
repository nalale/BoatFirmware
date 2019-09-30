#ifndef _ECU_CONFIG_H_
#define _ECU_CONFIG_H_

#define TABLE_SIZE				12

#pragma anon_unions

#pragma pack(4)

typedef struct
{
	// РћР±С‰РёРµ
    uint8_t DiagnosticID;					// OBD ID
	uint8_t Index;							// Порядковый номер ECU.
		
	uint16_t MotorRpm[TABLE_SIZE];
	uint16_t SoC[TABLE_SIZE];
	uint16_t TrimPosition[TABLE_SIZE];
	uint16_t SpecPower[TABLE_SIZE];
	
	uint16_t PowerOffDelay_ms;
	uint16_t KeyOffTime_ms;
	
	uint16_t addition_1;
	uint16_t addition_2;
	uint16_t addition_3;
	uint16_t addition_4;
	uint16_t addition_5;
	uint16_t addition_6;
//	uint16_t addition_7;
	
	
	// Контрольная сумма
    uint16_t CRC;
} EcuConfig_t;


void cfgApply(void);	
EcuConfig_t GetConfigInstance(void);


#endif

