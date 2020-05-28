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
	// Общие
    uint8_t DiagnosticID;					// OBD ID
	uint16_t BaseCanId;
	
	// Acceleration
	uint8_t AccPedalFstCh_MaxV;
	uint8_t AccPedalFstCh_0V;
	uint8_t AccPedalSndCh_MaxV;
	uint8_t AccPedalTable[12];
	
	// Motor Data
	uint16_t MaxMotorSpeedD;
	uint16_t MaxMotorTorque;
	int16_t MaxMotorT;
	int16_t MaxInverterT;
    
    // Steering Data
	uint8_t SteeringMinVal_0p1V;
	uint8_t SteeringMaxVal_0p1V;
    int16_t SteeringMaxCurrent_0p1A;
	
    int16_t SteeringBrakeSpeedTable[12];
	
	uint16_t SteeringKp;
	uint16_t SteeringKi;
	uint16_t SteeringKd;
	
	// Trim
	uint8_t TrimMinVal_0p1V;
	uint8_t TrimMaxVal_0p1V;
    int16_t DriveUpperLimitFB_0p1V;
		
	// Power
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
	
	uint8_t InvCoolingOn;
	uint8_t MotorCoolingOn;
	
	// Charging
	uint8_t MaxChargingCurrent_A;
	uint8_t ChargersNumber;
	
	uint16_t RateMotorTorque;
	uint16_t addition_3;
	uint16_t addition_4;
	uint16_t addition_5;
	uint16_t addition_6;
	//uint16_t addition_7;	
	
	 // Контрольная сумма
    uint16_t CRC;
} EcuConfig_t;


void cfgApply(void);	
EcuConfig_t GetConfigInstance(void);


#endif

