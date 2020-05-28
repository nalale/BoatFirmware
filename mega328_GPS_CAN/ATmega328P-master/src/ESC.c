/*
 * ESC.c
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



#include <util/delay.h>
#include "../inc/SERVO.h"
#include "../inc/ESC.h"



void ESC_init(uint8_t** DDRs, uint8_t** PORTs, uint8_t* masks, size_t n)
{
    SERVO_init(DDRs, PORTs, masks, n);
    
    SERVO_setAllServos(0x00);
    _delay_ms(4000);
}

void ESC_initThrottle(uint8_t** DDRs, uint8_t** PORTs, uint8_t* masks, size_t n)
{
    SERVO_init(DDRs, PORTs, masks, n);
    
    SERVO_setAllServos(0xFF);
    _delay_ms(4000);
    
    SERVO_setAllServos(0x00);
    _delay_ms(4000);
}



void ESC_setMotor(size_t index, uint8_t value)
{
    SERVO_setServo(index, value);
}

void ESC_setMotorScaled(size_t index, double percent)
{
    SERVO_setServoScaled(index, percent);
}

void ESC_setMotors(uint8_t* values)
{
    SERVO_setServos(values);
}

void ESC_setMotorsScaled(double* percents)
{
    SERVO_setServosScaled(percents);
}

void ESC_setAllMotors(uint8_t value)
{
    SERVO_setAllServos(value);
}

void ESC_setAllMotorsScaled(double percent)
{
    SERVO_setAllServosScaled(percent);
}
