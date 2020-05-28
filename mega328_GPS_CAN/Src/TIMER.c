/*
 * TIMER.c
 *
 *  Created on: 28 мая 2020 г.
 *      Author: a.lazko
 */


#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

static volatile uint32_t cnt_ms = 0;

ISR (TIMER0_COMPA_vect)  // timer0 overflow interrupt
{
    //event to be exicuted every ms here
    cnt_ms++;
}

void TIMER_init(void)
{
    // Set the Timer Mode to CTC
    TCCR0A |= (1 << WGM01);

    // Set the value that you want to count to
    OCR0A = 0xF9;

    TIMSK0 |= (1 << OCIE0A);    //Set the ISR COMPA vect

    sei();         //enable interrupts

    TCCR0B |= (1 << CS01) | (1 << CS00);
    // set prescaler to 64 and start the timer
}


uint32_t GetTime_msStamp()
{
    return cnt_ms;
}

uint32_t GetTime_msFrom(uint32_t msStamp)
{
    return (cnt_ms >= msStamp)?
        cnt_ms - msStamp :
        (0xFFffFFff - msStamp) + cnt_ms;
}
