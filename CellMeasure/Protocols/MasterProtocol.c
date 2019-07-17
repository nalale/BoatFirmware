#include "Main.h"
#include "../MolniaLib/MF_Can_1v1.h"
#include "protocol.h"
#include "CanFunc.h"
#include "TimerFunc.h"
#include "User.h"

#define MASTER_SEND_MSG_AMOUNT	2

static uint32_t mstSendTime[MASTER_SEND_MSG_AMOUNT];
// Периоды отправки сообщений
const uint16_t mstPeriod[MASTER_SEND_MSG_AMOUNT] = {
								250,
								100,
								 };


uint8_t MasterRx(CanMsg *msg)
{
	// Сообщение от мастера
	if (msg->ID == Bmu_ECU_CAN_ID + 1)
	{		
		BatM_Ext2_t *d = (BatM_Ext2_t*)msg->data;
				
		OD.MasterControl.RequestState = (WorkStates_e)d->RequestState;
		OD.MasterControl.BalancingEnabled = d->BalancingEnabled;
		
		OD.MasterControl.CCL = d->CCL;
		OD.MasterControl.DCL = d->DCL;
		OD.MasterControl.TargetVoltage_mV = d->TargetVoltage_mV;
		
		OD.SB.MsgFromSystem = 1;
		
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
	
	return 1;
}
								 
								 
void MasterMesGenerate(void)
{
    // Номер отправляемого сообщения
	static uint8_t extSendMesNumber = 0;
    
    CanMsg *msg;
    
    // Генерация сообщений
	if (++extSendMesNumber >= MASTER_SEND_MSG_AMOUNT)
		extSendMesNumber = 0;

	if (GetTimeFrom(mstSendTime[extSendMesNumber]) > mstPeriod[extSendMesNumber])
	{
         msg = ecanGetEmptyTxMsg(P_CAN_CH);
        
        if(msg == 0)
            return;
        
        mstSendTime[extSendMesNumber] = GetTimeStamp();
		
		msg->ID = Bmu_ECU_CAN_ID + extSendMesNumber;
		msg->DLC = 8;
		msg->Ext = 0;

		switch (extSendMesNumber)
		{            
            case 0:
            {           
                BatM_Ext1_t *d = (BatM_Ext1_t*)msg->data;
            
				d->MainState = OD.StateMachine.MainState;
				d->SubState = OD.StateMachine.SubState;
				d->SOC = OD.MasterData.SoC / 10;
                d->TotalCurrent_0p1A = OD.MasterData.TotalCurrent;
                d->TotalVoltage_0p1V = OD.MasterData.TotalVoltage;
				d->MaxTemperature = OD.MasterData.MaxModuleTemperature.Temperature + 40;
				d->MinTemperature = OD.MasterData.MinModuleTemperature.Temperature + 40;
                
            }
            break;
			
			case 1:
			{
				BatM_Ext2_t *d = (BatM_Ext2_t*)msg->data;
            
				d->RequestState = OD.MasterControl.RequestState;
				d->BalancingEnabled = OD.MasterControl.BalancingEnabled;	
				d->TargetVoltage_mV = OD.MasterControl.TargetVoltage_mV;
                d->DCL = OD.MasterControl.DCL;
                d->CCL = OD.MasterControl.CCL;							
			}
			break;
        }

    }
    
    return;
}
