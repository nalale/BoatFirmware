#ifndef _MAX_11612_H_
#define _MAX_11612_H_

#include <stdint.h>

#pragma anon_unions
/** Own Slave address in Slave I2C device */
#define I2CDEV_S_ADDR	0x34


void Max11612_Init(void);
void Max11612_StartConversion(void);
void Max11612_GetResult(uint8_t *Buf, uint8_t V_ref);
void Max11612_SetData(void);
void Max11612_ClearData(void);
// Return value: 0 - response is received, 1 - request is sended
uint8_t GetMax11612State(void);
#endif

