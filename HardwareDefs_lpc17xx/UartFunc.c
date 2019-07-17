#include <stdint.h>
#include "UartFunc.h"
#include "lpc17xx_uart.h"
#include "lpc17xx_pinsel.h"


/* Buf mask */
#define __BUF_MASK (UART_RING_BUFSIZE-1)
/* Check buf is full or not */
#define __BUF_IS_FULL(head, tail) ((tail&__BUF_MASK)==((head+1)&__BUF_MASK))
/* Check buf will be full in next receiving or not */
#define __BUF_WILL_FULL(head, tail) ((tail&__BUF_MASK)==((head+2)&__BUF_MASK))
/* Check buf is empty */
#define __BUF_IS_EMPTY(head, tail) ((head&__BUF_MASK)==(tail&__BUF_MASK))
/* Reset buf */
#define __BUF_RESET(bufidx)	(bufidx=0)
#define __BUF_INCR(bufidx)	(bufidx=(bufidx+1)&__BUF_MASK)

LPC_UART_TypeDef *CurrentUART;


void Pin_Configurate(uint8_t uart_channel);
void UART_IntReceive(uint8_t Channel);

static uint16_t receive_cnt = 0;

typedef struct
{
    __IO uint32_t tx_head;                /*!< UART Tx ring buffer head index */
    __IO uint32_t tx_tail;                /*!< UART Tx ring buffer tail index */
    __IO uint32_t rx_head;                /*!< UART Rx ring buffer head index */
    __IO uint32_t rx_tail;                /*!< UART Rx ring buffer tail index */
    __IO uint8_t  tx[UART_RING_BUFSIZE];  /*!< UART Tx data ring buffer */
    __IO uint8_t  rx[UART_RING_BUFSIZE];  /*!< UART Rx data ring buffer */
} UART_RING_BUFFER_T;


UART_RING_BUFFER_T rb;

/*----------------- INTERRUPT SERVICE ROUTINES --------------------------*/
/*********************************************************************//**
 * @brief		UART2 interrupt handler sub-routine
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void UART0_IRQHandler(void)
{
	uint32_t intsrc, tmp;

	/* Determine the interrupt source */
	intsrc = UART_GetIntId(LPC_UART0);
	tmp = intsrc & UART_IIR_INTID_MASK;

	// Receive Data Available or Character time-out
	if ((tmp == UART_IIR_INTID_RDA) || (tmp == UART_IIR_INTID_CTI)){
		UART_IntReceive(0);
	}

}

void Uart_Init(uint8_t Channel, uint32_t baudrate)
{
    Pin_Configurate(Channel);

    // UART Configuration structure variable
	UART_CFG_Type UARTConfigStruct;
	// UART FIFO configuration Struct variable
	UART_FIFO_CFG_Type UARTFIFOConfigStruct;


    UART_ConfigStructInit(&UARTConfigStruct);
	UARTConfigStruct.Baud_rate = baudrate;
    UARTConfigStruct.Parity = UART_PARITY_NONE;
	UARTConfigStruct.Databits = UART_DATABIT_8;
	UARTConfigStruct.Stopbits = UART_STOPBIT_1;
	
	switch(Channel)
	{
		case 0:
			CurrentUART = LPC_UART0;
			break;
		
		default:
			break;
	}

	// Initialize UART0 peripheral with given to corresponding parameter
	UART_Init(CurrentUART, &UARTConfigStruct);

    UART_FIFOConfigStructInit(&UARTFIFOConfigStruct);

	// Initialize FIFO for UART0, UART2, UART3 peripheral
	UART_FIFOConfig((LPC_UART_TypeDef *)CurrentUART, &UARTFIFOConfigStruct);

    /* Enable UART Rx interrupt */
	//UART_IntConfig((LPC_UART_TypeDef *)CurrentUART, UART_INTCFG_RBR, ENABLE);
    /* Enable Interrupt for UART0 channel */
	//NVIC_EnableIRQ(UART0_IRQn);

	// Enable UART Transmit
	UART_TxCmd(CurrentUART, ENABLE);

}

void Pin_Configurate(uint8_t uart_channel)
{
    PINSEL_CFG_Type PinCfg;


    switch(uart_channel)
    {
        case 0:

            PinCfg.Funcnum = 1;
            PinCfg.OpenDrain = 0;
            PinCfg.Pinmode = 0;
            PinCfg.Pinnum = 2;
            PinCfg.Portnum = 0;
            PINSEL_ConfigPin(&PinCfg);
            PinCfg.Pinnum = 3;
            PINSEL_ConfigPin(&PinCfg);
        break;

        case 1:
        
		break;			
		
        case 2:
            PinCfg.Funcnum = 2;
            PinCfg.OpenDrain = 0;
            PinCfg.Pinmode = 0;
            PinCfg.Pinnum = 8;
            PinCfg.Portnum = 2;
            PINSEL_ConfigPin(&PinCfg);
            PinCfg.Pinnum = 9;
            PINSEL_ConfigPin(&PinCfg);
        break;

        case 3:

        break;
    }
}

void UART_IntReceive(uint8_t Channel)
{
	uint8_t tmpc;
	uint32_t rLen;
	
	switch(Channel)
	{
		case 0:
			CurrentUART = LPC_UART0;
		break;
		
		default:
			return;
	}

	while(1){
		// Call UART read function in UART driver
		rLen = UART_Receive((LPC_UART_TypeDef *)CurrentUART, &tmpc, 1, NONE_BLOCKING);
		// If data received
		if (rLen){
			receive_cnt += rLen;
			/* Check if buffer is more space
			 * If no more space, remaining character will be trimmed out
			 */
			if (!__BUF_IS_FULL(rb.rx_head,rb.rx_tail)){
				rb.rx[rb.rx_head] = tmpc;
				__BUF_INCR(rb.rx_head);
			}
		}
		// no more data
		else {
			break;
		}
	}
}

void Uart_SendData(uint8_t buf, uint8_t *data, uint8_t size)
{
	switch(buf)
	{
		case 0:
			UART_Send(LPC_UART0, data, size, BLOCKING);
		break;
		
		default:
			return;
	}
}

