#ifndef _USER_H_
#define _USER_H_

#include "lpc17xx_gpio.h"
#include "Main.h"
#include "../BoardDefinitions/MarineEcu_Board.h"
#include "../Libs/max11612.h"

void AppInit(ObjectDictionary_t *dictionary);
uint8_t CheckTestingState(const WorkStates_e MainState, const uint8_t IsExternalControlReceived);

void AccPedalInit(uint8_t MaxV, uint8_t NeuV);
int8_t GetDriveDirection(uint8_t sensor_voltage_0p1, uint8_t sensor2_voltage_0p1);
uint8_t GetAccelerationPosition(uint8_t sensor_voltage_0p1, uint8_t sensor2_voltage_0p1);
int16_t GetTargetSpeed(uint8_t AccPosition, int8_t DriveDir);
int16_t GetTargetTorque(uint8_t AccPosition, int8_t GearSelected);
uint8_t InverterControl(uint8_t LockupEnable, uint8_t BatContactorClose, uint8_t IsTimeout, uint8_t AccPosition);

uint8_t TrimProc(TrimData_t *trim);
uint8_t CheckChargingCond(uint8_t TerminalState, uint8_t BatteryState);
void InverterPower(uint8_t Cmd);
void SteeringPumpPower(uint8_t Cmd);
uint8_t SystemThreat(void);
#endif
