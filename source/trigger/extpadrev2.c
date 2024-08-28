/*
 * extpadrev2.c
 *
 *  Created on: 2021/06/03
 *      Author: akino
 */

#include <stdint.h>
#include "adc.h"
#include "trigger.h"
#include "extpad.h"

#include "../midi_debug_monitor/midi_debug_monitor.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define INPUTFILTER (1)	// 0:none,1:BPF,2:LPF
#define INPUTFILTER600	// INPUTFILTER=1の時に1kHz->600Hzへ
#define ENVMASK			// 定義すると envelope mask 有効

typedef struct {
	uint8_t		flag;
	uint16_t	maskCount;
	uint16_t	triggervalue[EXTPAD_ON_COUNT_MAX];
	int16_t		triggermin;
	int16_t		triggermax;
#if INPUTFILTER
#if (INPUTFILTER == 1)
	int16_t		filter1[2];
#endif	//#if INPUTFILTER EQ 1
#if (INPUTFILTER == 2)
	int16_t		filter1;
#endif	//#if INPUTFILTER EQ 1
	int16_t		filter2[2];
#endif	//#if INPUTFILTER
#if defined(ENVMASK)
	int16_t		envMaskCount;
	int16_t		envTriggerMin;
	int16_t		envTriggerMax;
	int16_t		envPreviousLevel;
#endif	//#if defined(ENVMASK)
}EXTPADWORK_t;

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
//extern void midi_IF_send_usb_blocking(uint8_t *str, uint16_t cnt);
//extern void midi_IF_send_uart_blocking(uint8_t *str, uint16_t cnt);

//extern void AdcAudioSet(uint16_t value, uint8_t ch);	// for debug

/*******************************************************************************
 * Variables
 ******************************************************************************/

#if INPUTFILTER
#if (INPUTFILTER == 1)
static EXTPADWORK_t extPadWork[EXTPAD_NUM_OF] = {{.flag=EXTPAD_VEL_WINDOW,.filter1={2048,2048}},
												  {.flag=EXTPAD_VEL_WINDOW,.filter1={2048,2048}},
												  {.flag=EXTPAD_VEL_WINDOW,.filter1={2048,2048}},
												  {.flag=EXTPAD_VEL_WINDOW,.filter1={2048,2048}}};
#endif	//#if (INPUTFILTER == 1)
#if (INPUTFILTER == 2)
static EXTPADWORK_t extPadWork[EXTPAD_NUM_OF] = {{.flag=EXTPAD_VEL_WINDOW,.filter1=2048},
												  {.flag=EXTPAD_VEL_WINDOW,.filter1=2048},
												  {.flag=EXTPAD_VEL_WINDOW,.filter1=2048},
												  {.flag=EXTPAD_VEL_WINDOW,.filter1=2048}};
#endif	//#if (INPUTFILTER == 2)
#else	//#if INPUTFILTER
static EXTPADWORK_t extPadWork[EXTPAD_NUM_OF] = {{.flag=EXTPAD_VEL_WINDOW},
												  {.flag=EXTPAD_VEL_WINDOW},
												  {.flag=EXTPAD_VEL_WINDOW},
												  {.flag=EXTPAD_VEL_WINDOW}};
#endif	//#if INPUTFILTER

/*******************************************************************************
 * Code
 ******************************************************************************/

static uint16_t getOtherLevel(int id)
{
	uint16_t ret = 0;
	int pairId = id ^ 1;

	for (int i = 0; i < EXTPAD_NUM_OF; i++)
	{
		if (i != id)
		{
//			TRIGSCN_t *pTrgScn = &triggerGetParamPtr()[eTrig_extPad1+i];
			uint16_t trg = extPadWork[i].triggermax - extPadWork[i].triggermin;

			trg >>= (i == pairId) ? 1 : 2;
			if (ret < trg)
			{
				ret = trg;
			}
		}
	}

	return ret;
}

/* --- extPad --- */
void extPadRev2(TRIGSCN_t *ptrigscn)
{
	EXTPAD_t		*pExtPad = &(ptrigscn->extPad);
	EXTPADWORK_t	*pExtPadWork = &(extPadWork[pExtPad->id]);
	uint16_t		trigger = adc_getValue(ptrigscn->adcCh1st);
	uint16_t		sigmax = pExtPad->vel.smax / 2;
	MINMAXCONV_t	cnvIn = {2048-sigmax, 2048+sigmax, 0, 4095};
	MINMAXCONV_t	cnvOut = {0, 4095, pExtPad->vel.tmin, pExtPad->vel.tmax};

	// trigger making part
	adc_clrFlag(ptrigscn->adcCh1st, ptrigscn->adcCh2nd);
#if INPUTFILTER
	{
#if (INPUTFILTER == 1)
#ifdef INPUTFILTER600
		int16_t fil1 = (trigger - (pExtPadWork->filter1[0] + pExtPadWork->filter1[1]) / 2) * 3 / 16;
		int16_t fil2 = fil1 + (pExtPadWork->filter2[0] * 7 / 4 - pExtPadWork->filter2[1] * 8 / 10);
#else	//#ifdef INPUTFILTER600
		int16_t fil1 = (trigger - (pExtPadWork->filter1[0] + pExtPadWork->filter1[1]) / 2) / 3;
		int16_t fil2 = fil1 + (pExtPadWork->filter2[0] * 3 / 2 - pExtPadWork->filter2[1] * 5 / 8);
#endif	//#ifdef INPUTFILTER600
		pExtPadWork->filter1[1] = pExtPadWork->filter1[0];
		pExtPadWork->filter1[0] = trigger;
#endif	//#if (INPUTFILTER == 1)
#if (INPUTFILTER == 2)
		int16_t filter = trigger - 2048;
		int16_t fil1 = (filter + pExtPadWork->filter1) / 8;
		int16_t fil2 = fil1 + (pExtPadWork->filter2[0] * 9 / 8 - pExtPadWork->filter2[1] * 3 / 8);
		pExtPadWork->filter1 = filter;
#endif	//#if INPUTFILTER
		pExtPadWork->filter2[1] = pExtPadWork->filter2[0];
		pExtPadWork->filter2[0] = fil2;
		fil2 += 2048;
		trigger = fil2 < 0 ? 0 : fil2 > 4095 ? 4095 : fil2;
	}
#endif	//#if (INPUTFILTER == 2)
	trigger = minmaxconv(trigger, &cnvIn);
	//AdcAudioSet(trigger, ptrigscn->adcCh1st);
	uint16_t onLevel = pExtPad->onLvl * EXTPAD_46_VEL_MAX50 / pExtPad->vel.smax;
	/* trigger refresh */
	if (pExtPadWork->flag && (pExtPadWork->flag < pExtPad->velWnd))
	{	// trigger velocity window
		for (int i = 0; i < pExtPad->onCnt-1; i++)
		{
			pExtPadWork->triggervalue[i] = pExtPadWork->triggervalue[i+1];
		}
		pExtPadWork->triggervalue[pExtPad->onCnt-1] = trigger;
		if (trigger < pExtPadWork->triggermin)
		{
			pExtPadWork->triggermin = trigger;
		}
		else if (trigger > pExtPadWork->triggermax)
		{
			pExtPadWork->triggermax = trigger;
		}
#if 1
		if (pExtPad->crsCan)
		{
			if (pExtPadWork->flag == (pExtPad->velWnd / 2))
			{
				uint16_t triggerLevel = pExtPadWork->triggermax - pExtPadWork->triggermin;
				uint16_t xtalkcan = getOtherLevel(pExtPad->id);	// -6dB(1/2) or -12dB(1/4)

				if (xtalkcan)
				{
					xtalkcan >>= pExtPad->crsCan;	// -> -12dB(1/4) or -18dB(1/8)
					if (triggerLevel > xtalkcan)
					{
						triggerLevel -= xtalkcan;
					}
					else
					{
						triggerLevel = 0;
					}
					if (triggerLevel <= onLevel)
					{	// cancel !
						pExtPadWork->flag = 0;
						if (trigger_debug_flag)
						{
							//dprintf(SDIR_USBMIDI, "\n trigger cancel %d !", pExtPad->id);
						}
					}
				}
			}
		}
#endif
	}
	else
	{	// off or on release
		pExtPadWork->triggermin = trigger;
		pExtPadWork->triggermax = trigger;
		for (int i = 0; i < pExtPad->onCnt-1; i++)
		{
			int value = pExtPadWork->triggervalue[i+1];

			pExtPadWork->triggervalue[i] = value;
			if (value < pExtPadWork->triggermin)
			{
				pExtPadWork->triggermin = value;
			}
			else if (value > pExtPadWork->triggermax)
			{
				pExtPadWork->triggermax = value;
			}
		}
		pExtPadWork->triggervalue[pExtPad->onCnt-1] = trigger;
	}

#if defined(ENVMASK)
	// envelope mask
	if (!pExtPadWork->envMaskCount)
	{	// next
		pExtPadWork->envPreviousLevel = pExtPadWork->envTriggerMax - pExtPadWork->envTriggerMin + onLevel;
		pExtPadWork->envTriggerMax = pExtPadWork->envTriggerMin = trigger;
		pExtPadWork->envMaskCount = pExtPad->velWnd + pExtPad->mskTim * 3 -1;	// x3 scan cycle
//		dprintf(SDIR_USBMIDI,"\n%d", pExtPadWork->envPreviousLevel);
	}
	else
	{	// now mask
		if (pExtPadWork->envTriggerMax < trigger)
		{
			pExtPadWork->envTriggerMax = trigger;
		}
		if (pExtPadWork->envTriggerMin > trigger)
		{
			pExtPadWork->envTriggerMin = trigger;
		}
		pExtPadWork->envMaskCount--;
	}
#endif	//#if defined(ENVMASK)

	// trigger detect check part
	trigger = pExtPadWork->triggermax - pExtPadWork->triggermin;
	if (pExtPadWork->maskCount)
	{
		pExtPadWork->maskCount--;
	}
	if (pExtPadWork->flag == 0)
	{	// now off
		uint16_t triggerLevel = trigger;

		if (pExtPad->crsCan)
		{
			uint16_t xtalkcan = getOtherLevel(pExtPad->id);	// -6dB(1/2) or -12dB(1/4)

			if (xtalkcan)
			{
				xtalkcan >>= pExtPad->crsCan;	// -> -12dB(1/4) or -18dB(1/8)
				if (triggerLevel > xtalkcan)
				{
					triggerLevel -= xtalkcan;
				}
				else
				{
					triggerLevel = 0;
				}
			}
		}
#if defined(ENVMASK)
		if (triggerLevel > pExtPadWork->envPreviousLevel)
#else	//#if defined(ENVMASK)
		if (triggerLevel > onLevel)
#endif	//#if defined(ENVMASK)
		{
			if (pExtPadWork->maskCount == 0)
			{
				pExtPadWork->flag++;
#if defined(ENVMASK)
				pExtPadWork->envMaskCount = pExtPad->mskTim * 3 - 1;	// x3 scan cycle;
				pExtPadWork->envTriggerMax = pExtPadWork->triggermax;
				pExtPadWork->envTriggerMin = pExtPadWork->triggermin;
#endif	//#if defined(ENVMASK)
			}
		}
	}
	else if (pExtPadWork->flag < pExtPad->velWnd)
	{	// now velocity window

		/* on check */
		if (++pExtPadWork->flag == pExtPad->velWnd)
		{	// on !
			uint16_t velocity = trigger;
			uint16_t sigmin = pExtPad->vel.smin * 4095 / pExtPad->vel.smax;

			velocity = velocity < sigmin ? 0 : minmaxconvtbl(velocity, &cnvOut, pExtPad->id + 10);
			if (ptrigscn->enable)
			{
				uint8_t txbuf[6] = {0x90+TRIGGER_CHANNEL, EXTPAD_NOTE_BASE, 0,	// note on
									0x90+TRIGGER_CHANNEL, EXTPAD_NOTE_BASE, 0};	// note off

				if (velocity)
				{
					txbuf[1] += pExtPad->id;
					txbuf[2] = velocity;
					txbuf[4] += pExtPad->id;
					//midi_IF_send_uart_blocking(txbuf, 6);
				}
			}
			if (trigger_debug_flag)
			{
				dprintf(SDIR_USBMIDI, "\n %c ext pad on %1d, %3d(%4d)", ptrigscn->enable ? 'S' : '-', pExtPad->id,
						velocity, trigger);
			}
			pExtPadWork->maskCount = pExtPad->mskTim * 3;	// scan cycle
		}
	}
	else
	{	// now on release
		if (pExtPadWork->maskCount == 0)
		{
			if (trigger_debug_flag)
			{
				dprintf(SDIR_USBMIDI, "\n %c ext pad off %d", ptrigscn->enable ? 'S' : '-', pExtPad->id);
			}
			pExtPadWork->flag = 0;
		}
	}

	return;
}
