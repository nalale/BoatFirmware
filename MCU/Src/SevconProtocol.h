#ifndef __SEVCON__
#define __SEVCON__

#include "CanFunc.h"

// Таймаут Sevcona
#define SEVCON_TIMEOUT      1000
// Количество моторов
#define SEVCON_MOTOR_AMOUNT 2


#define SEV_NMT_CAN_ID			0x00
#define SEV_PDO_TX_CAN_ID(x)	((x == LeftMotor)?	0x180 : 0x1c0)
#define SEV_PDO_RX_CAN_ID(x)	((x == LeftMotor)?	0x189 : 0x1c9)

/***********************************************************************************************************

															Пользовательские типы

************************************************************************************************************/
// Состояния  Sevcon
typedef enum {SevTurnOff, SevTurnOn} OperationStates;
// Состояния  CanOpen
typedef enum {NoSevconInformation, PreOperationMode, OperationMode} CanOpenStates;
// Статус ворды
typedef enum {NOT_READY_TO_SWITCH_ON = 0x00, FAULT = 0x08, READY_TO_SWITCH_ON_SW = 0x21, OPERATION_ENABLED_SW = 0x27, SWITCH_ON_DISABLE = 0x40, SWITCH_ON = 0x23} SEVCON_STATUSWORDS_ENUM;


// Данные Sevcon
typedef struct
{
	int16_t StatusWord;	
	int16_t ActualRpm;
	int16_t ActualTorque;					// Акутальный момент в %
	int16_t AverageStatorCurrent;			// factor = 0.0625
	int16_t AveragePhaseVoltage;			// factor = 0.0625
	int16_t BatteryCurrent;
	uint16_t CapacitorVoltage;   			// factor = 0.0625	Напряжение на севконе		
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
	int16_t LimitSpeed;			// Максимальная скорость мотора
	int16_t TargetTorque;		// Требуемый крутящий момент
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
 *                       ПРОТОТИПЫ
*******************************************************************************/
uint8_t InvProtocolRx(CanMsg* msg);
// Генерация сообщений для Sevcon
void InvProtocolMesGenerate(void);
// Установить состояние привод
void SevconSetState(OperationStates State);

#endif
