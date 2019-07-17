#include "Main.h"
#include "PCanProtocol.h"
#include "SkfTypes.h"
#include "TimerFunc.h"
#include "../BoardDefinitions/MarineEcu_Board.h"

#include "../MolniaLib/MF_CAN_1v1.h"
#include <string.h>

#include "../Libs/Btn8982.h"
#include "Sim100.h"

#define EXTERNAL_SEND_MSG_AMOUNT	5

static uint32_t extSendTime[EXTERNAL_SEND_MSG_AMOUNT];
// Периоды отправки сообщений
static uint16_t extPeriod[EXTERNAL_SEND_MSG_AMOUNT] = {
								250,	
                                250,
								100,
								50,			// sim100
								250,		// Status 1	
								 };

uint8_t PCanRx(CanMsg *msg)
{
	EcuConfig_t config = GetConfigInstance();	
	
    if(msg->ID == General_ECU_CAN_ID)
    {
		smEcuStatus1* d = (smEcuStatus1*)msg->data;
		
		OD.LogicTimers.AcceleratorTimer_ms = GetTimeStamp();
		
        OD.AccPedalChannels[0] = d->AnalogInput[0];
		OD.AccPedalChannels[1] = d->AnalogInput[1];
		OD.SB.ChargingTerminalConnected = (d->LogicInputsOutputs >> 1) & 0x01;
		OD.SB.ManualDrainSwitch = (d->LogicInputsOutputs >> 2) & 0x01;
		OD.SB.WaterSwitch1 = !(d->LogicInputsOutputs >> 3) & 0x01; 
		return 0;	
    }
	else if(msg->ID == General_ECU_CAN_ID + 1)
    {
		smEcuStatus1* d = (smEcuStatus1*)msg->data;
		OD.TrimDataRx.MovCmd = d->LogicInputsOutputs & 3;	
		
		return 0;	
    }
	else if(msg->ID == General_ECU_CAN_ID + 3)
	{
		smEcuStatus1* d = (smEcuStatus1*)msg->data;
		OD.SB.WaterSwitch2 = !(d->LogicInputsOutputs >> 0) & 0x01;
	}
	else if(msg->ID == Bmu_ECU_CAN_ID)
	{
		BatM_Ext1_t *d = (BatM_Ext1_t*)msg->data;
		
		OD.BatteryDataRx.MainState = d->MainState;
		OD.BatteryDataRx.SubState = d->SubState;
		OD.BatteryDataRx.SoC = d->SOC;
		OD.BatteryDataRx.TotalCurrent = d->TotalCurrent_0p1A;
		OD.BatteryDataRx.TotalVoltage = d->TotalVoltage_0p1V;
		
		OD.LogicTimers.BatteryTimer_ms = GetTimeStamp();
	}
	else if(msg->ID == Bmu_ECU_CAN_ID + 1)
	{
		BatM_Ext2_t *d = (BatM_Ext2_t*)msg->data;
		
		OD.BatteryDataRx.CCL = d->CCL;
		OD.BatteryDataRx.DCL = d->DCL;
	}
	if (msg->ID == 0xA100100)
	{
		switch(msg->data[0])
		{
			case opCode_ReadIsolation:
				OD.LogicTimers.Sim100OfflineTime_ms = GetTimeStamp();
				
				OD.Sim100DataRx.Status.Value = msg->data[1];
				OD.Sim100DataRx.Isolation = (msg->data[2] << 8) + msg->data[3];
				OD.Sim100DataRx.IsolationUncertainity = msg->data[4];
				OD.Sim100DataRx.StoredEnergy = (msg->data[5] << 8) + msg->data[6];
				OD.Sim100DataRx.CapUncertainity = msg->data[7];
				break;
			case opCode_ReadResistance:
				OD.Sim100DataRx.Status.Value = msg->data[1];
				OD.Sim100DataRx.Rp = (msg->data[2] << 8) + msg->data[3];
				OD.Sim100DataRx.RpUncertainity = msg->data[4];
				OD.Sim100DataRx.Rn = (msg->data[5] << 8) + msg->data[6];
				OD.Sim100DataRx.RnUncertainity = msg->data[7];
				break;
			case opCode_ReadVoltage:
				OD.Sim100DataRx.Status.Value = msg->data[1];
				OD.Sim100DataRx.Vp = (msg->data[2] << 8) + msg->data[3];
				OD.Sim100DataRx.VpUncertainity = msg->data[4];
				OD.Sim100DataRx.Vn = (msg->data[5] << 8) + msg->data[6];
				OD.Sim100DataRx.VnUncertainity = msg->data[7];
				break;
			case opCode_ReadBatteryVoltage:
				OD.Sim100DataRx.Status.Value = msg->data[1];
				OD.Sim100DataRx.Vb = (msg->data[2] << 8) + msg->data[3];
				OD.Sim100DataRx.VbUncertainity = msg->data[4];
				OD.Sim100DataRx.Vmax = (msg->data[5] << 8) + msg->data[6];
				OD.Sim100DataRx.VmaxUncertainity = msg->data[7];
				break;
			case opCode_ReadError:
				OD.Sim100DataRx.Status.Value = msg->data[1];
				OD.Sim100DataRx.Errors.Value = msg->data[2];
				break;
		}		
	}
	
	
    return 1;
}



void PCanMesGenerate(void)
{
    // Номер отправляемого сообщения
	static uint8_t extSendMesNumber = 0;
	
	EcuConfig_t config = GetConfigInstance();
	
    CanMsg *msg;
    
    // Генерация сообщений
	if (++extSendMesNumber >= EXTERNAL_SEND_MSG_AMOUNT)
		extSendMesNumber = 0;

	if (GetTimeFrom(extSendTime[extSendMesNumber]) >= extPeriod[extSendMesNumber])
	{
         msg = ecanGetEmptyTxMsg(P_CAN_CH);
        
        if(msg == 0)
            return;		
        
        extSendTime[extSendMesNumber] = GetTimeStamp();
		msg->Ext = 0;

		switch (extSendMesNumber)
		{
			case 0:
			{
				uint16_t loc_CCL = OD.TargetChargingCurrent_A;
				uint16_t loc_TargetVoltage = 4000;

				loc_CCL = (loc_CCL > 12)? 12 : loc_CCL;
				loc_CCL *= 10;			

				msg->ID = 0x1806E5F4;
				msg->DLC = 8;
				msg->Ext = 1;

				msg->data[0] = (uint8_t) (loc_TargetVoltage >> 8);
				msg->data[1] = (uint8_t) loc_TargetVoltage;
				msg->data[2] = (uint8_t) (loc_CCL >> 8);
				msg->data[3] = (uint8_t) loc_CCL;
				msg->data[4] = 0;
				msg->data[5] = 0;
				msg->data[6] = 0;
				msg->data[7] = 0;
			}
			break;
			
			case 1:
			{
				msg->ID = General_Control1_CAN_ID;
				msg->DLC = 8;
				msg->Ext = 0;
				
				cmGeneralControl1 *d = (cmGeneralControl1*)msg->data;
				
				d->LogicOutput = (OD.SB.InvPumpCooling) | (OD.SB.MotorPumpCooling << 1) | (OD.SB.HeatsinkPump << 2) | (OD.SB.DrainOn << 3);
				
			}
			break;
			
			case 2:
			{
				msg->ID = Bmu_ECU_RX_ID;
				msg->DLC = 8;
				msg->Ext = 0;
				
				BatM_ExtRx_t *d = (BatM_ExtRx_t*)msg->data;
				
				d->OpEnabled = OD.BatteryReqState;
				
				
			}
			break;
			
			case 3:
			{
				static uint8_t sim_100_msg_num = 0;
				
				msg->ID = 0xA100101;
				msg->DLC = 1;
				msg->Ext = 1;
				
				switch(sim_100_msg_num)
				{
					case 0:
						msg->data[0] = opCode_ReadIsolation;
						break;
					case 1:
						msg->data[0] = opCode_ReadResistance;
						break;
					case 2:
						msg->data[0] = opCode_ReadVoltage;
						break;
					case 3:
						msg->data[0] = opCode_ReadBatteryVoltage;
						break;
					case 4:
						msg->data[0] = opCode_ReadError;
						break;
				}
				
				sim_100_msg_num = (sim_100_msg_num >= 5)? 0 : sim_100_msg_num + 1;
			}
			break;
			
			case 4:
			{
				msg->ID = Main_ECU_CAN_ID;
				msg->DLC = 8;
				msg->Ext = 0;
				
				MainEcuStatus1_Msg_t* d = (MainEcuStatus1_Msg_t*)msg->data;
				
				d->MotorRpm = OD.MovControlDataRx.ActualSpeed;
				d->MotorTemperature = OD.InvertorDataRx.MotorTemperature + 40;
				d->InverterTemperature = OD.InvertorDataRx.InverterTemperature + 40;
				d->TargetTorque = OD.MovControlDataRx.AccPosition;
				d->ActualTorque = OD.MovControlDataRx.ActualTorque * 100 / config.MaxMotorTorque;
				d->SteeringAngle = (((uint32_t)OD.SteeringDataRx.TargetAngle << 7) - 1) >> 11;	// angle_value * 255 / 4095
				d->FeedbackAngle = (((uint32_t)OD.steeringFeedbackAngle << 7) - 1) >> 11;			// angle_value * 255 / 4095
				
			}
			break;
        }
    }
    
    return;
}
