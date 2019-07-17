#ifndef _INVERTER_PROTOCOL_H_
#define _INVERTER_PROTOCOL_H_

#include "CanFunc.h"
#include "Protocol.h"

uint8_t InvProtocolRx(CanMsg *Msg);
void InvProtocolMesGenerate(void);

// Tx Msgs
#define InvCmdMsg_ID            0x0c0
#define BMSCurrentLimitMsg_ID   0x0ca

// Rx Msgs
#define TempSet1Msg_ID          0x0a0
#define TempSet2Msg_ID          0x0a1
#define TempSet3Msg_ID          0x0a2
#define MotorPositionMsg_ID     0x0a5
#define InvCurrentMsg_ID        0x0a6
#define InvVoltageMsg_ID        0x0a7
#define InvInternalStatesMsg_ID 0x0aa
#define InvFaultCodesMsg_ID     0x0ab
#define TorqueAndTimerMsg_ID    0x0ac


/* 
    Tx Messages
*/

typedef struct
{
    int16_t TorqueCmd_0p1;
    int16_t SpeedCmd;
    
    uint8_t
    DirCmd              :   1,
    dummy1              :   7;
    
    uint8_t
    InvEnable           :   1,
    InvDischarge        :   1,
    SpeedModeEnable     :   1,
    dummy2              :   5;
    
    int16_t TorqueLimitCmd_0p1;
}InvCmdMsg_t;

typedef struct
{
    uint16_t MaxDischargeCurrent;
    uint16_t MaxChargeCurrent;
} BMSCurrentLimitMsg_t;

/*
    Rx Messages
*/

typedef struct
{
    uint16_t VSMState;
    uint8_t InvState;
    
    uint8_t
    Relay1_Status       :   1,
    Relay2_Status       :   1,
    Relay3_Status       :   1,
    Relay4_Status       :   1,
    Relay5_Status       :   1,
    Relay6_Status       :   1,
    dummy1              :   2;
    
    uint8_t
    InvRunMode          :   1,
    dummy2              :   4,
    InvDischargeState   :   3;
    
    uint8_t
    InvCmdMode          :   1,
    dummy3              :   7;
    
    uint8_t 
    InvEnableState      :   1,
    dummy4              :   6,
    InvEnableLockout    :   1;
    
    uint8_t
    DirCmd              :   1,
    BMSActive           :   1,
    BMSTorqueLimit      :   1,
    dummy               :   5;
} InvInternalStatesMsg_t;


typedef struct
{
    uint32_t PostCode;
    uint32_t RunCode;
} InvFaultCodesMsg_t;

typedef struct
{
    uint16_t MotorAngleElectrical_0p1;
    int16_t MotorSpeed;
    int16_t ElectricalOutputFreq_0p1;
    int16_t DeltaResolverFiltered_0p1;
} MotorPositionMsg_t;

typedef struct
{
    int16_t CmdTorque_0p1;
    int16_t TorqueFeedback_0p1;
    uint32_t PowerOnTimer_0p003;
} TorqueAndTimerMsg_t;

typedef struct
{
    int16_t PhaseACurrent_0p1;
    int16_t PhaseBCurrent_0p1;
    int16_t PhaseCCurrent_0p1;
    int16_t DCBusCurrent_0p1;
} InvCurrentMsg_t;

typedef struct
{
    int16_t DCBusVoltage_0p1;
    int16_t OutputVoltage_0p1;
    int16_t VdVoltage_0p1;
    int16_t VqVoltage_0p1;
} InvVoltageMsg_t;

typedef struct
{
    int16_t ModuleATemp_0p1;
    int16_t ModuleBTemp_0p1;
    int16_t ModuleCTemp_0p1;
    int16_t GateDriverBoardTemp_0p1;
} TempSet1Msg_t;

typedef struct
{
    int16_t ControlBoardTemp_0p1;
    int16_t RTD1Temp_0p1;
    int16_t RTD2Temp_0p1;
    int16_t RTD3Temp_0p1;
} TempSet2Msg_t;

typedef struct
{
    int16_t RTD4Temp_0p1;
    int16_t RTD5Temp_0p1;
    int16_t MotorTem_0p1;
    int16_t TorqueShudder_0p1;
} TempSet3Msg_t;








#endif
