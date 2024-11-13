/*
 * CLIPHIT2_voice.h
 *
 *  Created on: 2024/10/24
 *      Author: koki_arai
 */

#ifndef VOICE_CLIPHIT2_VOICE_H_
#define VOICE_CLIPHIT2_VOICE_H_

#include <stdint.h>
#include "fader.h"
#include "../definitions.h"

#define VOICEIMGBIT		(28)
#define VOICEEQPITCH	(1<<VOICEIMGBIT)
#define VOICEIMGMSK		(VOICEEQPITCH-1)

typedef struct Sample_{
  int16_t		*data_p;	// StartAdrPointer or etc.
  uint32_t	length;
}Sample_t;

typedef struct Voice_ {
	union {
		struct {
			uint8_t active:1;	// 生きている
			uint8_t offReq:1;	// 停止要求
			uint8_t onReq:1;	// 開始要求
		};
		uint8_t all;
	} flag;
	int8_t vcbNum;
	float	level_f;
	Fader_t		msFader;
  Sample_t  sample_p;

	uint32_t	Pitch;		// 4:Real.28:Image
	uint32_t	CurOfs;		//
	uint32_t	EndOfs;		//
	uint32_t	CurImg;		// 28bit

} Voice_t;

int8_t 	findFreeVoice(void);
int8_t entryVoice(uint8_t padId, uint16_t instId, float level);
void voiceOn(uint8_t voiceNum,float level,float pitch,int16_t *pData, uint32_t length);
uint8_t voicePutSampleToOut(Voice_t *v, int32_t* out);
void 		voiceFrameProcess(int32_t* out);
void 		voiceProcessWithFlag(Voice_t *v,int32_t* out);


#endif /* VOICE_CLIPHIT2_VOICE_H_ */
