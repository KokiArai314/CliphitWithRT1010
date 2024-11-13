
#include "audio_task.h"
#include "../utilities/testSample.h"
#include "../Oscillator/oscillatorManager.h"
#include "../assigner/assigner.h"
#include "../voice/CLIPHIT2_voice.h"

void audio_task_init(void)
{
	//entryVcb(0, 1.0f);
	//entryVoice(0, 0, 1.0f);
	//entryVoice(1, 0, 1.0f);
	return;
}

void audio_task(int32_t *out){
	*out = 0;
	voiceFrameProcess(out);
}
/*
void audio_task_(float **ppfOut, int fs){

	float *p = &pfWork[0][0];
	for (int i = 0; i < sizeof(pfWork)/sizeof(float); i++)
	{
		*p++ = 0.0f;
	}

	oscillatorProcess(ppfWork, fs);

	for (int i = 0; i < 2; i++)
	{
		static const int chTbl[] = {0,1};

		float *po = ppfOut[i];			// dst(main L/R)
		float *pw = ppfWork[chTbl[i]];	// src

		for (int j = 0; j < fs; j++)
		{
			*po++ = *pw++;
		}
	}

};*/
