#include <string.h>

#include "Main.h"
#include "Protocol.h"
#include "CanFunc.h"
#include "TimerFunc.h"
#include "ExternalProtocol.h"

#include "PCanProtocol.h"
#include "../MolniaLib/N_OBD.h"

CanMsg PCanRxMsg;
CanMsg PCanTxMsg;

CanMsg DCanRxMsg;
CanMsg DCanTxMsg;
CanMsg DCanTxMsg2;


void Protocol(void)
{
	CanMsg tmpCanTxMsg;

    uint8_t isNewMsg1 = canRxQueue_ReadMsg(D_CAN_CH, &DCanRxMsg);
    uint8_t isNewMsg2 = canRxQueue_ReadMsg(P_CAN_CH, &PCanRxMsg); //CanReceiveThread(P_CAN_CH, &PCanRxMsg); //
    
	
	if(isNewMsg2)
		isNewMsg2 = ObdThread(&PCanRxMsg);	
	if(isNewMsg2)
		isNewMsg2 = PCanRx(&PCanRxMsg);	
	if(isNewMsg2)
		isNewMsg2 = obcMessageHandler((ObcParameterTypes_e)PCanRxMsg.ID, PCanRxMsg.data, PCanRxMsg.DLC);

	if(isNewMsg1)
		isNewMsg1 = McuRinehartMsgHandler(&OD.mcHandler, DCanRxMsg.ID, DCanRxMsg.data, DCanRxMsg.DLC);
	if(isNewMsg1)
		isNewMsg1 = HelmMessageHandler((HelmParameterTypes_e)DCanRxMsg.ID, DCanRxMsg.data, DCanRxMsg.DLC);


    PCanMesGenerate();
    ObdSendMes();
    
	// Obc
    if((int)(tmpCanTxMsg.ID = (uint32_t)obcMessageGenerate(tmpCanTxMsg.data, &tmpCanTxMsg.DLC, GetTimeStamp())) != MSG_OBC_ERROR_ID)
    {
    	CanMsg *msgContainer = ecanGetEmptyTxMsg(P_CAN_CH);

		if(msgContainer > 0)
		{
			tmpCanTxMsg.Ext = 1;
			memcpy(msgContainer, &tmpCanTxMsg, sizeof(CanMsg));
		}
    }
	
    // Inverter
    if((int)(tmpCanTxMsg.ID = (uint32_t)McuRinehartTxMsgGenerate(&OD.mcHandler, tmpCanTxMsg.data, &tmpCanTxMsg.DLC, GetTimeStamp())) != -1)
    {
    	CanMsg *msgContainer = ecanGetEmptyTxMsg(D_CAN_CH);

    	if(msgContainer > 0)
		{
			tmpCanTxMsg.Ext = 0;
    		memcpy(msgContainer, &tmpCanTxMsg, sizeof(CanMsg));
		}
    }

    // Helm
    if((int)(tmpCanTxMsg.ID = HelmMessageGenerate(tmpCanTxMsg.data, &tmpCanTxMsg.DLC, GetTimeStamp())) != MSG_HELM_ERROR_ID)
    {
    	CanMsg *msgContainer = ecanGetEmptyTxMsg(D_CAN_CH);
		
		if(msgContainer > 0)
		{
			tmpCanTxMsg.Ext = 1;
			memcpy(msgContainer, &tmpCanTxMsg, sizeof(CanMsg));
		}
    }

    if (GetBufferAvailable(P_CAN_CH) && ecanGetTxMsg(&PCanTxMsg, P_CAN_CH))
		SendCanMsg(P_CAN_CH, &PCanTxMsg);
    

    if (GetBufferAvailable(D_CAN_CH) && ecanGetTxMsg(&DCanTxMsg, D_CAN_CH))
	{
		SendCanMsg(D_CAN_CH, &DCanTxMsg);
	}

}


