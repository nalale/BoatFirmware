#include "Main.h"
#include "WorkStates.h"
#include "TimerFunc.h"
#include "User.h"
#include "Protocol.h"
#include "AdcFunc.h"

#include "BatteryMeasuring.h"

#include "FaultTools.h"
#include "BMS_Combi_ECU.h"
#include "EcuConfig.h"
#include "../MolniaLib/PowerManager.h"

#include "../MolniaLib/Config.h"
#include "../MolniaLib/DateTime.h"

#include "../Libs/CurrentSens.h"
#include "../Libs/LTC6803.h"

#include "MemoryFunc.h"
#include "UartFunc.h"


// Description:
// Intermidiate module: battery module is battery line intermidiate. It doesn't include current sensor and precharge circuit;
// Terminal module: battery module terminates batteries line, includes current sensor and collects line's statistic. Can make precharging;

uint32_t timeStamp;

// ������ ���������� �� �������-������
void (*FunArray[])(uint8_t *) =	{ InitializationState, PreoperationState, OperatingState, ShutdownState, FaultState, TestingState };

void LedBlink(void);

void SetWorkState(StateMachine_t *state_machine, WorkStates_e new_state)
{
    if(state_machine->MainState == WORKSTATE_SHUTDOWN 
		|| (state_machine->MainState == WORKSTATE_FAULT && new_state != WORKSTATE_SHUTDOWN))
		return;

	state_machine->MainState = new_state;
	state_machine->SubState = 0;
	Algorithm = FunArray[new_state];
}


void InitializationState(uint8_t *SubState)
{
    switch(*SubState)
    {
		// Read saved faults
    	case 0:			
    		ModuleSetContactorPosition(OD.BatteryData, stBat_Disabled, OD.ConfigData, &timeStamp);    		

			if(ReadFaults(dtcList, dtcListSize))
				OD.OldFaultsNumber = FillFaultsList(dtcList, dtcListSize, OD.OldFaultList, 0);
			
			flashReadSData(&OD.SData);

			OD.SB.CheckFaults = 0;          // ��������� �������� ������
			OD.SB.CurrentSensorReady = 0;	// ������ ���� �� ������������
            OD.SB.FaultStateHandled = 1;	// ��� ������ ����������
            OD.SB.PrechargeDone = 0;		// ���� ��������� ����������
			OD.SB.PowerOff_SaveParams = 1;	// ���� ����������� ����������(������������� � ���� 3)
            
			OD.MasterControl.CCL = 0;
			OD.MasterControl.DCL = 0;

			// ���������� �������� ����������
			OD.LastPrechargeMaxCurrent_0p1A = INT16_MIN;
			OD.LastPrechargeDuration = 0;

			*SubState = 1;
    	break;
			
		// Waiting for power on
        case 1:
            
            if(OD.SB.PowerOn == 1)
			{				
            	OD.SB.CheckFaults = 1;

				// Intermediate module goes to Operating state and closes contactors
            	// Terminal module goes to sensor calibrating proccess
				if(!ModuleIsPackHeader(OD.ConfigData))
					SetWorkState(&OD.StateMachine, WORKSTATE_OPERATE);
				else
					*SubState = 2;

				timeStamp = GetTimeStamp();
			}
            break;
        
		// Current sensor calibration
        case 2:
		{
            if(GetTimeFrom(timeStamp) > 3000)
            {
				timeStamp = GetTimeStamp();            	

            	if(OD.SData.Result == 0)
            	{
					OD.SB.PowerOff_SaveParams = 1;
            		OD.Energy_As = OD.SData.Buf_1st.ActualEnergy_As;
            		sysEnergy_Init(OD.SData.Buf_1st.TotalActualEnergy_As,
            				OD.ConfigData->VoltageCCLpoint[0][CCL_DCL_POINTS_NUM - 1] - 25,
							OD.ConfigData->VoltageCCLpoint[0][0] + 25);
            	}
            	else
            	{
					OD.SB.PowerOff_SaveParams = 0;
					
					if(OD.SData.Buf_1st.TotalActualEnergy_As == UINT32_MAX){
						OD.Energy_As = sysEnergy_InitEnergyFromMinUcell((int16_t*)OD.ConfigData->OCVpoint, 
								OD.BatteryData[OD.ConfigData->BatteryIndex].MinCellVoltage.Voltage_mv, 
								OD.ConfigData->ModuleCapacity * 3600);
						
						sysEnergy_Init(OD.ConfigData->ModuleCapacity * 3600,
								OD.ConfigData->VoltageCCLpoint[0][CCL_DCL_POINTS_NUM - 1] - 25,
								OD.ConfigData->VoltageCCLpoint[0][0] + 25);
					}
					else
					{
						OD.Energy_As = sysEnergy_InitEnergyFromMinUcell((int16_t*)OD.ConfigData->OCVpoint, 
								OD.BatteryData[OD.ConfigData->BatteryIndex].MinCellVoltage.Voltage_mv, 
								OD.SData.Buf_1st.TotalActualEnergy_As);
						
						sysEnergy_Init(OD.SData.Buf_1st.TotalActualEnergy_As,
								OD.ConfigData->VoltageCCLpoint[0][CCL_DCL_POINTS_NUM - 1] - 25,
								OD.ConfigData->VoltageCCLpoint[0][0] + 25);
					}
				}
				

            	OD.SB.CurrentSensorReady = 1;
				*SubState = 3;
            }
            else
            	csCalibrateCurrentSensor();
		}
            break;
            
        case 3:
			// Terminal module waits for all intermediate modules close contactors
        	if(packIsReady(OD.BatteryData, OD.ModuleData, OD.ConfigData, &timeStamp))
        	{        		
				timeStamp = GetTimeStamp();
				OD.BatteryData[OD.ConfigData->BatteryIndex].TotalEnergy_As = sysEnergy_EnergyEstimation(&OD.Energy_As,
						OD.BatteryData[OD.ConfigData->BatteryIndex].MaxCellVoltage.Voltage_mv,
						OD.BatteryData[OD.ConfigData->BatteryIndex].MinCellVoltage.Voltage_mv);

				SetWorkState(&OD.StateMachine, WORKSTATE_PREOP);
        	}
            
            break;
    }       
}

void PreoperationState(uint8_t *SubState)
{ 
	static uint32_t preChargingTmr = 0;

	ModuleSetContactorPosition(OD.BatteryData, stBat_Disabled, OD.ConfigData, &timeStamp);

	// ������ ���� ����� ������� ����� ������ � ����������
	if(MasterIsReady(&OD.MasterData, OD.BatteryData, OD.ConfigData, &preChargingTmr))
		ControlBatteriesState(&OD.MasterControl.RequestState, WORKSTATE_OPERATE);
}

void OperatingState(uint8_t *SubState)
{    
	switch(*SubState)
	{
		case 0:
		{
			if(ModuleSetContactorPosition(OD.BatteryData, stBat_Precharging, OD.ConfigData, &timeStamp))
			{
				OD.SB.PrechargeDone = 1;
				*SubState = 1;
			}
			else
			{
				// ������� ������������ ��� ����������
				if(OD.LastPrechargeMaxCurrent_0p1A < OD.BatteryData[OD.ConfigData->ModuleIndex].TotalCurrent)
					OD.LastPrechargeMaxCurrent_0p1A = OD.BatteryData[OD.ConfigData->ModuleIndex].TotalCurrent;

				// ������� ������������ ����������
				OD.LastPrechargeDuration = (uint16_t)GetTimeFrom(timeStamp);
			}
				}
			break;
		
		case 1:			
			ModuleSetContactorPosition(OD.BatteryData, stBat_Enabled, OD.ConfigData, &timeStamp);
			break;
	}	
}


void ShutdownState(uint8_t *SubState)
{
	OD.SB.CheckFaults = 0;     
	
	switch (*SubState)
	{
		// ��������� ����� ����� � �����, ��������� ������
		case 0:
			OD.LogicTimers.PowerOffTimer_ms = GetTimeStamp();
			OD.SData.Buf_1st.ActualEnergy_As = OD.Energy_As;
			OD.SData.Buf_1st.TotalActualEnergy_As = sysEnergy_EnergyEstimation(&OD.Energy_As, 
																				OD.BatteryData[OD.ConfigData->BatteryIndex].MaxCellVoltage.Voltage_mv, 
																				OD.BatteryData[OD.ConfigData->BatteryIndex].MinCellVoltage.Voltage_mv);
			OD.SData.Buf_1st.SystemTime = OD.SystemTime;
			OD.SData.Buf_1st.NormalPowerOff = 1;

			flashStoreData(&OD.SData);
			*SubState = 1;
		break;

		case 1:
			ModuleSetContactorPosition(OD.BatteryData, stBat_Disabled, OD.ConfigData, &timeStamp);
			if(GetTimeFrom(OD.LogicTimers.PowerOffTimer_ms) >= OD.ConfigData->PowerOffDelay_ms)
				*SubState = 2;
		break;

		case 2:

			ECU_GoToSleep();
			break;
	}
//	if(!OD.SB.PowerOff_SaveParams)
//	{
//		OD.SB.PowerOff_SaveParams = 1;
//
//		if(OD.AfterResetTime > 3)
//		{
//			SaveFaults();
//		}
//	}
}

void FaultState(uint8_t *SubState)
{
	switch(*SubState)
	{
		case 0:
			ModuleSetContactorPosition(OD.BatteryData, stBat_Disabled, OD.ConfigData, &timeStamp);
			timeStamp = GetTimeStamp();
		break;
			
		case 1:
			if(GetTimeFrom(timeStamp) > 5000)
				OD.SB.FaultStateHandled = 1;
		break;
	}
	
}

void TestingState(uint8_t *SubState)
{
    
}

void CommonState(void)
{
    Protocol();
    
    if(GetTimeFrom(OD.LogicTimers.Timer_1ms) >= OD.DelayValues.Time1_ms)
    {        
        OD.LogicTimers.Timer_1ms = GetTimeStamp();
		
        // Code      
        OD.ecuPowerSupply_0p1 = boardBMSCombi_GetVoltage();
		OD.A_IN[0] = GetVoltageValue(4);
		OD.A_IN[1] = GetVoltageValue(5);		

		// Power Manager thread
		PM_Proc(OD.ecuPowerSupply_0p1, OD.ConfigData->IsPowerManager);
		boardThread();
		vs_thread(OD.CellVoltageArray_mV, OD.CellTemperatureArray);
		packCapacityCalculating(OD.BatteryData, OD.ConfigData, OD.SB.CurrentSensorReady);		
		
		if(FaultsTest())
		{
			OD.SB.FaultStateHandled = 0;
			ControlBatteriesState(&OD.MasterControl.RequestState, WORKSTATE_FAULT);
			SetWorkState(&OD.StateMachine, WORKSTATE_FAULT);
		}

		OD.BatteryData[OD.ConfigData->ModuleIndex].StateMachine.MainState = OD.StateMachine.MainState;
		OD.BatteryData[OD.ConfigData->ModuleIndex].StateMachine.SubState = OD.StateMachine.SubState;
    }
    
	if(GetTimeFrom(OD.LogicTimers.Timer_10ms) >= OD.DelayValues.Time10_ms)
	{
		OD.LogicTimers.Timer_10ms = GetTimeStamp();		
		
		// Battery Module
        ModuleStatisticCalculating(&OD.ModuleData[OD.ConfigData->ModuleIndex], OD.ConfigData, OD.CellVoltageArray_mV, OD.CellTemperatureArray);
		// Battery Pack
		BatteryStatisticCalculating(&OD.BatteryData[OD.ConfigData->BatteryIndex], OD.ModuleData, OD.ConfigData);
		// Master
		BatteryStatisticCalculating(&OD.MasterData, OD.BatteryData, OD.ConfigData);

		OD.LocalPMState = (OD.ConfigData->IsPowerManager)? PM_GetPowerState() : OD.PowerMaganerCmd;
		
		if(OD.LocalPMState == PM_PowerOn1)
		{
			ECU_GoToPowerSupply();
			OD.SB.PowerOn = 1;
		}
		else if((OD.LocalPMState == PM_ShutDown) &&
				(OD.SB.PowerOn == 1 && OD.StateMachine.MainState != WORKSTATE_SHUTDOWN))
		{
			OD.SB.PowerOn = 0;
			SetWorkState(&OD.StateMachine, WORKSTATE_SHUTDOWN);		
		}
		
		if(ModuleIsPackHeader(OD.ConfigData) && 
			(OD.MasterControl.RequestState != OD.StateMachine.MainState && OD.MasterControl.RequestState > WORKSTATE_INIT) &&
			(OD.StateMachine.MainState != WORKSTATE_FAULT && OD.StateMachine.MainState != WORKSTATE_INIT))
		{			
			SetWorkState(&OD.StateMachine, OD.MasterControl.RequestState);
		}
	}
	
    if(GetTimeFrom(OD.LogicTimers.Timer_100ms) >= OD.DelayValues.Time100_ms)
    {
        OD.LogicTimers.Timer_100ms = GetTimeStamp();
		
		OD.PackControl.TargetVoltage_mV = packGetBalancingVoltage(&OD.BatteryData[OD.ConfigData->BatteryIndex], &OD.PackControl, OD.ConfigData);
        OD.PackControl.BalancingEnabled = packGetBalancingPermission(&OD.BatteryData[OD.ConfigData->BatteryIndex], &OD.PackControl, OD.ConfigData);
		GetCurrentLimit(&OD.BatteryData[OD.ConfigData->BatteryIndex], OD.ConfigData, &OD.MasterControl.DCL, &OD.MasterControl.CCL);
		
		vs_ban_balancing(!OD.PackControl.BalancingEnabled);
		vs_set_min_dis_chars(OD.PackControl.TargetVoltage_mV);
			
		LedStatus(FB_PLUS & FB_MINUS, OD.ModuleData[OD.ConfigData->ModuleIndex].DischargingCellsFlag);
		
		OD.InOutState = boardBMSCombi_GetDiscreteIO();
    }
    
    if(GetTimeFrom(OD.LogicTimers.Timer_1s) >= OD.DelayValues.Time1_s)
    {
        OD.LogicTimers.Timer_1s = GetTimeStamp();
		OD.AfterResetTime++;
		OD.SystemTime = dateTime_GetCurrentTotalSeconds();
        
		if(OD.SData.DataChanged && OD.SB.PowerOn)
			flashStoreData(&OD.SData);
        
		uint16_t discharge_mask1 = ltc6803_GetDischargingMask(0);
		uint16_t discharge_mask2 = ltc6803_GetDischargingMask(1);		
        OD.ModuleData[OD.ConfigData->ModuleIndex].DischargingCellsFlag = (uint32_t)discharge_mask1 + ((uint32_t)discharge_mask2 << 12);
		
		OD.BatteryData[OD.ConfigData->BatteryIndex].ActualEnergy_As = OD.Energy_As;
    }
}




