#include "MotionControllers/SevconProtocol.h"

#include "main.h"
#include "CanFunc.h"
#include "TimerFunc.h"
#include "user.h"
#include "protocol.h"


#define LOCAL_SEND_MSG_AMOUNT	3

// Извлекает номер модуля из Id сообщения.
#define GET_NODE_NUMBER_FROM_ID(id) ((id>>6) & 0x7F)
// Извлекает номер сообщения из Id сообщения.
#define GET_MES_NUMBER_FROM_ID(id)   (id & 0x3F)
// Макса для StatusWord
#define STATUS_WORD_MASK 	0x6F
// Период отправки
#define SEVCON_SEND_PERIOD  100


// ControlWors
typedef enum {SHUTDOWN_REQUEST = 0x06, SWITCH_ON_REQUEST = 0x07, ENABLE_OPERATION = 0x0F} SEVCON_CONTROLWORDS_ENUM;


// Таймеры
uint32_t SevconSendStamp[LOCAL_SEND_MSG_AMOUNT];
uint32_t SevconSendPeriod[LOCAL_SEND_MSG_AMOUNT] =  {
														500,
														50,
														125
													};
	

static OperationStates RequestState;
static CanOpenStates ActualState;
static SEVCON_STATUSWORDS_ENUM ActualStatus;
static uint16_t RequestControlWord;
													

// ID сообщений
const uint16_t SevconCanOpenID = 0x06;
const uint8_t SevconNodeState[2] = {0x80, 0x01};
// Двигатели




// Обработка входящих сообщений
uint8_t InvProtocolRx(CanMsg * msg)
{
	int32_t tmp_speed = 0;
	//	Sevcon TPDO
	if ((0xFF0 & msg->ID) == 0x180)
	{
		switch (GET_MES_NUMBER_FROM_ID(msg->ID))
		{
			case 0:
				OD.SB.InverterIsOperate = 0;
				ActualStatus = (SEVCON_STATUSWORDS_ENUM) (STATUS_WORD_MASK & ((msg->data[1] << 8) + msg->data[0]));				
				tmp_speed = (int16_t)((int32_t)(msg->data[7] << 24) | (int32_t)(msg->data[6] << 16) | (int32_t)(msg->data[5] << 8) | (int32_t)msg->data[4]);
				OD.MovControlDataRx.ActualSpeed = (tmp_speed < 0)? -tmp_speed : tmp_speed;
			
				switch (ActualStatus)
				{
					case NOT_READY_TO_SWITCH_ON:
						RequestControlWord = SHUTDOWN_REQUEST;
						break;
					case SWITCH_ON_DISABLE:
						RequestControlWord = SHUTDOWN_REQUEST;
						break;

					case READY_TO_SWITCH_ON_SW:
						RequestControlWord = SWITCH_ON_REQUEST;
						break;

					case OPERATION_ENABLED_SW:                
						OD.SB.InverterIsOperate = 1;
						RequestControlWord = ENABLE_OPERATION;
						break;
					
					case FAULT:
						RequestControlWord = SHUTDOWN_REQUEST;
						break;
					
					case SWITCH_ON:
						RequestControlWord = ENABLE_OPERATION;
					break;
						
				}
				break;
			case 1:
				OD.InvertorDataRx.MotorTemperature = (int16_t) (msg->data[1] << 8) + msg->data[0];
				OD.InvertorDataRx.InverterTemperature = msg->data[2];
				break;
			case 4:
				OD.InvertorDataRx.FaultCode = (uint16_t) ((msg->data[1] << 8) + msg->data[0]);				
				break;
		}
    }
    
    if(msg->ID == (0x700 | SevconCanOpenID))
    {
		OD.LogicTimers.InverterTimer_ms = GetTimeStamp();

		// Обновить сосотояние
		if (msg->data[0] == 0x7F )
		{
			ActualState = PreOperationMode;
			ActualStatus = NOT_READY_TO_SWITCH_ON;
			OD.MovControlDataRx.ActualSpeed = 0;
			OD.InvertorDataRx.MotorTemperature = 0;
			OD.InvertorDataRx.InverterTemperature = 0;
			OD.SB.InverterIsOperate = 0;
		}
		else if (msg->data[0] == 0x05)
		{
			ActualState = OperationMode;
		}

		return 0;
	}
    
	return 1;
}


// Генерация сообщений для Sevcon
void InvProtocolMesGenerate(void)
{
	static uint8_t msgNumber = 0;	
	CanMsg* SevconMsg;
    
	if(++msgNumber >= LOCAL_SEND_MSG_AMOUNT)
		msgNumber = 0;

	// Генерация сообщений
	if (GetTimeFrom(SevconSendStamp[msgNumber]) >= SevconSendPeriod[msgNumber])
	{				
		SevconMsg = ecanGetEmptyTxMsg(D_CAN_CH);
		if(SevconMsg == 0)
		{
			return;
		}		

		SevconMsg->Ext = 0;
		SevconSendStamp[msgNumber] = GetTimeStamp();		
		
		switch (msgNumber)
		{
			case 0:
			{
				CanOpenNMT* data = (CanOpenNMT*)SevconMsg->data;
			
				SevconMsg->ID = SEV_NMT_CAN_ID;
				SevconMsg->DLC = 2;
			
				
				data->CanOpenNodeId = SevconCanOpenID;
				data->CanOpenNodeState = SevconNodeState[RequestState];                    
				
			}
				break;
			
			case 1:
			{
				SevconRPDO_0* data = (SevconRPDO_0*)SevconMsg->data;
				
				SevconMsg->ID = 0x189;
				SevconMsg->DLC = 8;				
									
				data->ControlWord = RequestControlWord;
				data->TargetSpeed = (OD.TractionData.Direction)? OD.TractionData.TargetTorque : -OD.TractionData.TargetTorque;
				data->MaximumTorque = 1000;				
			}
				break;
			
			case 2:
			{			
				SevconRPDO_1* data = (SevconRPDO_1*)SevconMsg->data;
				
				SevconMsg->ID = 0x189 + 1;
				SevconMsg->DLC = 8;
				
				data->DCL = (int16_t)3000;
				data->CCL = (int16_t)-100;
			}
			break;
		}		
	}    
}


// Установить состояние привод
void SevconSetState(OperationStates State)
{
	RequestState = State;
}

