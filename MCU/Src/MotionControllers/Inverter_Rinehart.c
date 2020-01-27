/*
 * Inverter_Rinehart.c
 *
 *  Created on: 7 дек. 2019 г.
 *      Author: a.lazko
 */

#include <string.h>

#include "Inverter_Rinehart.h"


static const int tx_msg_cnt = 2;

const uint16_t msgPeriod[tx_msg_cnt] = {
								50,                         // CmdMsg
                                250,                        // CurrentLimit
								 };

#define ID_Offset				0x200

// Tx Msgs
#define InvCmdMsg_ID            0x0c0 + ID_Offset
#define BMSCurrentLimitMsg_ID   0x0ca + ID_Offset

// Rx Msgs
#define TempSet1Msg_ID          0x0a0 + ID_Offset
#define TempSet2Msg_ID          0x0a1 + ID_Offset
#define TempSet3Msg_ID          0x0a2 + ID_Offset
#define MotorPositionMsg_ID     0x0a5 + ID_Offset
#define InvCurrentMsg_ID        0x0a6 + ID_Offset
#define InvVoltageMsg_ID        0x0a7 + ID_Offset
#define InvInternalStatesMsg_ID 0x0aa + ID_Offset
#define InvFaultCodesMsg_ID     0x0ab + ID_Offset
#define TorqueAndTimerMsg_ID    0x0ac + ID_Offset

int8_t McuRinehartInit(McuRinehart_t *mcu)
{

	return 0;
}
int8_t McuRinehartThread(McuRinehart_t *mcu)
{
	if(mcu == 0)
		return -1;

	// Clear error if inverter is reinitialized
	if(mcu->EnableCommand == stateCmd_Enable)
	{
		switch(mcu->rmsMsgTx_1.VSMState)
		{
		case VSMst_Start:
			mcu->rmsMsgRx_1.InvEnable = 0;
			mcu->Status = mcu_Disabled;
			mcu->LastError = 0;
			break;

		case VSMst_PreChargeInit:
		case VSMst_PreChargeActive:
		case VSMst_PreChargeComplete:
			mcu->Status = mcu_Disabled;
			break;

		case VSMst_Wait:
		{
			mcu->rmsMsgRx_1.InvEnable = (mcu->rmsMsgTx_1.InvEnableLockout == 1)? 0 : 1;
			mcu->Status = mcu_Disabled;
		}
			break;

		case VSMst_Ready:
			mcu->rmsMsgRx_1.InvEnable = 1;
			mcu->Status = mcu_Enabled;
			break;

		case VSMst_Running:
			mcu->rmsMsgRx_1.InvEnable = 1;
			mcu->Status = mcu_Enabled;
			break;

		case VSMst_Fault:
			mcu->Status = mcu_Fault;
			break;

		case VSMst_Shutdown:
		case VSMst_Reset:
			mcu->Status = mcu_Disabled;
			break;

		default:
			mcu->rmsMsgRx_1.InvEnable = 0;
			mcu->Status = mcu_Disabled;
			break;
		}
	}
	else if(mcu->EnableCommand == stateCmd_Disable)
		mcu->rmsMsgRx_1.InvEnable = 0;

	mcu->LastError = (mcu->LastError == 0)? mcu->rmsMsgTx_2.PostCode : mcu->LastError;

	mcu->rmsMsgRx_1.TorqueCmd_0p1 = (mcu->rmsMsgRx_1.InvEnable == 1)? mcu->RequestTorque : 0;
	mcu->rmsMsgRx_1.SpeedCmd = (mcu->rmsMsgRx_1.InvEnable == 1)? mcu->RequestSpeed : 0;
	mcu->rmsMsgRx_2.MaxChargeCurrent = mcu->GenerationCurrentLimit;
	mcu->rmsMsgRx_2.MaxDischargeCurrent = mcu->ConsumtionCurrentLimit;

	return 0;
}

int8_t McuRinehartSetCmd(McuRinehart_t *mcu, int16_t TorqueCmd, int16_t SpeedCmd, int16_t GenCurrentMax, int16_t ConsCurrentMax)
{
	if(mcu == 0)
		return -1;

	mcu->RequestTorque = TorqueCmd;
	mcu->RequestSpeed = SpeedCmd;
	mcu->ConsumtionCurrentLimit = ConsCurrentMax;
	mcu->GenerationCurrentLimit = GenCurrentMax;

	return 0;
}

int8_t McuRinehartSetState(McuRinehart_t *mcu, UnitState_e State)
{
	if(mcu == 0)
		return -1;

	mcu->EnableCommand = State;

	return 0;
}

mcuStatus_e McuRinehartGetState(McuRinehart_t *mcu)
{
	if(mcu == 0)
		return (mcuStatus_e)0;

	return mcu->Status;
}

int16_t McuRinehartGetLastError(McuRinehart_t *mcu)
{
	if(mcu == 0)
		return 0;

	return mcu->LastError;
}


int McuRinehartTxMsgGenerate(McuRinehart_t *mcu, uint8_t *MsgData, uint8_t *MsgDataLen, int32_t MsCounter)
{
	int32_t time_value = 0;
	int32_t msg_id = -1;

	if(mcu == 0)
		return -1;

	if(++mcu->PreparedMsgNumber >= tx_msg_cnt)
		mcu->PreparedMsgNumber = 0;

	time_value = MsCounter - mcu->PreparedMsgTimestamp[mcu->PreparedMsgNumber];

	if(time_value >= msgPeriod[mcu->PreparedMsgNumber])
	{
		mcu->PreparedMsgTimestamp[mcu->PreparedMsgNumber] = MsCounter;

		switch(mcu->PreparedMsgNumber)
		{
			case 0:
			{
				msg_id = InvCmdMsg_ID;

				mcu->rmsMsgRx_1.TorqueCmd_0p1 = mcu->rmsMsgRx_1.TorqueCmd_0p1 * 10;
				mcu->rmsMsgRx_1.DirCmd = 0;
				mcu->rmsMsgRx_1.SpeedModeEnable = 0;
				mcu->rmsMsgRx_1.InvDischarge = 0;
				mcu->rmsMsgRx_1.TorqueLimitCmd_0p1 = 0;

				memcpy(MsgData, &mcu->rmsMsgRx_1, sizeof(InvCmdMsg_t));
			}
				break;

			case 1:
			{
				msg_id = BMSCurrentLimitMsg_ID;

				memcpy(MsgData, &mcu->rmsMsgRx_2, sizeof(BMSCurrentLimitMsg_t));
			}
			break;
		}
	}

	return msg_id;
}



int8_t McuRinehartMsgHandle(McuRinehart_t *mcu, int MsgID, uint8_t *MsgData, uint8_t MsgDataLen)
{
	if(mcu == 0 || MsgData == 0)
		return 1;

	mcu->OnlineSign = 0;

	if(MsgID == TempSet1Msg_ID)
	{
		memcpy(&mcu->rmsMsgTx_7, MsgData, sizeof(TempSet1Msg_t));
	}
	else if(MsgID == TempSet2Msg_ID)
	{
		memcpy(&mcu->rmsMsgTx_8, MsgData, sizeof(TempSet2Msg_t));
	}
	else if(MsgID == TempSet3Msg_ID)
	{
		memcpy(&mcu->rmsMsgTx_9, MsgData, sizeof(TempSet3Msg_t));
	}
	else if(MsgID == MotorPositionMsg_ID)
	{
		memcpy(&mcu->rmsMsgTx_3, MsgData, sizeof(MotorPositionMsg_t));
	}
	else if(MsgID == InvCurrentMsg_ID)
	{
		memcpy(&mcu->rmsMsgTx_5, MsgData, sizeof(InvCurrentMsg_t));
	}
	else if(MsgID == InvVoltageMsg_ID)
	{
		memcpy(&mcu->rmsMsgTx_6, MsgData, sizeof(InvVoltageMsg_t));
	}
	else if(MsgID == InvInternalStatesMsg_ID)
	{
		memcpy(&mcu->rmsMsgTx_1, MsgData, sizeof(InvInternalStatesMsg_t));
	}
	else if(MsgID == InvFaultCodesMsg_ID)
	{
		memcpy(&mcu->rmsMsgTx_2, MsgData, sizeof(InvFaultCodesMsg_t));
	}
	else if(MsgID == TorqueAndTimerMsg_ID)
	{
		memcpy(&mcu->rmsMsgTx_4, MsgData, sizeof(TorqueAndTimerMsg_t));
	}
	else
		return 1;

	mcu->OnlineSign = 1;

	return 0;
}


