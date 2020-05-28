#include <string.h>

#include "Main.h"
#include "PCanProtocol.h"
#include "SkfTypes.h"
#include "TimerFunc.h"
#include "../BoardDefinitions/MarineEcu_Board.h"
#include "../MolniaLib/DateTime.h"


#include "../Libs/Btn8982.h"

#define EXTERNAL_SEND_MSG_AMOUNT	4


enum { PM_Msg = 3,} Msgs_e;

static uint32_t extSendTime[EXTERNAL_SEND_MSG_AMOUNT];
// Периоды отправки сообщений
static uint16_t extPeriod[EXTERNAL_SEND_MSG_AMOUNT] = {
								50,	
                                100,
								10,				// значение меняется в зависимости от текущего передаваемого сообщения из таблицы
								100,
								 };

uint8_t PCanRx(CanMsg *msg)
{
	EcuConfig_t config = GetConfigInstance();		
	OD.SB.PCanMsgReceived = 1;
	
	if(msg->ID == BASE_CAN_ID)
	{
		if(!dateTime_IsInit())
		{
			dateTime_InitCurrent(((cmTimeServer*)msg->data)->DateTime);
		}
	}
	else if(msg->ID == PM_CAN_ID)
	{
		OD.PowerManagerCmd = msg->data[0];
	}
	else if(msg->ID == General_Control1_CAN_ID)
    {
		cmGeneralControl1* d = (cmGeneralControl1*)msg->data;
		
        for(int i = 0; i < DIG_OUT_COUNT; i++)
		{
			if(config.DigitalOutput[i] < 32 && (d->LogicOutput & (1L << config.DigitalOutput[i])))
				SetDOutput(i, 1);
			else
				SetDOutput(i, 0);
		}
		
		return 0;	
    }
	else if(msg->ID == General_Control2_CAN_ID)
	{
		cmGeneralControl2* d = (cmGeneralControl2*)msg->data;
		
		for(int i = 0; i < AN_OUT_COUNT; i++)
		{			
			if(config.IsPowerManager && (i == 1))
				continue;		
			
			int byte_num = config.AnalogOutput[i];
			
			if(byte_num < 8)
				btnSetOutputLevel(i, d->AnalogOutput[byte_num]);
		}
		
		return 0;	
	}
    else if(msg->ID == General_Control3_CAN_ID)
    {
		cmGeneralControl3* d = (cmGeneralControl3*)msg->data;
		
		for(int i = 0; i < AN_OUT_COUNT; i++)
		{			
			int byte_num = config.AnalogOutput[i];
			
			if(byte_num >= 8 && byte_num < 16)
				btnSetOutputLevel(i, d->AnalogOutput[byte_num]);			
		}
		
		return 0;	
    }
	
	// Repeater
	if(OD.RepItemCount == 0)
	{
		CanMsg *_msg = ecanGetEmptyTxMsg(D_CAN_CH);
		if(_msg != 0)
			memcpy(_msg, msg, sizeof(CanMsg));
		return 0;	
	}
	else
	{	
		// Перебираем все шаблоны сообщений записанные в конфиге 
		for(uint8_t i = 0; i < OD.RepItemCount; i++)
		{
			canRepItem_t* it = &config.RepTable[i];
			
			if(it->Direction == 1)		// Пересылка PCAN -> ExtCAN
			{
				if(msg->Ext == it->Ext2 && msg->ID == it->Id2)
				{
					// Нашли сообщение для пересылки
					OD.msgTab[i].RepCount = it->RepCount;
					msg->Ext = it->Ext1;
					msg->ID = it->Id;
					memcpy(&OD.msgTab[i].msg, msg, sizeof(CanMsg));
					return 0;				
				}
			}
		}
	}
    
	OD.SB.PCanMsgReceived = 0;
    return 1;
}



void PCanMesGenerate(void)
{
    // Номер отправляемого сообщения
	static uint8_t extSendMesNumber = 0;
    static int8_t rep_msg_num = 0;		
	
	EcuConfig_t config = GetConfigInstance();
	
    CanMsg *msg;	
    
    // Генерация сообщений
	if (++extSendMesNumber >= EXTERNAL_SEND_MSG_AMOUNT)
		extSendMesNumber = 0;
	
	if(extSendMesNumber == PM_Msg && !config.IsPowerManager)
		return;

	// Для пересылки табличных сообщений
	if(extSendMesNumber == 2)
	{		
		rep_msg_num = ((++rep_msg_num) < OD.RepItemCount)? rep_msg_num : 0;	
		
		if(OD.RepItemCount > 0)
		{
			canRepItem_t* it = &config.RepTable[rep_msg_num];	
			
			// Пересылка ExtCAN -> PCAN
			if(it->Direction == 0 && OD.msgTab[rep_msg_num].RepCount)
			{
				extPeriod[extSendMesNumber] = it->SendPeriod;
				extSendTime[extSendMesNumber] = OD.msgTab[rep_msg_num].SendTime;
			}
			else 
				return;		
		}
		else 
			return;
	}

	if (GetTimeFrom(extSendTime[extSendMesNumber]) >= extPeriod[extSendMesNumber])
	{
         msg = ecanGetEmptyTxMsg(P_CAN_CH);
        
        if(msg == 0)
            return;		
        
        extSendTime[extSendMesNumber] = GetTimeStamp();
		msg->Ext = 0;

		switch (extSendMesNumber)
		{
            case 0:
			{
				smEcuStatus1* d = (smEcuStatus1*)msg->data;
			
                msg->ID = General_ECU_CAN_ID + config.Index * General_ECUx_CAN_ID_LEN;
                msg->DLC = 8;
				
				d->LogicInputsOutputs = (OD.IO & 0x3ff);
				
				for(int i = 4, j = 0; j < A_IN_NUM; i++, j++)				
					d->AnalogInput[j] = OD.A_CH_Voltage_0p1[j];
					
			}
                break;
			
            case 1:
            {
				smEcuStatus2* d = (smEcuStatus2*)msg->data;
				
                msg->ID = General_ECU_CAN_ID + (config.Index * General_ECUx_CAN_ID_LEN + 1);
                msg->DLC = 8;
				
				for(int i = 4, j = 0; j < DRIVER_NUM; i++, j++)				
					d->PwmLoadCurrent[j] = OD.Out_CSens[j].Out_CSens_Current;
				
				d->Faults = OD.Faults.Faults;		
				d->dummy1 = -1;
							
            }
                break;
			
			case 2:
			{	
				OD.msgTab[rep_msg_num].SendTime = GetTimeStamp();
				OD.msgTab[rep_msg_num].RepCount--;
				memcpy(msg, &OD.msgTab[rep_msg_num].msg, sizeof(CanMsg));								
			}
			break;
			
			case PM_Msg:
			{
				msg->ID = PM_CAN_ID;
				msg->DLC = 1;
				
				msg->data[0] = OD.PowerManagerCmd;
			}
			break;
        }
    }
    
    return;
}
