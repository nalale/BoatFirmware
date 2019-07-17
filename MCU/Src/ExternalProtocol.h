#ifndef _EXT_PROTOCOL_H
#define _EXT_PROTOCOL_H


#include "CanFunc.h"
#include "Protocol.h"


uint8_t ExternalRx(CanMsg *Msg);
void ExternalMesGenerate(void);

uint8_t SendSteeringStartupMsg(void);





#endif
