#include <string.h>
#include "Main.h"
#include "InverterProtocol.h"
#include "TimerFunc.h"

#define INV_PROTOCOL_SEND_MSG_AMOUNT	1

static uint32_t msgSendTime[INV_PROTOCOL_SEND_MSG_AMOUNT];
// Периоды отправки сообщений
const uint16_t msgPeriod[INV_PROTOCOL_SEND_MSG_AMOUNT] = {
								50,                         // CmdMsg
 //                               250,                        // CurrentLimit
								 };

uint8_t InvProtocolRx(CanMsg *msg)
{
    switch(msg->ID)
    {
        case InvInternalStatesMsg_ID:
        {
            InvInternalStatesMsg_t *d = (InvInternalStatesMsg_t*)msg->data;
            
            OD.InvertorDataRx.InverterIsEnable = d->InvEnableState;
            OD.InvertorDataRx.LockupIsEnable = d->InvEnableLockout;
            
            OD.LogicTimers.InverterTimer_ms = GetTimeStamp();
        }
            break;
		
		case MotorPositionMsg_ID:
		{
			MotorPositionMsg_t *d = (MotorPositionMsg_t*)msg->data;
			OD.MovControlDataRx.ActualSpeed = d->MotorSpeed;
		}
		break;
		
		case TempSet3Msg_ID:
		{
			TempSet3Msg_t *d = (TempSet3Msg_t*)msg->data;
			
			OD.InvertorDataRx.MotorTemperature = d->MotorTem_0p1 / 10;
		}
		break;
		
		case TempSet2Msg_ID:
		{
			TempSet2Msg_t *d = (TempSet2Msg_t*)msg->data;
			
			OD.InvertorDataRx.InverterTemperature = d->ControlBoardTemp_0p1;
		}
		break;
        
        default:
            return 1;
    }
    
    return 0;
}



void InvProtocolMesGenerate(void)
{
    // Номер отправляемого сообщения
	static uint8_t extSendMesNumber = 0;
    
    CanMsg *msg;
    
    // Генерация сообщений
	if (++extSendMesNumber >= INV_PROTOCOL_SEND_MSG_AMOUNT)
		extSendMesNumber = 0;


	if (GetTimeFrom(msgSendTime[extSendMesNumber]) > msgPeriod[extSendMesNumber])
	{
        msg = ecanGetEmptyTxMsg(D_CAN_CH);
        
        if(msg == 0)
            return;
        
        msgSendTime[extSendMesNumber] = GetTimeStamp();
		msg->Ext = 0;

		switch (extSendMesNumber)
		{
            case 0:
            {
                msg->ID = InvCmdMsg_ID;
                msg->DLC = 8;
                
                InvCmdMsg_t *d = (InvCmdMsg_t*)msg->data;               
                
                d->InvEnable = OD.TractionData.InverterEnable;
                
                d->TorqueCmd_0p1 = 0;
                d->SpeedCmd = OD.TractionData.TargetTorque;
                d->TorqueLimitCmd_0p1 = 0;
                d->DirCmd = OD.TractionData.Direction;
                
                d->InvDischarge = 0;
                d->SpeedModeEnable = 0;
            }
                break;
            
            case 1:
            {
                msg->ID = BMSCurrentLimitMsg_ID;
                msg->DLC = 8;
                
                BMSCurrentLimitMsg_t *d = (BMSCurrentLimitMsg_t*)msg->data;
				
				d->MaxChargeCurrent = OD.BatteryDataRx.CCL;
				d->MaxDischargeCurrent = OD.BatteryDataRx.DCL;
            }
            break;
        }       
    }    
    
    return;
}


