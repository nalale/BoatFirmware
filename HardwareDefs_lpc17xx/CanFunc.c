#include "lpc17xx_pinsel.h"
#include "lpc17xx_can.h"
#include "string.h"

#include "CanFunc.h"

CAN_MSG_Type TXMsg, RXMsg; // messages for test Bypass mode

rxBufCAN_t rxBufCan1, rxBufCan2;
txBufCAN_t txBufCan1, txBufCan2;

static void Pin_Configurate(LPC_CAN_TypeDef *CANx);

void Can_Init(uint8_t CanChannel1, uint8_t CanChannel2)
{	
	if(CanChannel1)
	{
		Pin_Configurate(LPC_CAN1);
		CAN_Init(LPC_CAN1, 250000);
    	CAN_IRQCmd(LPC_CAN1, CANINT_RIE, ENABLE);
	}
	if(CanChannel2)
	{
		Pin_Configurate(LPC_CAN2);
		CAN_Init(LPC_CAN2, 250000);
    	CAN_IRQCmd(LPC_CAN2, CANINT_RIE, ENABLE);
	}
	CAN_SetAFMode(LPC_CANAF,CAN_AccBP);
        
}


static void Pin_Configurate(LPC_CAN_TypeDef *CANx)
{
 /* Pin configuration
 * CAN1: select P0.0 as RD1. P0.1 as TD1
 * CAN2: select P2.7 as RD2, P2.8 as RD2
 */
    PINSEL_CFG_Type PinCfg;
	PinCfg.Funcnum = 1;
	PinCfg.OpenDrain = 0;
	PinCfg.Pinmode = 0;
	
	if(CANx == LPC_CAN1)
	{	
		// CAN 1
		PinCfg.Portnum = 0;
		
		PinCfg.Pinnum = 0;		
		PINSEL_ConfigPin(&PinCfg);
		PinCfg.Pinnum = 1;
		PINSEL_ConfigPin(&PinCfg);
	}
	else if(CANx == LPC_CAN2)
	{
		// CAN2
		PinCfg.Portnum = 2;
		
		PinCfg.Pinnum = 7;		
		PINSEL_ConfigPin(&PinCfg);
		PinCfg.Pinnum = 8;
		PINSEL_ConfigPin(&PinCfg);
	}
}

uint8_t CanReceiveThread(uint8_t CanChannel, CanMsg *Msg)
{ 
  //uint8_t can1_data_available = CAN_GetCTRLStatus (LPC_CAN1, CANCTRL_GLOBAL_STS) & CAN_GSR_RBS;
  
  if(CanChannel == 0)
  {
    if(CAN_ReceiveMsg(LPC_CAN1, &RXMsg))
    {
        Msg->ID = RXMsg.id;
        Msg->Ext = RXMsg.format;
        Msg->DLC = RXMsg.len;
            
        for(uint8_t i = 0; i < Msg->DLC; i++)
        {            
            if(i < 4)
                Msg->data[i] = RXMsg.dataA[i];
            
            else if(Msg->DLC > 4 && i >= 4)
                Msg->data[i] = RXMsg.dataB[i - 4];
        }
        
        return 1;
    }    
  }
  else if(CanChannel == 1)
  {
    if(CAN_ReceiveMsg(LPC_CAN2, &RXMsg))
    {
        Msg->ID = RXMsg.id;
        Msg->Ext = RXMsg.format;
        Msg->DLC = RXMsg.len;
            
        for(uint8_t i = 0; i < Msg->DLC; i++)
        {
             if(i < 4)
                Msg->data[i] = RXMsg.dataA[i];
            
            else if(Msg->DLC > 4 && i >= 4)
                Msg->data[i] = RXMsg.dataB[i - 4];
        }
        
        return 1;
    }    
  }
  
  return 0;
}


CanMsg* canRxQueue_GetWriteMsg(int32_t buf_num)
{
    int next = 0;
	CanMsg* msg;
	
	switch(buf_num)
	{
		case 0:
		{
			next = rxBufCan1.Head + 1;
			if (next >= RX_BUF_CAN_LEN)
				next = 0;

			if (next == rxBufCan1.Tail)		// Буфер полный
				return 0;

			msg = &rxBufCan1.List[rxBufCan1.Head];
			rxBufCan1.Head = next;
		}
		break;
		
		case 1:
		{
			next = rxBufCan2.Head + 1;
			if (next >= RX_BUF_CAN_LEN)
				next = 0;

			if (next == rxBufCan2.Tail)		// Буфер полный
				return 0;

			msg = &rxBufCan2.List[rxBufCan2.Head];
			rxBufCan2.Head = next;
		}
		break;
		
		default:
			return 0;
	}
    return msg;
}

// Чтение из очереди входящего сообщения
int canRxQueue_ReadMsg(int32_t buf_num, CanMsg* msg)
{
	int next = 0;
	
	switch(buf_num)
    {
        case 0:
            
			if (rxBufCan1.Head == rxBufCan1.Tail)		// Буфер пустой
				return 0;

			next = rxBufCan1.Tail + 1;
			if(next >= RX_BUF_CAN_LEN)
				next = 0;

			memcpy(msg, &rxBufCan1.List[rxBufCan1.Tail], sizeof(CanMsg));
			rxBufCan1.Tail = next;
			return 1;
	
		case 1:
			if (rxBufCan2.Head == rxBufCan2.Tail)		// Буфер пустой
				return 0;

			next = rxBufCan2.Tail + 1;
			if(next >= RX_BUF_CAN_LEN)
				next = 0;

			memcpy(msg, &rxBufCan2.List[rxBufCan2.Tail], sizeof(CanMsg));
			rxBufCan2.Tail = next;
			return 1;
	}
	
	return 0;
}

uint8_t ecanGetTxMsg(CanMsg* msg, int32_t buf_num)
{
    int next;
    
    switch(buf_num)
    {
        case 0:
            
            if (txBufCan1.Head == txBufCan1.Tail)		// Буфер пустой
                return 0;

            next = txBufCan1.Tail + 1;
            if(next >= TX_BUF_CAN_LEN)
                next = 0;

            memcpy(msg, &txBufCan1.List[txBufCan1.Tail], sizeof(CanMsg));
            txBufCan1.Tail = next;
        break;
        
        case 1:
            
            if (txBufCan2.Head == txBufCan2.Tail)		// Буфер пустой
                return 0;

            next = txBufCan2.Tail + 1;
            if(next >= TX_BUF_CAN_LEN)
                next = 0;

            memcpy(msg, &txBufCan2.List[txBufCan2.Tail], sizeof(CanMsg));
            txBufCan2.Tail = next;
        break;
    }
    
    return 1;
}

CanMsg* ecanGetEmptyTxMsg(int32_t buf_num)
{
    int next;
    CanMsg* msg = NULL;
    
    switch(buf_num)
    {
        case 0:
            next = txBufCan1.Head + 1;
            if (next >= TX_BUF_CAN_LEN)
                next = 0;

            if (next == txBufCan1.Tail)		// Буфер полный
                return 0;

            msg = &txBufCan1.List[txBufCan1.Head];
            
            txBufCan1.Head = next;
            break;
    
        case 1:
            next = txBufCan2.Head + 1;
            if (next >= TX_BUF_CAN_LEN)
                next = 0;

            if (next == txBufCan2.Tail)		// Буфер полный
                return 0;

            msg = &txBufCan2.List[txBufCan2.Head];
            txBufCan2.Head = next;
        break;
    }
    
    msg->ID = 0;
    msg->DLC = 0;
    for(uint32_t i = 0; i < 8; i++)
        msg->data[i] = 0;
    
    return msg;
}

uint32_t GetBufferAvailable(uint32_t channel)
{
    uint32_t CANStatus;
    
    switch(channel)
    {
        case 0:            
			if((LPC_CAN1->MOD & 0x01) == 1)
				LPC_CAN1->MOD &= ~0x01;
			
            CANStatus = LPC_CAN1->SR;
            if((CANStatus & 0x00040000) || (CANStatus & 0x00000400) || (CANStatus & 0x00000004))
                return 1;            
            break;
        
        case 1:
			if((LPC_CAN2->MOD & 0x01) == 1)
				LPC_CAN2->MOD &= ~0x01;
			
            CANStatus = LPC_CAN2->SR;
            if((CANStatus & 0x00040000) || (CANStatus & 0x00000400) || (CANStatus & 0x00000004))
                return 1;            
            break;
    }
    
    return 0;
}

uint8_t SendCanMsg(uint8_t channel, CanMsg *msg)
{
    uint8_t status = 0;
    if(channel == 0)
    {
        TXMsg.id = msg->ID;
        TXMsg.format = msg->Ext;
        TXMsg.len = msg->DLC;
        
        for(uint8_t i = 0; i < msg->DLC; i++)
        {
            if(i < 4)
                TXMsg.dataA[i] = msg->data[i];
            
            if(msg->DLC > 4 && i >= 4)
                TXMsg.dataB[i - 4] = msg->data[i];
        }
        
        status = CAN_SendMsg (LPC_CAN1, &TXMsg);
    }
    else if(channel == 1)
    {
        TXMsg.id = msg->ID;
        TXMsg.format = msg->Ext;
        TXMsg.len = msg->DLC;
        
        for(uint8_t i = 0; i < msg->DLC; i++)
        {
            if(i < 4)
                TXMsg.dataA[i] = msg->data[i];
            
            if(msg->DLC > 4 && i >= 4)
                TXMsg.dataB[i - 4] = msg->data[i];
        }
        
        status = CAN_SendMsg (LPC_CAN2, &TXMsg);
    }
    return status;
}
