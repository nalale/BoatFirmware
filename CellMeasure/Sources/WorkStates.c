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

// Массив указателей на функции-режимы
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
    		ModuleSetContactorPosition(OD.PackData, stBat_Disabled, OD.ConfigData, &timeStamp);    		

			if(ReadFaults(dtcList, dtcListSize))
				OD.OldFaultsNumber = FillFaultsList(dtcList, dtcListSize, OD.OldFaultList, 0);			

			OD.SB.CheckFaults = 0;          // запрещаем проверку ошибок
			OD.SB.CurrentSensorReady = 0;	// датчик тока не откалиброван
            OD.SB.FaultStateHandled = 1;	// все ошибки обработаны
            OD.SB.PrechargeDone = 0;		// Флаг успешного предзаряда
			OD.SB.PowerOff_SaveParams = 1;	// Флаг корректного выключения(analyze at next step)
            
			OD.MasterControl.CCL = 0;
			OD.MasterControl.DCL = 0;

			// Сбрасываем счетчики предзаряда
			OD.LastPrechargeMaxCurrent_0p1A = INT16_MIN;
			OD.LastPrechargeDuration = 0;

			OD.SB.FlashDataRead = 0;

			timeStamp = GetTimeStamp();
			*SubState = 1;
    	break;
			
    	// Current sensor calibration
        case 1:            
        {
        	// If power supply and cells data is ready
        	if(OD.SB.PowerOn)
        	{
				if(csGetCurrentSensorType() == cstNone)
				{
					*SubState = 2;
					break;
				}

				// if calibration is done and modules data is received
				if(!OD.SB.CurrentSensorReady)
				{
					csCalibrateCurrentSensor();

					if(GetTimeFrom(timeStamp) > 2500)
					{
						OD.SB.CurrentSensorReady = 1;
						csCalibrateIsDone();

						*SubState = 2;
					}
					else
						break;
				}
			}

        	timeStamp = GetTimeStamp();

        }
            break;

            // Analyze saved data
        case 2:
		{
			OD.SB.CheckFaults = 1;

			if(OD.ModuleData[OD.ConfigData->ModuleIndex].DataIsReady)
			{
				// Don't restore data without current sensor
				if(OD.SB.CurrentSensorReady == 0){
					OD.SB.PowerOff_SaveParams = 1;
					OD.SB.FlashDataRead = 1;
					timeStamp = GetTimeStamp();
					*SubState = 3;
				}
				else		// If bank has current sensor it inits local energy
				{
					uint32_t totalBankEnergy_As = UINT32_MAX, actualBankEnergy = UINT32_MAX;
					uint32_t totalPackEnergy_As = UINT32_MAX, actualPackEnergy = UINT32_MAX;

					if(OD.PackData[OD.ConfigData->BatteryIndex].DataIsReady)
					{
						//Init module energy
						sysEnergy_CapacityInit(&OD.ModuleData[OD.ConfigData->ModuleIndex], OD.ConfigData->ModuleCapacity);
						sysEnergy_InitEnergyFromMinUcell(&OD.ModuleData[OD.ConfigData->ModuleIndex],
								(int16_t*)OD.ConfigData->OCVpoint,
								OD.ModuleData[OD.ConfigData->ModuleIndex].MinCellVoltage.Voltage_mv);

						//Init pack energy
						sysEnergy_CapacityInit(&OD.PackData[OD.ConfigData->BatteryIndex], OD.ConfigData->ModuleCapacity);
						sysEnergy_InitEnergyFromMinUcell(&OD.PackData[OD.ConfigData->BatteryIndex],
								(int16_t*)OD.ConfigData->OCVpoint,
								OD.PackData[OD.ConfigData->BatteryIndex].MinCellVoltage.Voltage_mv);

						// check if sdata was read successful
						if(flashReadSData(&OD.SData) == 0)
						{
							OD.SB.PowerOff_SaveParams = 1;

							// Get Total Energy either from FLASH or from settings
							totalBankEnergy_As = OD.SData.Buf_1st.TotalActualEnergy_As;
							totalPackEnergy_As = OD.SData.Buf_1st.SystemTotalEnergy_As;
							// Get Actual energy from FLASH
							actualBankEnergy = OD.SData.Buf_1st.ActualEnergy_As;
							actualPackEnergy = OD.SData.Buf_1st.SystemActualEnergy_As;
						}
						else
						{
							OD.SB.PowerOff_SaveParams = 0;

							totalBankEnergy_As = OD.ModuleData[OD.ConfigData->ModuleIndex].TotalEnergy_As;
							totalPackEnergy_As = OD.PackData[OD.ConfigData->BatteryIndex].TotalEnergy_As;
							actualBankEnergy = OD.ModuleData[OD.ConfigData->ModuleIndex].ActualEnergy_As;
							actualPackEnergy = OD.PackData[OD.ConfigData->BatteryIndex].ActualEnergy_As;
						}

						flashClearData(&OD.SData);
						OD.SB.FlashDataRead = 1;

						// Init Total and Actual energy
						sysEnergy_Init(&OD.ModuleData[OD.ConfigData->ModuleIndex], totalBankEnergy_As, actualBankEnergy,
								OD.ConfigData->VoltageCCLpoint[0][CCL_DCL_POINTS_NUM - 1],
								OD.ConfigData->VoltageCCLpoint[0][0]);

						sysEnergy_Init(&OD.PackData[OD.ConfigData->BatteryIndex], totalPackEnergy_As, actualPackEnergy,
								OD.ConfigData->VoltageCCLpoint[0][CCL_DCL_POINTS_NUM - 1],
								OD.ConfigData->VoltageCCLpoint[0][0]);

						timeStamp = GetTimeStamp();
						*SubState = 3;
					}
				}
			}
		}
            break;
            
        case 3:
        	if(OD.SB.PowerOn == 1)
			{
				// Terminal module waits for all intermediate modules close contactors
				if(ModuleIsPackHeader(OD.ConfigData))
				{
					if(packIsReady(OD.PackData, OD.ModuleData, OD.ConfigData, &timeStamp))
					{
						timeStamp = GetTimeStamp();
						SetWorkState(&OD.StateMachine, WORKSTATE_PREOP);
						
					}
				}
				else
				{
					SetWorkState(&OD.StateMachine, WORKSTATE_OPERATE);
					OD.SB.CheckFaults = 1;
				}
			}
            
            break;
    }       
}

void PreoperationState(uint8_t *SubState)
{ 
	static uint32_t preChargingTmr = 0;

	ModuleSetContactorPosition(OD.PackData, stBat_Disabled, OD.ConfigData, &timeStamp);

	// Мастер ждет когда система будет готовы к предзаряду
	if(MasterIsReady(&OD.MasterData, OD.PackData, OD.ConfigData, &preChargingTmr))
	{
		ControlBatteriesState(&OD.MasterControl.RequestState, WORKSTATE_OPERATE);
		OD.MasterControl.CCL = OD.ConfigData->Sys_MaxCCL / OD.ConfigData->Sys_ModulesCountP;
		OD.MasterControl.DCL = OD.ConfigData->Sys_MaxDCL / OD.ConfigData->Sys_ModulesCountP;
	}
}

void OperatingState(uint8_t *SubState)
{    
	switch(*SubState)
	{
		case 0:
		{
			if(ModuleSetContactorPosition(OD.PackData, stBat_Precharging, OD.ConfigData, &timeStamp))
			{
				OD.SB.PrechargeDone = 1;
				*SubState = 1;
			}
			else
			{
				// Считаем максимальный ток предзаряда
				if(OD.LastPrechargeMaxCurrent_0p1A < OD.PackData[OD.ConfigData->ModuleIndex].TotalCurrent)
					OD.LastPrechargeMaxCurrent_0p1A = OD.PackData[OD.ConfigData->ModuleIndex].TotalCurrent;

				// Считаем длительность предзаряда
				OD.LastPrechargeDuration = (uint16_t)GetTimeFrom(timeStamp);
			}
				}
			break;
		
		case 1:			
			ModuleSetContactorPosition(OD.PackData, stBat_Enabled, OD.ConfigData, &timeStamp);
			break;
	}	
}


void ShutdownState(uint8_t *SubState)
{
	OD.SB.CheckFaults = 0;     
	
	switch (*SubState)
	{
		// Запомнить время входа в режим, сохранить данные
		case 0:
			OD.LogicTimers.PowerOffTimer_ms = GetTimeStamp();

			if(OD.SB.FlashDataRead)
			{
				OD.SData.Buf_1st.ActualEnergy_As = OD.ModuleData[OD.ConfigData->ModuleIndex].ActualEnergy_As;
				OD.SData.Buf_1st.TotalActualEnergy_As = OD.ModuleData[OD.ConfigData->ModuleIndex].TotalEnergy_As;

				OD.SData.Buf_1st.SystemActualEnergy_As = OD.PackData[OD.ConfigData->BatteryIndex].ActualEnergy_As;
				OD.SData.Buf_1st.SystemTotalEnergy_As = OD.PackData[OD.ConfigData->BatteryIndex].TotalEnergy_As;

	//			OD.SData.Buf_1st.BmsTotalEnergy_As = OD.MasterData.TotalEnergy_As;
	//			OD.SData.Buf_1st.BmsTotalEnergy_As = OD.MasterData.ActualEnergy_As;

				OD.SData.Buf_1st.SystemTime = OD.SystemTime;
				OD.SData.Buf_1st.NormalPowerOff = 1;

				flashStoreData(&OD.SData);
			}
			*SubState = 1;
		break;

		case 1:
			ModuleSetContactorPosition(OD.PackData, stBat_Disabled, OD.ConfigData, &timeStamp);
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
			ModuleSetContactorPosition(OD.PackData, stBat_Disabled, OD.ConfigData, &timeStamp);
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
		
		if(OD.SB.CheckFaults && FaultsTest())
		{
			OD.SB.FaultStateHandled = 0;
			ControlBatteriesState(&OD.MasterControl.RequestState, WORKSTATE_FAULT);
			SetWorkState(&OD.StateMachine, WORKSTATE_FAULT);
		}

		OD.ModuleData[OD.ConfigData->ModuleIndex].MainState = OD.StateMachine.MainState;
		OD.ModuleData[OD.ConfigData->ModuleIndex].SubState = OD.StateMachine.SubState;
    }
    
	if(GetTimeFrom(OD.LogicTimers.Timer_10ms) >= OD.DelayValues.Time10_ms)
	{
		OD.LogicTimers.Timer_10ms = GetTimeStamp();		
		
		// bank doesn't have current sensor get current from battery data
		int16_t current = (OD.SB.CurrentSensorReady)? csGetAverageCurrent() : INT16_MAX;

		// For each module
		sysEnergy_EnergyCounting(&OD.ModuleData[OD.ConfigData->ModuleIndex], current);
		ModuleStatisticCalculating(&OD.ModuleData[OD.ConfigData->ModuleIndex], OD.ConfigData, OD.CellVoltageArray_mV, OD.CellTemperatureArray);

		// For assemblies
/*		for(int8_t mod = 0; mod < OD.ConfigData->ModulesInAssembly - 1; mod++)
			assemblyAddData(&OD.ModuleData[OD.ConfigData->ModuleIndex], &OD.ModuleData[OD.ConfigData->ModuleIndex + mod]);
*/
		
		// For header modules or Bms
		if(ModuleIsPackHeader(OD.ConfigData))
		{
			sysEnergy_EnergyCounting(&OD.PackData[OD.ConfigData->BatteryIndex], current);
			BatteryCheckModulesOnline(&OD.PackData[OD.ConfigData->BatteryIndex], OD.ModuleData, OD.ConfigData->Sys_ModulesCountS);
			BatteryStatisticCalculating(&OD.PackData[OD.ConfigData->BatteryIndex], OD.ModuleData, OD.ConfigData->Sys_ModulesCountS);
			
			//sysEnergy_EnergyCounting(&OD.MasterData, OD.MasterData.TotalCurrent);
			BatteryCheckModulesOnline(&OD.MasterData, OD.PackData, OD.ConfigData->Sys_ModulesCountP);
			BatteryStatisticCalculating(&OD.MasterData, OD.PackData, OD.ConfigData->Sys_ModulesCountP);

			OD.PackData[OD.ConfigData->BatteryIndex].MainState = OD.ModuleData[OD.ConfigData->ModuleIndex].MainState;
			OD.PackData[OD.ConfigData->BatteryIndex].SubState = OD.ModuleData[OD.ConfigData->ModuleIndex].SubState;
		}
		
		// For modules in assembly
		if(ModuleIsAssemblyHeader(OD.ConfigData))
		{
			BatteryCheckModulesOnline(&OD.PackData[OD.ConfigData->BatteryIndex], OD.ModuleData, OD.ConfigData->ModulesInAssembly);
			
			sysEnergy_EnergyCounting(&OD.PackData[OD.ConfigData->BatteryIndex], current);
			BatteryStatisticCalculating(&OD.PackData[OD.ConfigData->BatteryIndex], OD.ModuleData, OD.ConfigData->ModulesInAssembly);
		}


		// Power Manager Functionality
//		OD.LocalPMState = (OD.ConfigData->IsPowerManager)? PM_GetPowerState() : OD.PowerMaganerCmd;
		
		OD.LocalPMState = PM_GetPowerState();

		if(OD.LocalPMState == PM_PowerOn1)
		{
			ECU_GoToPowerSupply();
			OD.SB.PowerOn = 1;
		}
		else if((OD.LocalPMState == PM_ShutDown || OD.PowerMaganerCmd == PM_ShutDown) &&
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

        // for header module or for pack modules
        if(ModuleIsAssemblyHeader(OD.ConfigData))
        {
			OD.PackControl.TargetVoltage_mV = packGetBalancingVoltage(&OD.PackData[OD.ConfigData->BatteryIndex], &OD.PackControl, OD.ConfigData);
			OD.PackControl.BalancingEnabled = packGetBalancingPermission(&OD.PackData[OD.ConfigData->BatteryIndex], &OD.PackControl, OD.ConfigData);
			//packGetCurrentsLimit(&OD.PackData[OD.ConfigData->BatteryIndex], OD.ConfigData, &OD.MasterControl, &OD.PackData[OD.ConfigData->BatteryIndex].DCL, &OD.PackData[OD.ConfigData->BatteryIndex].CCL);
			OD.PackData[OD.ConfigData->BatteryIndex].CCL = packGetChargeCurrentLimit(&OD.PackData[OD.ConfigData->BatteryIndex], OD.ConfigData, OD.MasterControl.CCL);
			OD.PackData[OD.ConfigData->BatteryIndex].DCL = packGetDischargeCurrentLimit(&OD.PackData[OD.ConfigData->BatteryIndex], OD.ConfigData, OD.MasterControl.DCL);
        }
        else
        {
        	// Get from CAN message
        }

        OD.MasterData.CCL = packGetChargeCurrentLimit(&OD.MasterData, OD.ConfigData, OD.MasterControl.CCL);
        OD.MasterData.DCL = packGetDischargeCurrentLimit(&OD.MasterData, OD.ConfigData, OD.MasterControl.DCL);


		vs_ban_balancing(!OD.PackControl.BalancingEnabled);
		vs_set_min_dis_chars(OD.PackControl.TargetVoltage_mV);
			
		if(OD.ConfigData->TestMode)
			TestContactorControl();
		else
			LedStatus(FB_PLUS & FB_MINUS, OD.ModuleData[OD.ConfigData->ModuleIndex].DischargingCellsFlag);
		
		OD.InOutState = boardBMSCombi_GetDiscreteIO();
    }
    
    if(GetTimeFrom(OD.LogicTimers.Timer_1s) >= OD.DelayValues.Time1_s)
    {
		//DEBUG
//		OD.PackData[OD.ConfigData->BatteryIndex].ActualEnergy_As++;
		
        OD.LogicTimers.Timer_1s = GetTimeStamp();
		OD.AfterResetTime++;
		OD.SystemTime = dateTime_GetCurrentTotalSeconds();
        
		if(OD.SData.DataChanged && OD.SB.PowerOn)
			flashStoreData(&OD.SData);
        
		uint16_t discharge_mask1 = ltc6803_GetDischargingMask(0);
		uint16_t discharge_mask2 = ltc6803_GetDischargingMask(1);		
        OD.ModuleData[OD.ConfigData->ModuleIndex].DischargingCellsFlag = (uint32_t)discharge_mask1 + ((uint32_t)discharge_mask2 << 12);
    }
}




