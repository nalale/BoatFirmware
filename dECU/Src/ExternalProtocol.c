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
	
	return 1;
}


void ExternalMesGenerate(void)
{
	return;	
}

