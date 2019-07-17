#include "Main.h"
#include "Protocol.h"
#include "PCanProtocol.h"
#include "ExternalProtocol.h"
#include "../MolniaLib/N_OBD.h"

CanMsg PCanRxMsg;
CanMsg PCanTxMsg;

CanMsg DCanRxMsg;
CanMsg DCanTxMsg;
CanMsg DCanTxMsg2;


void Protocol(void)
{
    uint8_t isNewMsg1 = canRxQueue_ReadMsg(D_CAN_CH, &DCanRxMsg); // CanReceiveThread(D_CAN_CH, &DCanRxMsg);
    uint8_t isNewMsg2 = canRxQueue_ReadMsg(P_CAN_CH, &PCanRxMsg); //CanReceiveThread(P_CAN_CH, &PCanRxMsg);
    
	
	if(isNewMsg2)
		isNewMsg2 = ObdThread(&PCanRxMsg);
	
	if(isNewMsg1)
        isNewMsg1 = ExternalRx(&DCanRxMsg);
		
    if(isNewMsg2)		
		isNewMsg2 = PCanRx(&PCanRxMsg);
	

		
	
	ExternalMesGenerate();
    PCanMesGenerate();	
	ObdSendMes();
    
    if (GetBufferAvailable(P_CAN_CH) && ecanGetTxMsg(&PCanTxMsg, P_CAN_CH))
    { 	
		SendCanMsg(P_CAN_CH, &PCanTxMsg);
    }
    
    if (GetBufferAvailable(D_CAN_CH) && ecanGetTxMsg(&DCanTxMsg, D_CAN_CH))
    { 	
		SendCanMsg(D_CAN_CH, &DCanTxMsg);
    }
    
}


