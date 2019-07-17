#ifndef _WORKSTATES_H_
#define _WORKSTATES_H_

#include "Main.h"



void SetWorkState(StateMachine_t *state_machine, WorkStates_e new_state);

void InitializationState(uint8_t *SubState);
void OperatingState(uint8_t *SubState);
void ShutdownState(uint8_t *SubState);
void FaultState(uint8_t *SubState);
void ChargingState(uint8_t *SubState);
void CommonState(void);

#endif
