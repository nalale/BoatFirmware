#include "main.h"
#include "TimerFunc.h"
#include "user.h"
#include "protocol.h"
#include "ExternalProtocol.h"

#include <string.h>


#define LOCAL_SEND_MSG_AMOUNT	1


// Таймеры
uint32_t MsgSendStamp[LOCAL_SEND_MSG_AMOUNT];
uint32_t MsgSendPeriod[LOCAL_SEND_MSG_AMOUNT] =  {
														10,	// значение меняется в зависимости от текущего передаваемого сообщения из таблицы
													};
	


// Обработка входящих сообщений
uint8_t ExternalRx(CanMsg * msg)
{
	EcuConfig_t config = GetConfigInstance();
	OD.SB.ExtCanMsgReceived = 1;
	
	// Repeater
	if(OD.RepItemCount == 0)
	{
		CanMsg *_msg = ecanGetEmptyTxMsg(P_CAN_CH);
		if(_msg != NULL)
			memcpy(_msg, msg, sizeof(CanMsg));		
		return 0;	
	}
	else
	{	
		// Перебираем все шаблоны сообщений записанные в конфиге 
		for(uint8_t i = 0; i < OD.RepItemCount; i++)
		{
			canRepItem_t* it = &config.RepTable[i];
			
			if(it->Direction == 0)		// Пересылка External CAN -> PCAN
			{
				if(msg->Ext == it->Ext1 && msg->ID == it->Id)
				{					
					// Нашли сообщение для пересылки
					msg->Ext = it->Ext2;
					msg->ID = it->Id2;
					OD.msgTab[i].RepCount = it->RepCount;
					memcpy(&OD.msgTab[i].msg, msg, sizeof(CanMsg));
					return 0;				
				}
			}
		}
	}
	
	OD.SB.ExtCanMsgReceived = 0;
	return 1;
}


void ExternalMesGenerate(void)
{
	static uint8_t msgNumber = 0;		
	static int8_t rep_msg_num = 0;	

	CanMsg* msg;			
    
	if(++msgNumber >= LOCAL_SEND_MSG_AMOUNT)
		msgNumber = 0;
	
	// Для пересылки табличных сообщений
	if(msgNumber == 0)
	{		
		EcuConfig_t config = GetConfigInstance();	
		rep_msg_num = ((++rep_msg_num) < OD.RepItemCount)? rep_msg_num : 0;	
		
		if(OD.RepItemCount > 0)
		{
			canRepItem_t* it = &config.RepTable[rep_msg_num];	
			
			// Если пересылка CAN1 -> CAN2 и не таймаут
			if(it->Direction == 1 && OD.msgTab[rep_msg_num].RepCount)
			{
				MsgSendPeriod[msgNumber] = it->SendPeriod;
				MsgSendStamp[msgNumber] = OD.msgTab[rep_msg_num].SendTime;
			}
			else 
				return;
		}
		else
			return;
	}

	// Генерация сообщений
	if (GetTimeFrom(MsgSendStamp[msgNumber]) >= MsgSendPeriod[msgNumber])
	{				
		msg = ecanGetEmptyTxMsg(D_CAN_CH);
		if(msg == 0)		
			return;				
		
		msg->Ext = 0;
		MsgSendStamp[msgNumber] = GetTimeStamp();		
		
		switch (msgNumber)
		{
			case 0:
			{               
				OD.msgTab[rep_msg_num].SendTime = GetTimeStamp();
				OD.msgTab[rep_msg_num].RepCount--;
				memcpy(msg, &OD.msgTab[rep_msg_num].msg, sizeof(CanMsg));
			}
			break;
		}		
	}    
}

