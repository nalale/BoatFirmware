/*
 * Extarnal_CAN.h
 *
 *  Created on: 22 мая 2020 г.
 *      Author: a.lazko
 */

#ifndef SRC_EXTARNAL_CAN_H_
#define SRC_EXTARNAL_CAN_H_

#pragma pack(1)

typedef struct
{
	uint8_t stBms_StateOfCharge;
	int16_t stBms_TotalCurrent;
	int16_t stBms_TotalVoltage;
} J_CAN_BmsMsg1_t;

typedef struct
{
	uint16_t stBms_SystemEnergy;
	uint16_t stBms_TotalEnergy;
	uint16_t MaxCellVoltage;
	uint16_t MinCellVoltage;
} J_CAN_BmsMsg2_t;

typedef struct
{
	int16_t stBms_CurrentPack1;
	int16_t stBms_CurrentPack2;
	uint16_t stBms_VoltagePack1;
	uint16_t stBms_VoltagePack2;
} J_CAN_BmsMsg3_t;

typedef struct
{
	int8_t stInverter_BoardTemperature;
	int8_t stInverter_MotorTemperature;
	uint16_t stInverter_ShaftSpeed;
	uint16_t stInverter_ShaftTorque;
} J_CAN_InverterMsg1_t;

typedef struct
{
	uint8_t stSystem_Velocity;
	uint16_t stSystem_Isolation;
	int8_t stSystem_Throttle;
	int8_t stSystem_Helm;
	int8_t stSystem_Steering;
} J_CAN_SystemMsg1;

#pragma pack(2)

uint8_t ExternalRx(CanMsg * msg);
void ExternalMesGenerate(void);



#endif /* SRC_EXTARNAL_CAN_H_ */
