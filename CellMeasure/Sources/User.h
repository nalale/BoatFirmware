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
uint8_t GetBalancingPermission(int16_t TotalCurrent);
void LedStatus(uint8_t ContFB, uint32_t Balancing);
uint8_t ContactorClose(uint8_t num);
void ECU_GoToSleep(void);
void ECU_GoToPowerSupply(void);


uint8_t ModuleIsTerminal(const EcuConfig_t *config);
uint8_t ModuleSetContactorPosition(const BatteryData_t* Handle, stBattery_e StateCmd, const EcuConfig_t *config, uint32_t *CompleteConditionTimeStamp);


uint8_t BatteryCapacityCalculating(BatteryData_t* Handle, const EcuConfig_t *config, uint8_t StartMeasuringPermission);
uint8_t BatteryIsReady(const BatteryData_t* Handle, const BatteryData_t *ModulesData, const EcuConfig_t *config, uint32_t *ReadyConditionTimeStamp);


uint8_t MasterIsReady(const BatteryData_t* Handle, const BatteryData_t *BatteriesData, const EcuConfig_t *config, uint32_t *ReadyConditionTimeStamp);

#endif
