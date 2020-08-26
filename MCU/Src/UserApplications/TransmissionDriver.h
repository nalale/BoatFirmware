/*
 * TransmissionDriver.h
 *
 *  Created on: 16 мая 2020 г.
 *      Author: a.lazko
 */

#ifndef SRC_USERAPPLICATIONS_TRANSMISSIONDRIVER_H_
#define SRC_USERAPPLICATIONS_TRANSMISSIONDRIVER_H_

typedef enum {
	GEAR_NEU = 0,
	GEAR_FW,
	GEAR_BW,
} TransmissionGear_e;

typedef struct{
	TransmissionGear_e ActualGear;
	TransmissionGear_e DemandGear;
	uint8_t IsShifting;
	int16_t SpeedForShifting;
	uint8_t ActuatorPosition;
	uint8_t ActuatorPositionPrev;
	uint32_t Timer;
	void (*CmdCallBack)(TransmissionGear_e gear);

} TransmissionHandle_t;

int8_t transmissionInit(uint16_t SpeedForShifting, void (*ShiftCommandCallBack)(TransmissionGear_e DemandGear));
int8_t transmissionThread(int16_t ShaftSpeed);
int8_t transmissionSetGear(TransmissionGear_e gear);
int8_t transmissionMovementPermission(void);
TransmissionGear_e transmissionGetActualGear(void);
int8_t transmissionShiftInProcess(void);
int8_t transmissionSetEndSwithPosition(uint8_t Position);

#endif /* SRC_USERAPPLICATIONS_TRANSMISSIONDRIVER_H_ */
