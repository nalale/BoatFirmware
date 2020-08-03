#ifndef _USER_H_
#define _USER_H_

#include "lpc17xx_gpio.h"
#include "Main.h"
#include "../BoardDefinitions/MarineEcu_Board.h"
#include "../Libs/max11612.h"

void AppInit(ObjectDictionary_t *dictionary);
uint8_t CheckTestingState(const WorkStates_e MainState, const uint8_t IsExternalControlReceived);


uint8_t CheckChargingCond(uint8_t TerminalState, uint8_t BatteryState);
void InverterPower(uint8_t Cmd);
void SteeringPumpPower(uint8_t Cmd);
uint8_t SystemThreat(ObjectDictionary_t* OD);

void InverterPower(uint8_t Cmd);
void SteeringPumpPower(uint8_t Cmd);
uint8_t ChargersCircuitOn(uint8_t Cmd);
void TransmissionSetGear(TransmissionGear_e Cmd);

uint8_t DrainSupply(void);

void LedBlink(void);

int8_t flashReadSData(StorageData_t *sdata);
int8_t flashClearFaults(StorageData_t *sdata);
int8_t flashStoreData(StorageData_t *sdata);
int8_t flashClearData(StorageData_t *sdata);

#endif
