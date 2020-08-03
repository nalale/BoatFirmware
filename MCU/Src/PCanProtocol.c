#include <string.h>

#include "Main.h"
#include "TimerFunc.h"

#include "../Libs/Btn8982.h"
#include "../BoardDefinitions/MarineEcu_Board.h"

#include "../MolniaLib/MF_CAN_1v1.h"
#include "../MolniaLib/DateTime.h"

#include "PCanProtocol.h"

#define EXTERNAL_SEND_MSG_AMOUNT	6

static uint32_t extSendTime[EXTERNAL_SEND_MSG_AMOUNT];
// Периоды отправки сообщений
static uint16_t extPeriod[EXTERNAL_SEND_MSG_AMOUNT] = {
                                250,		// cmGen1
								100,
								50,			// sim100
								250,		// Status 1	
								100,		// cmGen2
								250,
								 };

uint8_t PCanRx(CanMsg *msg)
{
	EcuConfig_t config = GetConfigInstance();	
	
	if (msg->ID == BASE_CAN_ID)		// Date Time
	{
		if(!dateTime_IsInit())
		{
			dateTime_InitCurrent(((cmTimeServer*)msg->data)->DateTime);
		}
	}
	else if(msg->ID == PM_CAN_ID)
	{
		OD.PowerManagerCmd = (PowerStates_e)msg->data[0];
	}
	else if(msg->ID == General_ECU_CAN_ID)
    {
		smEcuStatus1* d = (smEcuStatus1*)msg->data;
		
		OD.LogicTimers.AcceleratorTimer_ms = GetTimeStamp();
		
        OD.AccPedalChannels[0] = d->AnalogInput[0];
		OD.AccPedalChannels[1] = d->AnalogInput[1];

		OD.SB.stChargingTerminal = GET_IN_STATE(d->LogicInputsOutputs, D_IN_KL15_CHARGE);//(d->LogicInputsOutputs >> 1) & 0x01;
		OD.SB.stManualDrainSwitch = GET_IN_STATE(d->LogicInputsOutputs, D_IN_MANUAL_DRAIN);//(d->LogicInputsOutputs >> 2) & 0x01;
		OD.SB.stWaterSwitch1 = GET_IN_STATE(d->LogicInputsOutputs, D_IN_WATER_SWITCH_1);//(d->LogicInputsOutputs >> 3) & 0x01;
		return 0;	
    }
    else if(msg->ID == General_ECU_CAN_ID + 1)
    {
    	smEcuStatus2* d = (smEcuStatus2*)msg->data;

    	gECU_Fauls_t f;
		f.Faults = d->Faults;
    	OD.SB.Ecu1MeasDataActual = !f.MeasuringCircuit;
    }
	else if(msg->ID == General_ECU_CAN_ID + (1 * General_ECUx_CAN_ID_LEN))
    {
		smEcuStatus1* d = (smEcuStatus1*)msg->data;

		OD.SB.cmdTrimUp = GET_IN_STATE(d->LogicInputsOutputs, D_IN_TRIM_JOY_1);//d->LogicInputsOutputs & 3;
		OD.SB.cmdTrimDown = GET_IN_STATE(d->LogicInputsOutputs, D_IN_TRIM_JOY_2);
		
		return 0;	
    }
	else if(msg->ID == General_ECU_CAN_ID + (1 * General_ECUx_CAN_ID_LEN) + 1)
	{
		smEcuStatus2* d = (smEcuStatus2*)msg->data;

		gECU_Fauls_t f;
		f.Faults = d->Faults;
		OD.SB.Ecu2MeasDataActual = !f.MeasuringCircuit;
	}
	else if(msg->ID == General_ECU_CAN_ID + (3 * General_ECUx_CAN_ID_LEN))
	{
		smEcuStatus1* d = (smEcuStatus1*)msg->data;

		OD.SB.stWaterSwitch2 = GET_IN_STATE(d->LogicInputsOutputs, D_IN_WATER_SWITCH_2);//(d->LogicInputsOutputs >> 0) & 0x01;
	}
	else if(msg->ID == General_ECU_CAN_ID + (3 * General_ECUx_CAN_ID_LEN) + 1)
	{
		smEcuStatus2* d = (smEcuStatus2*)msg->data;

		gECU_Fauls_t f;
		f.Faults = d->Faults;
		OD.SB.Ecu4MeasDataActual = !f.MeasuringCircuit;
	}
	else if(msg->ID == Display_ECU_CAN_ID)
	{
		smEcuStatus1* d = (smEcuStatus1*)msg->data;

		OD.SB.cmdLightNavi = GET_IN_STATE(d->LogicInputsOutputs, D_IN_D_ECU_LIGHT_SWITCH_1);
		OD.SB.cmdLightAll = GET_IN_STATE(d->LogicInputsOutputs, D_IN_D_ECU_LIGHT_SWITCH_2);
		OD.SB.cmdLightCockPit = GET_IN_STATE(d->LogicInputsOutputs, D_IN_D_ECU_LIGHT_SWITCH_3);
		OD.SB.BoostModeRequest = GET_IN_STATE(d->LogicInputsOutputs, D_IN_D_ECU_BOOST_SWITCH);

		OD.MaxChargingCurrentRequest = msg->data[4];
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
	else if(msg->ID == Bmu_ECU_CAN_ID + 2)
	{
		BatM_Ext3_t *d = (BatM_Ext3_t*)msg->data;
		
		OD.BatteryDataRx.CCL = d->SystemCCL - 32767;
		OD.BatteryDataRx.DCL = d->SystemDCL - 32767;
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
    
    // Генерация сообщений
	if (++extSendMesNumber >= EXTERNAL_SEND_MSG_AMOUNT)
		extSendMesNumber = 0;

	if (GetTimeFrom(extSendTime[extSendMesNumber]) >= extPeriod[extSendMesNumber])
	{
         CanMsg *msg = ecanGetEmptyTxMsg(P_CAN_CH);
        
        if(msg == 0)
            return;		
        
        extSendTime[extSendMesNumber] = GetTimeStamp();
		msg->Ext = 0;

		switch (extSendMesNumber)
		{
			case 0:
			{
				msg->ID = General_Control1_CAN_ID;
				msg->DLC = 8;
				msg->Ext = 0;
				
				cmGeneralControl1 *d = (cmGeneralControl1*)msg->data;
				
				d->LogicOutput = SET_OUT_STATE(OD.SB.cmdHeatsinkPump, D_OUT_CPUMP_0) |
								SET_OUT_STATE(OD.SB.cmdMotorPumpCooling, D_OUT_CPUMP_1) |
								SET_OUT_STATE(OD.SB.cmdDrainPumpOn, D_OUT_DRAIN_1) |
								SET_OUT_STATE(OD.SB.cmdLightAll, D_OUT_LAMP_1) |
								SET_OUT_STATE(OD.SB.cmdLightAll, D_OUT_LAMP_2) |
								SET_OUT_STATE(OD.SB.cmdLightAll | OD.SB.cmdLightNavi, D_OUT_LAMP_3) |
								SET_OUT_STATE(OD.SB.cmdLightCockPit, D_OUT_LAMP_4) |
								SET_OUT_STATE(OD.SB.cmdLightCockPit, D_OUT_LAMP_5);
			}
			break;
			
			case 1:
			{
				msg->ID = Bmu_ECU_RX_ID;
				msg->DLC = 8;
				msg->Ext = 0;
				
				BatM_ExtRx_t *d = (BatM_ExtRx_t*)msg->data;
				
				d->OpEnabled = OD.BatteryReqState;
				
				
			}
			break;
			
			case 2:
			{
				static uint8_t sim_100_msg_num = 0;
				
				sim_100_msg_num = (++sim_100_msg_num > 4)? 0 : sim_100_msg_num;

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
			}
			break;
			
			case 3:
			{
				uint16_t current_val = (OD.BatteryDataRx.TotalCurrent < 0)? -OD.BatteryDataRx.TotalCurrent >> 4
						: OD.BatteryDataRx.TotalCurrent >> 4;

				msg->ID = Main_ECU_CAN_ID;
				msg->DLC = 8;
				msg->Ext = 0;
				
				MainEcuStatus1_Msg_t* d = (MainEcuStatus1_Msg_t*)msg->data;
				
				d->MotorRpm = McuRinegartGetParameter(&OD.mcHandler, mcu_ActualSpeed);
				d->MotorTemperature = McuRinegartGetParameter(&OD.mcHandler, mcu_MotorTemperature) + 40;
				d->InverterTemperature = McuRinegartGetParameter(&OD.mcHandler, mcu_BoardTemperature) + 40;
				d->TargetTorque = McuRinegartGetParameter(&OD.mcHandler, mcu_TargetTorque);
				d->ActualTorque = McuRinegartGetParameter(&OD.mcHandler, mcu_ActualTorque);
				d->TrimPosition = TrimGetParameter(&OD.TrimDataRx, paramTrim_Position);
				d->SpecPowerCons = (current_val > 255)? 255 : current_val;
				
			}
			break;

			case 4:
			{
				msg->ID = General_Control2_CAN_ID;
				msg->DLC = 8;
				msg->Ext = 0;

				memset(msg->data, 0, msg->DLC);

				cmGeneralControl2 *d = (cmGeneralControl2*)msg->data;

				d->AnalogOutput[0] = (OD.SB.cmdETForward)? 255 : 0;
				d->AnalogOutput[1] = (OD.SB.cmdETBackward)? 255 : 0;
			}
			break;

			case 5:
			{
				msg->ID = Main_ECU_CAN_ID + 1;
				msg->DLC = 8;
				msg->Ext = 0;

				MainEcuStatus2_Msg_t* d = (MainEcuStatus2_Msg_t*)msg->data;

				d->HelmAngle = HelmGetAnglePercent();
				d->SteeringAngle = SteeringGetFeedBackPercent(&OD.SteeringData);
				d->ThrottlePos = (thrHandleGetDirection() == dr_FwDirect)? thrHandleGetDemandAcceleration() : -thrHandleGetDemandAcceleration();
			}
        }
    }
    
    return;
}
