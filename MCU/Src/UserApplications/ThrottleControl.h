/*
 * DriveControl.h
 *
 *  Created on: 5 мая 2020 г.
 *      Author: a.lazko
 */

#ifndef SRC_USERAPPLICATIONS_THROTTLECONTROL_H_
#define SRC_USERAPPLICATIONS_THROTTLECONTROL_H_

#include <stdint.h>

typedef enum {
	dr_NonInit = 0,
	dr_OK,
	dr_Fault,
} driveAccUnitStatuses_e;

typedef enum {
	dr_NeuDirect,
	dr_FwDirect,
	dr_BwDirect,
} driveDemandDirection_e;

void thrHandleControlInit(uint8_t NeuV, uint8_t MinV, const uint8_t* Table, const uint8_t* FstChannelSrc, const uint8_t* SndChannelSrc);
driveDemandDirection_e thrHandleGetDirection(void);
uint8_t thrHandleGetDemandAcceleration(void);
driveAccUnitStatuses_e thrHandleGetStatus(void);
uint8_t thrHandleControlThread(int16_t MotorSpeed);

#endif /* SRC_USERAPPLICATIONS_THROTTLECONTROL_H_ */
