#include "Main.h"
#include "PCanProtocol.h"
#include "SkfTypes.h"
#include "TimerFunc.h"
#include "../BoardDefinitions/MarineEcu_Board.h"
#include <string.h>

#include "../Libs/Btn8982.h"

#define EXTERNAL_SEND_MSG_AMOUNT	1


enum { PM_Msg = 0,} Msgs_e;

static uint32_t extSendTime[EXTERNAL_SEND_MSG_AMOUNT];
// Периоды отправки сообщений
static uint16_t extPeriod[EXTERNAL_SEND_MSG_AMOUNT] = {
								100,
								 };

uint8_t PCanRx(CanMsg *msg)
{
	EcuConfig_t config = GetConfigInstance();		
	
	if(msg->ID == PM_CAN_ID)
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
		memcpy(&OD.MainEcuData2, msg->data, sizeof(MainEcuStatus2_Msg_t));
	}
	else if(msg->ID == Bmu_ECU_CAN_ID)
	{
		OD.SB.BatteryMsgReceived = 1;
		memcpy(&OD.BmuData1, msg->data, sizeof(BatM_Ext1_t));

	}
	else
		return 1;
    
	return 0;
}



void PCanMesGenerate(void)
{  
    
    return;
}
