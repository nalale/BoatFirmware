#ifndef __SEVCON__
#define __SEVCON__

#include "CanFunc.h"


/*******************************************************************************
 *                       ���������
*******************************************************************************/
uint8_t ExternalRx(CanMsg* msg);
// ��������� ��������� ��� Sevcon
void ExternalMesGenerate(void);

#endif
