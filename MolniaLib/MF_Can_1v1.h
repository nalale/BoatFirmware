#ifndef _MF_CAN_1V1_H_
#define _MF_CAN_1V1_H_

// Модели ЭБУ
#define GENERAL_ECU_DIAG_ID			1
#define MAIN_ECU_DIAG_ID			10
#define BMS_ECU_DIAG_ID				11
#define DISPLAY_ECU_DIAG_ID			51

// Модели контроллеров
#define BMS_COMBI					1
#define MARINE_ECU					2


#define MaxGEcuNum		6
#define MaxBatteryNum	4
#define MaxModuleNum	6

typedef enum
{
	BASE_CAN_ID_LEN = 10,
	
	General_ECUx_CAN_ID_LEN = 2,
	General_ECU_CAN_ID_LEN = General_ECUx_CAN_ID_LEN * MaxGEcuNum,
	
	Bmu_ECU_CAN_ID_LEN = 5,
	Bmu_ECU_RX_ID_LEN = 2,
	
	Bat_ECUx_CAN_ID_LEN = 4,
	Bat_ECU_CAN_ID_LEN = Bat_ECUx_CAN_ID_LEN * MaxBatteryNum,

	Module_ECUx_CAN_ID_LEN = 2,
	Module_ECU_CAN_ID_LEN = (Module_ECUx_CAN_ID_LEN * MaxModuleNum) * MaxBatteryNum,

} EcuIdsLen_e;

typedef enum
{
	BASE_CAN_ID = 0x100,
	PM_CAN_ID,
	General_Control1_CAN_ID,
	General_Control2_CAN_ID,
	General_Control3_CAN_ID,
	
	General_ECU_CAN_ID = BASE_CAN_ID + BASE_CAN_ID_LEN,				//0x100 + 0x0A = 0x10a
	Bmu_ECU_CAN_ID = General_ECU_CAN_ID + General_ECU_CAN_ID_LEN,	//0x10A + 0x0C = 0x116
	Bat_ECU_CAN_ID = Bmu_ECU_CAN_ID + Bmu_ECU_CAN_ID_LEN,			//0x116 + 0x05 = 0x11b
	Module_ECU_CAN_ID = Bat_ECU_CAN_ID + Bat_ECU_CAN_ID_LEN,		//0x11b + 0x10 = 0x12B
	
	Bmu_ECU_RX_ID = Module_ECU_CAN_ID + Module_ECU_CAN_ID_LEN,		//0x12B + 0x30 = 0x15B
	
	Main_ECU_CAN_ID = Bmu_ECU_RX_ID + Bmu_ECU_RX_ID_LEN,			//0x15B + 0x02 = 0x15D
	
} SystemIds_e;

#pragma anon_unions

#pragma pack(1)

// Сообщение от TimeServer

typedef struct {
    uint32_t DateTime;

} cmTimeServer;

typedef struct {
    uint8_t PowerState;
	uint8_t PowerLoad;
	uint16_t EcuVoltage_KL30;
} cmPowerManager;

typedef struct
{
	uint32_t  LogicOutput;
} cmGeneralControl1;

typedef struct
{
	uint8_t AnalogOutput[8];	// 8 каналов для управления аналоговыми/ШИМ выходами.
} cmGeneralControl2;

typedef struct
{
	uint8_t AnalogOutput[8];	// 8 каналов для управления аналоговыми/ШИМ выходами.
} cmGeneralControl3;

typedef struct
{
	uint32_t LogicInputsOutputs;
	uint8_t AnalogInput[4];		// 4 каналов для чтения аналоговых входов
} smEcuStatus1;

typedef struct
{
	uint8_t PwmLoadCurrent[4];
	uint16_t Faults;
	int16_t dummy1;
	
} smEcuStatus2;


typedef union
{
	uint16_t Faults;
	struct
	{
		uint8_t
		ConfigCrc				:	1,
		ExternalCanTimeout		:	1,
		PCanTimeout				:	1,
		PwmCircuit				:	1,
		MeasuringCircuit		:	1,
		PowerSupplyCircuit		:	1,
		dummy					:	2;
	};
} gECU_Fauls_t;

typedef union
{
	uint16_t Faults;
	struct
	{
		uint8_t
		ConfigCrc				:	1,
		mEcuTimeout				:	1,
		BatteryTimeout			:	1,
		PwmCircuit				:	1,
		MeasuringCircuit		:	1,
		PowerSupplyCircuit		:	1,
		dummy					:	2;
	};
} dECU_Fauls_t;


// Battery Messages

typedef struct
{	
	uint16_t Mod_Voltage_0p1V;
	uint16_t MaxCellVoltage;
	uint16_t MinCellVoltage;
	uint8_t MaxTemp;
	uint8_t MinTemp;	
} BatModuleStatus1Msg_t;

typedef struct
{
	uint8_t
	Mod_MainState	: 4,
	Mod_SubState	: 4;
	uint16_t Faults;	
	
} BatModuleStatus2Msg_t;

typedef struct
{    
	uint16_t BatTotalVoltage_0p1A;
    uint16_t BatTotalCurrent_0p1A;
    uint16_t BatMaxModVoltage_0p1V;
	uint16_t BatMinModVoltage_0p1V;
} BatBatteryStatus1Msg_t;

typedef struct
{    	
    uint16_t MaxCellVoltage_0p1V;
    uint16_t MinCellVoltage_0p1V;
	uint8_t MaxModTemperature;
	uint8_t MinModTemperature;
	uint8_t Faults;
	uint8_t Soc;
} BatBatteryStatus2Msg_t;

typedef struct
{
	uint8_t
	MainState	:	4,
	SubState	:	4;	
	
	int16_t TotalCurrent_0p1A;
	int16_t TotalVoltage_0p1V;
	int8_t SOC;
	uint8_t MaxTemperature;
	uint8_t MinTemperature;
} BatM_Ext1_t;

typedef struct
{
	uint8_t 
	RequestState		: 4,
	BalancingEnabled	: 1,
	dummy				: 3;
	
    // Лимит зарядного тока
    int16_t CCL;
    // Лимит разрядного тока
    int16_t DCL;
	// Напряжение балансировки
	uint16_t TargetVoltage_mV;
	
}BatM_Ext2_t;

typedef struct
{
	uint16_t MaxCellVoltage_mV;
	uint16_t MinCellVoltage_mV;
	uint32_t dummy1;
} BatM_Ext3_t;

typedef struct
{
	uint8_t
	OpEnabled		:	1,
	dummy1			:	7;	
}BatM_ExtRx_t;

typedef enum {
    PS_PowerOff = 0,
    PS_SavePower3 = 1,
    PS_SavePower2 = 2,
    PS_SavePower1 = 3,
    PS_FullPower = 4,
} PowerState_e;

typedef struct
{
	uint16_t MotorRpm;
	uint8_t MotorTemperature;
	uint8_t InverterTemperature;
	uint8_t TargetTorque;
	uint8_t ActualTorque;
	uint8_t SteeringAngle;
	uint8_t FeedbackAngle;
} MainEcuStatus1_Msg_t;

typedef struct
{
	uint8_t TrimPosition;
	uint16_t SpecPowerCons;
	uint16_t dummy1;
	uint16_t dummy2;
	uint8_t dummy3;
} MainEcuStatus2_Msg_t;

#pragma pack(4)


#endif

