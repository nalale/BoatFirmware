#ifndef __SEVCON__
#define __SEVCON__

#include "CanFunc.h"

// ������� Sevcona
#define SEVCON_TIMEOUT      1000
// ���������� �������
#define SEVCON_MOTOR_AMOUNT 2


#define SEV_NMT_CAN_ID			0x00
#define SEV_PDO_TX_CAN_ID(x)	((x == LeftMotor)?	0x180 : 0x1c0)
#define SEV_PDO_RX_CAN_ID(x)	((x == LeftMotor)?	0x189 : 0x1c9)

/***********************************************************************************************************

															���������������� ����

************************************************************************************************************/
// ���������  Sevcon
typedef enum {SevTurnOff, SevTurnOn} OperationStates;
// ���������  CanOpen
typedef enum {NoSevconInformation, PreOperationMode, OperationMode} CanOpenStates;
// ������ �����
typedef enum {NOT_READY_TO_SWITCH_ON = 0x00, FAULT = 0x08, READY_TO_SWITCH_ON_SW = 0x21, OPERATION_ENABLED_SW = 0x27, SWITCH_ON_DISABLE = 0x40, SWITCH_ON = 0x23} SEVCON_STATUSWORDS_ENUM;


// ������ Sevcon
typedef struct
{
	int16_t StatusWord;	
	int16_t ActualRpm;
	int16_t ActualTorque;					// ���������� ������ � %
	int16_t AverageStatorCurrent;			// factor = 0.0625
	int16_t AveragePhaseVoltage;			// factor = 0.0625
	int16_t BatteryCurrent;
	uint16_t CapacitorVoltage;   			// factor = 0.0625	���������� �� �������		
	int16_t MotorTemperature;				
	int16_t SevconTemperature;
	int16_t TargetId;
	int16_t TargetIq;
	int16_t Id;
	int16_t Iq;		
	int16_t OverallDspTemp;
	int16_t ModulationIndex;
	int16_t MaxPowerLimitTorque;
	int16_t PeakTorque;
	
	// TargetData
	int16_t ControlWord;		
	int16_t LimitSpeed;			// ������������ �������� ������
	int16_t TargetTorque;		// ��������� �������� ������
	uint16_t FaultCode;
} SEVCON_DATA;

#pragma pack(1)

typedef struct
{
	uint8_t CanOpenNodeState;
	uint8_t CanOpenNodeId;
} CanOpenNMT;

typedef struct
{
	int16_t ControlWord;
	int32_t TargetSpeed;
	int16_t MaximumTorque;
} SevconRPDO_0;

typedef struct
{
	int16_t DCL;
	int16_t CCL;
} SevconRPDO_1;

#pragma pack(4)

extern OperationStates RequestState;

/*******************************************************************************
 *                       ���������
*******************************************************************************/
uint8_t InvProtocolRx(CanMsg* msg);
// ��������� ��������� ��� Sevcon
void InvProtocolMesGenerate(void);
// ���������� ��������� ������
void SevconSetState(OperationStates State);

#endif
