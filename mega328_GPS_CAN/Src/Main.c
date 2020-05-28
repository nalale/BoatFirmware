/*
 * Main.c
 *
 *  Created on: 17 мая 2020 г.
 *      Author: a.lazko
 */

#include "Main.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <stdio.h>
#include <string.h>


#include "../ATmega328P-master/inc/SPI.h"
#include "../ATmega328P-master/inc/UART2.h"
#include "../ExternalDrivers/mcp2515.h"
#include "../ExternalDrivers/gpsNeo6n.h"

#include "TIMER.h"

#define PORT_DIR_OUT 1
#define PORT_DIR_IN !PORT_DIR_OUT

int main(void)
{
    uint32_t ts = 0;
    char s = 0, led = 0;

    CanMsg msg;
    msg.ID = 363;
    msg.DLC = 8;
    msg.Ext = 0;

    DDRD = (PORT_DIR_OUT << DDD3) |
            (PORT_DIR_OUT << DDD4) |
            (PORT_DIR_OUT << DDD5);

    UART2_init();
    SPI_init();
    TIMER_init();

    //MCP2515_SELECT(MCP_CS1_U6);
    mcp2515_setSpiCallBack(SPI_writeRead);
    mcp2515_init(MCP_CS1_U6, MCP_CAN1_NUM, kBAUD250);

    sei();

    while(1)
    {
        if(UART2_ngets(&s, sizeof(s)))
            neo6m_ProcessCharacter(s);

        neo6m_Thread();

        if(GetTime_msFrom(ts) >= 250)
        {
            ts = GetTime_msStamp();
            led = !led;
            PORTD = (led << PORTD3) | (led << PORTD4) | (led << PORTD5);

            uint16_t vel = GetVelocityKmph();

            msg.data[0] = vel;
            msg.data[1] = vel >> 8;
            msg.data[2] = (uint8_t)GetTime();
            msg.data[3] = (uint8_t)((uint32_t)GetTime() >> 8);
            msg.data[4] = (uint8_t)((uint32_t)GetTime() >> 16);
            msg.data[5] = (uint8_t)((uint32_t)GetTime() >> 24);

            mcp2515_write_canMsg(MCP_CS1_U6, &msg);

        }
    }
}



