#ifndef _USER_H_
#define _USER_H_

#include "lpc17xx_gpio.h"
#include "Main.h"

typedef enum
{
	stBat_Disabled,
	stBat_Precharging,
	stBat_Enabled,
} stBattery_e;

void AppInit(ObjectDictionary_t *dictionary);
uint8_t FaultsTest(void);
void ApplyConfig(void);

void ControlBatteriesState(WorkStates_e *CurrentState, WorkStates_e RequestState);

void LedStatus(uint8_t ContFB, uint32_t Balancing);
uint8_t ContactorClose(uint8_t num);
void ECU_GoToSleep(void);
void ECU_GoToPowerSupply(void);


uint8_t ModuleIsPackHeader(const EcuConfig_t *config);
uint8_t ModuleSetContactorPosition(const BatteryData_t* Handle, stBattery_e StateCmd, const EcuConfig_t *config, uint32_t *CompleteConditionTimeStamp);

uint8_t packGetBalancingPermission(const BatteryData_t *PackHandle, const PackControl_t *PackControl, const EcuConfig_t *ecuConfig);
uint16_t packGetBalancingVoltage(const BatteryData_t *PackHandle, const PackControl_t *PackControl, const EcuConfig_t *ecuConfig);
uint8_t packCapacityCalculating(BatteryData_t* Handle, const EcuConfig_t *config, uint8_t StartMeasuringPermission);
uint8_t packIsReady(const BatteryData_t* Handle, const BatteryData_t *ModulesData, const EcuConfig_t *config, uint32_t *ReadyConditionTimeStamp);


uint8_t MasterIsReady(const BatteryData_t* Handle, const BatteryData_t *BatteriesData, const EcuConfig_t *config, uint32_t *ReadyConditionTimeStamp);


int8_t flashWriteSData(const StorageData_t *sdata);
// Return 0 if success, otherwise return nonzero
int8_t flashReadSData(StorageData_t *sdata);

int8_t flashStoreData(StorageData_t *sdata);
int8_t flashClearFaults(StorageData_t *sdata);

#endif
