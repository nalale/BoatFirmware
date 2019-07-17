#include "Main.h"
#include "ExternalProtocol.h"
#include "InverterProtocol.h"
#include "SkfTypes.h"
#include "TimerFunc.h"
#include "Inverter.h"

#define EXTERNAL_SEND_MSG_AMOUNT	3

static uint32_t extSendTime[EXTERNAL_SEND_MSG_AMOUNT];
// Периоды отправки сообщений
const uint16_t extPeriod[EXTERNAL_SEND_MSG_AMOUNT] = {								
                                250,
								100,
								100,
								 };

uint8_t ExternalRx(CanMsg *msg)
{
	static uint16_t heart_cntr_prev = 0;
    switch(msg->ID)
	{
		case 0x7D:
		{
			MSG_ABS_ANGLE *d = (MSG_ABS_ANGLE*)msg->data;
			OD.LogicTimers.SteeringTimer_ms = GetTimeStamp();
			
			if(d->C_NUM_STEERING_REV == 0)
			{           
				OD.SteeringDataRx.TargetAngle = d->C_ANGLE;
				OD.SteeringDataRx.NumSteeringRev = d->C_NUM_STEERING_REV;
				OD.SteeringDataRx.SteeringSpeed_rads = (d->C_STEERING_SPEED_HIG << 8) | d->C_STEERING_SPEED_LOW;
				
				if(d->C_HEART_COUNTER != heart_cntr_prev)
					OD.LogicTimers.SteeringTimer_ms = GetTimeStamp();
				
				heart_cntr_prev = d->C_HEART_COUNTER;
			}
			return 0;
		}
		case 0xac:
		{
			switch(msg->data[0])
			{
				case 0xe5:
					OD.SteeringDataRx.SetZeroRegBrake_Ack = 1;
					break;
				
				case 0xea:
					OD.SteeringDataRx.EndStopLeft_Ack = 1;
					break;
				
				case 0xe9:
					OD.SteeringDataRx.EndStopRight_Ack = 1;
					break;
			}
			return 0;
		}
		case InvInternalStatesMsg_ID:
        {
            InvInternalStatesMsg_t *d = (InvInternalStatesMsg_t*)msg->data;
            
            OD.InvertorDataRx.InverterIsEnable = d->InvEnableState;
            OD.InvertorDataRx.LockupIsEnable = d->InvEnableLockout;
            OD.InvertorDataRx.InverterState = (InverterStates_e)d->VSMState;
            OD.LogicTimers.InverterTimer_ms = GetTimeStamp();
        }
            return 0;
		
		case MotorPositionMsg_ID:
		{
			MotorPositionMsg_t *d = (MotorPositionMsg_t*)msg->data;
			OD.MovControlDataRx.ActualSpeed = (d->MotorSpeed < 0)? -d->MotorSpeed : d->MotorSpeed;
		}
		return 0;
		
		case TorqueAndTimerMsg_ID:
		{
			TorqueAndTimerMsg_t *d = (TorqueAndTimerMsg_t*)msg->data;
			OD.MovControlDataRx.ActualTorque = (d->TorqueFeedback_0p1 < 0)? -d->TorqueFeedback_0p1 / 10 : d->TorqueFeedback_0p1 / 10;
		}
		return 0;
		
		case TempSet3Msg_ID:
		{
			TempSet3Msg_t *d = (TempSet3Msg_t*)msg->data;
			
			OD.InvertorDataRx.MotorTemperature = d->MotorTem_0p1 / 10;
		}
		return 0;
		
		case TempSet2Msg_ID:
		{
			TempSet2Msg_t *d = (TempSet2Msg_t*)msg->data;
			
			OD.InvertorDataRx.InverterTemperature = d->ControlBoardTemp_0p1 / 10;
		}
		return 0;
		
		case InvCurrentMsg_ID:
		{
			InvCurrentMsg_t *d = (InvCurrentMsg_t*)msg->data;
			
			OD.InvertorDataRx.CurrentDC = d->DCBusCurrent_0p1;
		}
		return 0;
		
		case InvVoltageMsg_ID:
		{
			InvVoltageMsg_t *d = (InvVoltageMsg_t*)msg->data;
			
			OD.InvertorDataRx.VoltageDC = d->DCBusVoltage_0p1;
		}
		return 0;
	}
    return 1;
}



void ExternalMesGenerate(void)
{
    // Номер отправляемого сообщения
	static uint8_t extSendMesNumber = 0;
    
    CanMsg *msg;
    
    // Генерация сообщений
	if (++extSendMesNumber >= EXTERNAL_SEND_MSG_AMOUNT)
		extSendMesNumber = 0;


	if (GetTimeFrom(extSendTime[extSendMesNumber]) > extPeriod[extSendMesNumber])
	{
         msg = ecanGetEmptyTxMsg(D_CAN_CH);
        
        if(msg == 0)
            return;
        
        extSendTime[extSendMesNumber] = GetTimeStamp();
		msg->Ext = 0;

		switch (extSendMesNumber)
		{
            case 0:
            {
                msg->ID = 0xe5;
                msg->DLC = 1;
                msg->Ext = 1;
                
                MSG_SET_REGULATED_BRAKE_FRICTION* d = (MSG_SET_REGULATED_BRAKE_FRICTION*) msg->data;
                d->C_PERC_MAX_BRAKE_FRICTION = OD.SteeringData.TargetSteeringBrake;
            }
                break;
			
			case 1:
            {
                msg->ID = InvCmdMsg_ID;
                msg->DLC = 8;
                
                InvCmdMsg_t *d = (InvCmdMsg_t*)msg->data;               
                
                d->InvEnable = OD.TractionData.InverterEnable;
                d->DirCmd = (OD.TractionData.Direction > 0)? 0 : 1;
				d->TorqueCmd_0p1 = OD.TractionData.TargetTorque * 10;
				
                d->TorqueLimitCmd_0p1 = 0;
                d->InvDischarge = 0;
                d->SpeedModeEnable = 0;
            }
                break;
            
            case 2:
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

uint8_t SendSteeringStartupMsg(void)
{
    static uint32_t send_timestamp = 0;
    CanMsg *msg;     
    
    
    if(GetTimeFrom(send_timestamp) >= 250)
    {           
		if(!OD.SteeringDataRx.EndStopRight_Ack || !OD.SteeringDataRx.EndStopLeft_Ack)
		{
			msg = ecanGetEmptyTxMsg(D_CAN_CH);
    
			if(msg == 0)
				return 0;       
			
			send_timestamp = GetTimeStamp();    
		}
  
        if(!OD.SteeringDataRx.EndStopRight_Ack)
        {
            MSG_SET_STEERING_ENDSTOP *d = (MSG_SET_STEERING_ENDSTOP *)msg->data;
            msg->ID = 0xe9;
            
            msg->DLC = 3;
            msg->Ext = 1;
            d->C_NUM_REV = OD.SteeringDataRx.NumSteeringRev;
            d->C_ABS_ANGLE_HIGH = 0x0f;
            d->C_ABS_ANGLE_LOW = 0xff;
        }
        else if (!OD.SteeringDataRx.EndStopLeft_Ack)
        {
            MSG_SET_STEERING_ENDSTOP *d = (MSG_SET_STEERING_ENDSTOP *)msg->data;
            msg->ID = 0xea;
            
            msg->DLC = 3;
            msg->Ext = 1;
            d->C_NUM_REV = OD.SteeringDataRx.NumSteeringRev;
            d->C_ABS_ANGLE_HIGH = 0x00;
            d->C_ABS_ANGLE_LOW = 0x00;
        }       	
		else if(!OD.SteeringDataRx.SetZeroRegBrake_Ack)
		{
			MSG_SET_ZERO_REGULATED_BRAKE_FRICTION *d = (MSG_SET_ZERO_REGULATED_BRAKE_FRICTION *)msg->data;
			msg->ID = 0xe5;
			msg->DLC = 1;
			msg->Ext = 1;
			d->C_PERC_MAX_BRAKE_CURRENT = 50;
		}
 
        else
        {
            return 1;
        }
    }
    
    return 0;
    
}
