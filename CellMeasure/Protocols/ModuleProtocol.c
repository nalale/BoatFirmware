#include "Main.h"
#include "CanFunc.h"
#include "Protocol.h"
#include "TimerFunc.h"

#define TX_MSG	4

static uint32_t modSendTime[TX_MSG];
// Периоды отправки сообщений
const uint16_t msgPeriod[TX_MSG] = {
								250,
								250,
								250,
								250,
								 };

uint8_t moduleMsgHandler(CanMsg *msg)
{	
	if((msg->ID < Module_ECU_CAN_ID) || (msg->ID >= Module_ECU_CAN_ID + Module_ECU_CAN_ID_LEN))
		return 1;	
	
	EcuConfig_t ecuConfig = GetConfigInstance();
	
	// Проверяем индекс сообщения. Если ID нечетный, значит пришло второе сообщение из группы сообщений Battery

	uint16_t msg_id = msg->ID;
	uint8_t module_id = ((msg_id - Module_ECU_CAN_ID) / Module_ECUx_CAN_ID_LEN) - (MaxModuleNum * ecuConfig.BatteryIndex);
	uint8_t msg_num = ((msg->ID - Module_ECU_CAN_ID) - (Module_ECUx_CAN_ID_LEN * module_id)) - (Module_ECUx_CAN_ID_LEN * ecuConfig.BatteryIndex * MaxModuleNum);
	
	if(module_id >= MaxModuleNum)
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
			
			OD.ModuleData[module_id].MainState = (WorkStates_e)d->Mod_MainState;
			OD.ModuleData[module_id].SubState = d->Mod_SubState;
			OD.ModuleData[module_id].Faults = d->Faults;
			OD.ModuleData[module_id].ActualEnergy_As = (d->ActualEnergy_0p01Ah == UINT16_MAX)?
					UINT32_MAX : (uint32_t)d->ActualEnergy_0p01Ah * 36;
		}
			break;

		case 2:
		{
			cmAssemblyMsg1_t *d = (cmAssemblyMsg1_t*)msg->data;

			if((OD.ConfigData->ModuleIndex > module_id) && (OD.ConfigData->ModuleIndex < module_id + d->ModulesInAssembly))
			{
				OD.PackControl.ModulesInAssembly = d->ModulesInAssembly;
				OD.PackControl.BalancingEnabled = d->BalancingEnabled;
				OD.PackControl.TargetVoltage_mV = d->TargetBalancingVoltage;
			}
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
    uint8_t max_msg_num = ((OD.ConfigData->Sys_ModulesCountS == OD.ConfigData->ModulesInAssembly) || 
							(OD.ConfigData->ModulesInAssembly <= 1))? TX_MSG - 1 : TX_MSG;
    // Генерация сообщений
	if (++SendMesNumber >= max_msg_num)
		SendMesNumber = 0;

	if (GetTimeFrom(modSendTime[SendMesNumber]) > msgPeriod[SendMesNumber])
	{		
         msg = ecanGetEmptyTxMsg(P_CAN_CH);
        
        if(msg == 0)
            return;
        
		EcuConfig_t ecuConfig = GetConfigInstance();
		
        modSendTime[SendMesNumber] = GetTimeStamp();
		
		msg->ID = Module_ECU_CAN_ID + (Module_ECUx_CAN_ID_LEN * (ecuConfig.ModuleIndex + (MaxModuleNum * ecuConfig.BatteryIndex))) + SendMesNumber;	
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
				d->StateOfCharge = OD.ModuleData[ecuConfig.ModuleIndex].SoC;
				d->dummy2 = OD.ModuleData[ecuConfig.ModuleIndex].TotalCurrent; 
				d->ActualEnergy_0p01Ah = (OD.ModuleData[OD.ConfigData->ModuleIndex].ActualEnergy_As == UINT32_MAX)?
						UINT16_MAX : (uint16_t)(OD.ModuleData[OD.ConfigData->ModuleIndex].ActualEnergy_As / (uint32_t)36);
            }
                break;

			case 2:
			{
				cmAssemblyMsg1_t *d = (cmAssemblyMsg1_t*)msg->data;

				d->ModulesInAssembly = OD.ConfigData->ModulesInAssembly;
				d->BalancingEnabled = OD.PackControl.BalancingEnabled;
				d->TargetBalancingVoltage = OD.PackControl.TargetVoltage_mV;
				d->CCL = -OD.PackData[OD.ConfigData->BatteryIndex].CCL;
				d->DCL = OD.PackData[OD.ConfigData->BatteryIndex].DCL;

			}
			break;

			case 3:
			{
				cmAssemblyMsg2_t *d = (cmAssemblyMsg2_t*)msg->data;

				d->TotalVoltage = OD.ModuleData[OD.ConfigData->ModuleIndex].TotalVoltage;
				d->TotalCurrent = OD.ModuleData[OD.ConfigData->ModuleIndex].TotalCurrent;

			}
			break;
        }

    }
    
    return;
}

