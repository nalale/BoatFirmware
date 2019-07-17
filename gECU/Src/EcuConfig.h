#ifndef _ECU_CONFIG_H_
#define _ECU_CONFIG_H_

#define AN_IN_COUNT				4
#define DIG_IN_COUNT			10
#define AN_OUT_COUNT			4
#define DIG_OUT_COUNT			10

#define REPEATER_TABLE_SIZE				20

#pragma anon_unions

#pragma pack(4)

typedef struct
{
	uint32_t Id;
	uint32_t Id2;
	uint16_t SendPeriod;
	
	struct
	{
		uint8_t IsActive	: 1;
		uint8_t Ext1		: 1;	
		uint8_t Direction	: 1;	// 0 - пересылка CAN2 -> CAN1
		uint8_t Ext2		: 1;
	};
	
	uint8_t RepCount;
	
} canRepItem_t;



typedef struct
{
	// Общие
    uint8_t DiagnosticID;					// OBD ID
	uint8_t Index;							// Номер блока
		
	uint8_t AnalogOutput[AN_OUT_COUNT];
	uint8_t DigitalOutput[DIG_OUT_COUNT];
	
	uint8_t CurrentThreshold_A[AN_OUT_COUNT];
	
	canRepItem_t RepTable[REPEATER_TABLE_SIZE];
	
	uint16_t PowerOffDelay_ms;
	uint16_t KeyOffTime_ms;
	
	struct
	{
		uint8_t 
		IsPowerManager	: 1,
		dummy			: 3,
		
		PU_IN1			: 1,
		PU_IN2			: 1,
		PU_IN3			: 1,
		PU_IN4			: 1;
	};
	
	uint16_t addition_1;
	uint16_t addition_2;
	uint16_t addition_3;
	uint16_t addition_4;
	uint16_t addition_5;
	uint16_t addition_6;
	uint16_t addition_7;
	
	
	 // Контрольная сумма
    uint16_t CRC;
} EcuConfig_t;


void cfgApply(void);	
EcuConfig_t GetConfigInstance(void);


#endif

