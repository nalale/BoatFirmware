/* 
 * File:   OBD.h
 * Author: Ag
 *
 * Created on 6 Февраль 2015 г., 14:33
 */

#ifndef OBD_H
#define	OBD_H


enum {
			OK_UPDATE_PROFILE = 0,         // Всё ок, ждём следующей послки
			UPDATE_PROFILE_FINISIHED,      // Обновление завершено
			CRC_MISTMATCH_ERROR,           // Не совпали контрольные суммы
			INDEX_OUTSIDE_BOUNDSE_ERROR,   // Индекс за пределами допустимых границ
			SERIES_ERROR,                  // Ошибка последовательности
			END_OF_COMMUNICATION,          // Конец обновления
			GENERAL_PROGRAMMING_FAILURE,
        };

#define ECAN_NTYPE_NONE			0
#define ECAN_PROFILE_FUN_READ   1
#define ECAN_PROFILE_FUN_WRITE  2
#define ECAN_GET_ARRAY_FUN      3
#define ECAN_DIAG_VALUE_READ	4
		
#define SET_SYSTEM_TIME         0xFE
#define KEEP_ALIVE_FUN          0xFF
/*******************************************************************************
 *                           ПРОТОТИПЫ ФУНКЦИЙ
*******************************************************************************/
uint8_t ObdThread(CanMsg * msg );
void ObdSendMes(void);
	  
	  
#endif	/* OBD_H */

