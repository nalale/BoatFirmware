/*
 * DateTime.c
 *
 *  Created on: 08.01.2011
 *      Author: green
 */

//#include <time.h>
#include <string.h>
#include "DateTime.h"


uint32_t CalcDOW(RTC_TIME_Type* time);
uint32_t CalcDOY(RTC_TIME_Type* time);

// ���������� ���� �� �������
uint8_t months[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

RTC_TIME_Type currentRtc;
uint32_t secondsTotal;			// ���������� ���������� ������ ������� � 01.01.2000
uint32_t secondsToday;			// ���������� ���������� ������ � ������� ���
//uint32_t secondsAtStartYear;	// ���������� ���������� ������ �� ������ �������� ����




void newSecond()
{
	// �������� ������� � ��������
	secondsTotal++;
	secondsToday++;

	// ������� ������� �����
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

	// ���� ����� �����
	if (secondsToday >= SECONDS_IN_DAY) // 86400 ������ � ����� ���
	{
		secondsToday = 0;
		NewDay();
		// ��������� ��������� � �����
		//board.SB.CheckBat = 1;
	}
}

// ���������� ���� � ������ 2000-�� ����
// ����������� ~ 1,6 ���
double GetDayAt2000(int y, int m, int d)
{
	uint32_t luku = -7 * (y + (m + 9) / 12) / 4 + 275 * m / 9 + d;
	luku += y * 367;

	//double h = 12;	// 12/24.0 = 0.5
	//return (double) luku - 730531.5 + 0.5;
	return (double) luku - 730531;
}



void SetDateTimeInt(uint32_t seconds)
{
	SecondsToRtc(seconds, &currentRtc);
	SetDateTime(&currentRtc);
}


uint32_t SetDateTime(RTC_TIME_Type* time)
{
	// �������� ������������ ������
	if (time->YEAR < 2012)
		time->YEAR = 2012;
	if (time->YEAR > 2099)
		time->YEAR = 2099;

	if (time->MONTH < 1)
		time->MONTH = 1;
	if (time->MONTH > 12)
		time->MONTH = 12;

	// �������� �� ���������� ���
	if (time->YEAR % 4 == 0)
		months[1] = 29; // ���� ��� ����������, �� � ������� 29 ����
	else
		months[1] = 28;

	if (time->DOM < 1)
		time->DOM = 1;
	if (time->DOM > months[time->MONTH - 1])
		time->DOM = months[time->MONTH - 1];

	// ������ ��� ������
	time->DOW = CalcDOW(time);
	// ������ ��� ����
	time->DOY = CalcDOY(time);


	// ��������� ����/�������
	//currentDate = *time;	// ���� time - ��������� �� uip_appdata, �� �������� ����������. ������� memcpy...
	memcpy((void*)&currentRtc, time, sizeof(RTC_TIME_Type));
	 
	secondsTotal = RtcToSeconds(time);
	secondsToday = time->HOUR * 3600 + time->MIN * 60 + time->SEC;

	return secondsTotal;
}

// ������� ������ � rtc-������
void SecondsToRtc(uint32_t seconds, RTC_TIME_Type* rtc)
{
	uint32_t cnt = 0;
	// ���������� ���� ************************************************
	for (rtc->YEAR = 0; rtc->YEAR < 99; rtc->YEAR++)
	{
		uint32_t secInYear = 31536000;
		if ((rtc->YEAR & 0x03) == 0)		// ���������� ���
			secInYear += SECONDS_IN_DAY;     // + 1 ����

		if (cnt + secInYear > seconds)
			break;

		cnt += secInYear;
	}

	// ���������� ������ **********************************************
	if ((rtc->YEAR & 0x03) == 0)
		months[1] = 29;		// ���� ��� ����������, �� � ������� 29 ����
	else
		months[1] = 28;

	seconds = seconds - cnt;      // ���. ������ � ������ ���� year
	cnt = 0;
	for (rtc->MONTH = 0; rtc->MONTH < 12; rtc->MONTH++)
	{
		uint32_t secInMonth = (uint32_t)months[rtc->MONTH] * SECONDS_IN_DAY;
		if (cnt + secInMonth > seconds)
			break;

		cnt += secInMonth;
	}

	seconds = seconds - cnt;      // ���. ������ � ������ ������ month

	// ���� ������
	rtc->DOM = (uint8_t)(seconds / SECONDS_IN_DAY);

	seconds = seconds - (uint32_t)rtc->DOM * SECONDS_IN_DAY;      // ���. ������ � ������ ���

	// ���
	rtc->HOUR = (uint8_t)(seconds / 3600);
	seconds = seconds - (uint32_t)rtc->HOUR * 3600;      // ���. ������ � ������ ����

	// ������
	rtc->MIN = (uint8_t)(seconds / 60);
	// �������
	rtc->SEC = (uint8_t)(seconds - (uint32_t)rtc->MIN * 60);

	rtc->MONTH++;
	rtc->DOM++;
	rtc->YEAR += 2000;
}

// ������� ����-������� � �������, ������� � 01.01.2000 �.
uint32_t RtcToSeconds(RTC_TIME_Type* rtc)
{
	uint32_t res = 0;

	// ���������� ����, ������� � 2000-��
	res = GetDayAt2000(rtc->YEAR, rtc->MONTH, rtc->DOM);


//	for (int y = 2000; y < time->YEAR; y++)
//	{
//		if(y % 4 == 0)
//			months[1] = 29;	// ���� ��� ����������, �� � ������� 29 ����
//		else
//			months[1] = 28;
//
//		for (int m = 0; m < 12; m++)
//			res += months[m];
//	}
//	// ��������� ���������� ���� � ������� ���� ����� �������
//	res += CalcDOY(time) - 1;


	// ������� � �������
	res = res * SECONDS_IN_DAY; // 86400 ������ � ����� ���
	// ��������� ���������� ������ � ������� ���
	res += rtc->HOUR * 3600 + rtc->MIN * 60 + rtc->SEC;

	return res;
}

// ������� ���� (������ ����) � �������, ������� � 01.01.2000 �.
uint32_t GetSecondsFromDate(RTC_TIME_Type* time)
{
	uint32_t res = 0;

	// ���������� ����, ������� � 2000-��
	res = GetDayAt2000(time->YEAR, time->MONTH, time->DOM);

	// ������� � �������
	res = res * SECONDS_IN_DAY; // 86400 ������ � ����� ���

	return res;
}

// ������� ������� (������ �������) � �������
uint32_t GetSecondsFromTime(uint32_t h, uint32_t m, uint32_t s)
{
	return h * 3600 + m * 60 + s;
}


// ������� ���� ������ ���� � �������, ������� � 01.01.2000 �.
uint32_t GetSecondsAtStartYear(uint32_t year)
{
	uint32_t res = 0;

	// ���������� ����, ������� � 2000-��
	res = GetDayAt2000(year, 1, 1);

	// ������� � �������
	res = res * SECONDS_IN_DAY; // 86400 ������ � ����� ���

	return res;
}

// ���������� ���������� ���� � ��������� ������
uint8_t GetDaysInMonth(int month, int year)
{
	if (month < 1 || month > 12)
		return 0;

	// �������� �� ���������� ���
	if (year % 4 == 0)
		months[1] = 29; // ���� ��� ����������, �� � ������� 29 ����
	else
		months[1] = 28;

	return months[month - 1];
}

// ������ ��� ������ ��������� ����
uint32_t CalcDOW(RTC_TIME_Type* time)
{
	uint32_t n;
	uint32_t nyy;
	uint32_t nmm;

	// �������� �� ���������� ���
	if (time->YEAR % 4 == 0)
		months[1] = 29; // ���� ��� ����������, �� � ������� 29 ����
	else
		months[1] = 28;

	// ������ ��� ������
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

	return n - (n / 7) * 7; // ���� ������ (0 - �����������, ...)
}

// ������ ��� ���� ��������� ����
uint32_t CalcDOY(RTC_TIME_Type* time)
{
	uint32_t res = 0;

	if(time->YEAR % 4 == 0)
		months[1] = 29;	// ���� ��� ����������, �� � ������� 29 ����
	else
		months[1] = 28;

	// ���������� ���� � ���������� ������� �������� ����
	for (int m = 0; m < time->MONTH - 1; m++)
		res += months[m];

	// ��������� ���������� ���� � ������� ������
	res += time->DOM;

	return res;
}

void NewDay()
{
	currentRtc.DOM++;
	currentRtc.DOW++;
	currentRtc.DOY++;

	// �������� �� ���������� ���
	if (currentRtc.YEAR % 4 == 0)
		months[1] = 29; // ���� ��� ����������, �� � ������� 29 ����
	else
		months[1] = 28;


	if(currentRtc.DOW > 6)
		currentRtc.DOW = 0;

	// ���� �������� ����� �����, ��
	if (currentRtc.DOM > months[currentRtc.MONTH - 1])
	{
		currentRtc.DOM = 1; 	// ��������� ����� �� ������ ������,
		currentRtc.MONTH++; 	// � ����� ����������� �� 1
	}

	// ���� �������� ����� ���
	if (currentRtc.MONTH > 12)
	{
		currentRtc.MONTH = 1; 	// ����� �� ������ ����, �
		currentRtc.YEAR++;		// ��� ����������� �� 1
		currentRtc.DOY = 1;
	}
}

// ���������� ���������� ������ ����� ������ date2 - date1
uint32_t GetDiffTimeInSeconds(RTC_TIME_Type* date2, RTC_TIME_Type* date1)
{
	uint32_t res = RtcToSeconds(date2) - RtcToSeconds(date1);
	return res;
}
// ���������� ���������� ������ ���� ����� ������ date2 - date1 (��� ����� �������)
uint32_t GetDiffTimeInDays(RTC_TIME_Type* date2, RTC_TIME_Type* date1)
{
	uint32_t res = GetSecondsFromDate(date2) - GetSecondsFromDate(date1);
	return res / SECONDS_IN_DAY;
}

// ���������� � ���� ���������� ������
void AddSecondsToRtc(RTC_TIME_Type* rtc, int32_t sec)
{
	uint32_t seconds = RtcToSeconds(rtc);
	seconds += sec;
	SecondsToRtc(seconds, rtc);
}

// ���������� � ���� date ���������� �������
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










