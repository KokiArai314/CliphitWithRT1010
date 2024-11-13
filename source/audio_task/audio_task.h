#ifndef AUDIO_TASK_H_
#define AUDIO_TASK_H_

#define NumOfSlot	(1)		//1fs = 1page
#include <stdint.h>

void audio_task_init();
void audio_task_(float **ppfOut, int fs);
void audio_task(int32_t *out);

#endif
