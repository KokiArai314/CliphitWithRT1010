/*
 * midi_player.h
 *
 *  Created on: 2020/02/06
 *      Author: higuchi
 */

#ifndef MIDI_PLAYER_H_
#define MIDI_PLAYER_H_

#include <stdint.h>

#include "p_usb.h"

/*******************************************************************************
* Definitions
******************************************************************************/


/*******************************************************************************
* API
******************************************************************************/

/* Idle routine */
void USB_MIDI_IDLE(void);

/* MIDI送信関連 */
#define USB_RSLT_OK (0x00)
#define USB_RSLT_BUFFULL (0x01)
#define USB_RSLT_FIFOFULL (0x01)
#define USB_RSLT_NOTRDY (0xFF)

uint8_t WriteData4(uint32_t UsbMidiData);
uint8_t Send2USB(void);

#endif /* MIDI_PLAYER_H_ */
