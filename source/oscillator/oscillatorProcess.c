/*
	oscillatorProcess.cpp to .c
	copy from x19850
	FsMsecTask.hのシュリンク
*/

#include <stdint.h>
#include <stdio.h>
#include "../utilities/numericUtil.h"
//#include "wave/wavesampleplayer.h"
//#include "assigner/assigner.h"
//#include "assigner/assignerInst.h"
//#include "assigner/assignerOsc.h"
#include "egCurve.h"
#include "oscillatorProcess.h"
#include "../midi_debug_monitor/midi_debug_monitor.h"

#define VOICEMAX	(MEMVOICEMAX+DRVVOICEMAX)

void jobTimeStart(int index);
void jobTimeStop(int index);
void jobTimeInterval(int index);

/*
	pre

	active	offReq	egMute	onReq	setupReq			active	offReq	egMute	onReq	setupReq
	1		x		0		1		x			->		1		0		1		1		x
	1		x		1		1		x			->		1		x		1		1		x
	1		1		x		x		x			->		1		0		1		x		x
	0		x		x		1		x			->		1		x		x		0		0

	post (eg end)

	active	offReq	egMute	onReq	setupReq			active	offReq	egMute	onReq	setupReq

	1		x		1		x		x			->		0		x		0		x		x
*/

Vcb_t vcb[VOICEMAX] = {0};	// 前半が memory voice, 後半が drive voice

static const float decaytbl[] ={
        2,      2,      2,      3,      4,
        5,      6,      7,      8,     10,
       12,     16,     18,     20,     23,
       26,     29,     32,     35,     38,
       44,     48,     52,     58,     64,
       72,     80,     86,     91,     96,
      104,    112,    128,    136,    144,
      160,    176,    184,    192,    208,
      224,    240,    272,    288,    304,
      336,    352,    384,    400,    432,
      464,    496,    544,    576,    624,
      672,    720,    768,    816,    880,
      944,   1008,   1088,   1168,   1248,
     1344,   1440,   1536,   1648,   1776,
     1888,   2032,   2176,   2336,   2496,
     2688,   2880,   3072,   3296,   3536,
     3808,   4112,   4448,   4848,   5312,
     5856,   6512,   7280,   8256,   9440,
    10912,  12784,  15184,  18384,  22784,
    29120,  37440,  49920,  65536,  87376,
   100000
};

float getEgTimeValueF(int value)
{
	float ret = 1.0f;

	if ((value >= 0) && (value < (sizeof(decaytbl)/sizeof(decaytbl[0]))))
	{
		ret = decaytbl[value];
	}

	return ret;
}

uint32_t chkFreeMemVcb(void)
{
	uint32_t ret = 0;

	for (int i = 0; i < MEMVOICEMAX; i++)
	{
		if ((!vcb[i].flag.active) && (!vcb[i].flag.onReq))
		{
			ret |= 1 << i;
		}
	}

	return ret;
}

uint32_t chkFreeDrvVcb(void)
{
	uint32_t ret = 0;

	for (int i = MEMVOICEMAX; i < VOICEMAX; i++)
	{
		if ((!vcb[i].flag.active) && (!vcb[i].flag.onReq))
		{
			ret |= 1 << (i - MEMVOICEMAX);
		}
	}

	return ret;
}

#define INTERRUPT_TIME			1			// msec

/**
 * 発音開始用Initialize
 * @param egP
 * @param attack : minus=NoAttack
 * @param decay : minus=NoDecay
 * @param AmpEGCurve
 */
static void init_ampeg_coef(sAmpEGCoef* egP, const int attack, const int decay, const int AmpEGCurve, unsigned long length)
{
	egP->EGMasterPhase = 0.0f;
	if (attack < 0) {
		egP->attack = 255;
		if (decay < 0) {
			egP->phase = eAmpEgPhaseStop;
			egP->decay = 255;
			egP->TimeIncValue = 0.0f;
		}
		else {
			egP->phase = eAmpEgPhaseDecay;
			egP->decay = uint8Clip(decay, 0, 100);
			if (length)
			{
				length = (uint32_t)(length * (decay / 100.0f));
				egP->TimeIncValue = 0.0f;
				egP->decayStart = 0;
				egP->decayLength = length == 0 ? 1 : length;
			}
			else
			{
				egP->TimeIncValue = INTERRUPT_TIME / decaytbl[egP->decay];
				egP->decayLength = 0;
			}
		}
	}
	else {
		egP->phase = eAmpEgPhaseAttack;
		if (decay < 0) {
			egP->decay = 255;
		}
		else {
			egP->decay = uint8Clip(decay, 0, 100);
		}
		egP->attack = uint8Clip(attack, 0, 100);
		egP->TimeIncValue = INTERRUPT_TIME / decaytbl[egP->attack];
		egP->decayLength = 0;
	}

	egP->LPF_State = 0.0f;
	egP->AmpEGCurve = AmpEGCurve;
}

void oscillatorProcessAmpEgReset(Vcb_t *pVcb)
{
	int attack = pVcb->ampEg.attack > 100 ? -1 : pVcb->ampEg.attack;
	int decay = pVcb->ampEg.decay > 100 ? -1 : pVcb->ampEg.decay;
	int AmpEGCurve = pVcb->ampEg.AmpEGCurve;
	unsigned long length = pVcb->runOscParam.length;

	return init_ampeg_coef(&pVcb->ampEg, attack, decay, AmpEGCurve, length);
}

static void oscSetup(Vcb_t *pVcb)
{
	pVcb->runOscParam = pVcb->nxtOscParam;
	if (pVcb->runOscParam.oscFunc.setup)
	{
		(pVcb->runOscParam.oscFunc.setup)(pVcb);
		pVcb->voiceLevel = pVcb->runOscParam.fLevel * pVcb->runOscParam.fVerocity;
		init_ampeg_coef(&pVcb->ampEg, pVcb->runOscParam.egAttack, pVcb->runOscParam.egDecay, pVcb->runOscParam.egDecayCurve, pVcb->runOscParam.length);
		if (pVcb->ampEg.phase == eAmpEgPhaseStop)
		{
			pVcb->oscLevel = pVcb->voiceLevel;
		}
		else
		{
			pVcb->oscLevel = 0.0f;
		}
		pVcb->msFader.fGoal = pVcb->msFader.fCur = pVcb->oscLevel;
		pVcb->msFader.fRate = 0.0f;
	}

	return;
}

static int oscEnd(Vcb_t *pVcb)
{
	int stp = 1;


	if (pVcb->flag.active)
	{
		pVcb->flag.active = 0;
		pVcb->flag.egMute = 0;	//m
		/*
		if (pVcb->runOscParam.repeat)
		{	// repeat !
			//int padNum = assignerInstGetPadNum(pVcb->vcbNum);

			pVcb->flag.onReq = 1;
			pVcb->flag.setupReq = 1;
			if ((padNum >= 0) && (padNum < constNumOfKitInst))
			{
				int masterPitchEnable = pVcb->runOscParam.repeat - 1;
				float centPitch = pVcb->nxtOscParam.fCentPitch;
				float fsAdjust = pVcb->nxtOscParam.psSampleData->FsAdjust;

				pVcb->nxtOscParam.fPitch = assignerOscGetPitch(centPitch, padNum, masterPitchEnable) * fsAdjust;
			}
			else if (pVcb->runOscParam.fTempo == -1.0f)
			{	// stretcher
				int tempoX100 = ((Stretcher_t *)pVcb->runOscParam.oscObject)->TempoX100;

				pVcb->nxtOscParam.fTempo = (float)tempoX100 / 100.0f;
			}
			stp = 0;
		}
		else
		{	// no repeat
			if(pVcb->runOscParam.oscFunc.end)
			{
				(pVcb->runOscParam.oscFunc.end)(pVcb);
			}
			assignerInstVoiceRelease(pVcb->vcbNum);
		}*/
	// no repeat
		if(pVcb->runOscParam.oscFunc.end)
		{
			(pVcb->runOscParam.oscFunc.end)(pVcb);
		}
		//assignerInstVoiceRelease(pVcb->vcbNum);
	}


	return stp;
}

/*
	worker @ 4fs
*/
static int oscillatorProcessSub(Vcb_t *pVcb, float **ppfDst)
{
	const int workSamples = 1;
	int stp = 0;

	if (pVcb->flag.active)
	{	// active
		if (pVcb->flag.onReq)
		{	// on req
			if (!pVcb->flag.egMute)
			{	// not mute
				pVcb->flag.offReq = 0;	// オフ要求消化
				pVcb->flag.egMute = 1;
			}
		}
		else if (pVcb->flag.offReq)
		{	// off req
			pVcb->flag.offReq = 0;		// オフ要求消化
			if (!pVcb->flag.egMute)
			{	// not mute
				pVcb->flag.egMute = 1;
			}
		}
		if ((pVcb->flag.egMute || pVcb->flag.pauseReq) && (pVcb->msFader.fGoal != 0.0f))
		{	// mute request
			pVcb->msFader.fGoal = 0.0f;
			pVcb->msFader.fRate = pVcb->msFader.fCur / (44.1f/workSamples);	// 1msec fade
			if (pVcb->msFader.fCur > pVcb->msFader.fRate)
			{	// 1st substruct
				pVcb->msFader.fCur -= pVcb->msFader.fRate;
			}
		}
	}
	else
	{	// not active
		if (pVcb->flag.onReq)
		{	// on req
			pVcb->flag.active = 1;
			pVcb->flag.onReq = 0;		// オン要求消化
			if (pVcb->flag.setupReq)
			{	// setup req
				pVcb->flag.setupReq = 0;// セットアップ要求消化
				oscSetup(pVcb);
			}
			pVcb->fsCount = 0;
			if (pVcb->flag.pauseReq)
			{
				pVcb->msFader.fRate = pVcb->msFader.fCur = pVcb->msFader.fGoal = 0.0f;
			}
		}
		else
		{	// not on req
			return stp;
		}
	}

	if (pVcb->runOscParam.oscFunc.exec)
	{	// exist
		if (pVcb->pfLevel)
		{	// exist
			*(pVcb->pfLevel) = pVcb->msFader.fCur;
		}

		stp = (pVcb->runOscParam.oscFunc.exec)(pVcb->runOscParam.oscObject, ppfDst, workSamples);

		pVcb->fsCount += workSamples;

		if (pVcb->runOscParam.checkCount)
		{
			stp = pVcb->fsCount < pVcb->runOscParam.checkCount ? 0 : 1;
		}
		if (stp)
		{	// stop
			stp = oscEnd(pVcb);
		}
	}

	if (pVcb->msFader.fRate != 0.0f)
	{	// msfader running
		xfaderExecute(&pVcb->msFader);
		if (pVcb->msFader.fCur < AMPEG_ZERO_CLIP)
		{	// off level
			if (pVcb->msFader.fGoal == 0.0f)
			{	// off ! for mute
				pVcb->msFader.fRate = 0.0f;
				pVcb->msFader.fCur = 0.0f;
			}
		}
	}
	if (pVcb->msFader.fRate == 0.0f)
	{	// msfader end
		if (pVcb->flag.egMute)
		{	// mute
//m			pVcb->flag.egMute = 0;
			pVcb->runOscParam.repeat = 0;
			stp = oscEnd(pVcb);	// with pVcb->flag.egMute = 0;
		}
		else if (pVcb->flag.pauseReq)
		{
			pVcb->flag.pause = 1;
			pVcb->flag.pauseReq = 0;
		}
		else
		{	// normal
			if (pVcb->msFader.fGoal != pVcb->oscLevel)
			{	// change
				pVcb->msFader.fGoal = pVcb->oscLevel;

				float fTemp = pVcb->msFader.fGoal - pVcb->msFader.fCur;
				fTemp = fTemp < 0.0f ? - fTemp : fTemp;
				pVcb->msFader.fRate = fTemp / (44.1f/workSamples);	// 1msec fade
			}
			else if (pVcb->ampEg.phase == eAmpEgPhaseOff)
			{	// off
				pVcb->ampEg.phase = eAmpEgPhaseStop;
				stp = oscEnd(pVcb);
			}
		}
	}

	return stp;
}

static void egProcessSub(Vcb_t *pVcb)
{
	if (pVcb->ampEg.phase)
	{	// eg active
		sAmpEGCoef* const egP = &(pVcb->ampEg);
		float egCoef = 0.0f;	// off

		if (egP->decayLength)
		{
			float pos = pVcb->runOscParam.oscFunc.tell(pVcb);

			pos -= egP->decayStart;
			pos /= egP->decayLength;
			egP->EGMasterPhase = pos;
		}
		else
		{
			egP->EGMasterPhase += egP->TimeIncValue;
		}
		// clip
		if(egP->EGMasterPhase>1.0f){
			egP->EGMasterPhase = 1.0f;
		}
		if (egP->phase == eAmpEgPhaseAttack)
		{	// attack
			egCoef = egP->EGMasterPhase;
			if (egP->EGMasterPhase == 1.0f)
			{	// end
				int decay = egP->decay;

				if (decay < sizeof(decaytbl)/sizeof(decaytbl[0]))
				{	// decay valid
					uint32_t length = pVcb->runOscParam.length;

					egP->phase = eAmpEgPhaseDecay;
					egP->EGMasterPhase = 0.0f;
					if (length)
					{
						egP->decayStart = pVcb->runOscParam.oscFunc.tell(pVcb);
						length -= length < egP->decayStart ? length : egP->decayStart;
						length = (uint32_t)(length * (decay / 100.0f));
						egP->decayLength = length == 0 ? 1 : length;
					}
					else
					{
						egP->TimeIncValue = INTERRUPT_TIME / decaytbl[decay];
						egP->decayLength = 0;
					}
				}
				else
				{	// infinit
					egP->phase = eAmpEgPhaseStop;
				}
			}
		}
		else if (egP->phase == eAmpEgPhaseDecay)
		{	// decay
			static const float eg_coef = 0.0158386f;

			// LPF
			egP->LPF_State += eg_coef * (egP->EGMasterPhase - egP->LPF_State);
			// clip
			if(egP->LPF_State > (1.0f - AMPEG_ZERO_CLIP)){
				egP->LPF_State = 1.0f;
			}

			egCoef = (1.0f - egP->LPF_State);
			if (egCoef != 0.0f) {
				// Curve処理
				egCoef = 1.0f - egCurveCalc(egP->AmpEGCurve, 1.0f - egCoef);
			}
			else {
				// to off
				egP->phase = eAmpEgPhaseOff;
			}
		}
		pVcb->oscLevel = egCoef * pVcb->voiceLevel;
	}
	else
	{	// eg stop then level thru
		pVcb->oscLevel = pVcb->voiceLevel;
	}

	return;
}

/*
	EG process called @ 1fs //4fs
	4fs to 1fs(MPS-10 to ClipHit)
*/
static void egProcess(void)
{
	for (int i = 0; i < 6; i++)
	{
		if (i < VOICEMAX)
		{	// valid
			if ((vcb[i].flag.active) && (!vcb[i].flag.egMute) && (!vcb[i].flag.pause))
			{	// active && not mute && not pause
				jobTimeStart(4);
				egProcessSub(&vcb[i]);
				jobTimeStop(4);
			}
		}
	}

	return;
}

/*
	called from audio process
*/
void oscillatorProcess(float **ppfOut, int fs)
{
	for (int i = 0; i < VOICEMAX; i++)
	{
		if ((vcb[i].flag.active || vcb[i].flag.onReq) && (!vcb[i].flag.pause))
		{
			jobTimeStart(3);
			const int workSamples = 1;
			volatile Vcb_t *pVcb = &vcb[i];

			for (int cnt = 0; cnt < fs; cnt += workSamples)
			{
				int outofs = pVcb->runOscParam.outCh * 2;
				float *ppfDWrk[2] = {&ppfOut[outofs][cnt],&ppfOut[outofs+1][cnt]};
				int stp = oscillatorProcessSub((Vcb_t *)pVcb, ppfDWrk);

				if ((stp) || (vcb[i].flag.pause))
				{	// stop or pause
					break;
				}
			}
			jobTimeStop(3);
		}
	}
	while (fs >= 1)
	{
		egProcess();
		fs -= 1;
	}

	return;
}

/*
	called from other task
*/
void oscillatorStart(uint8_t oscNum, OscSetup_t *psOscSetup)
{
	if (oscNum < VOICEMAX)
	{
		Vcb_t *pVcb = &vcb[oscNum];

		pVcb->vcbNum = oscNum;	// 自身の番号

		pVcb->runOscParam.repeat = 0;	// 再スタートを抑止
		pVcb->flag.onReq = 0;			// スタートキャンセル
		pVcb->flag.setupReq = 0;		// 設定キャンセル
		pVcb->flag.offReq = 0;			// オフキャンセル

		pVcb->nxtOscParam = *psOscSetup;

		if (oscNum < MEMVOICEMAX)
		{	// memory
		}
		else
		{	// drive
		}

		if (!pVcb->flag.active)
		{
			oscSetup(pVcb);
		}
		else
		{
			pVcb->flag.setupReq = 1;
		}

		pVcb->flag.onReq = 1;
	}

	return;
}

void oscillatorStop(uint8_t oscNum)
{
	if (oscNum < VOICEMAX)
	{
		Vcb_t *pVcb = &vcb[oscNum];
#if 1
		pVcb->runOscParam.repeat = 0;	// 再スタートを抑止
		pVcb->nxtOscParam.repeat = 0;	//

		if (!pVcb->flag.egMute)
		{
			pVcb->flag.offReq = 1;
			pVcb->flag.pauseReq = 0;	// ポーズは
			pVcb->flag.pause = 0;		// 解除する
		}
#else
		pVcb->runOscParam.repeat = 0;	// 再スタートを抑止
		pVcb->flag.onReq = 0;			// スタートキャンセル
		pVcb->flag.setupReq = 0;		// 設定キャンセル

		if (pVcb->flag.active)
		{
			if (!pVcb->flag.egMute)
			{
				pVcb->flag.offReq = 1;
				pVcb->flag.pauseReq = 0;	// ポーズは
				pVcb->flag.pause = 0;		// 解除する
			}
		}
#endif
	}

	return;
}

bool oscillatorPauseTgl(uint8_t oscNum)
{
	bool ret = false;

	if (oscNum < VOICEMAX)
	{
		Vcb_t *pVcb = &vcb[oscNum];

		if (pVcb->flag.active)
		{	// now active
			if (pVcb->flag.pause || pVcb->flag.pauseReq)
			{	// now pause;
				pVcb->flag.pauseReq = 0;
				pVcb->flag.pause = 0;
			}
			else
			{	// now not pause
				pVcb->flag.pauseReq = 1;
				ret = true;
			}
		}
		else
		{	// now not active
			if (pVcb->flag.pause)
			{	// now pause;
				pVcb->flag.pause = 0;
			}
			else
			{	// now not pause
				pVcb->flag.pause = 1;
				ret = true;
			}
			pVcb->flag.pauseReq = 0;
		}
	}

	return ret;
}

bool oscillatorPauseGet(uint8_t oscNum)
{
	bool ret = false;

	if (oscNum < VOICEMAX)
	{
		Vcb_t *pVcb = &vcb[oscNum];

		if ((pVcb->flag.pause || pVcb->flag.pauseReq))
		{	// now pause
			ret = true;
		}
	}

	return ret;
}

void oscillatorPauseSet(uint8_t oscNum, bool pause)
{
	if (oscNum < VOICEMAX)
	{
		Vcb_t *pVcb = &vcb[oscNum];

		if (pVcb->flag.active)
		{	// now active
			if (pause)
			{	// to pause
				if ((!pVcb->flag.pause) && (!pVcb->flag.pauseReq))
				{	// now not pause
					pVcb->flag.pauseReq = 1;
				}
			}
			else
			{	// to not pause
				pVcb->flag.pauseReq = 0;
				pVcb->flag.pause = 0;
			}
		}
		else
		{	// now not active
			if (pause)
			{	// to pause
				pVcb->flag.pause = 1;
			}
			else
			{	// to not pause
				pVcb->flag.pause = 0;
			}
			pVcb->flag.pauseReq = 0;
		}
	}

	return;
}

int oscillatorSeek(uint8_t oscNum, int32_t seekSample)
{
	int ret = 0;	// end then 1

	if (oscNum < VOICEMAX)
	{
		volatile Vcb_t *pVcb = &vcb[oscNum];
//		float dumyL[4], dumyR[4];
//		float *ppfDWrk[2] = {&dumyL[0], &dumyR[0]};

		if (!oscillatorPauseGet(oscNum))
		{	// now not pause
			oscillatorPauseSet(oscNum, true);
		}
		if (pVcb->flag.active)
		{	// now active
			while (!pVcb->flag.pause) {}	// wait to pause
		}
#if 1
		if (pVcb->runOscParam.oscFunc.seek)
		{
			ret = (pVcb->runOscParam.oscFunc.seek)((Vcb_t *)pVcb, seekSample, oscillatorProcessSub, egProcessSub);
			if (ret)
			{
				ret = oscEnd((Vcb_t *)pVcb);
			}
		}
#else
		if (seekSample > 0)
		{	// forward
			while (seekSample >= 44)
			{
				for (int i = 0; i < 11; i++)
				{	// 1msec loop
					if (oscNum >= MEMVOICEMAX)
					{
						wavesampleplayerWaitSampleDataRequestClear((Vcb_t *)pVcb);
					}
					ret = oscillatorProcessSub((Vcb_t *)pVcb, ppfDWrk);
					if (ret)
					{	// end !
						break;
					}
				}
				if (ret)
				{	// end !
					break;
				}
				egProcessSub((Vcb_t *)pVcb);
				seekSample -= 44;
			}
		}
		else if (seekSample < 0)
		{
			// @todo
		}
#endif
	}

	return ret;
}

Vcb_t *getVcbPtr(int num)
{
	Vcb_t *pVcb = NULL;

	if ((num >= 0) && (num < VOICEMAX))
	{
		pVcb = &vcb[num];
	}

	return pVcb;
}
