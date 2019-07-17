#ifndef _UartFunc_H_
#define _UartFunc_H_

#include "lpc17xx_uart.h"

/* buffer size definition */
#define UART_RING_BUFSIZE 256

void Uart_Init(uint8_t Channel, uint32_t baudrate);
void Uart_SendData(uint8_t buf, uint8_t *data, uint8_t size);








#endif
