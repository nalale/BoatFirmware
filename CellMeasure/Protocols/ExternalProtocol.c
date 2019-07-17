#include "Main.h"
#include "ExternalProtocol.h"
#include "protocol.h"
#include "TimerFunc.h"
#include "../MolniaLib/DateTime.h"

#define EXTERNAL_SEND_MSG_AMOUNT	1

static uint32_t extSendTime[EXTERNAL_SEND_MSG_AMOUNT];
// Периоды отправки сообщений
const uint16_t extPeriod[EXTERNAL_SEND_MSG_AMOUNT] = {
								1000,
								 };

uint8_t ExternalRx(CanMsg *msg)
{   
	OD.SB.MsgFromPM = 0;
	
	if(msg->Ext)
		return 1;	
	
	EcuConfig_t _config = GetConfigInstance();
	
	if (msg->ID == BASE_CAN_ID)		// Дата время
	{
		if(_config.IsTimeServer)
		{
			//TODO: Кто-то еще кроме нас в роли сервера времени. Ошибка конфигурации системы.
			// .....
		}
		else if(msg->DLC >= sizeof(cmTimeServer))
		{
			secondsTotal = ((cmTimeServer*)msg->data)->DateTime;
		}

		return 0;
	}
	else if(msg->ID == PM_CAN_ID)
	{
		cmPowerManager *d = (cmPowerManager*)msg->data;
		
		//OD.PowerMaganerState = d->PowerState;
		OD.SB.MsgFromPM = 1;

//		if(d->PowerState == PS_PowerOff)
//		{
//			if(OD.SB.PowerOn)
//			{
//				OD.SB.PowerOn = 0;
//			}
//		}
	}
	
    return 1;
}



void ExternalMesGenerate(void)
{
    // Номер отправляемого сообщения
	static uint8_t extSendMesNumber = 0;
    CanMsg *msg;
    
	EcuConfig_t _config = GetConfigInstance();
	
    // Генерация сообщений
	if (++extSendMesNumber >= EXTERNAL_SEND_MSG_AMOUNT)
	{
		extSendMesNumber = 0;
	}

	if(!_config.IsTimeServer)
	{
		if(extSendMesNumber == 0)
			return;
	}

	if (GetTimeFrom(extSendTime[extSendMesNumber]) > extPeriod[extSendMesNumber])
	{
         msg = ecanGetEmptyTxMsg(P_CAN_CH);
        
        if(msg == 0)
            return;
        
        extSendTime[extSendMesNumber] = GetTimeStamp();
		
		msg->ID = BASE_CAN_ID + extSendMesNumber;
		msg->DLC = 8;
		msg->Ext = 0;
		
		switch (extSendMesNumber)
		{
            case 0:
			{				
				msg->DLC = 4;
				cmTimeServer *d = (cmTimeServer*)msg->data;
				d->DateTime = secondsTotal;
			}            
            break;
        }

    }  
    
    return;
}

