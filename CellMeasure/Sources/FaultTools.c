#include <stdlib.h>

#include "FaultTools.h"
#include "Main.h"
#include "TimerFunc.h"
#include "MemoryFunc.h"
#include "BMS_Combi_ECU.h"
#include "../Libs/LTC6803.h"
#include "../Libs/CurrentSens.h"
#include "../MolniaLib/FaultCategory.h"




static uint8_t pointer[512];

typedef struct
{
	struct
	{
		uint8_t
		CriticalFaultExist				: 1,				// �������� �� �������� ������ ��� ��� (��������� ������)
		WarningIndicatorRequested		: 1,				// ��������� �� ������������ � ������������� ��� ���
		PowerOff						: 1,				// ����������� ��������� ��� ������������� �������������

		dummy : 5;
	};
} dtcEnvironment_t;



uint8_t MasterFaultTest(EcuConfig_t *ecuConfig);
uint8_t BatteryFaultTest(EcuConfig_t *ecuConfig);
uint8_t ModuleFaultTest(EcuConfig_t *ecuConfig);

// Handlers
static uint8_t ContactorWeldingHandler(uint32_t *timeStamp);

void SetGeneralFRZR(dtcFRZF_General* f);
void dtcSetFault(dtcItem_t* it, dtcEnvironment_t* env);
uint8_t dtcFaultDetection(dtcItem_t* it, dtcEnvironment_t *env, uint8_t isFault);





uint8_t FaultsTest()
{
	EcuConfig_t ecuConfig = GetConfigInstance();
	
	// ��� �������� ��������� ��� ������
	uint8_t is_critical_fault = ModuleFaultTest(&ecuConfig);
	
	if(!OD.SB.CheckFaults || OD.StateMachine.MainState == WORKSTATE_FAULT)
	{
		FillFaultsList(OD.FaultList, &OD.FaultsNumber, 1);
		return 0;	
	}

	// ��� �������� ��������� ������ ������� ������ � �����
	if(ecuConfig.ModuleIndex == 0)
		is_critical_fault |= BatteryFaultTest(&ecuConfig);

	// ��� �������� ��������� ������ ������
	if(ecuConfig.IsMaster)
		is_critical_fault |= MasterFaultTest(&ecuConfig);	
	
	FillFaultsList(OD.FaultList, &OD.FaultsNumber, 1);
	
    return is_critical_fault;
}


uint8_t FillFaultsList(uint16_t *Array, uint8_t *FaultNum, uint8_t IsActualFaults)
{
	// ������� �� ���� ���������� ��������������
	// ������� ���� ��������� ��������������
	// ���� ������������� ������������
	
	for(uint8_t i = 0; i < dtcListSize; i++)
	{	
		// �������� ����� �������� �������� � �������
		uint8_t TestFailed = (IsActualFaults)? dtcList[i]->Status.TestFailed : 1;
		
		if(dtcList[i]->Status.ConfirmedDTC & TestFailed)
		{
			uint16_t faultCode = (uint16_t)(dtcList[i]->Property->Code << 8) + (dtcList[i]->Category);
			
			int i = 0;
			for(i = 0; i < *FaultNum; i++)
			{
				// ���� � ������ ��� ���� ������� ��� ������ - ������� �� �����, 
				if(Array[i] == faultCode || i >= MAX_FAULTS_NUM)
					break;			
			}
			
			if(i == *FaultNum)
			{
				Array[i] = faultCode;
				(*FaultNum)++;
			}
		}
	}
	
	return 0;
}

uint8_t MasterFaultTest(EcuConfig_t *ecuConfig)
{
	/*
	1. �� ��� ������� � ����
	2. ������������ ��������� ����� �� �������
	3. ��� ����� � Vcu
	4. ������� ���������� ����� ���������
	5. 
	*/
	
	dtcItem_t* it;
	dtcEnvironment_t env;
	uint8_t TestFailedThisOperationCycle;
	
	//�� ��� ������� � ����
	it = &dtcMst_BatNumber;
	if(++it->SamplePeriodCounter >= it->Property->TestSamplePeriod)
	{
		it->SamplePeriodCounter = 0;
		if(OD.MasterData.OnlineNumber != ecuConfig->Sys_ModulesCountP)
		{
			TestFailedThisOperationCycle = it->Status.TestFailedThisOperationCycle;
			if(dtcFaultDetection(it, &env, 1) == DTC_TEST_RESULT_FAILED)
			{
				if(!TestFailedThisOperationCycle)
				{
					dtcSetFault(it, &env);
					it->Category = 0;
					SetGeneralFRZR(&frzfBatNumber);							
				}
				OD.Faults.Mst_BatteryWrongCount = 1;
			}
		}
		else if(dtcFaultDetection(it, &env, 0) == DTC_TEST_RESULT_PASSED)
		{
			OD.Faults.Mst_BatteryWrongCount = 0;
		}
	}
	
	//������� ���������� ����� ���������
	it = &dtcMst_BatVoltageDiff;
	if(++it->SamplePeriodCounter >= it->Property->TestSamplePeriod)
	{
		it->SamplePeriodCounter = 0;
		if(OD.MasterData.MaxBatteryVoltage.Voltage - OD.MasterData.MinBatteryVoltage.Voltage > ecuConfig->Sys_MaxVoltageDisbalanceP)
		{
			TestFailedThisOperationCycle = it->Status.TestFailedThisOperationCycle;
			if(dtcFaultDetection(it, &env, 1) == DTC_TEST_RESULT_FAILED)
			{				
				if(!TestFailedThisOperationCycle)
				{
					dtcSetFault(it, &env);
					it->Category = 0;
					SetGeneralFRZR(&frzfBatVoltageDiff);							
				}
				OD.Faults.Mst_BatteryVoltageDiff = 1;
			}
		}
		else if(dtcFaultDetection(it, &env, 0) == DTC_TEST_RESULT_PASSED)
		{
			OD.Faults.Mst_BatteryVoltageDiff = 0;
		}
	}
	
	//������������ ��������� ����� �� �������
	it = &dtcMst_BatState;
	if(++it->SamplePeriodCounter >= it->Property->TestSamplePeriod)
	{
		uint8_t flag = 0;
		it->SamplePeriodCounter = 0;
		for(uint8_t i = 0; i < ecuConfig->Sys_ModulesCountP; i++)
		{
			if(OD.BatteryData[i].StateMachine.MainState != OD.MasterControl.RequestState)
			{
				flag = 1;
				TestFailedThisOperationCycle = it->Status.TestFailedThisOperationCycle;
				if(dtcFaultDetection(it, &env, 1) == DTC_TEST_RESULT_FAILED)
				{
					if(!TestFailedThisOperationCycle)
					{
						dtcSetFault(it, &env);
						it->Category = 0;
						SetGeneralFRZR(&frzfBatState);							
					}
					OD.Faults.Mst_BatteryFault = 1;			
				}
			}
		}
		if(flag == 0)
		{
			if(dtcFaultDetection(it, &env, 0) == DTC_TEST_RESULT_PASSED)
				OD.Faults.Mst_BatteryFault = 0;
		}
	}
	
	// ������ ���������� ������ ���� ��� ���� �������
	it = &dtcCellVoltage;
	if(++it->SamplePeriodCounter >= it->Property->TestSamplePeriod)
	{
		it->SamplePeriodCounter = 0;
		if(OD.MasterData.MinCellVoltage.Voltage_mv < ecuConfig->Sys_MinCellVoltage_mV)
		{
			TestFailedThisOperationCycle = it->Status.TestFailedThisOperationCycle;
			if(dtcFaultDetection(it, &env, 1) == DTC_TEST_RESULT_FAILED)
			{
				if(TestFailedThisOperationCycle == 0)
				{
					dtcSetFault(it, &env);
					it->Category = dctCat_SignalStuckLow;
					SetGeneralFRZR(&frzfCellVoltage);
				}
				OD.Faults.Mst_CellVoltageOutOfRange = 1;
			}

		}else if(OD.MasterData.MaxCellVoltage.Voltage_mv > ecuConfig->Sys_MaxCellVoltage_mV)
		{
			TestFailedThisOperationCycle = it->Status.TestFailedThisOperationCycle;
			if(dtcFaultDetection(it, &env, 1) == DTC_TEST_RESULT_FAILED)
			{
				if(TestFailedThisOperationCycle == 0)
				{
					dtcSetFault(it, &env);
					it->Category = dctCat_SignalStuckHigh;
					SetGeneralFRZR(&frzfCellVoltage);
				}
				OD.Faults.Mst_CellVoltageOutOfRange = 1;
			}
		}
		else
		{
			if(dtcFaultDetection(it, &env, 0) == DTC_TEST_RESULT_PASSED)
			{
				OD.Faults.Mst_CellVoltageOutOfRange = 0;
			}
		}
		
	}
	
	return env.CriticalFaultExist;
}

uint8_t BatteryFaultTest(EcuConfig_t *ecuConfig)
{
	/*
	1. �� ��� ������ � ����
	2. ������������ ��������� ������ �� �������
	3. ���������� �� ���� ��������
	4. ���������� �� ���� ������
	5. ������� ���������� ����� ��������
	6. ������������ ���������� ������ ��������
	7. �� � �������� ����������
	8. ������ ����������� ������� ����
	*/
	
	dtcItem_t* it;
	dtcEnvironment_t env;
	uint8_t TestFailedThisOperationCycle;
	
	//�� ��� ������ � ����
	it = &dtcBat_WrongModNumber;
	if(++it->SamplePeriodCounter >= it->Property->TestSamplePeriod)
	{
		it->SamplePeriodCounter = 0;
		if(OD.BatteryData[ecuConfig->BatteryIndex].OnlineNumber != ecuConfig->Sys_ModulesCountS)
		{
			TestFailedThisOperationCycle = it->Status.TestFailedThisOperationCycle;
			if(dtcFaultDetection(it, &env, 1) == DTC_TEST_RESULT_FAILED)
			{				
				if(TestFailedThisOperationCycle == 0)
				{
					dtcSetFault(it, &env);
					it->Category = 0;
					SetGeneralFRZR(&frzfModuleNumber);				
					OD.Faults.Bat_ModuleWrongCount = 1;								
				}
			}
		}
		else
		{
			if(dtcFaultDetection(it, &env, 0) == DTC_TEST_RESULT_PASSED)
			{	
				OD.Faults.Bat_ModuleWrongCount = 0;		
			}
		}			
		
	}
	
	//������������ ��������� ������ �� �������
	static uint8_t module_fault = 0;
	it = &dtcBat_WrongModState;
	if(++it->SamplePeriodCounter >= it->Property->TestSamplePeriod)
	{
		uint8_t flag = 0;
		it->SamplePeriodCounter = 0;
		// ���� ���������� � ������� 1, ����� �� ������������� ��������� ����
		for(uint8_t i = 1; i < ecuConfig->Sys_ModulesCountS; i++)
		{
			// � ������ ������������� ���� ��������� ���� �������
			if(OD.StateMachine.MainState == WORKSTATE_INIT && OD.StateMachine.SubState > 3)
			{
				if(OD.ModuleData[i].StateMachine.MainState != WORKSTATE_OPERATE)
				{
					flag = 1;
					TestFailedThisOperationCycle = it->Status.TestFailedThisOperationCycle;
					if(dtcFaultDetection(it, &env, 1) == DTC_TEST_RESULT_FAILED)
					{			
						if(TestFailedThisOperationCycle == 0)
						{						
							it->Category = 0;
							dtcSetFault(it, &env);
							SetGeneralFRZR(&frzfModuleNumber);							
						}
						OD.Faults.Bat_ModuleFault = 1;
					}
				}				
			}
			// � ������ ������ ����� ��������� ������ ���� ���� �� ������� ������� � ������������ ���������
			else if(OD.StateMachine.MainState == WORKSTATE_OPERATE)
			{
				if(OD.ModuleData[i].StateMachine.MainState != WORKSTATE_OPERATE)
				{
					flag = 1;
					TestFailedThisOperationCycle = it->Status.TestFailedThisOperationCycle;
					if(dtcFaultDetection(it, &env, 1) == DTC_TEST_RESULT_FAILED)
					{
						if(TestFailedThisOperationCycle == 0)
						{		
							it->Category = 0;
							dtcSetFault(it, &env);
							SetGeneralFRZR(&frzfModuleNumber);					
						}
						OD.Faults.Bat_ModuleFault = 1;
					}
				}					
			}
			module_fault = flag;
		}
		if(module_fault == 0)
		{
			if(dtcFaultDetection(it, &env, 0) == DTC_TEST_RESULT_PASSED)
			{			
				OD.Faults.Bat_ModuleFault = 0;
			}
		}
	}
	
	// ���������� �� ���� ��������
	it = &dtcBat_OverCurrent;
	if(++it->SamplePeriodCounter >= it->Property->TestSamplePeriod)
	{
		it->SamplePeriodCounter = 0;		
		
		int16_t _system_current = OD.BatteryData[ecuConfig->BatteryIndex].TotalCurrent / 10;
		
		if(_system_current > OD.BatteryData[ecuConfig->BatteryIndex].DCL)
		{
			TestFailedThisOperationCycle = it->Status.TestFailedThisOperationCycle;
			if(dtcFaultDetection(it, &env, 1) == DTC_TEST_RESULT_FAILED)
			{			
				if(TestFailedThisOperationCycle == 0)
				{		
					it->Category = dctCat_CircuitCurrentAboveThreshold;
					dtcSetFault(it, &env);
					SetGeneralFRZR(&frzfOverCurrent);			
					OD.Faults.Bat_OverLoad = 1;
				}
			}			
		}
		else if(_system_current < OD.BatteryData[ecuConfig->BatteryIndex].CCL)
		{
			TestFailedThisOperationCycle = it->Status.TestFailedThisOperationCycle;
			if(dtcFaultDetection(it, &env, 1) == DTC_TEST_RESULT_FAILED)
			{			
				if(TestFailedThisOperationCycle == 0)
				{		
					it->Category = dctCat_CircuitCurrentBelowThreshold;
					dtcSetFault(it, &env);
					SetGeneralFRZR(&frzfOverCurrent);			
					OD.Faults.Bat_OverLoad = 1;
				}
			}			
		}
		else if(dtcFaultDetection(it, &env, 0) == DTC_TEST_RESULT_PASSED)
		{
			OD.Faults.Bat_OverLoad = 0;
		}
	}
	
	//������� ���������� ����� ��������
	it = &dtcBat_VoltageDiff;
	if(++it->SamplePeriodCounter >= it->Property->TestSamplePeriod)
	{
		it->SamplePeriodCounter = 0;		
		if(OD.BatteryData[ecuConfig->BatteryIndex].MaxBatteryVoltage.Voltage - OD.BatteryData[ecuConfig->BatteryIndex].MinBatteryVoltage.Voltage > (ecuConfig->Sys_MaxVoltageDisbalanceS * 10))
		{			
			TestFailedThisOperationCycle = it->Status.TestFailedThisOperationCycle;
			if(dtcFaultDetection(it, &env, 1) == DTC_TEST_RESULT_FAILED)
			{		
				OD.Faults.Bat_ModuleVoltageDiff = 1;
				if(!TestFailedThisOperationCycle)
				{
					it->Category = dctCat_SignalCompareFailure;
					dtcSetFault(it, &env);
					SetGeneralFRZR(&frzfModuleVoltageDiff);			
				}					
			}		
		}
		else if(dtcFaultDetection(it, &env, 0) == DTC_TEST_RESULT_PASSED)
		{
			OD.Faults.Bat_ModuleVoltageDiff = 0;
		}
	}
	
	//������ ���������
	it = &dtcBat_Precharge;
	if(GET_PRECHARGE)
	{
		if(++it->SamplePeriodCounter >= it->Property->TestSamplePeriod)
		{
			it->SamplePeriodCounter = 0;
			// ���� �� � ��������
			if(OD.ModuleData[ecuConfig->ModuleIndex].TotalCurrent >= ((int16_t)ecuConfig->PreMaxCurrent * 10))
			{	
				OD.Faults.Bat_ModuleVoltageDiff = 1;
				TestFailedThisOperationCycle = it->Status.TestFailedThisOperationCycle;
				if(!TestFailedThisOperationCycle)
				{
					it->Category = dctCat_CircuitShortToGround;
					dtcSetFault(it, &env);
					SetGeneralFRZR(&frzfPrecharge);			
				}
				PRECHARGE(BAT_OPEN);
				OD.Faults.Bat_Precharge = 1;
			}
			
			// ���� ������������ ���������� ������ �������� 
			if ((OD.BatteryData[ecuConfig->BatteryIndex].TotalCurrent > ((uint16_t)ecuConfig->PreZeroCurrent * 10)) || 
				(OD.BatteryData[ecuConfig->BatteryIndex].TotalCurrent < -((int16_t)ecuConfig->PreZeroCurrent * 10)))
			{
				TestFailedThisOperationCycle = it->Status.TestFailedThisOperationCycle;
				if(dtcFaultDetection(it, &env, 1) == DTC_TEST_RESULT_FAILED)
				{		
					TestFailedThisOperationCycle = it->Status.TestFailedThisOperationCycle;
					if(!TestFailedThisOperationCycle)
					{
						it->Category = dctCat_SignalStuckHigh;
						dtcSetFault(it, &env);
						SetGeneralFRZR(&frzfPrecharge);			
					}
					PRECHARGE(BAT_OPEN);
					OD.Faults.Bat_Precharge = 1;	
				}					
			}
			else if(dtcFaultDetection(it, &env, 0) == DTC_TEST_RESULT_PASSED)
			{
				OD.SB.PrechargeDone = 1;
				OD.Faults.Bat_Precharge = 0;	
			}
		}
	}
	
	//������ ����������� ������� ����
	it = &dtcBat_CurrentSensor;
	if(++it->SamplePeriodCounter >= it->Property->TestSamplePeriod)
	{
		uint8_t tmp1;
		it->SamplePeriodCounter = 0;
		
		tmp1 = csGetCircuitState();
		if (tmp1 != 0)
		{
			TestFailedThisOperationCycle = it->Status.TestFailedThisOperationCycle;
			if(dtcFaultDetection(it, &env, 1) == DTC_TEST_RESULT_FAILED)
			{				
				// ����-����
				if(TestFailedThisOperationCycle == 0)
				{
					it->Category = tmp1;
					dtcSetFault(it, &env);
					SetGeneralFRZR(&frzfBatCurrentSensor);
				}
				
				OD.Faults.Bat_CurrentSens = 1;
			}						
		}
		else if(dtcFaultDetection(it, &env, 0) == DTC_TEST_RESULT_PASSED)
		{
			OD.Faults.Bat_CurrentSens = 0;
			
		}
	}
	
	//������� ����������
	it = &dtcBat_BmsTimeout;
	if(++it->SamplePeriodCounter >= it->Property->TestSamplePeriod)
	{
		it->SamplePeriodCounter = 0;
		if(OD.SB.MsgFromSystem == 0)
		{
			TestFailedThisOperationCycle = it->Status.TestFailedThisOperationCycle;
			if(dtcFaultDetection(it, &env, 1) == DTC_TEST_RESULT_FAILED)
			{				
				if(TestFailedThisOperationCycle == 0)
				{
					dtcSetFault(it, &env);
					it->Category = 0;
					SetGeneralFRZR(&frzfBatBmsTimeout);				
					OD.Faults.Bat_ExtTimeout = 1;								
				}
			}
		}
		else
		{
			dtcFaultDetection(it, &env, 0);
			OD.SB.MsgFromSystem = 0;
		}		
	}
	
	return env.CriticalFaultExist;
}

uint8_t ModuleFaultTest(EcuConfig_t *ecuConfig)
{
	/*
	0. ������ CRC �������
	1. ��������� �������� ��������
	2. ��������� �� ����������
	3. ����������� ��������� ����. ��������
	4. ����������� ���� ���. ��������
	5. ����� ���� ��������� ltc6803
	6. �������� ������� ���������� ltc6803
	7. ������� PM
	*/
	
	dtcItem_t* it;
	uint8_t TestFailedThisOperationCycle;
	dtcEnvironment_t env;
	int tmp1, tmp2;
	
	env.CriticalFaultExist = 0;
	env.PowerOff = 0;
	env.WarningIndicatorRequested = 0;
	
	// ������ ����������� ������
	it = &dtcEcuConfigMemory;
	if(OD.Faults.Mod_ConfigCrc)
	{
		if(!it->Status.TestFailedThisOperationCycle)
		{
			dtcSetFault(it, &env);
			it->Category = dctCat_MissingCalibration;
			SetGeneralFRZR(&frzfEcuConfigMemory);	
			OD.Faults.Mod_ConfigCrc = 1;
		}
	}
	else
	{
		dtcFaultDetection(it, &env, 0);
		OD.Faults.Mod_ConfigCrc = 0;
	}
		
	
	// �������� �����������	
	it = &dtcContactor;
	// �� ��������� ����������� ��������� ���������
	if (OD.StateMachine.MainState == WORKSTATE_INIT)
	{
		if ((FB_PLUS && !GET_BAT_PLUS) || (FB_MINUS && !GET_BAT_MINUS))
		{
			if(!it->Status.TestFailedThisOperationCycle && !OD.SB.EmergencyMode)
			{				
				dtcSetFault(it, &env);
				it->Category = dctCat_ActuatorStuckClosed;
				SetGeneralFRZR(&frzfContactor);
			}
			if((FB_PLUS && !GET_BAT_PLUS))
				OD.Faults.Mod_ContactorPlusFB = 1;
			if(FB_MINUS && !GET_BAT_MINUS)
				OD.Faults.Mod_ContactorMinusFB = 1;
		}
		else
		{
			dtcFaultDetection(it, &env, 0);
			OD.Faults.Mod_ContactorPlusFB = 0;
			OD.Faults.Mod_ContactorMinusFB = 0;
		}
	}
	// �� ����������
	else
	{
		if(++it->SamplePeriodCounter >= it->Property->TestSamplePeriod)
		{
			it->SamplePeriodCounter = 0;
			uint8_t fb_p_st = FB_PLUS;
			uint8_t fb_n_st = FB_MINUS;
			uint8_t p_cmd = GET_BAT_PLUS;
			uint8_t n_cmd = GET_BAT_MINUS;
			
			if ((!fb_p_st && p_cmd) || (!fb_n_st && n_cmd))
			{
				TestFailedThisOperationCycle = it->Status.TestFailedThisOperationCycle;
				if(dtcFaultDetection(it, &env, 1) == DTC_TEST_RESULT_FAILED)
				{
					if(TestFailedThisOperationCycle == 0 && !OD.SB.EmergencyMode)
					{
						dtcSetFault(it, &env);
						it->Category = dctCat_ActuatorStuckOpen;
						SetGeneralFRZR(&frzfContactor);
					}
					if(!fb_p_st && p_cmd)
						OD.Faults.Mod_ContactorPlus = 1;
					if(!fb_n_st && n_cmd)
						OD.Faults.Mod_ContactorMinus = 1;
				}
			}else
			{
				if(dtcFaultDetection(it, &env, 0) == DTC_TEST_RESULT_PASSED)
				{
					OD.Faults.Mod_ContactorPlus = 0;
					OD.Faults.Mod_ContactorMinus = 0;
				}
			}
		}
	}
	
	
	//������ ����������� ������ ���� ��� ���� �������
	it = &dtcCellTemperature;
	if(it->Status.TestFailed == 0)
	{
		// �� ���������/���������� ����� �������� ���������,
		tmp1 = ecuConfig->TemperatureCCLpoint[0][CCL_DCL_POINTS_NUM - 1];
		tmp2 = ecuConfig->TemperatureCCLpoint[0][0];
	}
	else
	{
		// � ��� ����, ����� ����� �� ������ - ��������� ��������
		tmp1 = ecuConfig->TemperatureCCLpoint[0][CCL_DCL_POINTS_NUM - 1] - 3;
		tmp2 = ecuConfig->TemperatureCCLpoint[0][0] + 3;
	}
	
	if(++it->SamplePeriodCounter >= it->Property->TestSamplePeriod)
	{
		it->SamplePeriodCounter = 0;
		if(OD.ModuleData[ecuConfig->ModuleIndex].MinModuleTemperature.Temperature < tmp2)
		{
			TestFailedThisOperationCycle = it->Status.TestFailedThisOperationCycle;
			if(dtcFaultDetection(it, &env, 1) == DTC_TEST_RESULT_FAILED)
			{				
				if(TestFailedThisOperationCycle == 0)
				{
					dtcSetFault(it, &env);
					it->Category = dctCat_SignalStuckLow;
					SetGeneralFRZR(&frzfCellTemperature);							
				}
				OD.Faults.Mod_ModuleTempOutOfRange = 1;
			}			
		}
		else if(OD.ModuleData[ecuConfig->ModuleIndex].MaxModuleTemperature.Temperature > tmp1)
		{
			TestFailedThisOperationCycle = it->Status.TestFailedThisOperationCycle;
			if(dtcFaultDetection(it, &env, 1) == DTC_TEST_RESULT_FAILED)
			{				
				if(TestFailedThisOperationCycle == 0)
				{
					dtcSetFault(it, &env);			
					it->Category = dctCat_SignalStuckHigh;
					SetGeneralFRZR(&frzfCellTemperature);							
				}
				OD.Faults.Mod_ModuleTempOutOfRange = 1;
			}
		}
		else
		{
			dtcFaultDetection(it, &env, 0);
			OD.Faults.Mod_ModuleTempOutOfRange = 0;
		}
	}

	// ����� ��������� �����
	it = &dtcMeasuringCircuit;
	if(++it->SamplePeriodCounter >= it->Property->TestSamplePeriod)
	{
		it->SamplePeriodCounter = 0;

		uint8_t f = ltc6803_GetError(0);
		if(f == LTC6803_FAULTS_NONE)
			f = ltc6803_GetError(1);

		if(f != LTC6803_FAULTS_NONE)
		{
			TestFailedThisOperationCycle = it->Status.TestFailedThisOperationCycle;
			if(dtcFaultDetection(it, &env, 1) == DTC_TEST_RESULT_FAILED)
			{
				if(TestFailedThisOperationCycle == 0)
				{
					switch(f)
					{
						case LTC6803_F_PEC:
							it->Category = dctCat_GeneralChecksumFailure;
							break;

						case LTC6803_F_REF_VOLT:
							it->Category = dctCat_SignalBiasLevelFailure;
							break;

						case LTC6803_F_OPEN_CONNECTION:
							it->Category = dctCat_CircuitOpen;
							break;
						case LTC6803_F_SIGNAL_INVALID:
							it->Category = dctCat_SignalInvalid;
							break;
						default:
							it->Category = dctCat_NoSubtypeInformation;
							break;
					}

					dtcSetFault(it, &env);
					SetGeneralFRZR(&frzfMeasuringCircuit);
				}
				OD.Faults.Mod_Measurenment = 1;
			}
		}
		else
		{
			if(dtcFaultDetection(it, &env, 0) == DTC_TEST_RESULT_PASSED)
			{
				OD.Faults.Mod_Measurenment = 0;
			}
		}
	}
	
	//������� PM
	it = &dtcPMOffline;
	if(++it->SamplePeriodCounter >= it->Property->TestSamplePeriod)
	{
		it->SamplePeriodCounter = 0;
		if(OD.SB.MsgFromPM == 0 && !ecuConfig->IsPowerManager)
		{
			TestFailedThisOperationCycle = it->Status.TestFailedThisOperationCycle;
			if(dtcFaultDetection(it, &env, 1) == DTC_TEST_RESULT_FAILED)
			{				
				if(TestFailedThisOperationCycle == 0)
				{
					dtcSetFault(it, &env);
					it->Category = 0;
					SetGeneralFRZR(&frzfPMOffline);				
					OD.Faults.Mod_PowerManagerOffline = 1;								
				}
			}
		}
		else
		{
			dtcFaultDetection(it, &env, 0);
			OD.Faults.Mod_PowerManagerOffline = 0;			
		}			
		
	}


	return env.CriticalFaultExist;
}


// Faul Handlers
uint8_t ContactorWeldingHandler(uint32_t *timeStamp)
{
	static uint8_t cnt = 0;

	// Every 100ms check contactor conditions
	if(GetTimeFrom(*timeStamp) >= 100 * cnt)
	{
		if(!(cnt & 0x01))
		{
			// If positive contactor is welded negative contactor
			if((FB_PLUS && !GET_BAT_PLUS) && !FB_MINUS)
			{
				BAT_PLUS(BAT_CLOSE);
			}
		}
		else
			BAT_PLUS(BAT_OPEN);

		if(!(cnt & 0x01))
		{
			if(FB_MINUS && !GET_BAT_MINUS && !FB_PLUS)
			{
				BAT_MINUS(BAT_CLOSE);
			}
		}
		else
			BAT_MINUS(BAT_OPEN);

		cnt++;
	}

	if(GetTimeFrom(*timeStamp) >= 1000)
	{
		BAT_PLUS(BAT_OPEN);
		BAT_MINUS(BAT_OPEN);
		return 1;
	}

	return 0;
}


void SetGeneralFRZR(dtcFRZF_General* f)
{
	f->DateTime = OD.SystemTime;
	if (f->OccurrenceCounter < 255)
		f->OccurrenceCounter++;
	
	f->Voltage = 284;
	f->Load = 80;
	f->AmbientTemperature = 22;
	f->Mileage = 32000;
	
//	f->Voltage = OD.Voltage;
//	f->Load = OD.Load;
//	f->AmbientTemperature = OD.AmbientTemperature;
//	f->Mileage = OD.Mileage;
}


void dtcSetFault(dtcItem_t* it, dtcEnvironment_t* env)
{
	// ��������� (Failed) �������� ������������ �����
	it->Status.TestFailed = 1;
	// ������������� ������������
	it->Status.ConfirmedDTC = 1;
	// � ���� ������� ����� ������������� ���� ��� ������� 1 ���
	it->Status.TestFailedThisOperationCycle = 1;
	// ���� ��� �������� � ���� ������� ����� ��� ����� ������� ������ ��� ������� 1 ���
	it->Status.TestNotCompletedThisOperationCycle = 0;	
	// ������������ ������������� ��� ���
	it->Status.WarningIndicatorRequested = it->Property->Bits.WarningIndicatorRequested;

	env->CriticalFaultExist |= it->Property->Bits.IsCritical;
	env->WarningIndicatorRequested |= it->Property->Bits.WarningIndicatorRequested;
}

uint8_t dtcFaultDetection(dtcItem_t* it, dtcEnvironment_t *env, uint8_t isFault)
{
	if(isFault)
	{
		if(it->FaultDetectionCounter < it->Property->TestFailedThreshold)
		{
			if(it->FaultDetectionCounter < 0)
				it->FaultDetectionCounter = 1;
			else
				it->FaultDetectionCounter++;
		}
		else
		{
			// ���� ��������: ���������� �������������
			if(!it->Status.TestFailed)
			{
				dtcSetFault(it, env);
				return DTC_TEST_RESULT_FAILED;
			}
		}		
	}
	else
	{
		if(it->FaultDetectionCounter > it->Property->TestPassedThreshold)
		{
			it->FaultDetectionCounter--;
		}
		else
		{
			// ���� ��������: ������������� ��� ��� �������
			if(it->Status.TestFailed || it->Status.TestNotCompletedThisOperationCycle)
			{
				// ��������� (Passed) �������� ������������ �����
				it->Status.TestFailed = 0;
				// ���� ��� �������� � ���� ������� ����� ��� ����� ������� ������ ��� ������� 1 ���
				it->Status.TestNotCompletedThisOperationCycle = 0;
				
				// ���� ���� ����������� ������������� ��������������� �����������������
				if(it->Property->Bits.AutoRestore)
					it->Status.TestFailedThisOperationCycle = 0;
					
				return DTC_TEST_RESULT_PASSED;
			}
		}
	}
	
	return DTC_TEST_RESULT_UNCOMPLITED;
}


uint8_t SaveFaults()
{
	// ��������� ������ DTC � EEPROM:
	// - ������� ���� ������ ���� ��������� �������������� (��. dtcList), ������ ������� �������� �������� ������ Category � Status.ConfirmedDTC
	// - ����� ���� ����������� ����� (�� CRC16)
	// - �� ��� � ��� �� ������������������ ���� ����-����� (��������� ������ ���� ConfirmedDTC)
	
	// ��������� �� ����������� ������
	uint8_t res = 0;
	dtcItem_t* dtc;
	uint16_t sum = 0;
	uint16_t addr = 0;
	
	for(int i = 0; i < dtcListSize; i++)
	{
		dtc = dtcList[i];
		uint8_t isConfirmed = dtc->Status.ConfirmedDTC;
		sum += dtc->Category + isConfirmed;
		
		*(pointer + addr++) = dtc->Category;
		*(pointer + addr++) = isConfirmed;
	}
	
	*(pointer + addr++) = sum;
	*(pointer + addr++) = sum >> 8;
	
	// ����-�����
//	const DiagnosticValueFRZF* f;
//	
//	for(uint8_t i = 0; i < dtcListSize; i++)
//	{
//		dtc = dtcList[i];
//		f = dtc->Property->FreezeFrameItems;
//		// ���������� � ��������� ������ �������� ����-�����
//		for(uint8_t n = 0; n < dtc->Property->FreezeFrameItemsCount; n++)
//		{
//			uint8_t* p = (uint8_t*)f->Ref;
//			for(uint8_t k = 0; k < f->Length; k++)
//			{
//				if(dtc->Status.ConfirmedDTC)
//					*(pointer + addr) = p[k];
//				addr++;
//			}
//			
//			// ���� ������� ������� ������������ ����������� �� 256 ����
//			if(addr > buf_size)
//			{
//				buf_size = buf_size + 256;
//				realloc(pointer, buf_size);
//			}			
//			
//			f++;	// ��������� ��������
//		}
//	}

	
	res = MemEcuDtcWrite(pointer, addr);
	
//	free(pointer);
	
	return res;
}

uint8_t ReadFaults()
{	
	dtcItem_t* dtc;
	uint16_t sum1 = 0, sum2;
	uint16_t addr = 0;
	
	MemEcuDtcRead(pointer, 512);
	
	for(uint16_t cnt = 0; cnt < 512; cnt++)
	{
		if(pointer[cnt] == 0xFF)
			pointer[cnt] = 0;
	}
	
	for(int i = 0; i < dtcListSize; i++)
	{
		dtc = dtcList[i];
		dtc->Category = *(pointer + addr++);
		uint8_t isConfirmed = *(pointer + addr++);
		dtc->Status.ConfirmedDTC = isConfirmed;
		sum1 += dtc->Category + isConfirmed;
	}
	
	sum2 = *(pointer + addr++);
	sum2 += (*(pointer + addr++) << 8);
	
	if(sum1 != sum2)
	{
		ClearFaults();
		return 0;
	}
//	
//	// ����-�����
//	const DiagnosticValueFRZF* f;
//	
//	for(int i = 0; i < dtcListSize; i++)
//	{
//		dtc = dtcList[i];
//		f = dtc->Property->FreezeFrameItems;
//		// ���������� � ��������� ������ �������� ����-�����
//		for(uint8_t n = 0; n < dtc->Property->FreezeFrameItemsCount; n++)
//		{
//			uint8_t* p = (uint8_t*)f->Ref;
//			for(uint8_t k = 0; k < f->Length; k++)
//			{
//				if(dtc->Status.ConfirmedDTC)
//					p[k] = *(pointer + addr);
//				addr++;
//			}
//			
//			f++;	// ��������� ��������
//		}
//	}
//	
//	free(pointer);
	return 1;
}

int8_t ClearFaults(void)
{
	uint16_t dtc_cnt = 0;	
	
	// Clear saved fault
	int8_t status = MemEcuDtcClear();	
	
	// Clear runtime faults
	for(uint8_t i = 0; i < dtcListSize; i++)
	{
		if(dtcList[i]->Status.ConfirmedDTC)
			dtc_cnt++;
		
		dtcList[i]->Status.ConfirmedDTC = 0;
		dtcList[i]->Status.TestFailed = 0;
		dtcList[i]->Status.TestFailedThisOperationCycle = 0;
		dtcList[i]->Status.TestNotCompletedThisOperationCycle = 0;
		dtcList[i]->Status.WarningIndicatorRequested = 0;		
	}	
	
	FillFaultsList(OD.OldFaultList, &OD.OldFaultsNumber, 0);
	FillFaultsList(OD.FaultList, &OD.FaultsNumber, 1);
	
	if(status == CMD_SUCCESS)	
		return dtc_cnt;
	else
		return -1;
}


