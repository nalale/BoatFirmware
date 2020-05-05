#include <string.h>

#include "Main.h"
#include "PCanProtocol.h"
#include "TimerFunc.h"

#include "../MolniaLib/DateTime.h"

enum { PM_Msg = 0,} Msgs_e;


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
	else
		return 1;
    
	return 0;
}



void PCanMesGenerate(void)
{  
    
    return;
}
