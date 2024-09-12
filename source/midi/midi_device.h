/*
 * midi_device.h
 *
 *  Created on: 2020/03/25
 *      Author: higuchi
 */

#ifndef MIDI_DEVICE_H_
#define MIDI_DEVICE_H_

#include <stdint.h>

/*******************************************************************************
 * Definitions
 ******************************************************************************/
//#define TEST_VXII_SYSEX 1	// VX IIのMIDIインプリ仕様に倣う

#define AMP_MIDI_CH	(0)

#define NUM_OF_PRODUCT_ID	3
#if TEST_VXII_SYSEX	// VX IIのMIDIインプリ仕様に倣う
#define	AMP_PRODUCT_ID_1ST	0x00
#define AMP_PRODUCT_ID_2ND	0x01
#define AMP_PRODUCT_ID_3RD	0x32
#else	// default
#define	AMP_PRODUCT_ID_1ST	0x00
#define AMP_PRODUCT_ID_2ND	0x00
#define AMP_PRODUCT_ID_3RD	0x00
#endif

/*******************************************************************************
 * API
 ******************************************************************************/
void midi_AMP_RxInit(void);
void midi_AMP_RxProc_SysEx(uint8_t Data1, uint8_t Data2, uint8_t Data3);
void midi_AMP_RxProc_ReceiveByte(uint8_t Data);

#endif /* MIDI_DEVICE_H_ */
