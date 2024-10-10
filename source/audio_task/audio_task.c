
#include "audio_task.h"
#include "../utilities/testSample.h"
#include "../Oscillator/oscillatorManager.h"
#include "../assigner/assigner.h"

static float pfWork[2][NumOfSlot];
static float *ppfWork[2] = {&(pfWork[0][0]),  &(pfWork[1][0])};	// to main

void audio_task_init(void)
{
	//entryVcb(0, 1.0f);
	return;
}

void audio_task(float **ppfOut, int fs){
  /**
   * オーディオプロセスサブルーチン from x19850 ulz1 AudioProcess.cpp
   */

  /* clear */
	float *p = &pfWork[0][0];
	for (int i = 0; i < sizeof(pfWork)/sizeof(float); i++)
	{
		*p++ = 0.0f;
	}

	oscillatorProcess(ppfWork, fs);

	/* out */
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

};
