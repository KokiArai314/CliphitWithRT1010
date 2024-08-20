
#include "audio_task.h"
#include "../utilities/testSample.h"
#include "../Oscillator/oscillatorManager.h"

static float pfWork[6*2][NumOfSlot];

void audio_task_init(void)
{
	SampleData_t sampleData = get_test_sin_tone();
	onmemoryoscillatortest(&sampleData, 0);
	return;
}

void audio_task(float **ppfOut, int fs){
  /**
   * オーディオプロセスサブルーチン from x19850
   */

  /* clear */
	float *p = &pfWork[0][0];
	for (int i = 0; i < sizeof(pfWork)/sizeof(float); i++)
	{
		*p++ = 0.0f;
	}

	oscillatorProcess(ppfOut, fs);

};
