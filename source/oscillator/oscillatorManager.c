/*
	oscillatorManager.cpp to .c
	copy from x19850
	移植時にMidiDebugMonitorをシュリンク
	cpp -> c ： nullptrをNULLに変更 ちょっと怖い
*/

#include <stdint.h>
#include <stdio.h>
#include "oscillator.h"
#include "oscillatorProcess.h"
#include "oscillatorManager.h"


static OnMemOsc_t osc[MEMVOICEMAX];

static int onmemoscExec(void *ps, float **ppfOut, int fs)
{
	OnMemOsc_t *psOnMemOsc = (OnMemOsc_t *)ps;
	int ret = (psOnMemOsc->oscExec)(&psOnMemOsc->osc, ppfOut, fs);

	return psOnMemOsc->osc.Pitch ? 0 : ret;
}

static uint32_t onmemoscTell(Vcb_t *pVcb)
{
	OnMemOsc_t *pOsc = (OnMemOsc_t *)pVcb->runOscParam.oscObject;

	return pOsc->osc.CurOfs;
}

static void onmemoscSetup(Vcb_t *pVcb)
{
	OnMemOsc_t *pOsc = (OnMemOsc_t *)pVcb->runOscParam.oscObject;
	SampleData_t *psSampleData = pVcb->runOscParam.psSampleData;
	float pitch = pVcb->runOscParam.fPitch;

	OnMemoryOscillatorSetup(pOsc, psSampleData);

	pVcb->pfLevel = &(pOsc->osc.fLevel);
	pVcb->pfPan = &(pOsc->osc.fPan);

	(pOsc->oscSetup)(&pOsc->osc,psSampleData,psSampleData->SampleByte,
					 psSampleData->EndOfs, psSampleData->LoopOfs, 0,
					 psSampleData->StartOfs, 0, pOsc->osc.fLevel, pVcb->runOscParam.fPan);

	pOsc->osc.Pitch = (uint32_t)(OSCEQPITCH * pitch);	// valid
}

void onmemoryoscillatorstart(OscSetup_t *psOscSetup, int voiceNum)
{
	//psOscSetup->oscFunc = {onmemoscSetup,NULL,onmemoscTell,NULL,onmemoscExec};
	psOscSetup->oscFunc.setup = onmemoscSetup;
	psOscSetup->oscFunc.end = NULL;
	psOscSetup->oscFunc.tell= onmemoscTell;
	psOscSetup->oscFunc.seek = NULL;
	psOscSetup->oscFunc.exec = onmemoscExec;
	psOscSetup->oscObject = &osc[voiceNum];

	oscillatorStart(voiceNum, psOscSetup);

//	dprintf(SDIR_USBMIDI, (char *)"\n adr=%08x,len=%08x", psOscSetup->psSampleData->DataPtr, psOscSetup->psSampleData->Length);

	return;
}

/*
	for Test
*/
void onmemoryoscillatortest(SampleData_t *psSampleData, int bus)
{
	if (psSampleData)
	{
		OscSetup_t oscSetup;

		oscSetup.psSampleData = psSampleData;
		oscSetup.fPitch = 1.0f * psSampleData->FsAdjust;
		oscSetup.fVerocity = 1.0f;
		oscSetup.fLevel = 1.0f;
		oscSetup.fPan = 0.5f;
		oscSetup.fTempo = 1.0f;
		oscSetup.length = 0;//psSampleData->LoopOfs ? 0 : psSampleData->EndOfs + 1;
		oscSetup.egAttack = -1;
		oscSetup.egDecay = -1;
		oscSetup.egDecayCurve = 0;
		oscSetup.repeat = 1;
		oscSetup.outCh = bus;
		oscSetup.repeatCount = 1000;
		oscSetup.checkCount = 0;

		onmemoryoscillatorstart(&oscSetup, 0);
	}

	return;
}
