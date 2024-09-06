#ifndef __DECLARE_PCM_BUFFER_H__
#define __DECLARE_PCM_BUFFER_H__

#include <stdint.h>

static const int16_t pcm_buff_00[]={
  #include "pcm_00.txt" 
};

static const int16_t pcm_buff_01[]={
  #include "pcm_01.txt" 
};

static const int16_t pcm_buff_02[]={
  #include "pcm_02.txt" 
};

static const int16_t pcm_buff_03[]={
  #include "pcm_03.txt" 
};

#endif
