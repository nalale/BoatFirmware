#ifndef _SPI_FUNC_H_
#define _SPI_FUNC_H_

typedef enum
{
	DATABIT_8 = 0,
	DATABIT_16,
} DataBit_e;


void Spi_Init(DataBit_e Databit);
uint8_t SpiReadWrite(uint8_t *tx_buf, uint8_t lenght, uint8_t *rx_buf);

#endif

