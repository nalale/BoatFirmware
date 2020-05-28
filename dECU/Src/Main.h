#ifndef _MAIN_H_
#define _MAIN_H_

#include "LPC17xx.h"
#include "lpc_types.h"

#include "lpc17xx_libcfg_default.h"

#include "FaultTools.h"
#include "ObjectsIndex.h"
#include "protocol.h"
#include "EcuConfig.h"

#include "../BoardDefinitions/MarineEcu_Board.h"

#include "../MolniaLib/Config.h"
#include "../MolniaLib/MF_CAN_1v1.h"
#include "../MolniaLib/PowerManager.h"

#pragma anon_unions

// Максимальное количество в списке ошибок
#define MAX_FAULTS_NUM				10

#define GET_IN_STATE(x, y) (((x) >> (y)) & 0x01)
#define SET_OUT_STATE(x, y) ((x) << (y))

// Состояния конечного автомата
typedef enum {
    WORKSTATE_INIT = 0,
    WORKSTATE_OPERATE,
    WORKSTATE_SHUTDOWN,
    WORKSTATE_FAULT,
    WORKSTATE_CHARGE
} WorkStates_e ;

// Конечный автомат имеет двухуровневую структуру
typedef struct
{
	WorkStates_e MainState;
	uint8_t SubState;
} StateMachine_t;

typedef struct
{
	uint32_t Timer_1ms;	
    uint32_t Timer_10ms;	
	uint32_t Timer_100ms;
    uint32_t Timer_500ms;
    uint32_t Timer_1s;
	uint32_t MainLoopTimer_ms;

    //TimeoutsBlocks timers
    uint32_t ExtCanTimeout_ms;
    uint32_t PCanTimeout_ms;
	
	uint32_t PowerOffTimer_ms;
    uint32_t KeyOffTimer_ms;
	
	int16_t KeyCnt;
} Timers_t;

typedef struct
{
	uint16_t Time1_ms;
	uint16_t Time10_ms;
	uint16_t Time100_ms;
	uint16_t Time1_s;
	uint16_t MainLoopTime_ms;
    
} TimeValues_t;

typedef struct
{
    uint8_t
    CheckFaults                 :   1,
    PowerOff_SaveParams         :   1,
    ReceiveTestRequest          :   1,
	PowerOn						:	1,
    BatteryMsgReceived          :   1,
	mEcuMsgReceived				:   1;
} StateBits_t;

typedef struct
{
    uint8_t AdcChannel;
    uint16_t Out_CSens_Voltage;
    uint16_t Out_CSens_VoltageOffset;
    uint16_t Out_CSens_Current;
    
} A_OUT_CSENS_t;

typedef struct
{
	uint16_t MotorRpm;
} DisplayData_t;

typedef struct
{
	uint32_t SystemTime;
	uint32_t TotalActualEnergy_As;
	uint32_t ActualEnergy_As;

	uint32_t dummy;
	uint32_t dumm1;
	uint32_t dummy2;
	uint32_t dummy3;
	uint32_t dummy4;
	uint16_t NormalPowerOff;

	uint16_t Crc;
} sDataBuf_t;

typedef struct
{
	sDataBuf_t Buf_1st;
	//sDataBuf_t Buf_2nd;

	//const dtcItem_t* dtcListData;
	const EcuConfig_t *cfgData;

	//struct Data_st Data;
	uint8_t Result;
	uint8_t DataChanged;
} StorageData_t;

// Словарь объектов
typedef struct
{
	uint16_t EcuInfo[4];
    StateMachine_t StateMachine;
    Timers_t LogicTimers;
    TimeValues_t DelayValues;
	uint32_t SystemTime;

	const EcuConfig_t *cfg;
	StorageData_t SData;
    
    StateBits_t SB;
    dECU_Fauls_t Faults;
	
	uint8_t ecuIndex;
	uint8_t ecuPowerSupply_0p1;
    
    uint16_t A_CH_Results[A_IN_NUM];
    uint8_t A_CH_Voltage_0p1[A_IN_NUM];
    
    A_OUT_CSENS_t Out_CSens[3];
    
    uint32_t IO;
	uint8_t A_Out[4];
	uint8_t A_IN[4];
	
	uint16_t CurrentSensorVoltage[4];

	// PM
	PowerStates_e LocalPMState;
	PowerStates_e PowerManagerCmd;
	
	uint16_t Isolation;
	BatM_Ext1_t BmuData1;
	BatM_Ext3_t BmuData2;
	BatM_Ext4_t BmuData4;
	BatBatteryStatus1Msg_t PackData1;
	BatBatteryStatus1Msg_t PackData2;
	MainEcuStatus1_Msg_t MainEcuData1;
	MainEcuStatus2_Msg_t MainEcuData2;
	GpsStatus1_Msg_t GpsData1;

	uint8_t FaultsNumber;		
	uint16_t FaultList[MAX_FAULTS_NUM];
	
	uint8_t OldFaultsNumber;
	uint16_t OldFaultList[MAX_FAULTS_NUM];
    
} ObjectDictionary_t;


extern void (*Algorithm)(uint8_t *);
extern ObjectDictionary_t OD;

uint8_t GetDataByIndex(uint16_t Index, uint8_t subindex, uint8_t *Buf[]);


#endif
