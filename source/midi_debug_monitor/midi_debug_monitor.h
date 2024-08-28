/*
	Project		X-11100
	File name	MidiDebugMonitor.h
	Date		2011/3/2~	X-10210
				2011/11/10~	X-11100
				2012/4/4~	X-11100(on ARM)

	Copyright (c) 2011,12 by KORG All rights reserved.
*/

#ifndef __MIDIDEBUGMONITOR_H__
#define __MIDIDEBUGMONITOR_H__

#define MIDIDEBUGMONITOR	/* 定義するとデバッグモニタ有効 */

//namespace MidiDebugMonitor{

#if defined(MIDIDEBUGMONITOR)

// とりあえずコピペ from MidiCommon.h
typedef enum {
	SDIR_UNKNOWN	= -1,
	SDIR_ALLOUTPUT,
	SDIR_UARTMIDI,
	SDIR_USBMIDI,
	SDIR_UARTLOG
} ESendDir;
typedef enum {
	eSrcUnknown = -1,
	eSrcInt,
	eSrcUartMIDI,
	eSrcUsbMIDI,
} ESource;

void MidiDebugMonitorCommandExec(ESendDir eDir, char pos, char fx, long parval);
int MidiDebugMonitorCommandIn(unsigned int uiData, ESource eSource);

void dputbuf_flush(ESendDir eDir);
void dputc(ESendDir eDir, char dt);		// 要 dputbuf_flush
void dputs(ESendDir eDir, char *str);	// 要 dputbuf_flush

void dprintf(ESendDir eDir, char *format, ...);	// 込 dputbuf_flush

#else	//defined(MIDIDEBUGMONITOR)

#define MidiDebugMonitorCommandExec(eDir, pos, fx, parval)
#define MidiDebugMonitorCommandIn(uiData, eSource) (0)

#define dputbuf_flush(dir)
#define dputc(dir, dt)
#define dputs(dir, str)

#define dprintf(dir, format, ...)

#endif	//defined(MIDIDEBUGMONITOR)

//}

#endif	//__MIDIDEBUGMONITOR_H__
