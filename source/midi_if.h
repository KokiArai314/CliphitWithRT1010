/*
 * midi_if.h
 *
 *  Created on: 2020/05/07
 *      Author: higuchi
 */

#ifndef MIDI_IF_H_
#define MIDI_IF_H_

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * API
 ******************************************************************************/
void midi_IF_TxInit(void);
void midi_IF_TxProc_SysEx(uint8_t Data);
void midi_IF_TxProc_ReceiveByte(uint8_t Data);

void midi_IF_RxInit(void);

void MIDI_IF_IDLE(void);

#endif /* MIDI_IF_H_ */
