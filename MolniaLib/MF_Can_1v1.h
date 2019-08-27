#ifndef _MF_CAN_1V1_H_
#define _MF_CAN_1V1_H_

// ������ ���
#define GENERAL_ECU_DIAG_ID			1
#define MAIN_ECU_DIAG_ID			10
#define BMS_ECU_DIAG_ID				11

// ������ ������������
#define BMS_COMBI					1
#define MARINE_ECU					2





//#define BASE_CAN_ID					0x100
//#define BASE_CAN_ID_LEN				10

////#define General_Control1_CAN_ID		BASE_CAN_ID + 1
////#define General_Control2_CAN_ID 	BASE_CAN_ID + 2
////#define General_Control3_CAN_ID 	BASE_CAN_ID + 3


////#define General_ECU_CAN_ID			BASE_CAN_ID + BASE_CAN_ID_LEN
//#define General_ECUx_CAN_ID_LEN		2								//���������� ��������� ��� ������ ECU
//#define General_ECU_CAN_ID_LEN		12								//���������� ��������� ��� ����� ������ ECU


////#define Bmu_ECU_CAN_ID				General_ECU_CAN_ID + General_ECU_CAN_ID_LEN
//#define Bmu_ECU_CAN_ID_LEN	2

////#define Bat_ECU_CAN_ID				Bmu_ECU_CAN_ID + Bmu_ECU_CAN_ID_LEN
//#define Bat_ECUx_CAN_ID_LEN			2								//���������� ��������� ��� ������ ECU
//#define Bat_ECU_CAN_ID_LEN			8								//���������� ��������� ��� ����� ������ ECU

////#define Module_ECU_CAN_ID			Bat_ECU_CAN_ID + Bat_ECU_CAN_ID_LEN
//#define Module_ECUx_CAN_ID_LEN		2								//���������� ��������� ��� ������ ECU
//#define Module_ECU_CAN_ID_LEN		32								//���������� ��������� ��� ����� ������ ECU

typedef enum
{
	BASE_CAN_ID_LEN = 10,
	
	General_ECU_CAN_ID_LEN = 12,
	General_ECUx_CAN_ID_LEN = 2,
	
	Bmu_ECU_CAN_ID_LEN = 2,
	
	Bat_ECU_CAN_ID_LEN = 8,
	Bat_ECUx_CAN_ID_LEN = 2,
	
	Module_ECU_CAN_ID_LEN = 32,
	Module_ECUx_CAN_ID_LEN = 2,
	
	Bmu_ECU_RX_ID_LEN	= 2,
	
	
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
	Bat_ECU_CAN_ID = Bmu_ECU_CAN_ID + Bmu_ECU_CAN_ID_LEN,			//0x116 + 0x02 = 0x118
	Module_ECU_CAN_ID = Bat_ECU_CAN_ID + Bat_ECU_CAN_ID_LEN,		//0x118 + 0x08 = 0x120
	
	Bmu_ECU_RX_ID = Module_ECU_CAN_ID + Module_ECU_CAN_ID_LEN,		//0x120 + 0x20 = 0x140
	
	Main_ECU_CAN_ID = Bmu_ECU_RX_ID + Bmu_ECU_RX_ID_LEN,			//0x140 + 0x02 = 0x142
	
} SystemIds_e;

#pragma anon_unions

#pragma pack(1)

// ��������� �� TimeServer

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
	uint8_t AnalogOutput[8];	// 8 ������� ��� ���������� �����������/��� ��������.
} cmGeneralControl2;

typedef struct
{
	uint8_t AnalogOutput[8];	// 8 ������� ��� ���������� �����������/��� ��������.
} cmGeneralControl3;

typedef struct
{
	uint32_t LogicInputsOutputs;
	uint8_t AnalogInput[4];		// 4 ������� ��� ������ ���������� ������
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
		mEcuTimeout				:	1,
		ExternalCanTimeout		:	1,
		PwmCircuit				:	1,
		MeasuringCircuit		:	1,
		PowerSupplyCircuit		:	1,
		dummy					:	2;
	};
} gECU_Fauls_t;




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
	uint16_t Faults;
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
	
    // ����� ��������� ����
    int16_t CCL;
    // ����� ���������� ����
    int16_t DCL;
	// ���������� ������������
	uint16_t TargetVoltage_mV;
	
}BatM_Ext2_t;

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

#pragma pack(4)


#endif
