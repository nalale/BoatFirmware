#ifndef _MAIN_H_
#define _MAIN_H_


#define DIR_IN      0
#define DIR_OUT     1

#include "LPC17xx.h"
#include "lpc_types.h"

#include "FaultTools.h"
#include "ObjectsIndex.h"
#include "protocol.h"

#include "../BoardDefinitions/MarineEcu_Board.h"

#include "../MolniaLib/Config.h"
#include "../MolniaLib/MF_CAN_1v1.h"
#include "../MolniaLib/PowerManager.h"

#pragma anon_unions

// Максимальное количество в списке ошибок
#define MAX_FAULTS_NUM				10

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
    uint32_t Max11612Timeout_ms;
	
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
	InverterIsOperate			:	1,
	BatteryIsOperate			:	1,
    ExtCanMsgReceived           :   1,
	PCanMsgReceived				:   1;
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
	CanMsg msg;
	uint32_t SendTime;
	uint8_t RepCount;
} canSendItem_t;


// Словарь объектов
typedef struct
{
	uint16_t EcuInfo[4];
    StateMachine_t StateMachine;
    Timers_t LogicTimers;
    TimeValues_t DelayValues;
	uint32_t SystemTime;
    
    StateBits_t SB;
    gECU_Fauls_t Faults;
	
	uint8_t ecuIndex;
	uint8_t ecuPowerSupply_0p1;
    
    uint16_t A_CH_Results[A_IN_NUM];
    uint8_t A_CH_Voltage_0p1[A_IN_NUM];
    
    A_OUT_CSENS_t Out_CSens[3];
    
    uint32_t IO;
	uint8_t A_Out[4];
	
	// PM
	PowerStates_e LocalPMState;
	PowerStates_e PowerManagerCmd;
	
	uint8_t RepItemCount;	
	canSendItem_t msgTab[REPEATER_TABLE_SIZE];
	
	uint8_t FaultsNumber;		
	uint16_t FaultList[MAX_FAULTS_NUM];
	
	uint8_t OldFaultsNumber;
	uint16_t OldFaultList[MAX_FAULTS_NUM];
    
} ObjectDictionary_t;


extern void (*Algorithm)(uint8_t *);
extern ObjectDictionary_t OD;

uint8_t check_failed(uint8_t *file, uint8_t line);
uint8_t GetDataByIndex(uint16_t Index, uint8_t subindex, uint8_t *Buf[]);


#endif
