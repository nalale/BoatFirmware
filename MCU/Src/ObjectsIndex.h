#ifndef _OBJECTS_ID_H_
#define _OBJECTS_ID_H_




typedef enum
{
	didEcuInfo	= 0,
	didConfigStructIndex,
	didDateTime,
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
	
	didMachineState,
	didMachineSubState,
	
	didInOutState,
	didPwmOutState,
	
	didHelmDemandAngle,
	didHelmStatus,
	didSteeringFB,
	didSteeringCurent,
	didTrimFB,	
	didAccCh1,
	didAccCh2,
	didMotorTemp,
	didInvTemp,
	didActualRpm,
	didInverterCurrent,
	didBatteryCurrent,
	
	didTrimMovCmd,
	didActualGear,
	didTargetSpeed,
	didAccPosition,
	didInverterVoltage,
	
	didBatteryCmd,
	
	didInverterEnable,
	didInverterState,
	didWaterSwitches,
	
	didSteeringFBAngle,
	
	didIsoIsolation,
	
	// Диагностика ошибок
	didFaults_Actual = 100,
	didFaults_History,
	
	didFaults_FreezeFrame,
	
} ObjectsIndex_e;










#endif

