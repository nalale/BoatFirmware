#ifndef _EXT_PROTOCOL_H
#define _EXT_PROTOCOL_H


#include "CanFunc.h"
#include "Protocol.h"

uint8_t ExternalRx(CanMsg *Msg);
void ExternalMesGenerate(void);

uint8_t SlaveRx(CanMsg *Msg);
void SlaveMesGenerate(void);

uint8_t MasterRx(CanMsg *Msg);
void MasterMesGenerate(void);

uint8_t ModuleRx(CanMsg *msg);
void ModuleMesGenerate(void);

#endif
