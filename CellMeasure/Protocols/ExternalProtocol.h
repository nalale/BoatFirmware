#ifndef _EXT_PROTOCOL_H
#define _EXT_PROTOCOL_H


#include "CanFunc.h"
#include "Protocol.h"

uint8_t ExternalRx(CanMsg *Msg);
void ExternalMesGenerate(void);

uint8_t packMsgHandler(CanMsg *Msg);
void SlaveMesGenerate(void);

uint8_t MasterMsgHandler(CanMsg *Msg);
void MasterMesGenerate(void);

uint8_t moduleMsgHandler(CanMsg *msg);
void ModuleMesGenerate(void);

#endif
