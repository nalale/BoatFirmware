#include "main.h"
#include "TimerFunc.h"
#include "user.h"
#include "protocol.h"
#include "ExternalProtocol.h"

#include <string.h>


#define LOCAL_SEND_MSG_AMOUNT	1


// �������
uint32_t MsgSendStamp[LOCAL_SEND_MSG_AMOUNT];
uint32_t MsgSendPeriod[LOCAL_SEND_MSG_AMOUNT] =  {
														10,	// �������� �������� � ����������� �� �������� ������������� ��������� �� �������
													};
	


// ��������� �������� ���������
uint8_t ExternalRx(CanMsg * msg)
{
	EcuConfig_t config = GetConfigInstance();
	OD.SB.ExtCanMsgReceived = 1;
	
	if(msg->ID == Bmu_ECU_CAN_ID)
	{
		BatM_Ext1_t* d = (BatM_Ext1_t*)msg->data;

		memcpy(&OD.BmuData1, d, sizeof(BatM_Ext1_t));
	}

	OD.SB.ExtCanMsgReceived = 0;
	return 1;
}


void ExternalMesGenerate(void)
{
	return;	
}

