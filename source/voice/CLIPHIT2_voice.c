/*
 * CLIPHIT2_voice.c
 *
 *  Created on: 2024/10/24
 *      Author: koki_arai
 */

#ifndef VOICE_CLIPHIT2_VOICE_C_
#define VOICE_CLIPHIT2_VOICE_C_

#include "CLIPHIT2_voice.h"


/*for debug */
#include "../../data/pcmdata/declare_pcm_buffers.h"
#include "../midi_debug_monitor/midi_debug_monitor.h"
#include "../utilities/RTT/rtt_debugger.h"
extern const int16_t pcm_buff_01[];
extern const int16_t pcm_buff_02[];
void jobTimeStart(int index);
void jobTimeStop(int index);
void jobTimeInterval(int index);
/************/

static Voice_t voice[MAX_VOICE];

/**
 * フリーなvoiceを探し番号を返す
 * 全てのvoiceが使用されている場合は-1を返却
 */
int8_t findFreeVoice(void){
	for (int16_t i = 0; i < MAX_VOICE; i++)
	{
		if ((!voice[i].flag.active) && (!voice[i].flag.onReq))
		{
			return i;
		}
	}
	return -1;
}

int8_t entryVoice(uint8_t padId, uint16_t instId, float level){
	int8_t tgt = findFreeVoice();
	if(tgt<0)return -1;

  //Sample_t* s = &voice[tgt];
	const int16_t *pData;
	uint32_t length;
	if(padId == 0){
		pData = pcm_buff_02;
		length = 9000;
	}else{	
		pData = pcm_buff_01;
		length = 6000;
	}
	//dprintf(SDIR_USBMIDI, "\n %d pcm pointer", pData);
	
	/**fordebug */
	
	/********** */

	voiceOn(tgt,level,1.0f,pData,length);
	return 0;
}

void voiceOn(uint8_t voiceNum,float level,float pitch,int16_t *pData, uint32_t length){
	if(voiceNum>=MAX_VOICE)return;

	Voice_t *v = &voice[voiceNum];
	//本当はサンプルから
	/*for(int i=0; i<MAX_VOICE;i++){
		if(voice[i].flag.active == 1){
			dprintf(SDIR_USBMIDI, "\n %d voice active pcm pointer", voice[i].sample_p->data_p);
		} 
	}*/

	v->sample_p.data_p = pData;
	//dprintf(SDIR_USBMIDI, "\n %d pcm pointer", pData);
	v->sample_p.length = length;
	v->level_f = level;
	v->Pitch = (uint32_t)(VOICEEQPITCH * pitch);
	v->vcbNum = voiceNum;
	v->flag.onReq = 1;
	v->CurOfs = 0;
	v->CurImg = 0;

}

/**
 * @return 0:process complete 1:voice stop
 * @todo xfader
 */
uint8_t voicePutSampleToOut(Voice_t *v, int32_t* out){
	uint8_t voiceComplete = 0;

	uint32_t	pitch = v->Pitch;
	int16_t		*baseAdr = (int16_t *)(v->sample_p.data_p);

	if(!pitch)return 1;
	uint32_t	curOfs = v->CurOfs; //pitch操作のためのサンプル位置
	uint32_t	curImg = v->CurImg; //pitch操作のためのサンプル位置の小数点分
	uint32_t	endOfs = v->sample_p.length;
	uint32_t	pitchInt = (pitch >> VOICEIMGBIT);
	uint32_t	pitchImg = pitch & VOICEIMGMSK;

	/*Pitch Process */
	//jobTimeStart(0);
	int16_t dt, dt1, dt2;
	dt1 = baseAdr[curOfs];
	dt2 = baseAdr[curOfs+1];
	//jobTimeStop(0);

	// データ生成
	dt = dt2 - dt1;
	dt *= curImg;
	dt /= VOICEEQPITCH;
	dt += dt1;

	/*!!! */
	dt = dt1;
	/*!!! */

	// アドレス更新
	curOfs += pitchInt;
	curImg = pitchImg + curImg;
	if (curImg > VOICEEQPITCH)
	{
		curImg -= VOICEEQPITCH;
		curOfs++;
	}

	// 終了チェック
	if (curOfs >= endOfs)
	{
		v->Pitch = 0;	// 停止
		voiceComplete = 1;
	}

	v->CurOfs = curOfs;
	v->CurImg = curImg;

	//!! 16bitのサンプルを24bitの出力に入れておくので嵩上げする
	*out = *out + (int32_t)(((int32_t)dt * v->level_f));
	
	return voiceComplete;
}
/**
 * 
 */
void voiceProcessWithFlag(Voice_t *v,int32_t* out){
	
	if (v->flag.active)
	{	// active
		
		v->flag.onReq = 0;
		if (v->flag.offReq){ //off request, set msfader.
			v->flag.offReq = 0;
			v->flag.active = 0;
			return;
		}
	}else
	{//not active
		if (v->flag.onReq){
			v->flag.active = 1;
			v->flag.onReq = 0;		// オン要求消化
		}else{
			return; /**************************** activeでもon reqでもないならreturn!!!! ***/
		}
	}

	uint8_t voiceDone = voicePutSampleToOut(v,out);
	if(voiceDone){
		v->flag.active = 0;
	}
	return ;
}

/**
 * 
 */
void voiceFrameProcess(int32_t* out){
	for(int8_t i=0;i<MAX_VOICE;i++){
		Voice_t *v = &voice[i];
		voiceProcessWithFlag(v, out);
	}
}

#endif /* CLIPHIT2_OSC_C_ */


