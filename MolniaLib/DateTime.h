/*
 * DateTime.h
 *
 *  Created on: 08.01.2011
 *      Author: green
 */

#ifndef DATETIME_H_
#define DATETIME_H_

#include "lpc17xx_rtc.h"

#define	SECONDS_IN_DAY		86400

typedef struct{
	uint32_t secondsTotal;			//  оличество пройденных секунд начина€ с 01.01.2000
	uint32_t secondsToday;			//  оличество пройденных секунд в текущем дне

	uint8_t
	IsInit		:	1,
	TimeServer	:	1,
	dummy1	: 6;
}SystemTime_t;



//extern uint32_t secondsAtStartYear;	//  оличество пройденных секунд на начало текущего года



//  оличество пройденных секунд на начало текущего дн€
//#define	secondsAtStartDay		(secondsTotal - secondsToday)

uint8_t dateTime_IsInit(void);
RTC_TIME_Type* dateTime_InitCurrent(uint32_t seconds);
uint32_t dateTime_GetCurrentTotalSeconds(void);
uint16_t dateTime_GetCurrentTotalDays(void);
RTC_TIME_Type* dateTime_GetCurrentRtcTime(void);

void dateTime_newSecond(void);
void NewDay(void);

uint32_t dateTime_SetCurrentTime(RTC_TIME_Type* time);

void SecondsToRtc(uint32_t seconds, RTC_TIME_Type* rtc);
uint32_t RtcToSeconds(RTC_TIME_Type* time);

uint32_t GetSecondsFromDate(RTC_TIME_Type* time);
uint32_t GetSecondsFromTime(uint32_t h, uint32_t m, uint32_t s);
uint32_t GetSecondsAtStartYear(uint32_t year);

uint8_t GetDaysInMonth(int month, int year);
uint32_t GetDiffTimeInSeconds(RTC_TIME_Type* time2, RTC_TIME_Type* time1);
uint32_t GetDiffTimeInDays(RTC_TIME_Type* date2, RTC_TIME_Type* date1);

void AddMonthsToDate(RTC_TIME_Type* date, int monthsCount);
void AddSecondsToRtc(RTC_TIME_Type* rtc, int32_t sec);

uint32_t GetDayOfWeek(RTC_TIME_Type* time);

uint32_t get_fattimeFromRtc(RTC_TIME_Type* rtc);

#endif /* DATETIME_H_ */




