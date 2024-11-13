/*
 * trigger.h
 *
 *  Created on: 2020/09/14
 *      Author: akino
 *
 *  MPS-10 to Cliphit2 by Arai
 */

#ifndef APPLICATION_TRIGGER_TRIGGER_H_
#define APPLICATION_TRIGGER_TRIGGER_H_

/*
#define EXTPAD_ON_COUNT		(32)	// =2msec
#define EXTPAD_ON_COUNT_MAX	(64)	// =4msec
#define EXTPAD_ON_LEVEL		(300)
#define EXTPAD_VEL_WINDOW	(64)	// =4msec
#define EXTPAD_MSK_TIME		(16*7/3)	// =7msec
#define EXTPAD_CLS_CAN		(0)		// 0:off
#define EXTPAD_46_VEL_MIN	 (50)
#define EXTPAD_46_VEL_MAX	(4095)*/

#include "parameter/STrigger.h"

typedef struct trigscn{
	int8_t	adcCh1st;
	int8_t	adcCh2nd;
	int8_t	enable;
	int8_t	ccValue;	// for cc
	void (*func)(struct trigscn *);
	union {
//		PAD_t		pad;
//		CCPAD_t		ccPad;
		EXTPAD_t	extPad;
//		SWPEDAL_t	swPedal;
//		VRPEDAL_t	vrPedal;
	};
} TRIGSCN_t;
/*
typedef struct {
	uint8_t			id;
	uint8_t			onCnt;
	uint8_t			velWnd;	// sample
	uint8_t			mskTim;	// sample
	uint8_t			crsCan;
	uint16_t		onLvl;
	MINMAXCONV_t	vel;
}EXTPAD_t;

typedef struct {
	uint8_t		flag;
	uint16_t	maskCount;
	uint16_t	triggervalue[EXTPAD_ON_COUNT_MAX];
	int16_t		triggermin;
	int16_t		triggermax;
#if INPUTFILTER
	int16_t		filter1[2];
#endif	
#if defined(ENVMASK)
	int16_t		envMaskCount;
	int16_t		envTriggerMin;
	int16_t		envTriggerMax;
	int16_t		envPreviousLevel;
#endif	//#if defined(ENVMASK)
}EXTPADWORK_t;
*/

void trigger_init(void);
void trigger_setVelocityCurve(int num, int ofs, int data);
uint8_t trigger_getVelocityCurve(int num, int ofs);
uint16_t minmaxconvtbl(uint16_t value, MINMAXCONV_t *cnv, int tblNum);
uint16_t minmaxconv(uint16_t value, MINMAXCONV_t *cnv);
void trigger_idle(void);

TRIGSCN_t *triggerGetParamPtr(void);

extern int trigger_debug_flag;

#endif /* APPLICATION_TRIGGER_TRIGGER_H_ */
