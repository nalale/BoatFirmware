#ifndef _FAULT_TOOLS_H_
#define _FAULT_TOOLS_H_

#include <stdint.h>




typedef enum
{
	dtc_General_EcuConfig = 0,
	dtc_General_EcuSupplyOutOfRange,
	dtc_General_UnexpectedPowerOff,
	dtc_General_Interlock,
	dtc_General_EcuDataTimeNotCorrect,	
	
	// Таймаут CAN
	dtc_CAN_Bms = 10,
	dtc_CAN_generalVcu,
	dtc_CAN_mainVcu,	
	dtc_CAN_PM,	
	
	dtc_Mod_Contactor = 20,		
	dtc_Mod_CellTempOutOfRange,
	dtc_Mod_MeasuringCircuit,
	
	dtc_Bat_WrongModNumber = 32,
	dtc_Bat_WrongModState,
	dtc_Bat_OverCurrent,
	dtc_Bat_ModVoltageDiff,
	dtc_Bat_Precharge,
	dtc_Bat_CurrentSensor,
	
	dtc_Mst_WrongBatNumber = 44,
	dtc_Mst_WrongBatState,
	dtc_Mst_BatVoltageDiff,
	dtc_Mst_CellVoltageOutOfRange,
	
} dtCodes_e;




uint8_t FaultsTest(void);






#endif
