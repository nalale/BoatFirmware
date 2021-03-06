#include "Main.h"
#include "ExternalProtocol.h"
#include "protocol.h"
#include "TimerFunc.h"
#include "../MolniaLib/DateTime.h"

#define EXTERNAL_SEND_MSG_AMOUNT	1

static uint32_t extSendTime[EXTERNAL_SEND_MSG_AMOUNT];
// ������� �������� ���������
static const uint16_t extPeriod[EXTERNAL_SEND_MSG_AMOUNT] = {
								1000,
								 };

uint8_t ExternalRx(CanMsg *msg)
{
	if(msg->Ext)
		return 1;	
	
	EcuConfig_t _config = GetConfigInstance();
	
	if (msg->ID == BASE_CAN_ID)		// ���� �����
	{
		if(_config.IsTimeServer)
		{
			//TODO: ���-�� ��� ����� ��� � ���� ������� �������. ������ ������������ �������.
			// .....
		}
		else if(msg->DLC >= sizeof(cmTimeServer))
		{
			if(!dateTime_IsInit())
			{
				dateTime_InitCurrent(((cmTimeServer*)msg->data)->DateTime);
			}
		}

		return 0;
	}
	else if(msg->ID == Bmu_ECU_RX_ID)
	{
		EcuConfig_t cfgEcu = GetConfigInstance();
		if(cfgEcu.IsMaster)
		{
			BatM_ExtRx_t *d = (BatM_ExtRx_t*)msg->data;

			OD.SystemOperateEnabled = d->OpEnabled;
			OD.SB.MsgFromSystem = 1;
		}
	}
	else if(msg->ID == PM_CAN_ID)
	{
		cmPowerManager *d = (cmPowerManager*)msg->data;
		
		OD.PowerMaganerCmd = d->PowerState;
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
    // ����� ������������� ���������
	static uint8_t extSendMesNumber = 0;
    CanMsg *msg;
    
	EcuConfig_t _config = GetConfigInstance();
	
    // ��������� ���������
	if (++extSendMesNumber >= EXTERNAL_SEND_MSG_AMOUNT)
	{
		extSendMesNumber = 0;
	}

	// Wait for init RTC
	if(!_config.IsTimeServer || !dateTime_IsInit())
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
				d->DateTime = OD.SystemTime;
			}            
            break;
        }

    }  
    
    return;
}

