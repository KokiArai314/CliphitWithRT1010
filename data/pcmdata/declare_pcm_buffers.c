/*
 * declare_pcm_buffer.c
 *
 *  Created on: 2024/10/08
 *      Author: koki_arai
 */

#include "declare_pcm_buffers.h"

//__DATA(RAM3)

const int16_t const pcm_buff_00[]={
  #include "pcm_00.txt"
};

const int16_t const pcm_buff_01[]={
  #include "pcm_01.txt"
};

const int16_t const pcm_buff_02[]={
  #include "pcm_02.txt"
};

const int16_t const pcm_buff_03[]={
  #include "pcm_03.txt"
};

/*
int16_t* getTestPcm(){
	return &pcm_buff_02;
}*/

