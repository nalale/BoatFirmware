/*
 * DateTime.c
 *
 *  Created on: 08.01.2011
 *      Author: green
 */

#include <string.h>
#include "DateTime.h"

// Количество дней по месяцам
static uint8_t months[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

RTC_TIME_Type currentRtc;
static SystemTime_t sysTime;

//uint32_t secondsTotal;			// Количество пройденных секунд начиная с 01.01.2000
//uint32_t secondsToday;			// Количество пройденных секунд в текущем дне
//uint32_t secondsAtStartYear;	// Количество пройденных секунд на начало текущего года

static void CheckDateTime(RTC_TIME_Type* dateTime);
static double GetDayAt2000(int y, int m, int d);
static uint32_t CalcDOY(RTC_TIME_Type* time);

RTC_TIME_Type* dateTime_GetCurrentRtcTime(void)
{
	return &currentRtc;
}

uint8_t dateTime_IsInit()
{
	return sysTime.IsInit;
}

void dateTime_newSecond()
{
	// Счетчики времени в секундах
	sysTime.secondsTotal++;
	sysTime.secondsToday++;

	// Считаем обычное время
	currentRtc.SEC += 1;
	if (currentRtc.SEC >= 60)
	{
		currentRtc.SEC = 0;
		currentRtc.MIN++;
		if (currentRtc.MIN >= 60)
		{
			currentRtc.MIN = 0;
			currentRtc.HOUR++;
			if (currentRtc.HOUR >= 24)
				currentRtc.HOUR = 0;
		}
	}

	// Если новые сутки
	if (sysTime.secondsToday >= SECONDS_IN_DAY) // 86400 секунд в одном дне
	{
		sysTime.secondsToday = 0;
		NewDay();
	}
}

uint32_t dateTime_GetCurrentTotalSeconds()
{
	if(!sysTime.IsInit)
		return sysTime.secondsToday;
	else
		return sysTime.secondsTotal;
}

uint16_t dateTime_GetCurrentTotalDays(){
	
	if(!sysTime.IsInit)
		return currentRtc.DOY;
	else
		return GetDayAt2000(currentRtc.YEAR, currentRtc.MONTH, currentRtc.DOM);
}

RTC_TIME_Type* dateTime_InitCurrent(uint32_t seconds)
{	
	SecondsToRtc(seconds, &currentRtc);
	dateTime_SetCurrentTime(&currentRtc);
	
	return &currentRtc;
}

void CheckDateTime(RTC_TIME_Type* dateTime)
{
	// Проверка правильности данных
	if (dateTime->YEAR < 2012)
		dateTime->YEAR = 2012;
	if (dateTime->YEAR > 2099)
		dateTime->YEAR = 2099;

	if (dateTime->MONTH < 1)
		dateTime->MONTH = 1;
	if (dateTime->MONTH > 12)
		dateTime->MONTH = 12;

	// Проверка на високосный год
	if (dateTime->YEAR % 4 == 0)
		months[1] = 29; // Если год високосный, то в феврале 29 дней
	else
		months[1] = 28;

	if (dateTime->DOM < 1)
		dateTime->DOM = 1;
	if (dateTime->DOM > months[dateTime->MONTH - 1])
		dateTime->DOM = months[dateTime->MONTH - 1];

	// Расчет дня недели
	dateTime->DOW = GetDayOfWeek(dateTime);
	// Расчет дня года
	dateTime->DOY = CalcDOY(dateTime);
}

uint32_t dateTime_SetCurrentTime(RTC_TIME_Type* time)
{
	sysTime.IsInit = 1;
	
	CheckDateTime(time);

	if(time != &currentRtc)
		memcpy((void*)&currentRtc, time, sizeof(RTC_TIME_Type));
	 
	sysTime.secondsTotal = RtcToSeconds(time);
	sysTime.secondsToday = time->HOUR * 3600 + time->MIN * 60 + time->SEC;

	return sysTime.secondsTotal;
}

// Перевод секунд в rtc-формат
void SecondsToRtc(uint32_t seconds, RTC_TIME_Type* rtc)
{
	uint32_t cnt = 0;
	// Извлечение года ************************************************
	for (rtc->YEAR = 0; rtc->YEAR < 99; rtc->YEAR++)
	{
		uint32_t secInYear = 31536000;
		if ((rtc->YEAR & 0x03) == 0)		// Високосный год
			secInYear += SECONDS_IN_DAY;     // + 1 день

		if (cnt + secInYear > seconds)
			break;

		cnt += secInYear;
	}

	// Извлечение месяца **********************************************
	if ((rtc->YEAR & 0x03) == 0)
		months[1] = 29;		// Если год високосный, то в феврале 29 дней
	else
		months[1] = 28;

	seconds = seconds - cnt;      // Кол. секунд с начала года year
	cnt = 0;
	for (rtc->MONTH = 0; rtc->MONTH < 12; rtc->MONTH++)
	{
		uint32_t secInMonth = (uint32_t)months[rtc->MONTH] * SECONDS_IN_DAY;
		if (cnt + secInMonth > seconds)
			break;

		cnt += secInMonth;
	}

	seconds = seconds - cnt;      // Кол. секунд с начала месяца month

	// День месяца
	rtc->DOM = (uint8_t)(seconds / SECONDS_IN_DAY);

	seconds = seconds - (uint32_t)rtc->DOM * SECONDS_IN_DAY;      // Кол. секунд с начала дня

	// Час
	rtc->HOUR = (uint8_t)(seconds / 3600);
	seconds = seconds - (uint32_t)rtc->HOUR * 3600;      // Кол. секунд с начала часа

	// Минуты
	rtc->MIN = (uint8_t)(seconds / 60);
	// Секунды
	rtc->SEC = (uint8_t)(seconds - (uint32_t)rtc->MIN * 60);

	rtc->MONTH++;
	rtc->DOM++;
	rtc->YEAR += 2000;
}

// Перевод даты-времени в секунды, начиная с 01.01.2000 г.
uint32_t RtcToSeconds(RTC_TIME_Type* rtc)
{
	uint32_t res = 0;

	// Количество дней, начиная с 2000-го
	res = GetDayAt2000(rtc->YEAR, rtc->MONTH, rtc->DOM);


//	for (int y = 2000; y < time->YEAR; y++)
//	{
//		if(y % 4 == 0)
//			months[1] = 29;	// Если год високосный, то в феврале 29 дней
//		else
//			months[1] = 28;
//
//		for (int m = 0; m < 12; m++)
//			res += months[m];
//	}
//	// Добавляем количество дней в текущем году минус текущий
//	res += CalcDOY(time) - 1;


	// Перевод в секунды
	res = res * SECONDS_IN_DAY; // 86400 секунд в одном дне
	// Добавляем количество секунд в текущем дне
	res += rtc->HOUR * 3600 + rtc->MIN * 60 + rtc->SEC;

	return res;
}

// Количество дней с начала 2000-го года
// Выполняется ~ 1,6 мкс
double GetDayAt2000(int y, int m, int d)
{
	uint32_t luku = -7 * (y + (m + 9) / 12) / 4 + 275 * m / 9 + d;
	luku += y * 367;

	//double h = 12;	// 12/24.0 = 0.5
	//return (double) luku - 730531.5 + 0.5;
	return (double) luku - 730531;
}

// Перевод даты (ТОЛЬКО даты) в секунды, начиная с 01.01.2000 г.
uint32_t GetSecondsFromDate(RTC_TIME_Type* time)
{
	uint32_t res = 0;

	// Количество дней, начиная с 2000-го
	res = GetDayAt2000(time->YEAR, time->MONTH, time->DOM);

	// Перевод в секунды
	res = res * SECONDS_IN_DAY; // 86400 секунд в одном дне

	return res;
}

// Перевод времени (ТОЛЬКО времени) в секунды
uint32_t GetSecondsFromTime(uint32_t h, uint32_t m, uint32_t s)
{
	return h * 3600 + m * 60 + s;
}


// Перевод даты начала года в секунды, начиная с 01.01.2000 г.
uint32_t GetSecondsAtStartYear(uint32_t year)
{
	uint32_t res = 0;

	// Количество дней, начиная с 2000-го
	res = GetDayAt2000(year, 1, 1);

	// Перевод в секунды
	res = res * SECONDS_IN_DAY; // 86400 секунд в одном дне

	return res;
}

// Возвращает количество дней в указанном месяце
uint8_t GetDaysInMonth(int month, int year)
{
	if (month < 1 || month > 12)
		return 0;

	// Проверка на високосный год
	if (year % 4 == 0)
		months[1] = 29; // Если год високосный, то в феврале 29 дней
	else
		months[1] = 28;

	return months[month - 1];
}

// Расчет дня недели указанной даты
uint32_t GetDayOfWeek(RTC_TIME_Type* time)
{
	uint32_t n;
	uint32_t nyy;
	uint32_t nmm;

	// Проверка на високосный год
	if (time->YEAR % 4 == 0)
		months[1] = 29; // Если год високосный, то в феврале 29 дней
	else
		months[1] = 28;

	// Расчет дня недели
	if (time->MONTH > 2)
	{
		nmm = time->MONTH + 1;
		nyy = time->YEAR;
	}
	else
	{
		nmm = time->MONTH + 13;
		nyy = time->YEAR - 1;
	}

	n = (long) (365.25 * nyy) + (int) (30.6 * nmm) + time->DOM - 621050;

	return n - (n / 7) * 7; // День недели (0 - понедельник, ...)
}

// Расчет дня года указанной даты
uint32_t CalcDOY(RTC_TIME_Type* time)
{
	uint32_t res = 0;

	if(time->YEAR % 4 == 0)
		months[1] = 29;	// Если год високосный, то в феврале 29 дней
	else
		months[1] = 28;

	// Количество дней в предыдущих месяцах текущего года
	for (int m = 0; m < time->MONTH - 1; m++)
		res += months[m];

	// Добавляем количество дней в текущем месяце
	res += time->DOM;

	return res;
}

void NewDay()
{
	currentRtc.DOM++;
	currentRtc.DOW++;
	currentRtc.DOY++;

	// Проверка на високосный год
	if (currentRtc.YEAR % 4 == 0)
		months[1] = 29; // Если год високосный, то в феврале 29 дней
	else
		months[1] = 28;


	if(currentRtc.DOW > 6)
		currentRtc.DOW = 0;

	// Если наступил новый месяц, то
	if (currentRtc.DOM > months[currentRtc.MONTH - 1])
	{
		currentRtc.DOM = 1; 	// переводим число на начало месяца,
		currentRtc.MONTH++; 	// а месяц увеличиваем не 1
	}

	// Если наступил новый год
	if (currentRtc.MONTH > 12)
	{
		currentRtc.MONTH = 1; 	// месяц на начало года, а
		currentRtc.YEAR++;		// год увеличиваем на 1
		currentRtc.DOY = 1;
	}
}

// Возвращает количество секунд между датами date2 - date1
uint32_t GetDiffTimeInSeconds(RTC_TIME_Type* date2, RTC_TIME_Type* date1)
{
	uint32_t res = RtcToSeconds(date2) - RtcToSeconds(date1);
	return res;
}
// Возвращает количество полных дней между датами date2 - date1 (без учета времени)
uint32_t GetDiffTimeInDays(RTC_TIME_Type* date2, RTC_TIME_Type* date1)
{
	uint32_t res = GetSecondsFromDate(date2) - GetSecondsFromDate(date1);
	return res / SECONDS_IN_DAY;
}

// Прибавляет к дате количество секунд
void AddSecondsToRtc(RTC_TIME_Type* rtc, int32_t sec)
{
	uint32_t seconds = RtcToSeconds(rtc);
	seconds += sec;
	SecondsToRtc(seconds, rtc);
}

// Прибавляет к дате date количество месяцев
void AddMonthsToDate(RTC_TIME_Type* date, int monthsCount)
{
	int res = date->MONTH + monthsCount;

	date->YEAR += (res - 1) / 12;
	
	int tmp = res % 12;
	if(tmp == 0)
		date->MONTH = 12;
	else
		date->MONTH = tmp;

}



/*---------------------------------------------------------*/
/* User Provided RTC Function for FatFs module             */
/*---------------------------------------------------------*/
uint32_t get_fattimeFromRtc(RTC_TIME_Type* rtc)
{
	/* Pack date and time into a DWORD variable */
	return	((uint32_t)(rtc->YEAR - 1980) << 25)
			| ((uint32_t)rtc->MONTH << 21)
			| ((uint32_t)rtc->DOM << 16)
			| ((uint32_t)rtc->HOUR << 11)
			| ((uint32_t)rtc->MIN << 5)
			| ((uint32_t)rtc->SEC >> 1);
}

uint32_t get_fattime()
{
	/* Pack date and time into a DWORD variable */
	return get_fattimeFromRtc(&currentRtc);
}










