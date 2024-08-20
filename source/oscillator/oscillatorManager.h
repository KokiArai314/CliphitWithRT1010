/*
	oscillatorManager.hpp to .h
	copy from x19850
*/
#ifndef OSCILLATORMANAGER_HPP_
#define OSCILLATORMANAGER_HPP_

#include "oscillatorProcess.h"

void onmemoryoscillatorstart(OscSetup_t *psOscSetup, int voiceNum);

void onmemoryoscillatortest(SampleData_t *psSampleData, int bus);

#endif // OSCILLATORMANAGER_HPP_
