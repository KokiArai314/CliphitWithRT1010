/*
 * p_adc.h
 *
 *  Created on: 2024/11/01
 *      Author: koki_arai
 */

#ifndef PERIPHERAL_P_ADC_H_
#define PERIPHERAL_P_ADC_H_

#include "fsl_adc.h"
#include "fsl_gpio.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

void adcInit(void);
void adcStartFillingOnelap(void);
int16_t adcGetValue(int index);
uint32_t adcGetFlag(int index1st, int index2nd);
void adcClearFlag(int index1st, int index2nd);

#endif /* PERIPHERAL_P_ADC_H_ */
