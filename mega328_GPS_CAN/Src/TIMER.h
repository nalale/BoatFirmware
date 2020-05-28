/*
 * TIMER.h
 *
 *  Created on: 28 мая 2020 г.
 *      Author: a.lazko
 */

#ifndef SRC_TIMER_H_
#define SRC_TIMER_H_


void TIMER_init(void);
uint32_t GetTime_msStamp(void);
uint32_t GetTime_msFrom(uint32_t msStamp);


#endif /* SRC_TIMER_H_ */
