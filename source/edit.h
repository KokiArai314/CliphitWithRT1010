/*
 * edit.h
 *
 *  Created on: 2020/02/06
 *      Author: higuchi
 */

#ifndef EDIT_H_
#define EDIT_H_

#include "base_types.h"


/**
 * 暫定(VX II)
 */
#include "midi/midi_device.h"	// #define TEST_VXII_SYSEX 1

/**
 * 暫定(VX II)
 */
#if TEST_VXII_SYSEX

/* Mode ID */
#define	MODEID_CH_SEL	0	// Channel Select Mode
#define	MODEID_MANUAL	1	// Manual Mode
#define	MODEID_PRESET	2	// Preset Mode

extern uint8_t GetPresetNo(void);

#endif	// #if TEST_VXII_SYSEX

//------------------------------------------------------------------------------
//	variable
//------------------------------------------------------------------------------

extern fbuf_t editflags;
#define	bEditFxBypass	editflags.f.b0

//------------------------------------------------------------------------------
//	function declaration
//------------------------------------------------------------------------------

void EditIdle(void);
void EditResume1stStatus(void);

//void ChangeChannel(uint8_t chno);

void ChangeChMode(void);
void ChangeChModeEx(void);
void ChangeReverbOnOff(void);
void ChangeSendReturnOnOff(void);
void ChangeMaster12(void);

void EditAmpSelector(void);

//void EditFxOnOff(void);
//void EditFxSelector(void);
//void EditFxVal1(void);
//void EditFxVal2(void);

void EditFx1OnOff(void);
void EditFx1Selector(void);
void EditFx1Val1(void);
void EditFx1Val2(void);

void EditFx2OnOff(void);
void EditFx2Selector(void);
void EditFx2Val1(void);
void EditFx2Val2(void);

void EditFx3OnOff(void);
void EditFx3Selector(void);
void EditFx3Val1(void);
void EditFx3Val2(void);

void EditFxBypass(bool fBypass);
void EditSetFxBypass(bool bBypass);
bool EditGetFxBypass(void);

#endif /* EDIT_H_ */
