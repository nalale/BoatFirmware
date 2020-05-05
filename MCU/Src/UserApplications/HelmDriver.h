#ifndef _HELM_DRIVER_H
#define _HELM_DRIVER_H

#include <stdint.h>

typedef struct
{
    uint32_t C_CAN_ID_OFFSET;
    uint8_t C_CAN_ID_NUM;
    uint8_t C_CAN_SPEED;
}MSG_SET_CAN_BASE_ADDRESS;

typedef struct
{
    uint32_t C_CAN_CONF_ID_OFFSET;
    uint8_t C_CAN_CONF_ID_NUM;
    uint8_t C_CAN_CONF_SPEED;
}MSG_CONFIGURE_CAN_BASE_ADDRESS;

typedef struct
{
    uint8_t C_ANGLE_RATE_LOW;
}MSG_SET_UPDATE_SPEED;

typedef struct
{
    uint16_t C_ZERO_POS;
    int8_t C_REV_NUMBER;
}MSG_SET_ORIGIN;

typedef struct
{
    uint8_t C_ABS_ANGLE_HIGH;
    uint8_t C_ABS_ANGLE_LOW;
    int8_t C_NUM_REV;
}MSG_SET_STEERING_ENDSTOP;

typedef struct
{
    uint8_t C_PERC_MAX_BRAKE_FRICTION;
}MSG_SET_REGULATED_BRAKE_FRICTION;

typedef struct
{
    uint8_t C_PERC_MAX_BRAKE_CURRENT;
}MSG_SET_ZERO_REGULATED_BRAKE_FRICTION;


typedef struct
{
    uint8_t C_SPEED_DEP;
    uint8_t C_GAIN_FACTOR;
    uint8_t dummy1;
    uint16_t dummy2;
    
}MSG_SET_SPEED_DEPENDENT_FRICTION_BEHAVIOR;

typedef struct
{
    uint8_t C_MAX_FRICTION;
    uint8_t C_ANGLE_POS;
}MSG_SET_ESIU_SETTINGS;

/* **************************************************************************
 ******************** Received Messages ***********************************
 **************************************************************************/

typedef struct
{
    uint16_t C_ANGLE;
    
    uint8_t
    C_BRAKE_STS                     : 2,
    C_SET_ORIGIN_STS                : 2,
    C_SET_ENDSTOP_STS               : 2,
    C_SET_SPEED_DEP_FRICTION_STS    : 2;
    
    uint8_t C_CURRENT_BRAKE;
    uint8_t C_HEART_COUNTER;
    uint8_t C_STEERING_SPEED_LOW;
    uint8_t C_STEERING_SPEED_HIG;
    int8_t C_NUM_STEERING_REV;
}MSG_ABS_ANGLE;

typedef struct
{
    uint8_t 
    C_ERROR_STATE   : 2,
    dummy1          : 6;
    
    uint8_t C_ERROR_CODE;
    uint8_t C_ERROR_SEVERITY;
    uint8_t C_OCCURRENCE;
    uint8_t C_POWER_SUPPLY_HIGH;
    uint8_t C_POWER_SUPPLY_LOW;
    uint8_t C_PCB_TEMP_HIGH;
    uint8_t C_PCB_TEMP_LOW;
}MSG_ERROR_CODE;

typedef enum{
	MSG_ABS_ANGLE_VALUE = 0x7d,
	CAN_MSG_ERROR_CODE = 0x7e,
	MSG_ESIU_RESP = 0xac,
	MSG_SET_REGULATED_BRAKE_POS = 0xe5,
	MSG_SET_STEERING_ENDSTOP_RIGHT = 0xe9,
	MSG_SET_STEERING_ENDSTOP_LEFT = 0xea,
	MSG_SET_ZERO_BRAKE_POS = 0xec,

	MSG_HELM_ERROR_ID = -1,
} HelmParameterTypes_e;

uint8_t helmInit(int16_t DefaultCounterForce, const int16_t* ForceSpeedTable, int8_t TableSize);
uint16_t HelmGetTargetAngle(void);
uint8_t helmThread(int16_t ActualSpeed);
uint8_t HelmGetStatus(void);
uint8_t HelmGetCausedFault(void);
// Reset online sign after request
uint8_t helmGetOnlineSign(void);

HelmParameterTypes_e HelmMessageGenerate(uint8_t *MsgData, uint8_t *MsgDataLen, int32_t MsCounter);
uint8_t HelmMessageHandler(HelmParameterTypes_e MsgID, uint8_t *MsgData, uint8_t MsgDataLen);

#endif
