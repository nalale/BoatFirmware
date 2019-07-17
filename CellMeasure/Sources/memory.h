#ifndef _MEMORY_
#define _MEMORY_


#include "Main.h"

/**************************************************************************************************
									Прототипы функций
**************************************************************************************************/
void SaveDTC(void);
uint8_t ReadDTC(void);
uint8_t SaveProfile (const PROFILE * param);
uint8_t RecallProfile (PROFILE* param);
uint16_t CRC16(const uint8_t * buf, uint16_t len);
void CheckProfile(PROFILE* param);
uint8_t ecuClearAllDTC(void);

#endif
