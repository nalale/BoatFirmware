#include "Main.h"
#include "ExternalProtocol.h"
#include "TimerFunc.h"

#define EXTERNAL_SEND_MSG_AMOUNT	0

static uint32_t extSendTime[EXTERNAL_SEND_MSG_AMOUNT];
// Периоды отправки сообщений
const uint16_t extPeriod[EXTERNAL_SEND_MSG_AMOUNT] = {								
                                //250,
								 };

uint8_t ExternalRx(CanMsg *msg)
{


    return 1;
}



void ExternalMesGenerate(void)
{
    
    return;
}
