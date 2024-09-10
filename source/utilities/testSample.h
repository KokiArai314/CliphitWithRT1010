
#ifndef TEST_SAMPLE
#define TEST_SAMPLE
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include "../oscillator/oscillator.h"
#include "../data/pcmdata/declare_pcm_buffers.h"

_Alignas(16) int16_t test_sin_tone[100] = {
		0, 2057, 4106, 6140, 8148, 10125, 12062, 13951, 15785, 17557, 19260, 20886, 22430, 23886, 25247, 26509, 27666, 28714, 29648, 30466, 31163, 31738, 32187, 32509, 32702, 32767, 32702, 32509, 32187, 31738, 31163, 30466, 29648, 28714, 27666, 26509, 25247, 23886, 22430, 20886, 19260, 17557, 15785, 13951, 12062, 10125, 8148, 6140, 4106, 2057, 0, -2057, -4106, -6140, -8148, -10125, -12062, -13951, -15785, -17557, -19260, -20886, -22430, -23886, -25247, -26509, -27666, -28714, -29648, -30466, -31163, -31738, -32187, -32509, -32702, -32767, -32702, -32509, -32187, -31738, -31163, -30466, -29648, -28714, -27666, -26509, -25247, -23886, -22430, -20886, -19260, -17557, -15785, -13951, -12062, -10125, -8148, -6140, -4106, -2057

};

SampleData_t sinData;
SampleData_t sampleData;

static SampleData_t* get_test_sin_tone(){

  SampleData_t *pSampleData = &sinData;
  pSampleData->DataPtr = &test_sin_tone;
  pSampleData->SampleByte = 2;
  pSampleData->Length = 100;
  pSampleData->MonoStereo = 1;
  pSampleData->StartOfs = 0;
  pSampleData->TempoX100 = 120;
  pSampleData->FsAdjust = 1.0;
  pSampleData->EndOfs = 99;
  pSampleData->LoopOfs = 99;
  pSampleData->StartOfs = 0;

  return pSampleData;
}

static SampleData_t* get_test_sample(){

  int samples = sizeof(pcm_buff_00)/sizeof(pcm_buff_00[0]);

  SampleData_t *pSampleData = &sampleData;
  pSampleData->DataPtr = &pcm_buff_00;
  pSampleData->SampleByte = 2;
  pSampleData->Length = samples;
  pSampleData->MonoStereo = 1;
  pSampleData->StartOfs = 0;
  pSampleData->TempoX100 = 120;
  pSampleData->FsAdjust = 1.0;
  pSampleData->EndOfs = samples-1;
  pSampleData->LoopOfs = samples-1;
  pSampleData->StartOfs = 0;

  return pSampleData;
}

#endif
