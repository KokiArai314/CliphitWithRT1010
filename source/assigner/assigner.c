/*
 * assigner.c
 *
 *  Created on: 2024/09/17
 *      Author: koki_arai
 */

#include <stdint.h>
#include "assigner.h"
#include "../oscillator/oscillatorProcess.h"
#include "../oscillator/oscillatorManager.h"
#include "../data/pcmdata/declare_pcm_buffers.h"

//extern int16_t* getTestPcm();
extern Vcb_t vcb[];
extern int16_t pcm_buff_02[];

/**
 * フリーなvcbを探し番号を返す
 * 全てのvcbが使用されている場合は-1を返却
 */
int16_t findFreeVcb(void){
	for (int16_t i = 0; i < MEMVOICEMAX; i++)
	{
		if ((!vcb[i].flag.active) && (!vcb[i].flag.onReq))
		{
			return i;
		}
	}
	return -1;
}

/**
 * sampleNoを受け取ってvcbにオシレータを登録
 * MPS-10の発音系を簡単に使うためのクッション
 * フリーなvcbが存在せず、登録できなかった場合は-1を返す
 */
int16_t entryVcb(uint16_t sampleNo, float fVelocity){

	/*sample情報取得 for test*/
	int16_t* pcm_buff = &pcm_buff_02;
	SampleData_t sampleData;
	int samples = 1000;//sizeof(pcm_buff)/sizeof(pcm_buff[0]);

	SampleData_t *pSampleData = &sampleData;
	pSampleData->DataPtr = pcm_buff;
	pSampleData->SampleByte = 2;
	pSampleData->Length = samples;
	pSampleData->MonoStereo = 1;
	pSampleData->StartOfs = 0;
	pSampleData->TempoX100 = 120;
	pSampleData->FsAdjust = 1.0;
	pSampleData->EndOfs = samples-1;
	pSampleData->LoopOfs = 0;
	pSampleData->StartOfs = 0;

	/*sample情報からoscをスタート*/
	OscSetup_t oscSetup;
	oscSetup.psSampleData = pSampleData;
	oscSetup.fPitch = 1.0f * pSampleData->FsAdjust;
	oscSetup.fVerocity = fVelocity;
	oscSetup.fLevel = 1.0f;
	oscSetup.fPan = 0.5f;
	oscSetup.fTempo = 1.0f;
	oscSetup.length = pSampleData->LoopOfs ? 0 : pSampleData->EndOfs + 1;
	oscSetup.egAttack = -1;
	oscSetup.egDecay = -1;
	oscSetup.egDecayCurve = 0;
	oscSetup.repeat = 1;
	oscSetup.outCh = 0;
	oscSetup.repeatCount = 10;
	oscSetup.checkCount = 0;

	/*空きvcbを探す*/
	int16_t freeVcbIndex = findFreeVcb();
	if(freeVcbIndex==-1)return -1;

	onmemoryoscillatorstart(&oscSetup, freeVcbIndex);
	return 0;


}
