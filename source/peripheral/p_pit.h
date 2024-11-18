/*
 * p_pit.h
 *
 *  Created on: 2024/11/14
 *      Author: koki_arai
 */

#ifndef PERIPHERAL_P_PIT_H_
#define PERIPHERAL_P_PIT_H_

#define PIT_INTERRUPT_PRIORITY (0U)

void pitInit(void);
void pitStart(void);
void PIT_IRQHandler(void);

#endif /* PERIPHERAL_P_PIT_H_ */
