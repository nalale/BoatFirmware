#ifndef __SEVCON__
#define __SEVCON__

#include "CanFunc.h"


/*******************************************************************************
 *                       ПРОТОТИПЫ
*******************************************************************************/
uint8_t ExternalRx(CanMsg* msg);
// Генерация сообщений для Sevcon
void ExternalMesGenerate(void);

#endif
