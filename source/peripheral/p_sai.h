/*
 * p_sai.h
 *
 *  Created on: 2024/11/14
 *      Author: koki_arai
 */

#ifndef PERIPHERAL_P_SAI_H_
#define PERIPHERAL_P_SAI_H_
#include <stdint.h>
#include "p_usb.h"

#define SAI_INTERRUPT_PRIORITY (5U) //Audio

extern usb_device_composite_struct_t *g_deviceComposite;
extern uint8_t audioRecDataBuff[] __attribute__((aligned(4)));

void saiInit(void);
void SAI_UserIRQHandler(void);

#endif /* PERIPHERAL_P_SAI_H_ */
