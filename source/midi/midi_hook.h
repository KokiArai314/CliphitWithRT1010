/*
 * midi_hook.h
 *
 *  Created on: 2020/09/03
 *      Author: akino
 */

#ifndef APPLICATION_MIDI_MIDI_HOOK_H_
#define APPLICATION_MIDI_MIDI_HOOK_H_

void midi_hook_entry(uint8_t data, void (*func)(uint8_t), uint8_t attach);
void midi_hook_exec(void);

#endif /* APPLICATION_MIDI_MIDI_HOOK_H_ */
