/*
 * Inverter_Rinehart.h
 *
 *  Created on: 7 дек. 2019 г.
 *      Author: a.lazko
 */

#ifndef SRC_MOTIONCONTROLLERS_INVERTER_RINEHART_H_
#define SRC_MOTIONCONTROLLERS_INVERTER_RINEHART_H_


typedef enum
{
	mcu_Disabled = 0,
	mcu_Enabled,
	mcu_Warning,
	mcu_Fault,
} mcuStatus_e;

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
    uint32_t dummy1;
} BMSCurrentLimitMsg_t;

/*
    Rx Messages
*/

typedef enum
{
	VSMst_Start = 0,
	VSMst_PreChargeInit,
	VSMst_PreChargeActive,
	VSMst_PreChargeComplete,
	VSMst_Wait,
	VSMst_Ready,
	VSMst_Running,
	VSMst_Fault,
	VSMst_Shutdown = 14,
	VSMst_Reset = 15,
} VsmStates_e;

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

typedef enum
{
	stateCmd_None = 0,
	stateCmd_Enable = 1,
	stateCmd_Disable = 2,
} UnitState_e;

typedef struct
{
	int16_t EnableCommand;
	int16_t RequestTorque;
	int16_t RequestSpeed;

	int16_t ConsumtionCurrentLimit;
	int16_t GenerationCurrentLimit;

	uint8_t PreparedMsgNumber;
	int32_t PreparedMsgTimestamp[2];

	mcuStatus_e Status;
	uint8_t OnlineSign;
	uint16_t LastError;

	// Communication
	InvCmdMsg_t rmsMsgRx_1;
	BMSCurrentLimitMsg_t rmsMsgRx_2;

	InvInternalStatesMsg_t rmsMsgTx_1;
	InvFaultCodesMsg_t rmsMsgTx_2;
	MotorPositionMsg_t rmsMsgTx_3;
	TorqueAndTimerMsg_t rmsMsgTx_4;
	InvCurrentMsg_t rmsMsgTx_5;
	InvVoltageMsg_t rmsMsgTx_6;
	TempSet1Msg_t rmsMsgTx_7;
	TempSet2Msg_t rmsMsgTx_8;
	TempSet3Msg_t rmsMsgTx_9;

} McuRinehart_t;



int8_t McuRinehartInit(McuRinehart_t *mcu);
int8_t McuRinehartThread(McuRinehart_t *mcu);

int8_t McuRinehartSetCmd(McuRinehart_t *mcu, int16_t TorqueCmd, int16_t SpeedCmd, int16_t GenCurrentMax, int16_t ConsCurrentMax);
int8_t McuRinehartSetState(McuRinehart_t *mcu, UnitState_e State);

mcuStatus_e McuRinehartGetState(McuRinehart_t *mcu);
int16_t McuRinehartGetLastError(McuRinehart_t *mcu);

/***
 * Function fills MsgData array for transmit message
 *
 * Return value:
 * positive or zero - message ID,
 * negative value - message don't ready to transmit;
 *
 * Parameters:
 * MsgData - pointer to data buffer
 * MsgDataLen - not used, simplify - all messages have DLC 8 bytes
 */
int McuRinehartTxMsgGenerate(McuRinehart_t *mcu, uint8_t *MsgData, uint8_t *MsgDataLen);
int8_t McuRinehartMsgHandle(McuRinehart_t *mcu, int MsgLen, uint8_t *MsgData, uint8_t MsgDataLen);

#endif /* SRC_MOTIONCONTROLLERS_INVERTER_RINEHART_H_ */
