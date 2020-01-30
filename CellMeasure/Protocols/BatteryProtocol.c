#include <string.h>

#include "Main.h"
#include "protocol.h"
#include "CanFunc.h"
#include "TimerFunc.h"

static uint32_t extSendTime[Bat_ECUx_CAN_ID_LEN];
// Периоды отправки сообщений
const uint16_t slvPeriod[Bat_ECUx_CAN_ID_LEN] = {
								100,
								250,
								UINT16_MAX,
								UINT16_MAX
								 };

uint8_t SlaveRx(CanMsg *msg)
{  
	const EcuConfig_t *ecuConfig = OD.ConfigData;
	
	if(msg->ID > 0x700 && msg->ID < 0x71f)
	{		
		uint8_t _mod_index = ((msg->ID & 0x01f) - ecuConfig->DiagnosticID) % MAX_MODULE_NUM;
		if(_mod_index == 0)
		{
			uint8_t _bat_index = ((msg->ID & 0x01f) - ecuConfig->DiagnosticID) / MAX_MODULE_NUM;
			OD.BatteryData[_bat_index].StateMachine.MainState = (WorkStates_e)((msg->data[0] >> 4) & 0x0F);
			OD.BatteryData[_bat_index].StateMachine.SubState = msg->data[0] & 0x0F;
		}
	}
	
	if((msg->ID < Bat_ECU_CAN_ID) || (msg->ID >= Bat_ECU_CAN_ID + Bat_ECU_CAN_ID_LEN))																
		return 1;	

	uint16_t msg_id = (msg->ID & 0x01)? msg->ID - 1 : msg->ID;
	uint8_t bat_id = (msg_id - Bat_ECU_CAN_ID) / Bat_ECUx_CAN_ID_LEN;
	
	
	
	if((bat_id == ecuConfig->BatteryIndex) || (bat_id >= MAX_BATTERY_AMOUNT))
		return 1;

	uint8_t mes_num = msg->ID & 0x01;	
	
	switch(mes_num)
	{
		case 0:
		{
			BatBatteryStatus1Msg_t *d = (BatBatteryStatus1Msg_t*)msg->data;
			
			OD.BatteryData[bat_id].TotalCurrent = d->BatTotalCurrent_0p1A;
			OD.BatteryData[bat_id].TotalVoltage = d->BatTotalVoltage_0p1A;
			OD.BatteryData[bat_id].MaxBatteryVoltage.Voltage = d->BatMaxModVoltage_0p1V;
			OD.BatteryData[bat_id].MinBatteryVoltage.Voltage = d->BatMinModVoltage_0p1V;

		}
		break;
		
		case 1:
		{
			BatBatteryStatus2Msg_t *d = (BatBatteryStatus2Msg_t*)msg->data;
			
			OD.BatteryData[bat_id].MaxCellVoltage.Voltage_mv = d->MaxCellVoltage_0p1V;
			OD.BatteryData[bat_id].MinCellVoltage.Voltage_mv = d->MinCellVoltage_0p1V;
			OD.BatteryData[bat_id].MaxModuleTemperature.Temperature = d->MaxModTemperature - 40;
			OD.BatteryData[bat_id].MinModuleTemperature.Temperature = d->MinModTemperature - 40;			
			OD.BatteryData[bat_id].SoC = d->Soc;
		}
		break;
		
		default:
			return 1;
	}				
	
	OD.BatteryData[bat_id].TimeOutCnts = GetTimeStamp();   
	

    return 0;
}



void SlaveMesGenerate(void)
{
    // Номер отправляемого сообщения
	static uint8_t extSendMesNumber = 0;
    
    CanMsg *msg;
    
    // Генерация сообщений
	if (++extSendMesNumber >= Bat_ECUx_CAN_ID_LEN)
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
			
				d->BatTotalCurrent_0p1A = OD.BatteryData[ecuConfig->BatteryIndex].TotalCurrent;
				d->BatTotalVoltage_0p1A = OD.BatteryData[ecuConfig->BatteryIndex].TotalVoltage;
				d->BatMaxModVoltage_0p1V = OD.BatteryData[ecuConfig->BatteryIndex].MaxBatteryVoltage.Voltage;
				d->BatMinModVoltage_0p1V = OD.BatteryData[ecuConfig->BatteryIndex].MinBatteryVoltage.Voltage;
            }
            break;
			
			case 1:
			{
				BatBatteryStatus2Msg_t *d = (BatBatteryStatus2Msg_t*)msg->data;
				
				d->Faults = (OD.Faults.Flags >> 12) & 0x00FF;
				d->MaxCellVoltage_0p1V = OD.BatteryData[ecuConfig->BatteryIndex].MaxCellVoltage.Voltage_mv;
				d->MinCellVoltage_0p1V = OD.BatteryData[ecuConfig->BatteryIndex].MinCellVoltage.Voltage_mv;
				d->MaxModTemperature = OD.BatteryData[ecuConfig->BatteryIndex].MaxModuleTemperature.Temperature + 40;
				d->MinModTemperature = OD.BatteryData[ecuConfig->BatteryIndex].MinModuleTemperature.Temperature + 40;
				d->Soc = OD.BatteryData[ecuConfig->BatteryIndex].SoC;
			}
			break;
        }

    }
    
    return;
}

