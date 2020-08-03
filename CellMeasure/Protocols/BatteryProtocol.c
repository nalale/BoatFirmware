#include <string.h>

#include "Main.h"
#include "protocol.h"
#include "CanFunc.h"
#include "TimerFunc.h"
#include "BatteryMeasuring.h"

static const uint8_t TxMsg_NUM = 4;

static uint32_t extSendTime[TxMsg_NUM];
// Периоды отправки сообщений
const uint16_t slvPeriod[TxMsg_NUM] = {
								100,
								250,
								500,
								500
								 };

uint8_t packMsgHandler(CanMsg *msg)
{  
	const EcuConfig_t *ecuConfig = OD.ConfigData;
	
	if((msg->ID >= 0x700 + ecuConfig->DiagnosticID) && (msg->ID < 0x700 + (ecuConfig->DiagnosticID + (ecuConfig->Sys_ModulesCountP * MaxModuleNum)) + 1))//MaxBatteryNum * MaxModuleNum))))
	{
		uint8_t _mod_index = ((msg->ID & 0xef) - ecuConfig->DiagnosticID) % MaxModuleNum;
		if(_mod_index == 0)
		{
			uint8_t _bat_index = ((msg->ID & 0x01f) - ecuConfig->DiagnosticID) / MaxModuleNum;
			OD.PackData[_bat_index].MainState = (WorkStates_e)((msg->data[0] >> 4) & 0x0F);
			OD.PackData[_bat_index].SubState = msg->data[0] & 0x0F;
		}
	}

	if((msg->ID < Bat_ECU_CAN_ID) || (msg->ID >= Bat_ECU_CAN_ID + Bat_ECU_CAN_ID_LEN))																
		return 1;	

	uint16_t msg_id = msg->ID;
	uint8_t bat_id = (msg_id - Bat_ECU_CAN_ID) / Bat_ECUx_CAN_ID_LEN;	
	
	if((bat_id >= MaxBatteryNum))
		return 1;
	
	uint8_t mes_num = msg->ID - Bat_ECU_CAN_ID - (Bat_ECUx_CAN_ID_LEN * bat_id);	
	
	switch(mes_num)
	{
		case 0:
		{
			BatBatteryStatus1Msg_t *d = (BatBatteryStatus1Msg_t*)msg->data;
			
			OD.PackData[bat_id].TotalCurrent = d->BatTotalCurrent_0p1A;
			OD.PackData[bat_id].TotalVoltage = d->BatTotalVoltage_0p1A;
			OD.PackData[bat_id].MaxBatteryVoltage.Voltage = d->BatMaxModVoltage_0p1V;
			OD.PackData[bat_id].MinBatteryVoltage.Voltage = d->BatMinModVoltage_0p1V;

		}
		break;
		
		case 1:
		{
			BatBatteryStatus2Msg_t *d = (BatBatteryStatus2Msg_t*)msg->data;
			
			OD.PackData[bat_id].MaxCellVoltage.Voltage_mv = d->MaxCellVoltage_0p1V;
			OD.PackData[bat_id].MinCellVoltage.Voltage_mv = d->MinCellVoltage_0p1V;
			OD.PackData[bat_id].MaxModuleTemperature.Temperature = d->MaxModTemperature - 40;
			OD.PackData[bat_id].MinModuleTemperature.Temperature = d->MinModTemperature - 40;			
			OD.PackData[bat_id].SoC = d->Soc;
		}
		break;
		
		case 2:
		{
			cmPack_Tx1 *d = (cmPack_Tx1*)msg->data;

			if(bat_id == ecuConfig->BatteryIndex && OD.PackControl.ModulesInAssembly == 0)
			{
				OD.PackControl.BalancingEnabled = d->BalancingEnabled;
				OD.PackControl.TargetVoltage_mV = d->TargetBalancingVoltage;
			}
		}
		break;
		
		case 3:
		{
			smPack_Tx2 *d = (smPack_Tx2*)msg->data;
			
			OD.PackData[bat_id].ActualEnergy_As = d->ActualEnergy_As;
			OD.PackData[bat_id].TotalEnergy_As = d->TotalEnergy_As;
		}
		break;

		default:
			return 1;
	}				
	
	OD.PackData[bat_id].TimeOutCnts = GetTimeStamp();   
	

    return 0;
}



void SlaveMesGenerate(void)
{
    // Номер отправляемого сообщения
	static uint8_t extSendMesNumber = 0;
    
    CanMsg *msg;
    
    // Генерация сообщений
	if (++extSendMesNumber >= TxMsg_NUM)
		extSendMesNumber = 0;


	if (GetTimeFrom(extSendTime[extSendMesNumber]) > slvPeriod[extSendMesNumber])
	{
         msg = ecanGetEmptyTxMsg(P_CAN_CH);
        
        if(msg == 0)
            return;
        
		const EcuConfig_t *ecuConfig = OD.ConfigData;
        extSendTime[extSendMesNumber] = GetTimeStamp();
		
		msg->ID = Bat_ECU_CAN_ID + (Bat_ECUx_CAN_ID_LEN * ecuConfig->BatteryIndex) + extSendMesNumber;
		msg->DLC = 8;
		msg->Ext = 0;
		
		memset(msg->data, 0xff, 8);

		switch (extSendMesNumber)
		{
            case 0:
            {               
				BatBatteryStatus1Msg_t *d = (BatBatteryStatus1Msg_t*)msg->data;
			
				d->BatTotalCurrent_0p1A = OD.PackData[ecuConfig->BatteryIndex].TotalCurrent;
				d->BatTotalVoltage_0p1A = OD.PackData[ecuConfig->BatteryIndex].TotalVoltage;
				d->BatMaxModVoltage_0p1V = OD.PackData[ecuConfig->BatteryIndex].MaxBatteryVoltage.Voltage;
				d->BatMinModVoltage_0p1V = OD.PackData[ecuConfig->BatteryIndex].MinBatteryVoltage.Voltage;
            }
            break;
			
			case 1:
			{
				BatBatteryStatus2Msg_t *d = (BatBatteryStatus2Msg_t*)msg->data;
				
				d->Faults = (OD.Faults.Flags >> 12) & 0x00FF;
				d->MaxCellVoltage_0p1V = OD.PackData[ecuConfig->BatteryIndex].MaxCellVoltage.Voltage_mv;
				d->MinCellVoltage_0p1V = OD.PackData[ecuConfig->BatteryIndex].MinCellVoltage.Voltage_mv;
				d->MaxModTemperature = OD.PackData[ecuConfig->BatteryIndex].MaxModuleTemperature.Temperature + 40;
				d->MinModTemperature = OD.PackData[ecuConfig->BatteryIndex].MinModuleTemperature.Temperature + 40;
				d->Soc = OD.PackData[ecuConfig->BatteryIndex].SoC;
			}
			break;

			case 2:
			{
				cmPack_Tx1 *d = (cmPack_Tx1*)msg->data;

				d->BalancingEnabled = OD.PackControl.BalancingEnabled;
				d->TargetBalancingVoltage = OD.PackControl.TargetVoltage_mV;
			}
			break;
			
			case 3:
			{
				smPack_Tx2 *d = (smPack_Tx2*)msg->data;
				
				d->ActualEnergy_As = OD.PackData[ecuConfig->BatteryIndex].ActualEnergy_As;
				d->TotalEnergy_As = OD.PackData[ecuConfig->BatteryIndex].TotalEnergy_As;
			}
			break;
        }

    }
    
    return;
}

