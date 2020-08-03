#ifndef _OBJECTS_ID_H_
#define _OBJECTS_ID_H_




typedef enum
{
	didEcuInfo	= 0,
	didConfigStructIndex,
	didDateTime,
	didMachineState,
	didMachineSubState,
	didInOutState,
	didEcuVoltage,
	didPowerManagmentState,

	didVoltageSensor1 = 10,
	didVoltageSensor2,
	didVoltageSensor3,
	didVoltageSensor4,
	didVoltageSensor5,
	didVoltageSensor6,
	didVoltageSensor7,
	didVoltageSensor8,
	
	// Параметры модуля батареи
	didModMachineState = 30,
	didModMachineSubState,
	didCellsVoltages,
	didModuleTemperatures,		
	didModTotalVoltage,
	didModMaxTemperature,
	didModMinTemperature,
	didModMaxCellVoltage,
	didModMinCellVoltage,
	didModDischargeCellsFlag,
	didModInOutState,
	didModStateOfCharge,
	didModEnergy,
	didModTotalEnergy,
	didModTotalCurrent,
	didModCurrentCycleEnergy,
	
	// Battery parameters
	didBatTotalCurrent = 50,
	didBatTotalVoltage,
	didBatMaxVoltage,
	didBatMinVoltage,
	didBatMaxTemperature,
	didBatMinTemperature,
	didBatMaxCellVoltage,
	didBatMinCellVoltage,
	didBatStateOfCharge,
	didBatEnergy,
	didBatTotalEnergy,
	didBatCCL,
	didBatDCL,
	didBatLastPrechargeDuration,
	didBatLastPrechargeCurrent,
	didBatModulesOnlineFlag,
	
	// Master Parameters
	didMstTotalCurrent = 70,
	didMstTotalVoltage,
	didMstMaxCurrent,
	didMstMinCurrent,
	didMstMaxVoltage,
	didMstMinVoltage,
	didMstMaxTemperature,
	didMstMinTemperature,
	didMstCellMaxVoltage,
	didMstCellMinVoltage,
	didMstStateOfCharge,
	didMstEnergy,
	didMsgTotalEnergy,
	didMstCCL,
	didMstDCL,
	didMstPacksOnlineFlag,
	
	// Диагностика ошибок
	didFaults_Actual = 100,
	didFaults_History,
	
	didFaults_FreezeFrame,
	
	didFlashData = 105,

} ObjectsIndex_e;










#endif

