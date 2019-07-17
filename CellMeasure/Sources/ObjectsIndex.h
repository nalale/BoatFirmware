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
	
	// ��������� ������ �������
	didModMachineState,
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
	
	// Battery parameters
	didBatTotalCurrent,
	didBatTotalVoltage,
	didBatMaxVoltage,
	didBatMinVoltage,
	didBatMaxTemperature,
	didBatMinTemperature,
	didBatMaxCellVoltage,
	didBatMinCellVoltage,
	didBatStateOfCharge,
	didBatEnergy,
	didBatCCL,
	didBatDCL,
	
	// Master Parameters
	didMstTotalCurrent,
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
	didMstCCL,
	didMstDCL,
	
	// ����������� ������
	didFaults_Actual = 100,
	didFaults_History,
	
	didFaults_FreezeFrame,
	
} ObjectsIndex_e;










#endif

