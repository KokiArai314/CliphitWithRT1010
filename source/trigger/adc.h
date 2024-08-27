/*
 * adc.h
 *
 *  Created on: 2020/09/09
 *      Author: akino
 */

#ifndef APPLICATION_TRIGGER_ADC_H_
#define APPLICATION_TRIGGER_ADC_H_

void adc_init(void);
void adc_start(int select);
int16_t adc_getValue(int index);
uint32_t adc_getFlag(int index1st, int index2nd);
void adc_clrFlag(int index1st, int index2nd);

#endif /* APPLICATION_TRIGGER_ADC_H_ */
