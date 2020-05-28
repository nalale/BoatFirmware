#include "Main.h"
#include "../MolniaLib/MF_Can_1v1.h"
#include "protocol.h"
#include "CanFunc.h"
#include "TimerFunc.h"
#include "User.h"
#include "BatteryMeasuring.h"

#define MASTER_SEND_MSG_AMOUNT	4

static uint32_t mstSendTime[MASTER_SEND_MSG_AMOUNT];
// Периоды отправки сообщений
const uint16_t mstPeriod[MASTER_SEND_MSG_AMOUNT] = {
								250,
								100,
								100,
								500,
								 };


uint8_t MasterMsgHandler(CanMsg *msg)
{
	// Сообщение от мастера	
	if (msg->ID == Bmu_ECU_CAN_ID + 1)
	{		
		BatM_Ext2_t *d = (BatM_Ext2_t*)msg->data;
				
		OD.MasterControl.RequestState = (WorkStates_e)d->RequestState;

		OD.MasterControl.CCL = d->CCL;
		OD.MasterControl.DCL = d->DCL;
		

		OD.SB.MsgFromSystem = 1;
		
		return 0;
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
				d->SOC = OD.MasterData.SoC;
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
                d->DCL = OD.MasterControl.DCL;
                d->CCL = OD.MasterControl.CCL;							
			}
			break;

			case 2:
			{
				BatM_Ext3_t *d = (BatM_Ext3_t*)msg->data;

				d->MaxCellVoltage_mV = OD.MasterData.MaxCellVoltage.Voltage_mv;
				d->MinCellVoltage_mV = OD.MasterData.MinCellVoltage.Voltage_mv;
				d->SystemCCL = (int32_t)OD.MasterData.CCL + 32767;
				d->SystemDCL = (int32_t)OD.MasterData.DCL + 32767;
			}
			break;

			case 3:
			{
				BatM_Ext4_t *d = (BatM_Ext4_t*)msg->data;
				
				d->SystemActualEnergy_Ah = OD.MasterData.ActualEnergy_As / 3600;
				d->SystemTotalEnergy_Ah = OD.MasterData.TotalEnergy_As / 3600;
				
			}
        }

    }
    
    return;
}
