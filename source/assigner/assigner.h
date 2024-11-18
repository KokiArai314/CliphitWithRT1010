/*
 * assigner.h
 *
 *  Created on: 2024/09/17
 *      Author: koki_arai
 */

#ifndef ASSIGNER_H_
#define ASSIGNER_H_

#include <stddef.h>
#include <stdint.h>

#include "oscillator/oscillatorManager.h"

extern Vcb_t vcb[];
int16_t entryVcb(uint16_t sampleNo, float fVelocity);

#endif /* ASSIGNER_H_ */
