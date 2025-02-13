
#ifndef TEST_SAMPLE
#define TEST_SAMPLE
#include <stdint.h>
#include <stddef.h>
#include "../Oscillator/oscillator.h"

alignas(16) uint16_t test_sin_tone[100] = {
  32768, 34825, 36874, 38908, 40916, 42893, 44830, 46719, 48553, 50325, 52028, 53654, 55198, 56654, 58015, 59277, 60434, 61482, 62416, 63234, 63931, 64506, 64955, 65277, 65470, 65535, 65470, 65277, 64955, 64506, 63931, 63234, 62416, 61482, 60434, 59277, 58015, 56654, 55198, 53654, 52028, 50325, 48553, 46719, 44830, 42893, 40916, 38908, 36874, 34825, 32768, 30710, 28661, 26627, 24619, 22642, 20705, 18816, 16982, 15210, 13507, 11881, 10337, 8881, 7520, 6258, 5101, 4053, 3119, 2301, 1604, 1029, 580, 258, 65, 0, 65, 258, 580, 1029, 1604, 2301, 3119, 4053, 5101, 6258, 7520, 8881, 10337, 11881, 13507, 15210, 16982, 18816, 20705, 22642, 24619, 26627, 28661, 30710
};

SampleData_t get_test_sin_tone(){
  SampleData_t *sampleData;
  sampleData->DataPtr = &test_sin_tone;
  sampleData->SampleByte = 2;
  sampleData->Length = 100;
  sampleData->MonoStereo = 1;
  sampleData->StartOfs = 0;
  sampleData->TempoX100 = 120;
  sampleData->FsAdjust = 1.0;
  sampleData->EndOfs = 99;
  sampleData->LoopOfs = 99;
  sampleData->StartOfs = 0;
  return *sampleData;
}
#endif
