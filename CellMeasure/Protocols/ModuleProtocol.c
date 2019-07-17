#include "Main.h"
#include "CanFunc.h"
#include "Protocol.h"
#include "TimerFunc.h"

static uint32_t modSendTime[Module_ECUx_CAN_ID_LEN];
// Периоды отправки сообщений
const uint16_t msgPeriod[Module_ECUx_CAN_ID_LEN] = {
								250,
								250
								 };

uint8_t ModuleRx(CanMsg *msg)
{	
	if((msg->ID < Module_ECU_CAN_ID) || (msg->ID >= Module_ECU_CAN_ID + Module_ECU_CAN_ID))
		return 1;	
	
	EcuConfig_t ecuConfig = GetConfigInstance();
	
	// Проверяем индекс сообщения. Если ID нечетный, значит пришло второе сообщение из группы сообщений Battery
	uint16_t msg_id = (msg->ID & 0x01)? msg->ID - 1 : msg->ID;	
	uint8_t module_id = ((msg_id - Module_ECU_CAN_ID) / Module_ECUx_CAN_ID_LEN) - (MAX_MODULE_NUM * ecuConfig.BatteryIndex);
	uint8_t msg_num = msg->ID & 0x01;

	// Модуль не принадлежит ветке своей батареи
	if(module_id >= MAX_MODULE_NUM)
		return 1;
	
	switch(msg_num)
	{
		case 0:
		{
			BatModuleStatus1Msg_t *d = (BatModuleStatus1Msg_t*)msg->data;			
			
			OD.ModuleData[module_id].TotalVoltage = d->Mod_Voltage_0p1V;
			OD.ModuleData[module_id].MaxCellVoltage.Voltage_mv = d->MaxCellVoltage;
			OD.ModuleData[module_id].MinCellVoltage.Voltage_mv = d->MinCellVoltage;
			OD.ModuleData[module_id].MaxModuleTemperature.Temperature = d->MaxTemp - 40;
			OD.ModuleData[module_id].MinModuleTemperature.Temperature = d->MinTemp - 40;
		}
		break;
		
		case 1:
		{
			BatModuleStatus2Msg_t *d = (BatModuleStatus2Msg_t*)msg->data;
			
			OD.ModuleData[module_id].StateMachine.MainState = (WorkStates_e)d->Mod_MainState;
			OD.ModuleData[module_id].StateMachine.SubState = d->Mod_SubState;
			OD.ModuleData[module_id].Faults = d->Faults;
		}
			break;
		
		default:
			return 1;			
	}		
	
	OD.ModuleData[module_id].TimeOutCnts= GetTimeStamp();  
	return 0;	
}


void ModuleMesGenerate(void)
{
    // Номер отправляемого сообщения
	static uint8_t SendMesNumber = 0;
    
    CanMsg *msg;
    
    // Генерация сообщений
	if (++SendMesNumber >= Module_ECUx_CAN_ID_LEN)
		SendMesNumber = 0;

	if (GetTimeFrom(modSendTime[SendMesNumber]) > msgPeriod[SendMesNumber])
	{		
         msg = ecanGetEmptyTxMsg(P_CAN_CH);
        
        if(msg == 0)
            return;
        
		EcuConfig_t ecuConfig = GetConfigInstance();
		
        modSendTime[SendMesNumber] = GetTimeStamp();
		
		msg->ID = Module_ECU_CAN_ID + (Module_ECUx_CAN_ID_LEN * (ecuConfig.ModuleIndex + (MAX_MODULE_NUM * ecuConfig.BatteryIndex))) + SendMesNumber;	
		msg->DLC = 8;
		msg->Ext = 0;

		switch (SendMesNumber)
		{
            case 0:
            {               
				BatModuleStatus1Msg_t *d = (BatModuleStatus1Msg_t*)msg->data;
				
				d->MaxCellVoltage = OD.ModuleData[ecuConfig.ModuleIndex].MaxCellVoltage.Voltage_mv;
				d->MinCellVoltage = OD.ModuleData[ecuConfig.ModuleIndex].MinCellVoltage.Voltage_mv;				
				d->Mod_Voltage_0p1V = OD.ModuleData[ecuConfig.ModuleIndex].TotalVoltage;
				d->MaxTemp = OD.ModuleData[ecuConfig.ModuleIndex].MaxModuleTemperature.Temperature + 40;
				d->MinTemp = OD.ModuleData[ecuConfig.ModuleIndex].MinModuleTemperature.Temperature + 40;				
            }
                break;
			
			case 1:
            {               
				BatModuleStatus2Msg_t *d = (BatModuleStatus2Msg_t*)msg->data;
				
				d->Mod_MainState = (WorkStates_e)OD.StateMachine.MainState;
				d->Mod_SubState = OD.StateMachine.SubState;
				d->Faults = OD.Faults.Flags;
				
            }
                break;
        }

    }
    
    return;
}

