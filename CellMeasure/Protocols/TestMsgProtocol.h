/*
 * TestMsgProtocol.h
 *
 *  Created on: 1 ���. 2020 �.
 *      Author: a.lazko
 */

#ifndef PROTOCOLS_TESTMSGPROTOCOL_H_
#define PROTOCOLS_TESTMSGPROTOCOL_H_

typedef struct
{
	uint16_t Cell1_V;
	uint16_t Cell2_V;
	uint16_t Cell3_V;
	uint16_t Cell4_V;
} TestMsg_t;

typedef struct
{
	int32_t CurrentCycleEnerge_As;
} TestEnergyMsg_t;

void TestMesGenerate(void);

#endif /* PROTOCOLS_TESTMSGPROTOCOL_H_ */
