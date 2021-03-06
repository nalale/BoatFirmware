#include "EcuConfig.h"
#include "Protocol.h"

#include "../Protocols/TestMsgProtocol.h"
#include "../Protocols/ExternalProtocol.h"
#include "../MolniaLib/N_OBD.h"

#include "CanFunc.h"

CanMsg PCanRxMsg;
CanMsg PCanTxMsg;

//CanMsg DCanRxMsg;
//CanMsg DCanTxMsg;
//CanMsg DCanTxMsg2;


void Protocol(void)
{    
    uint8_t isNewMsg =  canRxQueue_ReadMsg(P_CAN_CH, &PCanRxMsg);//CanReceiveThread(P_CAN_CH, &PCanRxMsg);
	
	EcuConfig_t ecuConfig = GetConfigInstance();
    
	if(isNewMsg)
		isNewMsg = ObdThread(&PCanRxMsg);
    if(isNewMsg)
		isNewMsg = ExternalRx(&PCanRxMsg);
    if(isNewMsg)
		isNewMsg = moduleMsgHandler(&PCanRxMsg);
    if(isNewMsg)
		isNewMsg = packMsgHandler(&PCanRxMsg);
    if(isNewMsg)
		isNewMsg = MasterMsgHandler(&PCanRxMsg);
	
	//Master
	if(ecuConfig.IsMaster)
        MasterMesGenerate();
	
	//Battery
	if(ecuConfig.ModuleIndex == 0)
		SlaveMesGenerate();
	
	//Module
	ModuleMesGenerate();
	   
	ExternalMesGenerate();
	ObdSendMes();
	
	if(ecuConfig.TestMode)
		TestMesGenerate();

   
    if (GetBufferAvailable(P_CAN_CH) && ecanGetTxMsg(&PCanTxMsg, P_CAN_CH))
    { 	
		SendCanMsg(P_CAN_CH, &PCanTxMsg);
    }
    
    
    
    
		
    
}


