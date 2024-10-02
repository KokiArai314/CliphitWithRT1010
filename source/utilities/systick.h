/*
 * systick.h
 *
 *  Created on: 2020/09/11
 *      Author: akino
 */

#ifndef HARDWARE_SYSTICK_H_
#define HARDWARE_SYSTICK_H_

void systick_init(void);
uint32_t systick_getCount(void);
uint32_t systick_getCount64(uint32_t *pHi);

#ifdef TIMING_MEASURRING
void systick_logPut(int orbit, int andbit, int tglbit);
int systick_logGet(uint32_t *pTime, uint32_t *pBits, int cnt);
#else	//TIMING_MEASURRING
#define systick_logPut(orbit, andbit, tglbit) ((void)0)
#define systick_logGet(pTime, pBits, cnt) ((void)0)
#endif	//TIMING_MEASURRING

#endif /* HARDWARE_SYSTICK_H_ */
