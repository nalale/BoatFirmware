/*
 *  В этом файле реализованы протоколы взаимодействия по CAN
 */
#include <stdlib.h>
#include <string.h>

#include "Main.h"
#include "FaultTools.h"
#include "User.h"

#include "Protocol.h"
#include "WorkStates.h"

#include "TimerFunc.h"
#include "CanFunc.h"
#include "MemoryFunc.h"

#include "Config.h"
#include "DateTime.h"

#include "N_OBD.h"
#include "../MolniaLib/FaultsServices.h"

#define ABORT					-1					// процесс завершён
#define MAX_REQUEST_AMOUNT		1					// Максимальнрое количество запросов следующей посылки при
#define REQUEST_PEROD_MS		10					// Период запросов на следующую посылку, мс
#define KEEPALIVE_PEROD_MS		250					// Период отправки сообщения KeepAlive, мс

	
int16_t NoteNumber2Update 		= ABORT;			// Предыдущая запись, которую редактировали
int16_t UpdateProfileStatus 	= 0;				// Статус операции записи профайла с точки зрения контроллера (OK/ Код ошибки)
int16_t Index2Update			= ABORT;			// Номер индекса параметра для редактирования


int16_t NoteNumber2Read 		= ABORT;			// Номер записи для чтения
int16_t PrevNoteNumber2Read 	= ABORT;			// Предыдущая запись, которую редактировали
int16_t ReadDataCnt  		= INT16_MAX;			// Счётчик отправляемых записей
int16_t Index2Read			= INT16_MAX;			// Индекс параметра для чтения
uint8_t BytesLenght2Read			= 0;				// Длина отправляемых данных

uint32_t TimeStampProfileUpdateAck 	= 0;			// Штамп времени для отправки номера отредактированной записи профайла
uint32_t TimeStampProfileRead 		= 0;			// Штамп времени для для считывания профайла
uint32_t TimeStampKeepAlive 		= 0;			// Штамп времени для отправки сообщения KeepAlive

EcuConfig_t TempProfile;							// Временный профайла
uint8_t *DataSend;									// Указатель на данные для отправки
uint8_t N_Type;

/*
static uint8_t FramesCnt;
static uint8_t CurrentDtc;
static uint8_t FrameNoteNumber2Read;
*/

uint8_t ecuWriteDiagnosticValue(uint16_t did, uint32_t buf, uint16_t len);

void ObdSendMes(void)
{
	CanMsg *ObdMesSend;

     // Сообщение о результатах обноления профайла
    if (GetTimeFrom(TimeStampProfileUpdateAck) > REQUEST_PEROD_MS )
	{
		// Запись конфига
        if (N_Type == ECAN_PROFILE_FUN_WRITE)
		{			
			ObdMesSend = ecanGetEmptyTxMsg(P_CAN_CH);        

			if(ObdMesSend == 0)
				return;

			EcuConfig_t _config = GetConfigInstance();

			TimeStampProfileUpdateAck = GetTimeStamp();
			
			ObdMesSend->ID  = 0x580 | OD.ecuIndex;
			ObdMesSend->DLC = 8;             
			ObdMesSend->Ext = 0;
			ObdMesSend->data[0] = ECAN_PROFILE_FUN_WRITE | (UpdateProfileStatus << 4);              
			ObdMesSend->data[1] = Index2Update;
			ObdMesSend->data[2] = Index2Update >> 8;             
			ObdMesSend->data[3] = NoteNumber2Update;               

			N_Type = ECAN_NTYPE_NONE;	
         }
     }
     
     // Чтение конфига
     if (GetTimeFrom(TimeStampProfileRead) > REQUEST_PEROD_MS )
	 {
         if (N_Type == ECAN_PROFILE_FUN_READ && ReadDataCnt < MAX_REQUEST_AMOUNT)
		 {					
			ObdMesSend = ecanGetEmptyTxMsg(P_CAN_CH);     

			if(ObdMesSend == 0)
				return;

			EcuConfig_t _config = GetConfigInstance();

			TimeStampProfileRead = GetTimeStamp();

			ReadDataCnt++;
			ObdMesSend->ID = 0x580 | OD.ecuIndex;
			ObdMesSend->DLC = 8;             
			ObdMesSend->Ext = 0;
			
			ObdMesSend->data[1] = Index2Read;
			ObdMesSend->data[2] = Index2Read  >> 8;

			if (NoteNumber2Read < CONFIG_SIZE / 4)
			{							 
				UpdateProfileStatus = OK_UPDATE_PROFILE;
				ObdMesSend->data[4] = ((uint32_t *)(&_config))[NoteNumber2Read];
				ObdMesSend->data[5] = (((uint32_t *)(&_config))[NoteNumber2Read]) >> 8;	
				ObdMesSend->data[6] = (((uint32_t *)(&_config))[NoteNumber2Read]) >> 16;
				ObdMesSend->data[7] = (((uint32_t *)(&_config))[NoteNumber2Read]) >> 24;	
			}		  
			else
			{
				UpdateProfileStatus = END_OF_COMMUNICATION;
			}
			
			ObdMesSend->data[0] = ECAN_PROFILE_FUN_READ | (UpdateProfileStatus << 4);
			ObdMesSend->data[3] = NoteNumber2Read;
			N_Type = ECAN_NTYPE_NONE;
         }       
		else if(N_Type == ECAN_DIAG_VALUE_READ && ReadDataCnt < MAX_REQUEST_AMOUNT)
		{			
			ObdMesSend = ecanGetEmptyTxMsg(P_CAN_CH);  
			if(ObdMesSend == 0)
				return;

			TimeStampProfileRead = GetTimeStamp();
			EcuConfig_t _config = GetConfigInstance();
			
			ObdMesSend->ID = 0x580 | OD.ecuIndex;
			ObdMesSend->DLC = 8;       
			ObdMesSend->Ext = 0;			
			
			ObdMesSend->data[1] = Index2Read;
			ObdMesSend->data[2] = Index2Read  >> 8;

			if (BytesLenght2Read > 0)
			{							 
				UpdateProfileStatus = OK_UPDATE_PROFILE;				
				
				ObdMesSend->data[4] = DataSend[0];
				ObdMesSend->data[5] = (BytesLenght2Read > 1)? DataSend[1] : 0;				
				ObdMesSend->data[6] = (BytesLenght2Read > 2)? DataSend[2] : 0;
				ObdMesSend->data[7] = (BytesLenght2Read > 3)? DataSend[3] : 0;	
			}		  
			else
			{
				UpdateProfileStatus = END_OF_COMMUNICATION;
			}
			ObdMesSend->data[0] = ECAN_DIAG_VALUE_READ | (UpdateProfileStatus << 4);
			ObdMesSend->data[3] = NoteNumber2Read;
			N_Type = ECAN_NTYPE_NONE;
		}
     }
	 
	 if(GetTimeFrom(TimeStampKeepAlive) > KEEPALIVE_PEROD_MS)
	 {
		ObdMesSend = ecanGetEmptyTxMsg(P_CAN_CH);
		 
		if(ObdMesSend == 0)
			return;

		EcuConfig_t _config = GetConfigInstance();
		TimeStampKeepAlive = GetTimeStamp();    
		ObdMesSend->ID = 0x700 | OD.ecuIndex;
		ObdMesSend->DLC = 1;
		ObdMesSend->Ext = 0;
		ObdMesSend->data[0] = OD.StateMachine.MainState << 4 | OD.StateMachine.SubState;
	 }
}



uint8_t ObdThread (CanMsg * msg )
{				
	EcuConfig_t _config = GetConfigInstance();
	
	if (msg->ID == (0x600 | OD.ecuIndex))
	{
		N_Type = msg->data[0];
		switch(N_Type)
		{
			// Запись профайла
			case ECAN_PROFILE_FUN_WRITE:
			{				
				uint32_t _data;

				// Номер записи;
				Index2Update = ((msg->data[2]) << 8) + msg->data[1];
				NoteNumber2Update = msg->data[3];
				_data = (uint32_t)((msg->data[7] << 24) + (msg->data[6] << 16) + (msg->data[5] << 8) + msg->data[4]);
				
				UpdateProfileStatus = ecuWriteDiagnosticValue(Index2Update, _data, NoteNumber2Update);
			}
			break;

			// Чтение профайл
			case ECAN_PROFILE_FUN_READ:
			{				
				ReadDataCnt = 0;				 
				Index2Read  = ((msg->data[2]) << 8) + msg->data[1];
				NoteNumber2Read = msg->data[3];
			}
			break;
			
			case ECAN_DIAG_VALUE_READ:
			{
				ReadDataCnt = 0;
				Index2Read  = ((msg->data[2]) << 8) + msg->data[1];
				NoteNumber2Read = msg->data[3];
				BytesLenght2Read = GetDataByIndex(Index2Read, NoteNumber2Read, &DataSend);
			}
			break;		
			
		}
		

		return 0;
	}
	
 return 1;
}

uint8_t ecuWriteDiagnosticValue(uint16_t did, uint32_t buf, uint16_t NoteToWrite)
{
	static int16_t RequestCnt = 0;		// Счётчик запросов на следующую запись
	int8_t result = 0;
	if(did == didConfigStructIndex)		// Конфигурация
	{
		// Запрос в пределах диапазона
		if(NoteToWrite >= (CONFIG_SIZE / 4))
			return INDEX_OUTSIDE_BOUNDSE_ERROR;

		 ((uint32_t*)&TempProfile)[NoteToWrite] = buf;					 

		// Если это были последние данные
		if (NoteToWrite == (CONFIG_SIZE / 4) - 1)	
		{						
			result = (cfgCheck(&TempProfile))? UPDATE_PROFILE_FINISIHED : CRC_MISTMATCH_ERROR;
			RequestCnt++;						
		}
		else
		{
			RequestCnt = 0;
			return OK_UPDATE_PROFILE;
		}
		
		// Если профиль обновлен успешно, программная перезагрузка
		if(result == UPDATE_PROFILE_FINISIHED && RequestCnt == 1)			
		{		
			OD.SData.cfgData = &TempProfile;
			OD.SData.DataChanged = 1;
			//cfgWrite(&TempProfile);						
			//SetWorkState(&OD.StateMachine, WORKSTATE_INIT);
		}			
	}
	else if(did == didDateTime)		// Дата и время
	{
		if(NoteToWrite > 0)
			return INDEX_OUTSIDE_BOUNDSE_ERROR;
		
		uint32_t value = 0;
		memcpy(&value, &buf, sizeof(value));

		RTC_TIME_Type *time = dateTime_InitCurrent((uint32_t)value);
		// RTC
		RTC_SetFullTime(LPC_RTC, time);

	}
	else if(did == didFaults_History)
	{
		if(NoteToWrite > 0)
			return GENERAL_PROGRAMMING_FAILURE;		
		
		flashClearFaults(&OD.SData);
		OD.SData.DataChanged = 1;
	}
	else
	{
		return INDEX_OUTSIDE_BOUNDSE_ERROR;
	}

	return UPDATE_PROFILE_FINISIHED;
}


