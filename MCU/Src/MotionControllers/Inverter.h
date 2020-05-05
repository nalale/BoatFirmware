#ifndef _INVERTER_H_
#define _INVERTER_H_


typedef enum
{
	VSM_StartState = 0,
	VSM_PreChargeInitState,
	VSM_PreChargeActiveState,
	VSM_PreChargeCompleteState,
	VSM_WaitState,
	VSM_ReadyState,
	VSM_MotorRunningState,
	VSM_FaultState,
} InverterStates_e;















#endif

