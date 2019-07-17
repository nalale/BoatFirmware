#ifndef _I2C_FUNC_H_
#define _I2C_FUNC_H_

#define I2C_WRITE_BIT   0
#define I2C_READ_BIT    1

void I2c_Init(void);
uint8_t I2CReadWrite(uint8_t *tx_buf, uint8_t tx_lenght, uint8_t *rx_buf, uint8_t rx_lenght, uint16_t slave_address);

#endif
