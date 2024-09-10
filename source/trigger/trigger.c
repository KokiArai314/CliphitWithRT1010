/*
 * trigger.c
 *
 *  Created on: 2020/09/14
 *      Author: akino
 *
 *      MPS-10 to Cliphit2 by Arai
 */

#include <stdint.h>
//#include <extpadrev2.h>
#include "adc.h"
#include "trigger.h"
//#include "pad.h"
//#include "ccpad.h"
#include "extpadrev2.h"
#include "extpad.h"
//#include "swpedal.h"
//#include "vrpedal.h"

//#include "../MidiDebugMonitor/MidiDebugMonitor.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define ARRAYSIZE(aaa) (sizeof(aaa)/sizeof(aaa[0]))

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
//void jobTimeStart(int index);
//void jobTimeStop(int index);
//void jobTimeInterval(int index);

#define BOARD_RT1010TESTFORCLIPHIT

/*******************************************************************************
 * Variables
 ******************************************************************************/
static TRIGSCN_t trigscn[] = {
#ifdef BOARD_RT1010TESTFORCLIPHIT
	{EXTPAD_46_AD_CH,	-1,		1,	-1,	extPadRev2,
			{.extPad.id			= EXTPAD_46_ID,
			 .extPad.onCnt		= EXTPAD_ON_COUNT,
			 .extPad.velWnd		= EXTPAD_VEL_WINDOW,
			 .extPad.mskTim		= EXTPAD_MSK_TIME,
			 .extPad.crsCan		= EXTPAD_CLS_CAN,
			 .extPad.onLvl		= EXTPAD_ON_LEVEL,
			 .extPad.vel		= {EXTPAD_46_VEL_MIN, EXTPAD_46_VEL_MAX, 1,127}}},
#endif
};

int trigger_debug_flag = 0;

#define VELOCITYNUMOF (15)
#define VELOCITYCURVE (128)
static uint8_t velocityCurve[VELOCITYNUMOF][VELOCITYCURVE];

/*******************************************************************************
 * Code
 ******************************************************************************/

void trigger_init(void)
{
	for (int i = 0; i < VELOCITYNUMOF; i++)
	{
		for (int j = 0; j < VELOCITYCURVE; j++)
		{
			velocityCurve[i][j] = j;
		}
	}

	return;
}

void trigger_setVelocityCurve(int num, int ofs, int data)
{
	if ((num >= 0) && (num < VELOCITYNUMOF) &&
		(ofs >= 0) && (ofs < VELOCITYCURVE))
	{
		velocityCurve[num][ofs] = data & 0x7f;
	}

	return;
}

uint8_t trigger_getVelocityCurve(int num, int ofs)
{
	uint8_t ret = 0;

	if ((num >= 0) && (num < VELOCITYNUMOF) &&
		(ofs >= 0) && (ofs < VELOCITYCURVE))
	{
		ret = velocityCurve[num][ofs];
	}

	return ret;
}

uint16_t minmaxconvtbl(uint16_t value, MINMAXCONV_t *cnv, int tblNum)
{
	int16_t smin = cnv->smin;
	int16_t smax = cnv->smax;
	int16_t tmin = cnv->tmin;
	int16_t tmax = cnv->tmax;
	int32_t	temp = value;

	if (smin < smax)
	{	// normal clipping
		if (temp < smin)
		{
			temp = smin;
		}
		else if (value > smax)
		{
			temp = smax;
		}
	}
	else
	{	// invert clipping
		if (temp < smax)
		{
			temp = smax;
		}
		else if (value > smin)
		{
			temp = smin;
		}
	}
	temp = (temp - smin) * (tmax - tmin) * VELOCITYCURVE / (smax - smin) + tmin * VELOCITYCURVE;
	{
		int upper = temp >> 7;
		int lower = temp & 0x7f;
		int dt1 = velocityCurve[tblNum][upper >= VELOCITYCURVE ? 127 : upper];

		if (upper < (VELOCITYCURVE-1))
		{
			int dt2 = velocityCurve[tblNum][upper+1];

			dt1 += (dt2 - dt1) * lower / VELOCITYCURVE;
		}
		temp = dt1;
	}

	return temp;
}

uint16_t minmaxconv(uint16_t value, MINMAXCONV_t *cnv)
{
	int16_t smin = cnv->smin;
	int16_t smax = cnv->smax;
	int16_t tmin = cnv->tmin;
	int16_t tmax = cnv->tmax;
	int32_t	temp = value;

	if (smin < smax)
	{	// normal clipping
		if (temp < smin)
		{
			temp = smin;
		}
		else if (value > smax)
		{
			temp = smax;
		}
	}
	else
	{	// invert clipping
		if (temp < smax)
		{
			temp = smax;
		}
		else if (value > smin)
		{
			temp = smin;
		}
	}
	temp = (temp - smin) * (tmax - tmin) / (smax - smin) + tmin;

	return temp;
}

void trigger_idle(void)
{
	//jobTimeStart(2);
	for (int i = 0; i < ARRAYSIZE(trigscn); i++)
	{
		if (adc_getFlag(trigscn[i].adcCh1st, trigscn[i].adcCh2nd))
		{
			(trigscn[i].func)(&trigscn[i]);
		}
	}
	//jobTimeStop(2);

	return;
}

/*
 * for debug
 */

int debugSwitch(int sw)
{
	return trigger_debug_flag = sw;
}

TRIGSCN_t *triggerGetParamPtr(void)
{
	return trigscn;
}
