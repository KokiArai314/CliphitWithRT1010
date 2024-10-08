/*
 * rtt_debugger.c
 *
 *  Created on: 2024/10/04
 *      Author: koki_arai
 */

#include "rtt_debugger.h"
#include <stdint.h>

char RTT_UpBuffer1[4096];    // J-Scope RTT Buffer
char RTT_UpBuffer2[4096];    // J-Scope RTT Buffer
int  RTT_Channel = 1;       // J-Scope RTT Channel
int i;

void rtt_debugger_init(){
  SEGGER_RTT_ConfigUpBuffer(1, "JScope_u4", &RTT_UpBuffer1[0], sizeof(RTT_UpBuffer1), SEGGER_RTT_MODE_NO_BLOCK_SKIP);
  SEGGER_RTT_ConfigUpBuffer(2, "JScope_u4", &RTT_UpBuffer2[0], sizeof(RTT_UpBuffer2), SEGGER_RTT_MODE_NO_BLOCK_SKIP);
}

void rtt_debug_with_jscope(int channel,uint32_t val){
  #pragma pack(push, 1)
  struct {
    unsigned int vU32;
  } acValBuffer;

  acValBuffer.vU32 = val;
  SEGGER_RTT_Write(channel,&acValBuffer,sizeof(acValBuffer));
}


