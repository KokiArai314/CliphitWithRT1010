#ifndef AUDIO_TASK_H_
#define AUDIO_TASK_H_

#define NumOfSlot	(1)		//1fs = 1page

void audio_task_init();
void audio_task(float **ppfOut, int fs);

#endif
