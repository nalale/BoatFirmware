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
	
	didCurrentSensor1,
	didCurrentSensor2,
	didCurrentSensor3,
	didCurrentSensor4,
	
	didPwmOutState,
	
	// Диагностика ошибок
	didFaults_Actual = 100,
	didFaults_History,
	
	didFaults_FreezeFrame,
	
} ObjectsIndex_e;










#endif

