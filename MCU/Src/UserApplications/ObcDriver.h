/*
 * ObcDriver.h
 *
 *  Created on: 4 мая 2020 г.
 *      Author: a.lazko
 */

#ifndef SRC_USERAPPLICATIONS_OBCDRIVER_H_
#define SRC_USERAPPLICATIONS_OBCDRIVER_H_

#include <stdint.h>

#pragma anon_unions

typedef enum {
	st_OBC_DISABLE,
	st_OBC_ENABLE,
	st_OBC_STOP,
	st_OBC_FAULT,
} ObcStatus_e;

typedef enum{
	prm_OBC_CURRENT_REQ = 0,

	prm_OBC_NUMBER,
}ObcParameters_e;

typedef enum {
	MSG_TARGET_VALUE = 0x1806E5F4,
	MSG_OBC_STATUS_VALUE = 0x18FF50E5,
	
	MSG_OBC_ERROR_ID = -1,

} ObcParameterTypes_e;

typedef struct
{
	int16_t OutputVoltage;	// scale = 0.1
	int16_t OutputCurrent;	// scale = 0.1

	union
	{
		uint8_t status;
		struct
		{
			uint8_t
			HardwareFailure 	:1,
			TempOfCharger 		:1,
			InputVoltage 		:1,
			StartingStage 		:1,
			Communication 		:1;
		};
	};

	int8_t Temp;
	int16_t Reserved;
} MSG_OBC_STATUS_t;

typedef struct{
	uint16_t TargetVoltage_0p1A;
	uint16_t TargetCurrent_0p1A;
} MSG_SET_CHARGING_DATA_t;

typedef struct{

	ObcStatus_e Status;
	uint16_t RequestCurrent;
	uint16_t TargetVoltage;
	uint16_t BatteryCCL;
	uint16_t PlugMaxCurrent;

	uint16_t OutputVoltage;
	uint16_t OutputCurrent;
	uint8_t FaultsList;
	uint8_t OnlineUnits;

	uint32_t CausedFault;
	uint8_t UnitMaxCurrent;
	uint8_t UnitNumber;
	uint8_t
		OnlineSign 	: 	1,
		IsInit		:	1,
		TurnOnReq	:	1,
		dummy1		:	5;

	ObcParameters_e PreparedMsgNumber;
	int32_t PreparedMsgTimestamp[prm_OBC_NUMBER];

	MSG_OBC_STATUS_t MsgRxBuf;
} TcChargerHKJ_t;


uint8_t obcInit(int8_t ChargerCnt, uint16_t PlugMaxCurrent, uint16_t BatteryMaxVoltage);
uint8_t obcThread(void);

uint8_t obcSetState(uint8_t state);
uint8_t obcSetCurrentLimit(uint16_t BatteryCurrentLimit);

ObcStatus_e obcGetStatus(void);
uint16_t obcGetDemandCurrent(void);
uint16_t obcGetOutputCurrent(void);
uint8_t obcGetOnlineSign(void);
uint8_t obcGetOnlineUnitsNumber(void);

uint8_t obcMessageHandler(ObcParameterTypes_e MsgID, uint8_t *MsgData, uint8_t MsgDataLen);
ObcParameterTypes_e obcMessageGenerate(uint8_t *MsgData, uint8_t *MsgDataLen, int32_t MsCounter);


#endif /* SRC_USERAPPLICATIONS_OBCDRIVER_H_ */
