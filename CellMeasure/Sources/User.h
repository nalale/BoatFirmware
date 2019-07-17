#ifndef _USER_H_
#define _USER_H_

#include "lpc17xx_gpio.h"
#include "Main.h"

void AppInit(ObjectDictionary_t *dictionary);
uint8_t FaultsTest(void);
void ApplyConfig(void);

void ControlBatteriesState(WorkStates_e *CurrentState, WorkStates_e RequestState);
uint8_t GetBalancingPermission(int16_t TotalCurrent);
void LedStatus(uint8_t ContFB, uint32_t Balancing);
uint8_t ContactorClose(uint8_t num);
void ECU_GoToSleep(void);
void ECU_GoToPowerSupply(void);
#endif
