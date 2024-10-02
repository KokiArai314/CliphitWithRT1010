/*
 * systick.c
 *
 *  Created on: 2020/09/11
 *      Author: akino
 */

#include "clock_config.h"
#include "systick.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#ifdef TIMING_MEASURRING
#define LOGGING (1000)	// 1以上を定義するとログが有効
#endif	//#ifdef TIMING_MEASURRING

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
volatile uint32_t g_systickCounter = 0;

#if defined(LOGGING) && (LOGGING != 0)
typedef struct log_ {
	uint32_t time;
	uint32_t bits;
} LOG_t;

static int wpos = 0;

static LOG_t log[LOGGING] = {0};
#endif	//defined(LOGGING) && (LOGGING != 0)

/*******************************************************************************
 * Code
 ******************************************************************************/

void SysTick_Handler(void)
{
    g_systickCounter++;

    return;
}

void systick_init(void)
{
	if (SysTick_Config(0x1000000))
	{
		while(1)
		{
		}
	}

	return;
}

uint32_t systick_getCount(void)
{
	uint32_t ret, cnt1, cnt2;

	do {
		cnt1 = g_systickCounter;
		ret = (SysTick->VAL) ^ 0x00fffffful;	// for down counter !
		cnt2 = g_systickCounter;
	} while (cnt1 != cnt2);
	ret += cnt2 << 24;

	return ret;
}

uint32_t systick_getCount64(uint32_t *pHi)
{
	uint32_t ret, cnt1, cnt2;

	do {
		cnt1 = g_systickCounter;
		ret = (SysTick->VAL) ^ 0x00fffffful;	// for down counter !
		cnt2 = g_systickCounter;
	} while (cnt1 != cnt2);
	ret += cnt2 << 24;
	if (pHi)
	{
		*pHi = cnt2 >> 8;
	}

	return ret;
}

#if defined(LOGGING) && (LOGGING != 0)
void systick_logPut(int orbit, int andbit, int tglbit)
{
	static uint64_t start = 0;
	static uint32_t bits = 0;
	uint64_t time;
	uint32_t hi, lo;

	lo = systick_getCount64(&hi);
	time = ((uint64_t)hi << 32) + lo;
	if ((orbit < 0) && (andbit < 0) && (tglbit < 0))
	{	// reset
		start = time;
		bits = 0;
		wpos = 0;
	}
	else if (wpos < LOGGING)
	{
		time -= start;
		if (orbit >= 0)
		{
			bits |= 1 << orbit;
		}
		if (andbit >= 0)
		{
			bits &= ~(1 << andbit);
		}
		if (tglbit >= 0)
		{
			bits ^= 1 << tglbit;
		}
		log[wpos].time = time;
		log[wpos].bits = bits;
		wpos++;
	}

	return;
}
#endif	//defined(LOGGING) && (LOGGING != 0)

#if defined(LOGGING) && (LOGGING != 0)
int systick_logGet(uint32_t *pTime, uint32_t *pBits, int cnt)
{
	int ret = -1;

	if (cnt < wpos)
	{
		*pTime = log[cnt].time;
		*pBits = log[cnt].bits;
		ret = cnt;
	}

	return ret;
}
#endif	//defined(LOGGING) && (LOGGING != 0)
