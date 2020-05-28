/*
 * TWI.c
 * 
 * Author:      Sebastian Gössl
 * Hardware:    ATmega328P
 * 
 * LICENSE:
 * MIT License
 * 
 * Copyright (c) 2018 Sebastian Gössl
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */



#include <avr/io.h>
#include <util/twi.h>
#include "../inc/TWI.h"



#ifndef F_CPU
    #define F_CPU 16000000UL
    #warning "F_CPU not defined! Assuming 16MHz."
#endif


#if (F_CPU/TWI_FREQUENCY - 16) / (2 * 1) <= 0xFF
    #define TWI_PRESCALER 1
    #define TWPS0_VALUE 0
    #define TWPS1_VALUE 0
#elif (F_CPU/TWI_FREQUENCY - 16) / (2 * 4) <= 0xFF
    #define TWI_PRESCALER 4
    #define TWPS0_VALUE 1
    #define TWPS1_VALUE 0
#elif (F_CPU/TWI_FREQUENCY - 16) / (2 * 16) <= 0xFF
    #define TWI_PRESCALER 16
    #define TWPS0_VALUE 0
    #define TWPS1_VALUE 1
#elif (F_CPU/TWI_FREQUENCY - 16) / (2 * 64) <= 0xFF
    #define TWI_PRESCALER 64
    #define TWPS0_VALUE 1
    #define TWPS1_VALUE 1
#else
    #error "TWI_FREQUENCY too low!"
#endif

#define TWBR_VALUE ((F_CPU/TWI_FREQUENCY - 16) / (2 * TWI_PRESCALER))


#define TWI_ADDRESS_W(x)    (((x) << 1) & ~0x01)
#define TWI_ADDRESS_R(x)    (((x) << 1) | 0x01)



void TWI_init(void)
{
    TWBR = TWBR_VALUE;
    TWSR = (TWPS1_VALUE << TWPS1) | (TWPS0_VALUE << TWPS0);
    
    TWCR = (1 << TWEN);
}



static void TWI_waitForComplete(void)
{
    while(~TWCR & (1 << TWINT))
        ;
}



bool TWI_start(void)
{
    TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);
    
    TWI_waitForComplete();
    
    return TW_STATUS != TW_START;
}

bool TWI_repStart(void)
{
    TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);
    
    TWI_waitForComplete();
    
    return TW_STATUS != TW_REP_START;
}

void TWI_stop(void)
{
    TWCR = (1 << TWINT) | (1 << TWSTO) | (1 << TWEN);
}


bool TWI_addressWrite(uint8_t address)
{
    TWDR = TWI_ADDRESS_W(address);
    TWCR = (1 << TWINT) | (1 << TWEN);
    
    TWI_waitForComplete();
    
    return TW_STATUS != TW_MT_SLA_ACK;
}

bool TWI_addressRead(uint8_t address)
{
    TWDR = TWI_ADDRESS_R(address);
    TWCR = (1 << TWINT) | (1 << TWEN);
    
    TWI_waitForComplete();
    
    return TW_STATUS != TW_MR_SLA_ACK;
}


bool TWI_write(uint8_t data)
{
    TWDR = data;
    TWCR = (1 << TWINT) | (1 << TWEN);
    
    TWI_waitForComplete();
    
    return TW_STATUS != TW_MT_DATA_ACK;
}

bool TWI_writeBurst(uint8_t* data, size_t len)
{
    while(len--)
        if(TWI_write(*data++))
            return 1;
    
    return 0;
}


bool TWI_readAck(uint8_t* data)
{
    TWCR = (1 << TWINT) | (1 << TWEA) | (1 << TWEN);
    TWI_waitForComplete();
    
    *data = TWDR;
    
    return TW_STATUS != TW_MR_DATA_ACK;
}

bool TWI_readAckBurst(uint8_t* data, size_t len)
{
    while(len--)
        if(TWI_readAck(data++))
            return 1;
    
    return 0;
}

bool TWI_readNoAck(uint8_t* data)
{
    TWCR = (1 << TWINT) | (1 << TWEN);
    TWI_waitForComplete();
    
    *data = TWDR;
    
    return TW_STATUS != TW_MR_DATA_NACK;
}

bool TWI_readNoAckBurst(uint8_t* data, size_t len)
{
    while(len--)
        if(TWI_readNoAck(data++))
            return 1;
    
    return 0;
}


bool TWI_writeToSlave(uint8_t address, uint8_t* data, size_t len)
{
    if(TWI_start())
        return 1;
    if(TWI_addressWrite(address))
        return 1;
    
    if(TWI_writeBurst(data, len))
        return 1;
    
    TWI_stop();
    
    return 0;
}

bool TWI_readFromSlave(uint8_t address, uint8_t* data, size_t len)
{
    if(TWI_start())
        return 1;
    if(TWI_addressRead(address))
        return 1;
    
    if(TWI_readAckBurst(data, len-1))
        return 1;
    if(TWI_readNoAck(&data[len-1]))
        return 1;
    
    TWI_stop();
    
    return 0;
}

bool TWI_writeToSlaveRegister(uint8_t address, uint8_t reg,
    uint8_t* data, size_t len)
{
    if(TWI_start())
        return 1;
    if(TWI_addressWrite(address))
        return 1;
    
    if(TWI_write(reg))
        return 1;
    if(TWI_writeBurst(data, len))
        return 1;
    
    TWI_stop();
    
    return 0;
}

bool TWI_readFromSlaveRegister(uint8_t address, uint8_t reg,
    uint8_t* data, size_t len)
{
    if(TWI_start())
        return 1;
    if(TWI_addressWrite(address))
        return 1;
    if(TWI_write(reg))
        return 1;
     
    if(TWI_repStart())
        return 1;
    if(TWI_addressRead(address))
        return 1;
    if(TWI_readAckBurst(data, len-1))
        return 1;
    if(TWI_readNoAck(&data[len-1]))
        return 1;
    
    TWI_stop();
    
    return 0;
}
