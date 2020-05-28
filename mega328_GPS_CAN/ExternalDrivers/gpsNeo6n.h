/* 
 * File:   gpsNeo6n.h
 * Author: a.lazko
 *
 * Created on 25 мая 2020 г., 14:27
 */

#ifndef GPSNEO6N_H
#define	GPSNEO6N_H

#ifdef	__cplusplus
extern "C" {
#endif

void neo6m_Thread(void);
void neo6m_ProcessCharacter(char CChar);
uint16_t GetVelocityKmph(void);
uint32_t GetTime(void);

#ifdef	__cplusplus
}
#endif

#endif	/* GPSNEO6N_H */

