/*
 * TestMsgProtocol.c
 *
 *  Created on: 1 июн. 2020 г.
 *      Author: a.lazko
 */
#include "Main.h"
#include "protocol.h"

#include "TimerFunc.h"
#include "CanFunc.h"

#include "../MolniaLib/DateTime.h"

#include "TestMsgProtocol.h"

static const uint8_t msg_cnt = 7;

static uint32_t extSendTime[msg_cnt];
// Периоды отправки сообщений
static const uint16_t extPeriod[msg_cnt] = {
								100,
								100,
								100,
								100,
								100,
								100,
								100,
								 };


void TestMesGenerate(void)
{
    // Номер отправляемого сообщения
	static uint8_t extSendMesNumber = 0;
    CanMsg *msg;

	EcuConfig_t _config = GetConfigInstance();

    // Генерация сообщений
	if (++extSendMesNumber >= msg_cnt)
	{
		extSendMesNumber = 0;
	}

	if (GetTimeFrom(extSendTime[extSendMesNumber]) > extPeriod[extSendMesNumber])
	{
         msg = ecanGetEmptyTxMsg(P_CAN_CH);

        if(msg == 0)
            return;

        extSendTime[extSendMesNumber] = GetTimeStamp();

		msg->ID = 0x18ffa000 | (extSendMesNumber << 8) | (OD.ConfigData->BatteryIndex << 4) + OD.ConfigData->ModuleIndex;
		msg->DLC = 8;
		msg->Ext = 1;

		switch (extSendMesNumber)
		{
            case 0:
			{
				TestMsg_t* d = (TestMsg_t*)msg->data;
				d->Cell1_V = OD.CellVoltageArray_mV[0];
				d->Cell2_V = OD.CellVoltageArray_mV[1];
				d->Cell3_V = OD.CellVoltageArray_mV[2];
				d->Cell4_V = OD.CellVoltageArray_mV[3];
			}
            break;
            case 1:
			{
				TestMsg_t* d = (TestMsg_t*)msg->data;
				d->Cell1_V = OD.CellVoltageArray_mV[4];
				d->Cell2_V = OD.CellVoltageArray_mV[5];
				d->Cell3_V = OD.CellVoltageArray_mV[6];
				d->Cell4_V = OD.CellVoltageArray_mV[7];
			}
			break;
            case 2:
			{
				TestMsg_t* d = (TestMsg_t*)msg->data;
				d->Cell1_V = OD.CellVoltageArray_mV[8];
				d->Cell2_V = OD.CellVoltageArray_mV[9];
				d->Cell3_V = OD.CellVoltageArray_mV[10];
				d->Cell4_V = OD.CellVoltageArray_mV[11];
			}
			break;
            case 3:
			{
				TestMsg_t* d = (TestMsg_t*)msg->data;
				d->Cell1_V = OD.CellVoltageArray_mV[12];
				d->Cell2_V = OD.CellVoltageArray_mV[13];
				d->Cell3_V = OD.CellVoltageArray_mV[14];
				d->Cell4_V = OD.CellVoltageArray_mV[15];
			}
			break;
            case 4:
			{
				TestMsg_t* d = (TestMsg_t*)msg->data;
				d->Cell1_V = OD.CellVoltageArray_mV[16];
				d->Cell2_V = OD.CellVoltageArray_mV[17];
				d->Cell3_V = OD.CellVoltageArray_mV[18];
				d->Cell4_V = OD.CellVoltageArray_mV[19];
			}
			break;
            case 5:
			{
				TestMsg_t* d = (TestMsg_t*)msg->data;
				d->Cell1_V = OD.CellVoltageArray_mV[20];
				d->Cell2_V = OD.CellVoltageArray_mV[21];
				d->Cell3_V = OD.CellVoltageArray_mV[22];
				d->Cell4_V = OD.CellVoltageArray_mV[23];
			}
			break;
            case 6:
            {
            	TestEnergyMsg_t* d = (TestEnergyMsg_t*)msg->data;
            	d->CurrentCycleEnerge_As = OD.ModuleData[OD.ConfigData->ModuleIndex].CurrentCycleEnergy_As;
            }
            break;
        }

    }

    return;
}
