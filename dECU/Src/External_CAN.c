/*
 * External_CAN.c
 *
 *  Created on: 22 мая 2020 г.
 *      Author: a.lazko
 */


#include "main.h"
#include "TimerFunc.h"
#include "user.h"
#include "protocol.h"
#include "Extarnal_CAN.h"

#include <string.h>


#define LOCAL_SEND_MSG_AMOUNT	5


// Таймеры
uint32_t MsgSendStamp[LOCAL_SEND_MSG_AMOUNT];
uint32_t MsgSendPeriod[LOCAL_SEND_MSG_AMOUNT] =  {
														250,
														250,
														250,
														250,
														250,
													};



// Обработка входящих сообщений
uint8_t ExternalRx(CanMsg * msg)
{
	EcuConfig_t config = GetConfigInstance();

	return 1;
}


void ExternalMesGenerate(void)
{
	static uint8_t msgNumber = 0;	

	CanMsg* msg;

	if(++msgNumber >= LOCAL_SEND_MSG_AMOUNT)
		msgNumber = 0;

	// Генерация сообщений
	if (GetTimeFrom(MsgSendStamp[msgNumber]) >= MsgSendPeriod[msgNumber])
	{
		msg = ecanGetEmptyTxMsg(D_CAN_CH);
		if(msg == 0)
			return;

		msg->Ext = 0;
		MsgSendStamp[msgNumber] = GetTimeStamp();

		switch (msgNumber)
		{
			case 0:
			{
				J_CAN_BmsMsg1_t *d = (J_CAN_BmsMsg1_t*)msg->data;
				msg->DLC = 8;
				msg->Ext = 1;
				msg->ID = 0x18ff10e1;
				d->stBms_StateOfCharge = OD.BmuData1.SOC;
				d->stBms_TotalCurrent = OD.BmuData1.TotalCurrent_0p1A / 10;
				d->stBms_TotalVoltage = OD.BmuData1.TotalVoltage_0p1V / 10;
			}
			break;
			case 1:
			{
				J_CAN_BmsMsg2_t *d = (J_CAN_BmsMsg2_t*)msg->data;
				msg->DLC = 8;
				msg->Ext = 1;
				msg->ID = 0x18ff11e1;

				d->stBms_SystemEnergy = OD.BmuData4.SystemActualEnergy_Ah * 10;
				d->stBms_TotalEnergy = OD.BmuData4.SystemTotalEnergy_Ah * 10;
				d->MaxCellVoltage = OD.BmuData2.MaxCellVoltage_mV;
				d->MinCellVoltage = OD.BmuData2.MinCellVoltage_mV;
			}
			break;
			case 2:
			{
				J_CAN_BmsMsg3_t *d = (J_CAN_BmsMsg3_t*)msg->data;
				msg->DLC = 8;
				msg->Ext = 1;
				msg->ID = 0x18ff12e1;

				d->stBms_CurrentPack1 = OD.PackData1.BatTotalCurrent_0p1A / 10;
				d->stBms_CurrentPack2 = OD.PackData2.BatTotalCurrent_0p1A / 10;
				d->stBms_VoltagePack1 = OD.PackData1.BatTotalVoltage_0p1A / 10;
				d->stBms_VoltagePack2 = OD.PackData1.BatTotalVoltage_0p1A / 10;
			}
			break;
			case 3:
			{
				J_CAN_InverterMsg1_t *d = (J_CAN_InverterMsg1_t*)msg->data;
				msg->DLC = 8;
				msg->Ext = 1;
				msg->ID = 0x18ff15e1;

				d->stInverter_BoardTemperature = (int8_t)OD.MainEcuData1.InverterTemperature - 40;
				d->stInverter_MotorTemperature = (int8_t)OD.MainEcuData1.MotorTemperature - 40;
				d->stInverter_ShaftSpeed = OD.MainEcuData1.MotorRpm;
				d->stInverter_ShaftTorque = OD.MainEcuData1.TargetTorque;
			}
			break;
			case 4:
			{
				J_CAN_SystemMsg1 *d = (J_CAN_SystemMsg1*)msg->data;
				msg->DLC = 8;
				msg->Ext = 1;
				msg->ID = 0x18ff18e1;

				d->stSystem_Isolation = OD.Isolation;
				d->stSystem_Velocity = (uint8_t)OD.GpsData1.Velocity_Kmph;
				d->stSystem_Throttle = OD.MainEcuData2.ThrottlePos;
				d->stSystem_Helm = OD.MainEcuData2.HelmAngle;
				d->stSystem_Steering = OD.MainEcuData2.SteeringAngle;
			}
			break;
		}
	}
}

