/*
 * ObcDriver.c
 *
 *  Created on: 4 мая 2020 г.
 *      Author: a.lazko
 */

// Driver for TC-Charger HK-J-H440-10 unit

#include <string.h>

#include "ObcDriver.h"
#include "TimerFunc.h"

static TcChargerHKJ_t tcCharger;

static uint32_t extPeriod[prm_OBC_NUMBER] = {
	250,
};

uint8_t obcThread()
{
	if(!tcCharger.IsInit)
		return 1;

	if(tcCharger.TurnOnReq)
	{
		// Clear fault after reset
		tcCharger.CausedFault = (tcCharger.CausedFault > 0)?
				((tcCharger.Status == st_OBC_DISABLE)? 0 : tcCharger.CausedFault) :
				tcCharger.FaultsList;

		// Stop charging conditions
		if(//(tcCharger.OutputVoltage >= tcCharger.TargetVoltage - 1) ||
				(tcCharger.BatteryCCL < 10 && tcCharger.Status == st_OBC_ENABLE )
			|| tcCharger.Status == st_OBC_STOP)
		{
			tcCharger.Status = st_OBC_STOP;
			tcCharger.RequestCurrent = 0;
		}
		else
		{
			// Charging conditions
			uint8_t tmp_val;

			if(tcCharger.CausedFault > 0 && tcCharger.OutputCurrent == 0)
				tcCharger.Status = st_OBC_FAULT;
			else
				tcCharger.Status = st_OBC_ENABLE;

			tmp_val = (tcCharger.BatteryCCL < tcCharger.PlugMaxCurrent)?
							tcCharger.BatteryCCL / tcCharger.UnitNumber :
							tcCharger.PlugMaxCurrent / tcCharger.UnitNumber;

			tcCharger.RequestCurrent =	(tmp_val >= 12)? 12 : tmp_val;
		}

	}
	else
	{
		tcCharger.Status = st_OBC_DISABLE;
		tcCharger.RequestCurrent = 0;

		// Clear tccharger communication memory
		tcCharger.FaultsList = 0;
		tcCharger.OutputCurrent = 0;
		tcCharger.OutputVoltage = 0;
		memset(&tcCharger.MsgRxBuf, 0, 8);
	}

	return 0;
}

uint8_t obcInit(int8_t ChargerCnt, uint16_t PlugMaxCurrent, uint16_t BatteryMaxVoltage)
{
	tcCharger.IsInit = (ChargerCnt > 0)? 1 : 0;

	tcCharger.PlugMaxCurrent = PlugMaxCurrent;
	tcCharger.UnitNumber = ChargerCnt;
	tcCharger.TargetVoltage = BatteryMaxVoltage;

	return 0;
}

uint8_t obcSetCurrentLimit(uint16_t BatteryCurrentLimit)
{
	tcCharger.BatteryCCL = BatteryCurrentLimit;
	return 0;
}

uint8_t obcSetState(uint8_t state)
{
	tcCharger.TurnOnReq = (state == 1)? 1 : 0;

	return 0;
}

ObcStatus_e obcGetStatus()
{
	return tcCharger.Status;
}

uint8_t obcGetCausedFault()
{
	return tcCharger.CausedFault;
}

uint8_t obcGetOnlineSign()
{
	uint8_t tmp = tcCharger.OnlineSign;
	tcCharger.OnlineSign = 0;

	return tmp;
}

uint8_t obcGetOnlineUnitsNumber()
{
	return tcCharger.OnlineUnits;
}

uint16_t obcGetOutputCurrent()
{
	return tcCharger.OutputCurrent / 10;
}

uint16_t obcGetDemandCurrent()
{
	return tcCharger.RequestCurrent;
}

uint8_t obcMessageHandler(ObcParameterTypes_e MsgID, uint8_t *MsgData, uint8_t MsgDataLen)
{
	static uint8_t online_units = 0;
	static uint32_t window_ts = 0;

	switch(MsgID)
	{
		case MSG_OBC_STATUS_VALUE:
		{
			uint8_t tmp = 0;
//			if(tcCharger.Status == st_OBC_DISABLE)
//				return 1;
//			else
			{
				MSG_OBC_STATUS_t *input_data = (MSG_OBC_STATUS_t*)MsgData;

				tmp = MsgData[0];
				MsgData[0] = MsgData[1];
				MsgData[1] = tmp;

				tmp = MsgData[2];
				MsgData[2] = MsgData[3];
				MsgData[3] = tmp;

				tcCharger.OnlineSign = 1;
				//memcpy(&tcCharger.MsgRxBuf, MsgData, 8);

				// Remember occurence fault bits
				tcCharger.FaultsList |= input_data->status;
				tcCharger.MsgRxBuf.OutputVoltage += input_data->OutputVoltage;
				tcCharger.MsgRxBuf.OutputCurrent += input_data->OutputCurrent;
				online_units++;

				// msg period of 1 unit - 1000ms
				// if m msg was received for 1150ms (1st - 0ms, 2nd - 1000ms + 150ms delta) - online number - m / 2;
				if(GetTimeFrom(window_ts) >= 1450)
				{
					window_ts = GetTimeStamp();
					tcCharger.OnlineUnits = online_units >> 1;
					tcCharger.OutputCurrent = tcCharger.MsgRxBuf.OutputCurrent / 2;
					tcCharger.OutputVoltage = tcCharger.MsgRxBuf.OutputVoltage / online_units;
					
					tcCharger.MsgRxBuf.OutputCurrent = 0;
					tcCharger.MsgRxBuf.OutputVoltage = 0;
					online_units = 0;
				}
			}
		}
		break;

		default:
			return 1;
	}

	return 0;
}

ObcParameterTypes_e obcMessageGenerate(uint8_t *MsgData, uint8_t *MsgDataLen, int32_t MsCounter)
{
	int32_t time_value = 0;
	ObcParameterTypes_e msg_id = MSG_OBC_ERROR_ID;

	if(MsgData == 0 || !tcCharger.TurnOnReq)
		return msg_id;

	if(++tcCharger.PreparedMsgNumber >= prm_OBC_NUMBER)
		tcCharger.PreparedMsgNumber = (ObcParameters_e)0;

	time_value = MsCounter - tcCharger.PreparedMsgTimestamp[tcCharger.PreparedMsgNumber];

	if(time_value >= extPeriod[tcCharger.PreparedMsgNumber])
	{
		tcCharger.PreparedMsgTimestamp[tcCharger.PreparedMsgNumber] = MsCounter;

		memset(MsgData, 0, 8);

		switch(tcCharger.PreparedMsgNumber)
		{
			default:
			case prm_OBC_CURRENT_REQ:
			{
				uint8_t tmp;
				msg_id = MSG_TARGET_VALUE;
				*MsgDataLen = 8;

				MSG_SET_CHARGING_DATA_t* d = (MSG_SET_CHARGING_DATA_t*) MsgData;
				d->TargetVoltage_0p1A = tcCharger.TargetVoltage * 10;
				d->TargetCurrent_0p1A = tcCharger.RequestCurrent * 10;

				tmp = MsgData[0];
				MsgData[0] = MsgData[1];
				MsgData[1] = tmp;

				tmp = MsgData[2];
				MsgData[2] = MsgData[3];
				MsgData[3] = tmp;
			}
				break;
		}
	}

	return msg_id;
}
