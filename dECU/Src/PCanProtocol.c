#include <string.h>

#include "Main.h"
#include "PCanProtocol.h"
#include "TimerFunc.h"

#include "dVCU_ECU.h"

#include "../MolniaLib/MF_CAN_1v1.h"
#include "../MolniaLib/DateTime.h"

enum { PM_Msg = 0,} Msgs_e;

#define EXTERNAL_SEND_MSG_AMOUNT	1

static uint32_t extSendTime[EXTERNAL_SEND_MSG_AMOUNT];
// Периоды отправки сообщений
static uint16_t extPeriod[EXTERNAL_SEND_MSG_AMOUNT] = {
                                250,		// cmGen1
								 };

uint8_t PCanRx(CanMsg *msg)
{	
	if (msg->ID == BASE_CAN_ID)		// Date Time
	{
		if(!dateTime_IsInit())
		{
			dateTime_InitCurrent(((cmTimeServer*)msg->data)->DateTime);
		}

	}
	else if(msg->ID == PM_CAN_ID)
	{
		OD.PowerManagerCmd = (PowerStates_e)msg->data[0];
	}
	else if(msg->ID == Main_ECU_CAN_ID)
	{
		OD.SB.mEcuMsgReceived = 1;
		memcpy(&OD.MainEcuData1, msg->data, sizeof(MainEcuStatus1_Msg_t));
	}
	else if(msg->ID == Main_ECU_CAN_ID + 1)
	{
		OD.SB.mEcuMsgReceived = 1;
		memcpy(&OD.MainEcuData2, msg->data, sizeof(MainEcuStatus2_Msg_t));
	}
	else if(msg->ID == Bmu_ECU_CAN_ID)
	{
		OD.SB.BatteryMsgReceived = 1;
		memcpy(&OD.BmuData1, msg->data, sizeof(BatM_Ext1_t));
	}
	else if(msg->ID == Bmu_ECU_CAN_ID + 2)
	{
		memcpy(&OD.BmuData2, msg->data, sizeof(BatM_Ext3_t));
	}
	else if(msg->ID == Bat_ECU_CAN_ID)
	{
		memcpy(&OD.PackData1, msg->data, sizeof(BatBatteryStatus1Msg_t));
	}
	else if(msg->ID == Bat_ECU_CAN_ID + Bat_ECUx_CAN_ID_LEN)
	{
		memcpy(&OD.PackData2, msg->data, sizeof(BatBatteryStatus1Msg_t));
	}
	else if(msg->ID == 0xa100100)
	{
		if(msg->data[0] == 224)
			OD.Isolation = (msg->data[2] << 8) + msg->data[3];
	}
	else if(msg->ID == GPS_ECU_CAN_ID)
	{
		memcpy(&OD.GpsData1, msg->data, sizeof(GpsStatus1_Msg_t));
	}
	else
		return 1;
    
	return 0;
}



void PCanMesGenerate(void)
{  
	// Номер отправляемого сообщения
		static uint8_t extSendMesNumber = 0;

	    // Генерация сообщений
		if (++extSendMesNumber >= EXTERNAL_SEND_MSG_AMOUNT)
			extSendMesNumber = 0;

		if (GetTimeFrom(extSendTime[extSendMesNumber]) >= extPeriod[extSendMesNumber])
		{
	         CanMsg *msg = ecanGetEmptyTxMsg(P_CAN_CH);

	        if(msg == 0)
	            return;

	        extSendTime[extSendMesNumber] = GetTimeStamp();
			msg->Ext = 0;

			switch (extSendMesNumber)
			{
				case 0:
				{
					msg->ID = Display_ECU_CAN_ID;
					msg->DLC = 8;
					msg->Ext = 0;

					cmGeneralControl1 *d = (cmGeneralControl1*)msg->data;

					d->LogicOutput = SET_OUT_STATE(D_IN1, D_IN_D_ECU_LIGHT_SWITCH_1) |
									SET_OUT_STATE(D_IN3, D_IN_D_ECU_LIGHT_SWITCH_2) |
									SET_OUT_STATE(D_IN5, D_IN_D_ECU_LIGHT_SWITCH_3) |
									SET_OUT_STATE(D_IN9, D_IN_D_ECU_BOOST_SWITCH);

					msg->data[4] = OD.A_CH_Voltage_0p1[0];
				}
				break;
			}
		}
    return;
}
