/*
 * declare_pcm_buffer.c
 *
 *  Created on: 2024/10/08
 *      Author: koki_arai
 */

#include "declare_pcm_buffers.h"
#include "cr_section_macros.h"

//__DATA(RAM3)

//__DATA(RAM3) int16_t pcm_buff_00[]={10};
//#include "pcm_00.txt"

//__DATA(RAM3) int16_t pcm_buff_01[]={10};//#include "pcm_01.txt"

int16_t pcm_buff_02[]={
  #include "pcm_02.txt"
};

//__DATA(RAM3) int16_t pcm_buff_03[]={10};//#include "pcm_03.txt"

/*
int16_t* getTestPcm(){
	return &pcm_buff_02;
}*/

