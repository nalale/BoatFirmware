/*
 * HelmDriver.c
 *
 *  Created on: 1 мая 2020 г.
 *      Author: a.lazko
 */

#include "HelmDriver.h"
#include "../MolniaLib/MF_Tools.h"

typedef enum {
	prm_HELM_REGULATED_BRAKE_POS = 0,
	prm_HELM_START_UP,

	prm_HELM_NUMBER,
} HelmParameters_e;

// Периоды отправки сообщений
static const uint16_t extPeriod[prm_HELM_NUMBER] = {
                                250,
								250,
								 };

typedef struct
{
	int16_t TargetAngle;
    int16_t ActualAngle;
    int16_t SteeringSpeed_rads;
    uint8_t NumSteeringRev;
    uint8_t MaxSteeringRev;
    uint16_t HeartCount_prev;
    uint16_t SpeedValue_prev;
    uint8_t RegulatedCounterForce;
    uint8_t DefaultCounterForce;
    const int16_t *BrakeSpeedTable;
    uint8_t TableSize;
    uint8_t CausedFault;

    HelmParameters_e PreparedMsgNumber;
    uint32_t PreparedMsgTimestamp[prm_HELM_NUMBER];

    uint8_t
        EndStopLeft_Ack     :   1,
        EndStopRight_Ack    :   1,
        SetRegBrake_Ack		:   1,
		HelmIsReady			:	1,
		OnlineSign			:	1,
		SetZeroBrake_Ack	:	1,
		FaultReceive_Ack	:	1,
		dummy				:	1;

} HelmHandle_t;

static HelmHandle_t helmHandle;

uint8_t helmInit(const int16_t* ForceSpeedTable, int8_t TableSize)
{
	if(ForceSpeedTable == 0)
		return 1;

	helmHandle.BrakeSpeedTable = ForceSpeedTable;
	helmHandle.TableSize = TableSize;
	helmHandle.DefaultCounterForce = ForceSpeedTable[TableSize >> 1];
	helmHandle.MaxSteeringRev = 2;

	return 0;
}

uint8_t helmThread(int16_t ActualSpeed)
{
	if(helmHandle.BrakeSpeedTable == 0)
		return 1;

	helmHandle.HelmIsReady = helmHandle.EndStopRight_Ack && helmHandle.EndStopLeft_Ack;

	if(ActualSpeed - helmHandle.SpeedValue_prev > 500 || ActualSpeed - helmHandle.SpeedValue_prev < -500)
		helmHandle.RegulatedCounterForce = (uint8_t)interpol(helmHandle.BrakeSpeedTable, helmHandle.TableSize >> 1, ActualSpeed);

	if(helmHandle.RegulatedCounterForce < helmHandle.DefaultCounterForce)
		helmHandle.RegulatedCounterForce = helmHandle.DefaultCounterForce;

	helmHandle.SpeedValue_prev = ActualSpeed;
	return 0;
}

uint16_t HelmGetTargetAngle()
{
	uint16_t result;

	if(helmHandle.HelmIsReady)
	{
		result = helmHandle.TargetAngle + (helmHandle.NumSteeringRev * 4095);
	}
	else
		result = 2048 + ((helmHandle.MaxSteeringRev >> 1) * 4095);

	return result;
}

int8_t HelmGetAnglePercent()
{
	int8_t percent;
	int32_t result = HelmGetTargetAngle();
	int32_t max_pos = 4095 + (helmHandle.MaxSteeringRev * 4095);
	int32_t center = (2048 + ((helmHandle.MaxSteeringRev >> 1) * 4095));

	percent = (result - center) * 100 / (max_pos - center);

//	if(result >= center)
//	{
//		percent = (result - center) * 100 / (max_pos - center);
//	}
//	else
//	{
//
//	}

	return percent;
}

uint8_t HelmGetStatus()
{
	uint8_t value;

	// If helm is ready return 1, but if set reg brake ack wasn't received return 2
	if(helmHandle.HelmIsReady)
		value = (!helmHandle.SetRegBrake_Ack)? 2 : 1;
	else value = 0;

	return value;
}

uint8_t HelmGetCausedFault()
{
	return helmHandle.CausedFault;
}

uint8_t helmGetOnlineSign()
{
	uint8_t res = helmHandle.OnlineSign;
	helmHandle.OnlineSign = 0;
	return res;
}


uint8_t HelmMessageHandler(HelmParameterTypes_e MsgID, uint8_t *MsgData, uint8_t MsgDataLen)
{
	switch(MsgID)
	{
		case MSG_ABS_ANGLE_VALUE:
		{
			MSG_ABS_ANGLE *d = (MSG_ABS_ANGLE*)MsgData;
			

			if(d->C_NUM_STEERING_REV == 0 || d->C_NUM_STEERING_REV == 1 || d->C_NUM_STEERING_REV == 2)
			{
				helmHandle.TargetAngle = d->C_ANGLE;
				helmHandle.NumSteeringRev = d->C_NUM_STEERING_REV;
				helmHandle.SteeringSpeed_rads = (d->C_STEERING_SPEED_HIG << 8) | d->C_STEERING_SPEED_LOW;

				if(d->C_HEART_COUNTER != helmHandle.HeartCount_prev)
					helmHandle.OnlineSign = 1;

				helmHandle.HeartCount_prev = d->C_HEART_COUNTER;
			}
			return 0;
		}
		case CAN_MSG_ERROR_CODE:
		{
			MSG_ERROR_CODE *d = (MSG_ERROR_CODE*)MsgData;

			helmHandle.CausedFault = (!helmHandle.CausedFault && d->C_ERROR_STATE)? d->C_ERROR_CODE : helmHandle.CausedFault;

			return 0;
		}
		case MSG_ESIU_RESP:
		{
			switch(MsgData[0])
			{
				case MSG_SET_REGULATED_BRAKE_POS:
					helmHandle.SetRegBrake_Ack = (MsgData[1] != 0xFF)? 1 : 0;
					break;

				case MSG_SET_STEERING_ENDSTOP_LEFT:
					helmHandle.EndStopLeft_Ack = (MsgData[1] != 0xFF)? 1 : 0;
					break;

				case MSG_SET_STEERING_ENDSTOP_RIGHT:
					helmHandle.EndStopRight_Ack = (MsgData[1] != 0xFF)? 1 : 0;
					break;

				case MSG_SET_ZERO_BRAKE_POS:
					helmHandle.SetZeroBrake_Ack = (MsgData[1] != 0xFF)? 1 : 0;
				break;
			}
			return 0;
		}

		default:
			return 1;
	}
}

HelmParameterTypes_e HelmMessageGenerate(uint8_t *MsgData, uint8_t *MsgDataLen, int32_t MsCounter)
{
	int32_t time_value = 0;
	HelmParameterTypes_e msg_id = MSG_HELM_ERROR_ID;

	if(MsgData == 0)
		return (HelmParameterTypes_e)msg_id;

	if(helmHandle.PreparedMsgNumber == prm_HELM_START_UP &&
			(helmHandle.SetZeroBrake_Ack && helmHandle.EndStopLeft_Ack && helmHandle.EndStopRight_Ack))
		helmHandle.PreparedMsgNumber++;

	if(++helmHandle.PreparedMsgNumber >= prm_HELM_NUMBER)
		helmHandle.PreparedMsgNumber = (HelmParameters_e)0;


	time_value = MsCounter - helmHandle.PreparedMsgTimestamp[helmHandle.PreparedMsgNumber];

	if(time_value >= extPeriod[helmHandle.PreparedMsgNumber])
	{
		helmHandle.PreparedMsgTimestamp[helmHandle.PreparedMsgNumber] = MsCounter;

		switch(helmHandle.PreparedMsgNumber)
		{
			default:
			case prm_HELM_REGULATED_BRAKE_POS:
			{
				msg_id = MSG_SET_REGULATED_BRAKE_POS;
				*MsgDataLen = 1;
				//msg->Ext = 1;

				MSG_SET_REGULATED_BRAKE_FRICTION* d = (MSG_SET_REGULATED_BRAKE_FRICTION*) MsgData;
				d->C_PERC_MAX_BRAKE_FRICTION = (helmHandle.RegulatedCounterForce > 100)? 100 : helmHandle.RegulatedCounterForce;
			}
				break;

			case prm_HELM_START_UP:
			{
				if(!helmHandle.EndStopRight_Ack)
				{
					MSG_SET_STEERING_ENDSTOP *d = (MSG_SET_STEERING_ENDSTOP *)MsgData;
					msg_id = MSG_SET_STEERING_ENDSTOP_RIGHT;
					*MsgDataLen = 3;
					//msg->Ext = 1;

					//d->C_NUM_REV = helmHandle.NumSteeringRev;
					d->C_NUM_REV = helmHandle.MaxSteeringRev;
					d->C_ABS_ANGLE_HIGH = 0x0f;
					d->C_ABS_ANGLE_LOW = 0xff;
				}
				else if (!helmHandle.EndStopLeft_Ack)
				{
					MSG_SET_STEERING_ENDSTOP *d = (MSG_SET_STEERING_ENDSTOP *)MsgData;
					msg_id = MSG_SET_STEERING_ENDSTOP_LEFT;
					*MsgDataLen = 3;

					//d->C_NUM_REV = helmHandle.NumSteeringRev;
					d->C_NUM_REV = 0;
					d->C_ABS_ANGLE_HIGH = 0x00;
					d->C_ABS_ANGLE_LOW = 0x00;
				}
				else if(!helmHandle.SetRegBrake_Ack)
				{
					MSG_SET_ZERO_REGULATED_BRAKE_FRICTION *d = (MSG_SET_ZERO_REGULATED_BRAKE_FRICTION *)MsgData;
					msg_id = MSG_SET_ZERO_BRAKE_POS;
					*MsgDataLen = 1;

					d->C_PERC_MAX_BRAKE_CURRENT = helmHandle.DefaultCounterForce;
				}
			}

			break;
		}
	}

	return msg_id;
}
