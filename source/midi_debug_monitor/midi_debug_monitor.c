/*
	Project		X-11100
	File name	MidiDebugMonitor.cpp
	Date		2011/3/2~	X-10210
				2011/11/10~	X-11100
				2012/4/4~	X-11100(on ARM)
				2020/9/7~	X-19850

	Copyright (c) 2011,12 by KORG All rights reserved.
*/

#include	<stdio.h>
#include	<string.h>
#include	<stdarg.h>
#include	<stdlib.h>
#include	<stdint.h>

#include "midi_debug_monitor.h"
#include "../midi_if.h"

#if defined(MIDIDEBUGMONITOR)

//namespace MidiDebugMonitor {

#define MEMORYDUMP	/*	MemoryDump	*/
//#define SYSTEMVIEW	/*	SystemView	*/
#define JOBTIME	/*	JobTime		*/
#define ADCVIEW	/*	AdcView		*/
#define ADCAUDIO	/*	AdcAudio	*/
#define WM8960DUMP	/*	WM8960Dump	*/
#define TRIGDEBUG	/*	TrigDebug	*/
//#define UARTEXINFO	/*	UartExInfo	*/
//#define TRIGEDIT	/*	TrigEdit	*/
#define MIDIROOT	/*	MidiRoot	*/
//#define TRIGAUDIO	/*	TrigAudio	*/
#define VELCRVDUMP	/*	VelCrvDump	*/
//#define DTSIZAUDIO	/*	DtSizAudio	*/
#define TIMINGMEASURE	/*	TimingMeasure	*/

#ifndef BOARD_USE_CODEC
#ifdef WM8960DUMP
#undef WM8960DUMP
#endif	//WM8960DUMP
#endif	//BOARD_USE_CODEC
#ifndef BOARD_PROTO1
#ifdef MIDIROOT
#undef MIDIROOT
#endif	//MIDIROOT
#endif	//BOARD_PROTO1

/*	MIDIデバッグモニタ	*/
#define OUTBUFSIZ (80)	//(80*25)

struct OutBuf {
	unsigned char wcnt;
	unsigned char channel;
	unsigned char buf[7+OUTBUFSIZ+1+1];
};

static struct OutBuf uartOutBuf = {0,0,{0}};
static struct OutBuf usbOutBuf = {0,0,{0}};

#define channel(aaa) ((aaa) == SDIR_UARTMIDI ? uartOutBuf.channel : usbOutBuf.channel)
#define SetChannel(aaa,bbb) ((aaa) == SDIR_UARTMIDI ? (uartOutBuf.channel = (bbb)) : (usbOutBuf.channel = (bbb)))

extern void midi_IF_send_usb_blocking(uint8_t *str, uint16_t cnt);
//extern void midi_IF_send_uart_blocking(uint8_t *str, uint16_t cnt);

/********************/
/*	送信吐き出し	*/
/********************/
void dputbuf_flush_local_sub(ESendDir eDir, int c, struct OutBuf *outbuf)
{
	if( c == outbuf->channel ){
		if( outbuf->wcnt != 0 ){
			memcpy( outbuf->buf, "\xf0\x42\x21\x78\x7f\x72\x7f", 7 );
			outbuf->buf[7+outbuf->wcnt] = 0;
			outbuf->buf[7+outbuf->wcnt+1] = 0xf7;
			if (eDir == SDIR_USBMIDI)
			{
				midi_IF_send_usb_blocking(outbuf->buf, 7+outbuf->wcnt+1+1);
			}
			else if (eDir == SDIR_UARTMIDI)
			{
				//midi_IF_send_uart_blocking(outbuf->buf, 7+outbuf->wcnt+1+1);
			}
			outbuf->wcnt = 0;
		}
	}
}

void dputbuf_flush_local(ESendDir eDir, int c)
{
	if (eDir == SDIR_UARTMIDI) {
		dputbuf_flush_local_sub(eDir, c, &uartOutBuf);
	}
	if (eDir == SDIR_USBMIDI) {
		dputbuf_flush_local_sub(eDir, c, &usbOutBuf);
	}
	if (eDir == SDIR_ALLOUTPUT) {
		dputbuf_flush_local_sub(SDIR_UARTMIDI, c, &uartOutBuf);
		dputbuf_flush_local_sub(SDIR_USBMIDI, c, &usbOutBuf);
	}

	return;
}

void dputbuf_flush(ESendDir eDir)
{
	dputbuf_flush_local(eDir, 0);

	return;
}

/************************/
/*	デバッグ1文字出力	*/
/************************/
void dputc_local_sub(ESendDir eDir, int c, char dt, struct OutBuf *outbuf)
{
	if( c == outbuf->channel ){
		if( outbuf->wcnt >= OUTBUFSIZ ){
			dputbuf_flush_local(eDir, c);
		}
		if( outbuf->wcnt < OUTBUFSIZ ){
			outbuf->buf[7+outbuf->wcnt] = dt < 0 ? '?' : dt;	// for MIDI data byte
			outbuf->wcnt++;
		}
	}

	return;
}

void dputc_local(ESendDir eDir, int c, char dt)
{
	if (eDir == SDIR_UARTMIDI) {
		dputc_local_sub(eDir, c, dt, &uartOutBuf);
	}
	if (eDir == SDIR_USBMIDI) {
		dputc_local_sub(eDir, c, dt, &usbOutBuf);
	}
	if (eDir == SDIR_ALLOUTPUT){
		dputc_local_sub(SDIR_UARTMIDI, c, dt, &uartOutBuf);
		dputc_local_sub(SDIR_USBMIDI, c, dt, &usbOutBuf);
	}

	return;
}

void dputc(ESendDir eDir, char dt){
	dputc_local(eDir, 0, dt);

	return;
}

/************************/
/*	デバッグ文字列出力	*/
/************************/
void dputs_local(ESendDir eDir, int c, char *str)
{
	while( *str != 0 ){
		dputc_local(eDir, c, *str++);
	}

	return;
}

void dputs(ESendDir eDir, char *str){
	dputs_local(eDir, 0, str);
}

void dprintf_local(ESendDir eDir, int c, char *format, ...){
	char b_u_f[81];
	va_list p_vargs;        /* return value from vsnprintf  */

	va_start(p_vargs, format);
	vsnprintf(b_u_f, 81, format, p_vargs);
	va_end(p_vargs);
	dputs_local(eDir, c, b_u_f);
	dputbuf_flush_local(eDir, c);

	return;
}

void dprintf(ESendDir eDir, char *format, ...){
	char b_u_f[81];
	va_list p_vargs;        /* return value from vsnprintf  */

	va_start(p_vargs, format);
	vsnprintf(b_u_f, 81, format, p_vargs);
	va_end(p_vargs);
	dputs_local(eDir, 0, b_u_f);
	dputbuf_flush_local(eDir, 0);

	return;
}

#define dputbuf_flush(ccc) dputbuf_flush_local((ccc), channel(ccc))
#define dputc(ccc,aaa) dputc_local((ccc), channel(ccc), (aaa))
#define dputs(ccc,aaa) dputs_local((ccc), channel(ccc), (aaa))
#define dprintf(ccc,...) dprintf_local((ccc), channel(ccc), __VA_ARGS__)

/****************************************/
/*	下位四ビットをHexキャラクターで出力	*/
/****************************************/
void dputhexbl(ESendDir eDir, char d)
{
	d &= 0xf;
	d += d < 10 ? '0' : 'a'-10;
	dputc(eDir, d);

	return;
}

/************************************/
/*	１バイトをHexキャラクターで出力	*/
/************************************/
void dputhexb(ESendDir eDir, char d)
{
	dputhexbl(eDir, d >> 4);
	dputhexbl(eDir, d & 0xf);

	return;
}

/************************************/
/*	１ワードをHexキャラクターで出力	*/
/************************************/
void dputhexw(ESendDir eDir, short d)
{
	dputhexb(eDir, d >> 8);
	dputhexb(eDir, d & 0xff);

	return;
}

/************************************/
/*	１ロングをHexキャラクターで出力	*/
/************************************/
void dputhexl(ESendDir eDir, long d)
{
	dputhexw(eDir, d >> 16);
	dputhexw(eDir, d & 0xffff);

	return;
}

/*	コマンド戻り値定義	*/
#ifndef DEBUG_Result
#define	DEBUG_Result
typedef enum{
	CMD_OK = 0,
	CMD_ERR,
}Result;
#endif	/* DEBUG_Result */

/*	コマンド構造体定義	*/
typedef struct{
	char *cmd_str;										/*	コマンド(スペース)ヘルプ表示	*/
	int (*cmd_fnc)(ESendDir eDir, char *str, char ofs);	/*	str=command, ofs=space offset	*/
}Cmd;

/****************/
/*	各種処理	*/
/****************/

#if defined(MEMORYDUMP)
/* ===	メモリーダンプ	=== */

static Result MemoryDumpLong(ESendDir eDir, unsigned long start, unsigned long end)
{
	unsigned long data;
/*
 Address   +3+2+1+0 +7+6+5+4  +B+A+9+8 +F+E+D+C
00000000   00000000 00000000  00000000 00000000
*/
	dputs(eDir, "\n Address   +3+2+1+0 +7+6+5+4  +B+A+9+8 +F+E+D+C");
	data = start & ~0xf;
	if( data < start ){
		dputc(eDir, '\n');
		dputhexl(eDir, data);
		dputc(eDir, ' ');
		while( data < start ){
			if( (data & 0x7) == 0 ){
				dputc(eDir, ' ');
			}
			dputs(eDir, "         ");
			data += 4;
		}
	}
	while( start <= end ){
		if( (start & 0xf) == 0 ){
			dputc(eDir, '\n');
			dputhexl(eDir, start);
			dputc(eDir, ' ');
		}
		if( (start & 0x7) == 0 ){
			dputc(eDir, ' ');
		}
		data = *(long *)start;
		dputc(eDir, ' ');
		dputhexl(eDir, data);
		start += 4;
	}

	return CMD_OK;
}

static Result MemoryDumpWord(ESendDir eDir, unsigned long start, unsigned long end)
{
	unsigned long data;
/*
 Address   +1+0 +3+2 +5+4 +7+6  +9+8 +B+A +D+C +F+E
00000000   0000 0000 0000 0000  0000 0000 0000 0000
*/
	dputs(eDir, "\n Address   +1+0 +3+2 +5+4 +7+6  +9+8 +B+A +D+C +F+E");
	data = start & ~0xf;
	if( data < start ){
		dputc(eDir, '\n');
		dputhexl(eDir, data);
		dputc(eDir, ' ');
		while( data < start ){
			if( (data & 0x7) == 0 ){
				dputc(eDir, ' ');
			}
			dputs(eDir, "     ");
			data += 2;
		}
		if( (start & 2) != 0 ){
			data = *(long *)(start-2);
			dputc(eDir, ' ');
			dputhexw(eDir, data>>16);
			start += 2;
		}
	}
	while( start <= end ){
		if( (start & 0xf) == 0 ){
			dputc(eDir, '\n');
			dputhexl(eDir, start);
			dputc(eDir, ' ');
		}
		if( (start & 0x7) == 0 ){
			dputc(eDir, ' ');
		}
		data = *(long *)start;
		dputc(eDir, ' ');
		dputhexw(eDir, data);
		start += 2;
		if( start <= end ){
			dputc(eDir, ' ');
			dputhexw(eDir, data>>16);
			start += 2;
		}
	}

	return CMD_OK;
}

static Result MemoryDumpByte(ESendDir eDir, unsigned long start, unsigned long end)
{
	unsigned long data;
/*
 Address   +0 +1 +2 +3 +4 +5 +6 +7  +8 +9 +A +B +C +D +E +F
00000000   00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00
*/
	dputs(eDir, "\n Address   +0 +1 +2 +3 +4 +5 +6 +7  +8 +9 +A +B +C +D +E +F");
	data = start & ~0xf;
	if( data < start ){
		dputc(eDir, '\n');
		dputhexl(eDir, data);
		dputc(eDir, ' ');
		while( data < start ){
			if( (data & 0x7) == 0 ){
				dputc(eDir, ' ');
			}
			dputs(eDir, "   ");
			data++;
		}
		data = *(long *)(start & ~3);
		switch( start & 3 ){
		case 1:
			dputc(eDir, ' ');
			dputhexb(eDir, data>>8);
			start++;
		case 2:
			dputc(eDir, ' ');
			dputhexb(eDir, data>>16);
			start++;
		case 3:
			dputc(eDir, ' ');
			dputhexb(eDir, data>>24);
			start++;
		}
	}
	while( start <= end ){
		if( (start & 0xf) == 0 ){
			dputc(eDir, '\n');
			dputhexl(eDir, start);
			dputc(eDir, ' ');
		}
		if( (start & 0x7) == 0 ){
			dputc(eDir, ' ');
		}
		data = *(long *)start;
		dputc(eDir, ' ');
		dputhexb(eDir, data);
		start++;
		if( start <= end ){
			dputc(eDir, ' ');
			dputhexb(eDir, data>>8);
			start++;
		}
		if( start <= end ){
			dputc(eDir, ' ');
			dputhexb(eDir, data>>16);
			start++;
		}
		if( start <= end ){
			dputc(eDir, ' ');
			dputhexb(eDir, data>>24);
			start++;
		}
	}

	return CMD_OK;
}

static int MemoryDump(ESendDir eDir, char *cmdstr, char ofs)
{
	unsigned long start, end;
	int size = 4;
	char *wp;
	Result ret = CMD_OK;

	if( cmdstr[(int)ofs++] != ' ' ){
		return CMD_ERR;
	}
	start = strtoul( &cmdstr[(int)ofs], &wp, 0 );
	if( (*wp != ' ') && (*wp != ',') ){
		return CMD_ERR;
	}
	end = strtoul( wp+1, &wp, 0 );
	if( (*wp == ' ') || (*wp == ',') ){
		switch( wp[1] ){
		case 'L':
		case 'l':
			size = 4;
			break;
		case 'W':
		case 'w':
			size = 2;
			break;
		case 'B':
		case 'b':
			size = 1;
			break;
		default:
			break;
		}
	}
	start &= ~(size-1);
	end |= (size-1);
	if( start > end ){
		return CMD_ERR;
	}
	dputs(eDir, "\n Start=" );
	dputhexl(eDir, start );
	dputs(eDir, ",End=");
	dputhexl(eDir, end);
	dputs(eDir, ",Size=");
	dputhexb(eDir, size);

	switch( size ){
	case 4:
		ret = MemoryDumpLong(eDir, start, end);
		break;
	case 2:
		ret = MemoryDumpWord(eDir, start, end);
		break;
	case 1:
		ret = MemoryDumpByte(eDir, start, end);
		break;
	}
	return ret;
}

#define	MEMORYDUMPCMD	{"MemoryDump Start,End[,L(=long:default)|W(=woed)|B(=byte)]", MemoryDump},
#else	//defined(MEMORYDUMP)
#define	MEMORYDUMPCMD
#endif	//defined(MEMORYDUMP)

#ifdef SYSTEMVIEW

#include "fsl_device_registers.h"
#include "../../hardware/systick.h"
#include "board/board.h"
#include "composite.h"

extern uint32_t getrxoverrun(void);
extern uint32_t getrxbuffovr(void);
#ifdef BOARD_PROTO1
extern uint32_t getrxoverrun2(void);
extern uint32_t getrxbuffovr2(void);
#endif	//BOARD_PROTO1
extern bool g_bAdcCalib;

static int SystemView(ESendDir eDir, char *cmdstr, char ofs)
{
	Result ret = CMD_OK;

	SystemCoreClockUpdate();
	dprintf(eDir, "\n System Clock = %d", SystemCoreClock);
	dprintf(eDir, "\n SysTick = %08x", systick_getCount());
	dprintf(eDir, "\n RxOverRun = %d", getrxoverrun());
	dprintf(eDir, "\n RxBuffOvr = %d", getrxbuffovr());
#ifdef BOARD_PROTO1
	dprintf(eDir, "\n RxOverRun2 = %d", getrxoverrun2());
	dprintf(eDir, "\n RxBuffOvr2 = %d", getrxbuffovr2());
#endif	//BOARD_PROTO1
	dprintf(eDir, "\n adc calib = %s", g_bAdcCalib ? "Done" : "Failed");
	dprintf(eDir, "\n Priority UART = %d", NVIC_GetPriority(BOARD_UART_IRQ));
	dprintf(eDir, "\n Priority SAI  = %d", NVIC_GetPriority(SAI1_IRQn));
	dprintf(eDir, "\n Priority USB  = %d", NVIC_GetPriority(USB_OTG1_IRQn));
	dprintf(eDir, "\n Priority PIT  = %d", NVIC_GetPriority(PIT_IRQn));
	{
		extern usb_device_composite_struct_t *g_deviceComposite;
		usb_audio_player_struct_t audioPlayer = g_deviceComposite->audioPlayer;
		int act = audioPlayer.tdReadNumberRec <= audioPlayer.tdWriteNumberRec ?
				audioPlayer.tdWriteNumberRec - audioPlayer.tdReadNumberRec :
				(4*2*45*4*3) - audioPlayer.tdReadNumberRec + audioPlayer.tdWriteNumberRec;

		dprintf(eDir, "\n rec:wrp=%4d, rdp=%4d, act=%4d", audioPlayer.tdWriteNumberRec, audioPlayer.tdReadNumberRec, act);

		int rdp = audioPlayer.tdWriteNumberPlay;
		int wrp = audioPlayer.tdReadNumberPlay;
		act = rdp <= wrp ? wrp - rdp : (((4 * 2) * ((((44)+1) * (2) * 4)))) - rdp + wrp;

		dprintf(eDir, "\n ply:wrp=%4d, rdp=%4d, act=%4d", audioPlayer.tdWriteNumberPlay, audioPlayer.tdReadNumberPlay, act);
	}
#if 1
	{
		extern int16_t sendCueOverFlow;
		extern int16_t sendCueSubError;
		extern int16_t sendCueClear;
		dprintf(eDir, "\n ovr=%5d,sub=%5d,clr=%5d", sendCueOverFlow, sendCueSubError, sendCueClear);
	}
#endif

	return ret;
}

#define SYSTEMVIEWCMD		{"SystemView", SystemView},
#else	//SYSTEMVIEW
#define SYSTEMVIEWCMD
#endif	//SYSTEMVIEW

#ifdef JOBTIME

#define JOBTIMEMAXNUM	(10)

typedef struct {
	uint32_t sta;
	uint32_t stp;
	uint32_t tim;
} JobTime_t;

static volatile JobTime_t jobTime[JOBTIMEMAXNUM] = {0};

static uint32_t calcJobTime(uint32_t sta, uint32_t stp)
{
	uint32_t ret = 0;

	if (sta < stp)
	{
		ret = stp - sta;
	}
	else if (sta > stp)
	{
		ret = 0xfffffffful - sta + 1 + stp;
	}

	return ret;
}

void jobTimeStart(int index)
{
	if (index < JOBTIMEMAXNUM)
	{
		uint32_t now = systick_getCount();

		jobTime[index].sta = now;
		jobTime[index].stp = now + 1;
	}

	return;
}

void jobTimeStop(int index)
{
	if (index < JOBTIMEMAXNUM)
	{
		uint32_t now = systick_getCount();

		jobTime[index].stp = now;
		jobTime[index].tim = calcJobTime(jobTime[index].sta, now);
	}

	return;
}

void jobTimeInterval(int index)
{
	if (index < JOBTIMEMAXNUM)
	{
		if (jobTime[index].sta == jobTime[index].stp)
		{
			jobTimeStart(index);
		}
		else
		{
			jobTimeStop(index);

			jobTime[index].sta = jobTime[index].stp;
		}
	}

	return;
}

static int JobTime(ESendDir eDir, char *cmdstr, char ofs)
{
	Result ret = CMD_OK;
	int flag = 0;	// 0:normal,1:reset

	if (cmdstr[(int)ofs++] == ' ')
	{
		if (cmdstr[(int)ofs] == '0')
		{
			flag = 1;
		}
	}
	if (flag)
	{
		for (int i = 0; i < sizeof(jobTime)/sizeof(jobTime[0]); i++)
		{
			jobTime[i].sta = jobTime[i].stp = jobTime[i].tim = 0;
		}
		dprintf(eDir, "\n counter reset !");
	}
	else
	{
		for (int i = 0; i < sizeof(jobTime)/sizeof(jobTime[0]); i++)
		{
			uint32_t tim = jobTime[i].tim;

			if (tim)
			{
				dprintf(eDir, "\n %1d:%10d(%dnsec)", i, tim, tim*2);
			}
		}
	}

	return ret;
}

#define JOBTIMECMD		{"JobTime (0:reset)", JobTime},
#else	//JOBTIME
#define JOBTIMECMD
#endif	//JOBTIME

#ifdef ADCVIEW

#include "../trigger/adc.h"

static int AdcView(ESendDir eDir, char *cmdstr, char ofs)
{
	Result ret = CMD_OK;
	int index = 0;
	int32_t val;

	dprintf(eDir, "\n===Adc===");
	while ((val = adc_getValue(index++)) >= 0)
	{
		dprintf(eDir, "\n %2d:%4d", index-1, val);
	}

	return ret;
}

#define ADCVIEWCMD		{"AdcView", AdcView},
#else	//ADCVIEW
#define ADCVIEWCMD
#endif	//ADCVIEW

#ifdef ADCAUDIO

void HC_AudioSetCallback(void (*callback)(int32_t *data_l, int32_t *data_r));

#define CCRBUFSIZ (4)

static int LchAdcNo = -1;
static int RchAdcNo = -1;
static int LchAdcNo2 = -1;
static int RchAdcNo2 = -1;

#if 1
typedef struct {
	int wrp;
	int rdp;
	int siz;
	int32_t *pbf;
}CCR_t;

static int32_t ccrbufch1[CCRBUFSIZ];
static int32_t ccrbufch2[CCRBUFSIZ];
static CCR_t ccrch1 = {0,0,CCRBUFSIZ,ccrbufch1};
static CCR_t ccrch2 = {0,0,CCRBUFSIZ,ccrbufch2};

static int putccrbuf(CCR_t *psc, int32_t *pdt)
{
	int ret = 0;
	int wrp = psc->wrp;
	int nxtwrp = (wrp + 1) >= psc->siz ? 0 : wrp + 1;
	int rdp = psc->rdp;

	if (nxtwrp != rdp)
	{
		(psc->pbf)[wrp] = *pdt;
		psc->wrp = nxtwrp;
		ret = 1;
	}

	return ret;
}

static int getccrbuf(CCR_t *psc, int32_t *pdt)
{
	int ret = 0;
	int wrp = psc->wrp;
	int rdp = psc->rdp;
	int nxtrdp = (rdp + 1) >= psc->siz ? 0 : rdp + 1;

	if (rdp != wrp)
	{
		*pdt = (psc->pbf)[rdp];
		psc->rdp = nxtrdp;
		ret = 1;
	}

	return ret;
}
#else
static int32_t LchData = 0;
static int32_t RchData = 0;
#endif

#ifdef TRIGAUDIO
int TrigAudioSet(uint32_t val, uint8_t ch);
#endif	//TRIGAUDIO

void AdcAudioSet(uint16_t value, uint8_t ch)
{
#if 1
	int32_t val = value;	// 0~4095(0x0~0xfff)

#ifdef TRIGAUDIO
	if (TrigAudioSet(val, ch))
	{
		return;
	}
#endif	//TRIGAUDIO
	val = (val << 19) | 0x100;	// 0x00000fff -> 0x7ff80100
	if (ch == LchAdcNo)
	{
		putccrbuf(&ccrch1, &val);
	}
	else if (ch == RchAdcNo)
	{
		putccrbuf(&ccrch2, &val);
	}
	else if (ch == LchAdcNo2)
	{
		val *= -1;
		putccrbuf(&ccrch1, &val);
	}
	else if (ch == RchAdcNo2)
	{
		val *= -1;
		putccrbuf(&ccrch2, &val);
	}
#else
#if 1
	value = value == 0 ? 1 : value;	// 0,1~4095 -> 0,1~4095
	int32_t val = ((int)value - 2048) * 16 * 65536;
	int32_t val2 = value * 256;

	if (ch == LchAdcNo)
	{
		LchData = val | (LchData & 0x000fff00);
	}
	else if (ch == RchAdcNo)
	{
		RchData = val | (RchData & 0x000fff00);
	}
	else if (ch == LchAdcNo2)
	{
		LchData = val2 | (LchData & 0xfff00000);
	}
	else if (ch == RchAdcNo2)
	{
		RchData = val2 | (RchData & 0xfff00000);
	}
#else
	int32_t val = ((int)value - 2048) * 16 * 65536 + 256;	// 0~4095 -> -2147483392(0x80000100)~2146435328(0x7FF00100)

	if (ch == LchAdcNo)
	{
		LchData = val;
	}
	else if (ch == RchAdcNo)
	{
		RchData = val;
	}
#endif
#endif

	return;
}

void AdcAudioGetLR(int32_t *pL, int32_t *pR)
{
	if ((pL) && (LchAdcNo >= 0))
	{
//		*pL = LchData;
//		LchData = 0;
		*pL = 0;
		getccrbuf(&ccrch1, pL);
	}
	if ((pR) && (RchAdcNo >= 0))
	{
//		*pR = RchData;
//		RchData = 0;
		*pR = 0;
		getccrbuf(&ccrch2, pR);
	}

	return;
}

static int AdcAudio(ESendDir eDir, char *cmdstr, char ofs)
{
	Result ret = CMD_OK;

	if (cmdstr[(int)ofs++] != 0)
	{
		int ch = -1;

		switch (cmdstr[(int)ofs++])
		{
		case 'L':
			ch = 0;
			break;
		case 'l':
			ch = 2;
			break;
		case 'R':
			ch = 1;
			break;
		case 'r':
			ch = 3;
			break;
		default:
			ret = CMD_ERR;
			break;
		}
		if (ch >= 0)
		{
			if ((cmdstr[(int)ofs] == ' ') || (cmdstr[(int)ofs] == ','))
			{
				int num = strtol(&cmdstr[(int)++ofs], NULL, 0);

				if (ch == 0)
				{
					LchAdcNo = num;
				}
				else if (ch == 1)
				{
					RchAdcNo = num;
				}
				else if (ch == 2)
				{
					LchAdcNo2 = num;
				}
				else if (ch == 3)
				{
					RchAdcNo2 = num;
				}
			}
			else
			{
				ret = CMD_ERR;
			}
			if ((LchAdcNo >= 0) || (RchAdcNo >= 0) || (LchAdcNo2 >= 0) || (RchAdcNo2 >= 0))
			{
				HC_AudioSetCallback(AdcAudioGetLR);
			}
			else if ((LchAdcNo < 0) && (RchAdcNo < 0) && (LchAdcNo2 < 0) && (RchAdcNo2 < 0))
			{
				HC_AudioSetCallback(NULL);
			}
		}
	}
	if (ret == CMD_OK)
	{
		dprintf(eDir, "\n Lch=%d:%d, Rch=%d:%d", LchAdcNo, LchAdcNo2, RchAdcNo, RchAdcNo2);
	}

	return ret;
}

#define ADCAUDIOCMD	{"AdcAudio", AdcAudio},
#else	//ADCAUDIO
#define ADCAUDIOCMD
#endif	//ADCAUDIO

#ifdef WM8960DUMP

#include "fsl_wm8960.h"

static int WM8960Dump(ESendDir eDir, char *cmdstr, char ofs)
{
	Result ret = CMD_OK;

	dprintf(eDir, "\n *** WM8960 Reggister Dump ***");
	for (int i = 0; i < WM8960_CACHEREGNUM; i++)
	{
		uint16_t reg;

		WM8960_ReadReg(i, &reg);
		dprintf(eDir, "\n R%2d(%02xh)=%03x", i, i, (int)reg);
	}

	return ret;
}

#define WM8960DUMPCMD	{"WM8960Dump", WM8960Dump},
#else	//WM8960DUMP
#define WM8960DUMPCMD
#endif	//WM8960DUMP

#ifdef TRIGDEBUG

int debugSwitch(int sw);

static int TrigDebug(ESendDir eDir, char *cmdstr, char ofs)
{
	Result ret = CMD_OK;
	int sw = 0;

	/*	デリミタ削除	*/
	while ((cmdstr[(int)ofs] != 0) && (cmdstr[(int)ofs] == ' '))
	{
		ofs++;
	}
	sw = strtol(&cmdstr[(int)ofs], NULL, 0);
	debugSwitch(sw);
	dprintf(eDir, "\n Trigger Debug %s !", sw ? "on" : "off");

	return ret;
}

#define TRIGDEBUGCMD	{"TrigDebug (0:off|1:on)", TrigDebug},
#else	//TRIGDEBUG
#define TRIGDEBUGCMD
#endif	//TRIGDEBUG

#ifdef UARTEXINFO

#include "../../../../shared/UartEx/uartex.h"
extern UartexHandle_t *uartexif_getHandlePtr(int sel);

static int UartExInfo(ESendDir eDir, char *cmdstr, char ofs)
{
	Result ret = CMD_OK;
#ifdef BOARD_PROTO1
	int num = 2;
#else	//BOARD_PROTO1
	int num = 1;
#endif	//BOARD_PROTO1

	for (int s = 0; s < num; s++)
	{
		UartexHandle_t *pHandle = uartexif_getHandlePtr(s);

		dprintf(eDir, "\n === UartExInfo (%d) ===", s);
		dprintf(eDir, "\n phase = %d", pHandle->phase);
		dprintf(eDir, "\n txlaneflag    = %08x", pHandle->txlaneflagall);
		dprintf(eDir, "\n rxlaneflag    = %08x", pHandle->rxlaneflagall);
		dprintf(eDir, "\n lastlane      = %d", pHandle->lastlane);
		dprintf(eDir, "\n pGetFunc      = %08x", pHandle->pGetFunc);
		dprintf(eDir, "\n pWupFunc      = %08x", pHandle->pWupFunc);
		dprintf(eDir, "\n pSendFunc     = %08x", pHandle->pSendFunc);
		dprintf(eDir, "\n databuff[]    = ");
		for (int i = 0; i < UARTEX_DATABUFFERSIZE; i++)
		{
			dprintf(eDir, "%02x", pHandle->databuffer[i]);
		}
		dprintf(eDir, "\n datalength    = %d", pHandle->datalength);
		dprintf(eDir, "\n datacounter   = %d", pHandle->datacounter);
		dprintf(eDir, "\n writelength   = %d", pHandle->writelength);
		dprintf(eDir, "\n writecounter  = %d", pHandle->writecounter);
		dprintf(eDir, "\n writebuffer   = %08x", pHandle->writebuffer);
		for (int i = 0; i < 3; i++)
		{
			dprintf(eDir, "\n --- lane %d ---", i);
			dprintf(eDir, "\n txenableflag  = %d", pHandle->sLane[i].txenableflag);
			dprintf(eDir, "\n pGetFunc      = %08x", pHandle->sLane[i].pGetFunc);
			dprintf(eDir, "\n pGetCountFunc = %08x (%d)", pHandle->sLane[i].pGetCountFunc,
					pHandle->sLane[i].pGetCountFunc ? pHandle->sLane[i].pGetCountFunc() : -1);
			dprintf(eDir, "\n pPutFunc      = %08x", pHandle->sLane[i].pPutFunc);
			dprintf(eDir, "\n pPutSpaceFunc = %08x (%d)", pHandle->sLane[i].pPutSpaceFunc,
					pHandle->sLane[i].pPutSpaceFunc ? pHandle->sLane[i].pPutSpaceFunc() : -1);
		}
	}

	return ret;
}

#define UARTEXINFOCMD	{"UartExInfo", UartExInfo},
#else	//UARTEXINFO
#define UARTEXINFOCMD
#endif	//UARTEXINFO

#ifdef TRIGEDIT

#include "../trigger/trigger.h"
#include "../trigger/pad.h"
#include "../trigger/ccPad.h"
#include "../trigger/extPad.h"
#include "../trigger/swpedal.h"
#include "../trigger/vrpedal.h"

#define ARRAYSIZE(aaa) (sizeof(aaa)/sizeof(aaa[0]))

static int TrigEditCmdGetType(void (* func)(TRIGSCN_t *))
{
	void (* funcTbl[])(TRIGSCN_t *) = {pad, ccPad, extPad, swPedal, vrPedal};
	int ret = -1;

	for (int i = 0; i < ARRAYSIZE(funcTbl); i++)
	{
		if (func == funcTbl[i])
		{
			ret = i;
		}
	}

	return ret;
}

static void TrigEditCmdDump(ESendDir eDir, int num)
{
	TRIGSCN_t *pTrigScn = &triggerGetParamPtr()[num];
	int type = TrigEditCmdGetType(pTrigScn->func);

	switch (type)
	{
	case 0:	// pad
		dprintf(eDir, "\n %2d a: onw=%3d(%5dusec),b:velw=%3d(%5dusec),c:mtim=%3d(%5dusec)",
				pTrigScn->pad.id,
				pTrigScn->pad.onCnt, (int)((float)pTrigScn->pad.onCnt * 1000.0f/16.0f),
				pTrigScn->pad.velWnd, (int)((float)pTrigScn->pad.velWnd * 1000.0f/16.0f),
				pTrigScn->pad.mskTim, (int)((float)pTrigScn->pad.mskTim * 3 * 1000.0f/16.0f));
		dprintf(eDir, "\n    d:offl=%4d,e:vmin=%4d,f:vmax=%4d,g:send=%s",
				pTrigScn->pad.padOff,
				pTrigScn->pad.vel.smin, pTrigScn->pad.vel.smax,
				pTrigScn->enable ? "Enable" : "Disable");
		break;
	case 1:	// ccpad
		dprintf(eDir, "\n %2d a: onw=%3d(%5dusec),b:velw=%3d(%5dusec),c:mtim=%3d(%5dusec)",
				pTrigScn->ccPad.id,
				pTrigScn->ccPad.onCnt, (int)((float)pTrigScn->ccPad.onCnt * 1000.0f/16.0f),
				pTrigScn->ccPad.velWnd, (int)((float)pTrigScn->ccPad.velWnd * 1000.0f/16.0f),
				pTrigScn->ccPad.mskTim, (int)((float)pTrigScn->ccPad.mskTim * 3 * 1000.0f/16.0f));
		dprintf(eDir, "\n    d:offl=%4d,e:cmin=%4d,f:cmax=%4d,g:vmin=%4d,h:vmax=%4d,i:send=%s",
				pTrigScn->ccPad.ribbonOff,
				pTrigScn->ccPad.cnv.smin, pTrigScn->ccPad.cnv.smax,
				pTrigScn->ccPad.vel.smin, pTrigScn->ccPad.vel.smax,
				pTrigScn->enable ? "Enable" : "Disable");
		break;
	case 2:	// extpad
		dprintf(eDir, "\n %2d a: onw=%3d(%5dusec),b:velw=%3d(%5dusec),c:mtim=%3d(%5dusec)",
				pTrigScn->extPad.id,
				pTrigScn->extPad.onCnt, (int)((float)pTrigScn->extPad.onCnt * 1000.0f/16.0f),
				pTrigScn->extPad.velWnd, (int)((float)pTrigScn->extPad.velWnd * 1000.0f/16.0f),
				pTrigScn->extPad.mskTim, (int)((float)pTrigScn->extPad.mskTim * 3 * 1000.0f/16.0f));
		dprintf(eDir, "\n    d:xcan=%1d,e: onl=%4d,f:vmin=%4d,g:vmax=%4d,h:send=%s",
				pTrigScn->extPad.crsCan,
				pTrigScn->extPad.onLvl,
				pTrigScn->extPad.vel.smin, pTrigScn->extPad.vel.smax,
				pTrigScn->enable ? "Enable" : "Disable");
		break;
	case 3:	// swPedal
		dprintf(eDir, "\n %2d a: onw=%3d(%5dusec),b:offw=%3d(%5dusec),c:offl=%4d,d:send=%s",
				pTrigScn->swPedal.id,
				pTrigScn->swPedal.onCnt, (int)((float)pTrigScn->swPedal.onCnt * 1000.0f/16.0f),
				pTrigScn->swPedal.offCnt, (int)((float)pTrigScn->swPedal.offCnt * 1000.0f/16.0f),
				pTrigScn->swPedal.swOff,
				pTrigScn->enable ? "Enable" : "Disable");
		break;
	case 4:	// vrPedal
		dprintf(eDir, "\n %2d a:flt1=%3d,b:flt2=%3d,c:hysw=%3d,d:mtim=%3d,e: min=%4d,f: max=%4d",
				pTrigScn->vrPedal.id,
				pTrigScn->vrPedal.flt1,
				pTrigScn->vrPedal.flt2,
				pTrigScn->vrPedal.hysWnd,
				pTrigScn->vrPedal.mskTim,
				pTrigScn->vrPedal.vol.smin, pTrigScn->vrPedal.vol.smax);
		dprintf(eDir, "\n    g:posA=%3d,h:posB=%3d,i:vmin=%4d,j:vmax=%4d,k: pow=%d,l:send=%s",
				pTrigScn->vrPedal.posA,
				pTrigScn->vrPedal.posB,
				pTrigScn->vrPedal.vel.smin,
				pTrigScn->vrPedal.vel.smax,
				pTrigScn->vrPedal.pow,
				pTrigScn->enable ? "Enable" : "Disable");
		break;
	default:
		dprintf(eDir, " unknown type");
		break;
	}

	return;
}

static void TrigEditCmdSetValue(ESendDir eDir, int num, int sel, int value)
{
	TRIGSCN_t *pTrigScn = &triggerGetParamPtr()[num];
	int type = TrigEditCmdGetType(pTrigScn->func);
	int old = -1;

	switch (type)
	{
	case 0:	// pad
		switch (sel)
		{
		case 0:	// a
			old = pTrigScn->pad.onCnt;
			pTrigScn->pad.onCnt = value;
			break;
		case 1:	// b
			old = pTrigScn->pad.velWnd;
			pTrigScn->pad.velWnd = value;
			break;
		case 2:	// c
			old = pTrigScn->pad.mskTim;
			pTrigScn->pad.mskTim = value;
			break;
		case 3:	// d
			old = pTrigScn->pad.padOff;
			pTrigScn->pad.padOff = value;
			break;
		case 4:	// e
			old = pTrigScn->pad.vel.smin;
			pTrigScn->pad.vel.smin = value;
			break;
		case 5:	// f
			old = pTrigScn->pad.vel.smax;
			pTrigScn->pad.vel.smax = value;
			break;
		case 6:	// g
			old = pTrigScn->enable;
			pTrigScn->enable = value;
			break;
		default:
			break;
		}
		break;
	case 1:	// ccPad
		switch (sel)
		{
		case 0:	// a
			old = pTrigScn->ccPad.onCnt;
			pTrigScn->ccPad.onCnt = value;
			break;
		case 1:	// b
			old = pTrigScn->ccPad.velWnd;
			pTrigScn->ccPad.velWnd = value;
			break;
		case 2:	// c
			old = pTrigScn->ccPad.mskTim;
			pTrigScn->ccPad.mskTim = value;
			break;
		case 3:	// d
			old = pTrigScn->ccPad.ribbonOff;
			pTrigScn->ccPad.ribbonOff = value;
			break;
		case 4:	// e
			old = pTrigScn->ccPad.cnv.smin;
			pTrigScn->ccPad.cnv.smin = value;
			break;
		case 5:	// f
			old = pTrigScn->ccPad.cnv.smax;
			pTrigScn->ccPad.cnv.smax = value;
			break;
		case 6:	// g
			old = pTrigScn->ccPad.vel.smin;
			pTrigScn->ccPad.vel.smin = value;
			break;
		case 7:	// h
			old = pTrigScn->ccPad.vel.smax;
			pTrigScn->ccPad.vel.smax = value;
			break;
		case 8:	// i
			old = pTrigScn->enable;
			pTrigScn->enable = value;
		default:
			break;
		}
		break;
	case 2:	// extPad
		switch (sel)
		{
		case 0:	// a
			old = pTrigScn->extPad.onCnt;
			pTrigScn->extPad.onCnt = value;
			break;
		case 1:	// b
			old = pTrigScn->extPad.velWnd;
			pTrigScn->extPad.velWnd = value;
			break;
		case 2:	// c
			old = pTrigScn->extPad.mskTim;
			pTrigScn->extPad.mskTim = value;
			break;
		case 3:	// d
			old = pTrigScn->extPad.crsCan;
			pTrigScn->extPad.crsCan = value;
			break;
		case 4:	// e
			old = pTrigScn->extPad.onLvl;
			pTrigScn->extPad.onLvl = value;
			break;
		case 5:	// f
			old = pTrigScn->extPad.vel.smin;
			pTrigScn->extPad.vel.smin = value;
			break;
		case 6:	// g
			old = pTrigScn->extPad.vel.smax;
			pTrigScn->extPad.vel.smax = value;
			break;
		case 7:	// h
			old = pTrigScn->enable;
			pTrigScn->enable = value;
		default:
			break;
		}
		break;
	case 3:	// swPedal
		switch (sel)
		{
		case 0:	// a
			old = pTrigScn->swPedal.onCnt;
			pTrigScn->swPedal.onCnt = value;
			break;
		case 1:	// b
			old = pTrigScn->swPedal.offCnt;
			pTrigScn->swPedal.offCnt = value;
			break;
		case 2:	// c
			old = pTrigScn->swPedal.swOff;
			pTrigScn->swPedal.swOff = value;
			break;
		case 3:	// d
			old = pTrigScn->enable;
			pTrigScn->enable = value;
		default:
			break;
		}
		break;
	case 4:	// vrPedal
		switch (sel)
		{
		case 0:	// a
			old = pTrigScn->vrPedal.flt1;
			pTrigScn->vrPedal.flt1 = value;
			break;
		case 1:	// b
			old = pTrigScn->vrPedal.flt2;
			pTrigScn->vrPedal.flt2 = value;
			break;
		case 2:	// c
			old = pTrigScn->vrPedal.hysWnd;
			pTrigScn->vrPedal.hysWnd = value;
			break;
		case 3:	// d
			old = pTrigScn->vrPedal.mskTim;
			pTrigScn->vrPedal.mskTim = value;
			break;
		case 4:	// e
			old = pTrigScn->vrPedal.vol.smin;
			pTrigScn->vrPedal.vol.smin = value;
			break;
		case 5:	// f
			old = pTrigScn->vrPedal.vol.smax;
			pTrigScn->vrPedal.vol.smax = value;
			break;
		case 6:	// g
			old = pTrigScn->vrPedal.posA;
			pTrigScn->vrPedal.posA = value;
			break;
		case 7:	// h
			old = pTrigScn->vrPedal.posB;
			pTrigScn->vrPedal.posB = value;
			break;
		case 8:	// i
			old = pTrigScn->vrPedal.vel.smin;
			pTrigScn->vrPedal.vel.smin = value;
			break;
		case 9:	// j
			old = pTrigScn->vrPedal.vel.smax;
			pTrigScn->vrPedal.vel.smax = value;
			break;
		case 10:	// k
			old = pTrigScn->vrPedal.pow;
			pTrigScn->vrPedal.pow = value;
			break;
		case 11:	// l
			old = pTrigScn->enable;
			pTrigScn->enable = value;
			break;
		default:
			break;
		}
		break;
	default:
		dprintf(eDir, "\n unknown type");
		break;
	}
	if (old >= 0)
	{
		if (old != value)
		{
			dprintf(eDir, "\n change %c:%4d->%4d", sel+'a', old, value);
			TrigEditCmdDump(eDir, num);
		}
	}
	else
	{
		dprintf(eDir, "\n select range error !");
	}

	return;
}

static int TrigEditCmdPad(ESendDir eDir, char *cmdstr, char ofs, int min, int cnt)
{
	Result ret = CMD_OK;
	int num = -1;

	if (cmdstr[(int)ofs] == '?')
	{
		for (int i = 0; i < cnt; i++)
		{
			TrigEditCmdDump(eDir, i+min);
		}
	}
	else if (cmdstr[(int)ofs] != 0)
	{
		char *w;

		num = strtol(&cmdstr[(int)ofs], &w, 0);
		if ((num >=0) && (num < cnt))
		{
			TrigEditCmdDump(eDir, num+min);
			while ((*w != 0) && (*w == ' '))
			{
				w++;
			}
			if (w[1] == '=')
			{
				if ((*w >= 'a') && (*w <= 'l'))
				{
					int val = strtol(&w[2], NULL, 0);

					TrigEditCmdSetValue(eDir, num+min, *w - 'a', val);
				}
				else
				{
					dprintf(eDir, "\n unknown select error !");
				}
			}
		}
		else
		{
			dprintf(eDir, "\n target range error !");
		}
	}
	else
	{
		dprintf(eDir, "\n subcmd:?        (all display)");
		dprintf(eDir, "\n        tg       (display tg=0~)");
		dprintf(eDir, "\n        tg X=val (set tg=0~, X=a~, val=value)");
	}

	return ret;
}

static int TrigEditCmd(ESendDir eDir, char *cmdstr, char ofs)
{
	Result ret = CMD_OK;

	char const *cmdlist[] = {"pad","ccPad","extPad","swPedal","vrPedal"};
	int cmdnum = -1;

	for (int i = 0; i < sizeof(cmdlist)/sizeof(char *); i++)
	{
		if (strncmp(&cmdstr[(int)ofs], cmdlist[i], strlen(cmdlist[i])) == 0)
		{
			cmdnum = i;
			break;
		}
	}
	if (cmdnum >= 0)
	{
		dprintf(eDir, "\n cmd = %s", cmdlist[cmdnum]);
		ofs += strlen(cmdlist[cmdnum]);
		while ((cmdstr[(int)ofs] != 0) && (cmdstr[(int)ofs] == ' '))
		{
			ofs++;
		}
		switch (cmdnum)
		{
		case 0:	// pad
			ret = (Result)TrigEditCmdPad(eDir, cmdstr, ofs, eTrig_Pad1, eTrig_PadNumOf);
			break;
		case 1:	// ccPad
			ret = (Result)TrigEditCmdPad(eDir, cmdstr, ofs, eTrig_ccPad1, eTrig_CCPadNumOf);
			break;
		case 2:	// extPad
			ret = (Result)TrigEditCmdPad(eDir, cmdstr, ofs, eTrig_extPad1, eTrig_ExtPadNumOf);
			break;
		case 3: // swPedal
			ret = (Result)TrigEditCmdPad(eDir, cmdstr, ofs, eTrig_swPedal1, eTrig_SwPedalNumOf);
			break;
		case 4: // vrPedal
			ret = (Result)TrigEditCmdPad(eDir, cmdstr, ofs, eTrig_vrPedal1, eTrig_VrPedalNumOf);
			break;
		default:	// debug
			break;
		}
	}
	else
	{
		dprintf(eDir, "\n unknown command -> %s", &cmdstr[(int)ofs]);
	}

	return ret;
}

static int TrigEdit(ESendDir eDir, char *cmdstr, char ofs)
{
	Result ret = CMD_OK;

	/*	デリミタ削除	*/
	while ((cmdstr[(int)ofs] != 0) && (cmdstr[(int)ofs] == ' '))
	{
		ofs++;
	}
	if (cmdstr[(int)ofs] != 0)
	{	// cmd...
		ret = (Result)TrigEditCmd(eDir, cmdstr, ofs);
	}
	else
	{	// help
		dprintf(eDir, "\n usage>TrigEdit cmd...");
		dprintf(eDir, "\n   cmd:pad (0~5)");
		dprintf(eDir, "\n       ccPad (0~3)");
		dprintf(eDir, "\n       extPad (0~3)");
		dprintf(eDir, "\n       swPedal (0~1)");
		dprintf(eDir, "\n       vrPedal (0~1)");
	}

	return ret;
}

#define TRIGEDITCMD	{"TrigEdit", TrigEdit},
#else	//TRIGEDIT
#define TRIGEDITCMD
#endif	//TRIGEDIT

#ifdef MIDIROOT

int txChangeSet(int x);

static int MidiRoot(ESendDir eDir, char *cmdstr, char ofs)
{
	Result ret = CMD_OK;
	int sw = -1;

	/*	デリミタ削除	*/
	while ((cmdstr[(int)ofs] != 0) && (cmdstr[(int)ofs] == ' '))
	{
		ofs++;
	}
	if (cmdstr[(int)ofs] != 0)
	{
		sw = strtol(&cmdstr[(int)ofs], NULL, 0);
		if (sw == 1)
		{
			sw = 1;
		}
		else if (sw == 2)
		{
			sw = 0;
		}
		else
		{
			sw = -1;
		}
	}
	if (sw >= 0)
	{
		dprintf(eDir, "\n set ulz%d", 2 - txChangeSet(sw));
	}
	else
	{
		dprintf(eDir, "\n now ulz%d", 2 - txChangeSet(sw));
	}

	return ret;
}

#define MIDIROOTCMD	{"MidiRoot (1|2)", MidiRoot},
#else	//MIDIROOT
#define MIDIROOTCMD
#endif	//MIDIROOT

#ifdef TRIGAUDIO

static int trigAudioLch = -1;
static int trigAudioRch = -1;
static int32_t trigAudioL = 0;
static int32_t trigAudioR = 0;

int TrigAudioSet(uint32_t value, uint8_t ch)
{
	int ret = 0;

	if ((trigAudioLch == ch) || (trigAudioRch == ch))
	{
		int32_t val = value << 19;	// 000~FFF -> 00000000~7FF80000

		val -= 0x40000000l;			// C0000000~3FF80000
		val *= 2;					// 80000000~7FF00000
		if (trigAudioLch == ch)
		{
			trigAudioL = val;
		}
		if (trigAudioRch == ch)
		{
			trigAudioR = val;
		}
		ret = 1;
	}

	return ret;
}

void TrigAudioGetLR(int32_t *pL, int32_t *pR)
{
	if (pL && (trigAudioLch >= 0))
	{
		*pL = trigAudioL;
	}
	if (pR && (trigAudioRch >= 0))
	{
		*pR = trigAudioR;
	}

	return;
}

static const int TrigAudioCnvTbl[] = {-1
							, PAD_36_AD_CH
							, PAD_37_AD_CH
							, PAD_38_AD_CH
							, PAD_39_AD_CH
							, PAD_40_AD_CH
							, PAD_41_AD_CH
							, LEFT_TRIG_AD_CH
							, CLEFT_TRIG_AD_CH
							, CRIGHT_TRIG_AD_CH
							, RIGHT_TRIG_AD_CH
							, EXTPAD_46_AD_CH
							, EXTPAD_47_AD_CH
							, EXTPAD_48_AD_CH
							, EXTPAD_49_AD_CH
							, VRPEDAL_8_AD_CH
							, HHPEDAL_8_AD_CH
							};

static int TrigAudioGetChNum(int adNum)
{
	int ret = -1;

	for (int i = 0; i <= 16; i++)
	{
		if (TrigAudioCnvTbl[i] == adNum)
		{
			ret = i;
			break;
		}
	}

	return ret;
}

static int TrigAudio(ESendDir eDir, char *cmdstr, char ofs)
{
	Result ret = CMD_OK;
	int disp = 0;

	if (cmdstr[(int)ofs])
	{
		if (cmdstr[(int)ofs+1] == '?')
		{
			disp = 2;
		}
		else if (cmdstr[(int)ofs+1])
		{
			char *w;
			int n;

			n = strtol(&cmdstr[ofs+1], &w, 0);
			if ((n >= 0) && (n <= 16))
			{
				trigAudioLch = TrigAudioCnvTbl[n];
			}
			if (*w)
			{
				n = strtol(++w, NULL, 0);
				if ((n >= 0) && (n <= 16))
				{
					trigAudioRch = TrigAudioCnvTbl[n];
				}
			}
			if ((trigAudioLch >= 0) || (trigAudioRch >= 0))
			{
				HC_AudioSetCallback(TrigAudioGetLR);
			}
			else
			{
				HC_AudioSetCallback(NULL);
			}
			disp = 1;
		}
		else
		{
			disp = 2;
		}
	}
	else
	{
		disp = 1;
	}
	if (disp == 1)
	{
		int Lch = TrigAudioGetChNum(trigAudioLch);
		int Rch = TrigAudioGetChNum(trigAudioRch);

		dprintf(eDir,"\n Lch=%d, Rch=%d", Lch, Rch);
	}
	else if (disp == 2)
	{
		dprintf(eDir, "\n usage>TrigAudio (?|Lch(,Rch))");
		dprintf(eDir, "\n       Lch = 0:off,1~16:pad1~15,HH");
		dprintf(eDir, "\n       Rch = 0:off,1~16:pad1~15,HH");
	}

	return ret;
}

#define TRIGAUDIOCMD	{"TrigAudio (?|Lch(,Rch))", TrigAudio},
#else	//TRIGAUDIO
#define TRIGAUDIOCMD
#endif	//TRIGAUDIO

#ifdef VELCRVDUMP

static int VelCrvDump(ESendDir eDir, char *cmdstr, char ofs)
{
	Result ret = CMD_OK;

	if (cmdstr[(int)ofs] != 0)
	{
		int num = strtol(&cmdstr[ofs+1], NULL, 0);

		if ((num >= 0) && (num <= 15))
		{
			int start = num ? num : 1;
			int end = num ? num : 15;

			for (int i = start; i <= end; i++)
			{
				dprintf(eDir, "\n *** %2d ***", i);
				for (int j = 0; j < 128; j++)
				{
					int val = trigger_getVelocityCurve(i-1, j);

					if ((j & 0xf) == 0)
					{
						dprintf(eDir, "\n %3d", val);
					}
					else
					{
						dprintf(eDir, ", %3d", val);
					}
				}
			}
		}
		else
		{
			ret = CMD_ERR;
		}
	}
	else
	{
		dprintf(eDir, "\n usage>VelCrvDump num");
		dprintf(eDir, "\n       num = 0:All or 1~15");
	}

	return ret;
}

#define VELCRVDUMPCMD	{"VelCrvDump num", VelCrvDump},
#else	//VELCRVDUMP
#define VELCRVDUMPCMD
#endif	//VELCRVDUMP

#ifdef DTSIZAUDIO
void DtSizAudioR(int32_t *pL, int32_t *pR)
{
	extern usb_device_composite_struct_t *g_deviceComposite;
	usb_audio_player_struct_t audioPlayer = g_deviceComposite->audioPlayer;
	int act = audioPlayer.tdReadNumberRec <= audioPlayer.tdWriteNumberRec ?
			audioPlayer.tdWriteNumberRec - audioPlayer.tdReadNumberRec :
			(4*2*45*4*3) - audioPlayer.tdReadNumberRec + audioPlayer.tdWriteNumberRec;

	*pR = (act - (4*2*45*4*3)/2) << 19;	// 0~4320 -> -2160~2159 -> -1132462080~1132462079

	return;
}

static int DtSizAudio(ESendDir eDir, char *cmdstr, char ofs)
{
	Result ret = CMD_OK;
	int offon = 0;	// 0:off,!0:on

	if (cmdstr[(int)ofs])
	{
		offon = strtol(&cmdstr[(int)ofs], NULL, 0);
	}
	dprintf(eDir, "\n Data Size To Audio %s", offon ? "On" : "Off");
	if (offon)
	{
		HC_AudioSetCallback(DtSizAudioR);
	}
	else
	{
		HC_AudioSetCallback(NULL);
	}

	return ret;
}

#define DTSIZAUDIOCMD	{"DtSizAudio [0/1]", DtSizAudio},
#else	//DTSIZAUDIO
#define DTSIZAUDIOCMD
#endif	//DTSIZAUDIO

#if defined(TIMINGMEASURE) && defined(TIMING_MEASURRING)
#include "hardware/systick.h"

static int TimingMeasure(ESendDir eDir, char *cmdstr, char ofs)
{
	int ret = CMD_OK;
	int cnt = 0;
	uint32_t preB = 0;

	dprintf(eDir, "\n *** Timing Log ***");
	dprintf(eDir, "\n%10d,%10d", 0, 0);
	while (1)
	{
		uint32_t time;
		uint32_t bits;
		int result = systick_logGet(&time, &bits, cnt);

		if (result < 0)
		{
			break;
		}
		if (time)
		{
			dprintf(eDir, "\n%10u,%10d", time-1, preB);
		}
		dprintf(eDir, "\n%10u,%10d", time, bits);
		preB = bits;
		cnt++;
	}

	return ret;
}

#define TIMINGMEASURECMD	{"TimingLog", TimingMeasure},
#else	//defined(TIMINGMEASURE) && defined(TIMING_MEASURRING)
#define TIMINGMEASURECMD
#endif	//defined(TIMINGMEASURE) && defined(TIMING_MEASURRING)

static int Help_Func(ESendDir eDir, char *cmdstr, char ofs);	/*	for cmd_tbl	*/

/********************/
/*	処理テーブル	*/
/********************/
static const Cmd cmd_tbl[] = {
	MEMORYDUMPCMD
	SYSTEMVIEWCMD
	JOBTIMECMD
	ADCVIEWCMD
	ADCAUDIOCMD
	WM8960DUMPCMD
	TRIGDEBUGCMD
	UARTEXINFOCMD
	TRIGEDITCMD
	MIDIROOTCMD
	TRIGAUDIOCMD
	VELCRVDUMPCMD
	DTSIZAUDIOCMD
	TIMINGMEASURECMD
	{"Help", Help_Func},
};

/*	コマンド表示	*/
static int Help_Func(ESendDir eDir, char *cmdstr, char ofs)
{
	unsigned int i;

	for (i = 0; i < sizeof(cmd_tbl)/sizeof(Cmd); i++) {
		dputs(eDir, "\n ");
		dputs(eDir, cmd_tbl[i].cmd_str);
	}

	return CMD_OK;
}

/*	実効部	*/
static void command_execute(ESendDir eDir, char *cmdstr )
{
	unsigned int i;
	int n, ret = CMD_ERR;
	char *wp;

	for (i = 0; i < sizeof(cmd_tbl)/sizeof(Cmd); i++) {
		wp = strchr(cmd_tbl[i].cmd_str, ' ');
		if (wp != NULL) {
			n = wp - cmd_tbl[i].cmd_str;
		}
		else {
			n = strlen(cmd_tbl[i].cmd_str);
		}
		if (strncmp(cmdstr, (const char *)(cmd_tbl[i].cmd_str), n) == 0) {
			if ((cmdstr[n] == 0) || (cmdstr[n] == ' ')) {
				ret = (cmd_tbl[i].cmd_fnc)(eDir, cmdstr, n);
				break;
			}
		}
	}
	if (ret != CMD_OK) {
		dputc(eDir, '\n');
		dputs(eDir, "Error !!");
	}

	return;
}

/*	ステート	*/
typedef enum{
	NO_ACTIVE,
	START_WAIT1,
	START_WAIT2,
	RUN_TOP,
}EState;

/*	キャラクター入り口	*/
/*		"@@@"で開始		*/
/*		"@"で終了		*/
static void DebugMonitor(ESendDir eDir, unsigned char dt)
{
	static int phase[2] = {NO_ACTIVE,NO_ACTIVE};
	static unsigned char cmdbuf[2][80];
	static unsigned char cmdcnt[2] = {0,0};
	static unsigned char cmdcntpre[2] = {0,0};
	int index = eDir == SDIR_UARTMIDI ? 0 : 1;

	switch (phase[index]) {
	case NO_ACTIVE:
		if (dt == '@') {
			phase[index] = START_WAIT1;
		}
		break;
	case START_WAIT1:
		if (dt == '@') {
			phase[index] = START_WAIT2;
		}
		else{
			phase[index] = NO_ACTIVE;
		}
		break;
	case START_WAIT2:
		if (dt == '@') {
			phase[index] = RUN_TOP;
			cmdcnt[index] = 0;
			SetChannel(eDir, 1);
			dprintf(eDir, "\n{{{ X-19850 (USBIF:%s) }}}\n", index == 0 ? "UART" : "USB");
			dputc(eDir, '*');	/*	プロンプト	*/
#if defined(DIR)
			path[index][0] = 0;
#endif	//defined(DIR)
		}
		else {
			phase[index] = NO_ACTIVE;
		}
		break;
	case RUN_TOP:
		if (dt == '@') {
			phase[index] = NO_ACTIVE;
			dputs(eDir, "\n{{{ exit }}}\n");
			SetChannel(eDir, 0);
		}
		else if (dt == '\n') {
			if ((cmdcnt[index] == 0) && (cmdcntpre[index] != 0)) {
				cmdcnt[index] = cmdcntpre[index];
				dputs(eDir, (char *)cmdbuf[index]);
				dputbuf_flush(eDir);
			}
			else {
				cmdbuf[index][cmdcnt[index]] = 0;
			}
			/*	exec	*/
			cmdbuf[index][cmdcnt[index]] = 0;
			command_execute(eDir, (char *)cmdbuf[index]);
#if defined(DIR)
			if (path[index][0] != 0) {
				dputc(eDir, '\n');
				dputs(eDir, path);
				dputc(eDir, '>');
			}
			else
#endif	//defined(DIR)
			dputs(eDir, "\n*");	/*	プロンプト	*/
			cmdcntpre[index] = cmdcnt[index];
			cmdcnt[index] = 0;
		}
		else{
			if( cmdcnt[index] < 80-1 ){
				dputc(eDir, dt);
				cmdbuf[index][cmdcnt[index]++] = dt;
			}
		}
		break;
	default:
		break;
	}

	return;
}

/********************************/
/*	デバッグモニタエントリー	*/
/********************************/
#if 0
static void MidiDebugMonitorIn(ESendDir eDir, unsigned char *str)
{
	while( *str != 0 ){
		DebugMonitor(eDir, *str++);
	}
	dputbuf_flush(eDir);

	return;
}
#endif

int MidiDebugMonitorCommandIn(unsigned int uiData, ESource eSource)
{
	ESendDir eDir = eSource == eSrcUartMIDI ? SDIR_UARTMIDI : SDIR_USBMIDI;

	if (uiData != 0)
	{
		DebugMonitor(eDir, uiData);
	}
	else
	{
		dputbuf_flush(eDir);
	}

	return 0;
}

#if 0
/********************/
/*	KVEエントリー	*/
/********************/
#define	KVE_POS_FXPARAM		(0x00)
#define	KVE_POS_LFO			(0x01)
#define	KVE_POS_INPUTMIXER	(0x05)
#define	KVE_POS_CONTROLLER	(0x08)
#define	KVE_POS_COMMAND		(0x7d)

static char ctrl[FX_CTRL_MAX_NUM][5];	// source,fxpos,fxparam,min,max

static void KVEditorMessage(ESendDir eDir, unsigned char posId, unsigned char fxId, unsigned char paraId, unsigned char value1, unsigned char value2)
{
	static unsigned char sendDisable = 0;
	unsigned short value;

	value = value1;
	value = (value<<7) + value2;
//	dprintf(eDir, "\n Pos:FxBox,ParaId,Val=%3d:%3d,%3d,%3d ", posId, fxId, paraId, value);
	switch (posId) {
	case KVE_POS_FXPARAM:	//(0x00)
		if ((fxId < 3) && (paraId < 32)) {
			value = value2;
			value = ((value<<7) + value1)<<1;
			FxValueUpDate(fxId, paraId, value);
			if (sendDisable == 0) {
				FxValueSend(fxId);
			}
		}
		break;
	case KVE_POS_LFO:	//(0x01)
#if defined(GLFORUN)
		if ((fxId == 0) && (paraId < NUM_GLfo1Param)) {		// LFO1
			FxValueUpDate(3, paraId+FX_LFO1_TOP_NUM, value);
			if (sendDisable == 0) {
				FxValueSend(fxId);
			}
		}
		else if ((fxId == 1) && (paraId < NUM_GLfo2Param)) {	// LFO2
			FxValueUpDate(3, paraId+FX_LFO2_TOP_NUM, value);
			if (sendDisable == 0) {
				FxValueSend(fxId);
			}
		}
#endif	//defined(GLFORUN)
		break;
	case KVE_POS_INPUTMIXER:	//(0x05)
		if ((paraId == 0) && (fxId < 3)) {
			if (fxId == 0) {
				if (value > 6) {
					value = 0;	// off
				}
			}
			else if (fxId == 1) {
				if ((value != 0) && ((value < 7) || (value > 12))) {
					value = 0;	// off
				}
			}
			else {
				if ((value != 0) && ((value < 13) || (value > 18))) {
					value = 0;	// off
				}
			}
			FxTypeUpDate(fxId, value);
			if (sendDisable == 0) {
				FxTypeSend(fxId);
			}
		}
		else if ((paraId == 1) && (fxId < 3)) {	// output level
			value = value2;
			value = ((value<<7) + value1)<<1;
			FxValueUpDate(fxId, FX_PARAM_OUT_LEVEL, value);
			if (sendDisable == 0) {
				FxValueSend(fxId);
			}
		}
		else if ((paraId == 5) && (fxId < 3)) {	// input level
			value = value2;
			value = ((value<<7) + value1)<<1;
			FxValueUpDate(fxId, FX_PARAM_IN_LEVEL, value);
			if (sendDisable == 0) {
				FxValueSend(fxId);
			}
		}
		break;
	case KVE_POS_CONTROLLER:	//(0x08)
		if ((fxId < FX_CTRL_MAX_NUM) &&
			((paraId == 0) ||
			 (paraId == 1) ||
			 (paraId == 2) ||
			 (paraId == 4) ||
			 (paraId == 5))) {
			int index = paraId;
			int box, src, para, min, max;

			if (paraId > 2) {
				index--;	// 4,5 -> 3,4
			}
			ctrl[fxId][index] = value;
			src = ctrl[fxId][0];
			box = ctrl[fxId][1];
			para = ctrl[fxId][2];
			min = ctrl[fxId][3];
			max = ctrl[fxId][4];
			if ((src == 0) || ((src >= 0x42) && (src <= 0x47))) {
				const char srctbl[] = {0,0x42,0x44,0x45,0x43,0x46,0x47};

				for (unsigned int i = 0; i < sizeof(srctbl); i++) {
					if (src == srctbl[i]) {
						src = i;
						break;
					}
				}
				if ((box >= 5) && (box <= 7)) {
					if (para == 1) {
						box -= 5;
						para = FX_PARAM_OUT_LEVEL;
//						dprintf(eDir,"\nOutput Level Assign");
					}
					else if(para == 5) {
						box -= 5;
						para = FX_PARAM_IN_LEVEL;
//						dprintf(eDir,"\nInput Level Assign");
					}
				}
				if ((box >= 0) && (box <= 2) && (para < 32)) {
					FxCtrlSet(box, fxId, src, para, min<<8, max<<8);
				}
			}
		}
		break;
	case KVE_POS_COMMAND:	//(0x7d)
		if (fxId == 0) {
			sendDisable = paraId;
			if (sendDisable == 0) {
				FxTypeSend(0);
				FxTypeSend(1);
				FxTypeSend(2);
				FxTypeSend(3);
			}
		}
		break;
	default:
		break;
	}

	return;
}

/************/
/*	受信部	*/
/************/

#define CMDBUFSIZ (1+40)	// 1+ for command store
//#define KVEBUFSIZ (256)

struct CmdBuf {
	unsigned char count;
	unsigned char buffer[CMDBUFSIZ];
#if defined(KVEBUFSIZ)
	unsigned char rdp;
	unsigned char wrp;
	unsigned char kvebuf[KVEBUFSIZ][5];
#endif //defined(KVEBUFSIZ)
};

#if defined(KVEBUFSIZ)
static struct CmdBuf cmdUartBuf = {0,0,0,0};
static struct CmdBuf cmdUsbBuf = {0,0,0,0};
#else	//defined(KVEBUFSIZ)
static struct CmdBuf cmdUartBuf = {0,0};
static struct CmdBuf cmdUsbBuf = {0,0};
#endif	//defined(KVEBUFSIZ)

/*	command execute	*/
void MidiDebugMonitorCommandExec(ESendDir eDir, char pos, char fx, long parval)
{
	struct CmdBuf *pCmdBuf = NULL;

	if (eDir == SDIR_UARTMIDI) {
		pCmdBuf = &cmdUartBuf;
	}
	else if (eDir == SDIR_USBMIDI) {
		pCmdBuf = &cmdUsbBuf;
	}
	if (pCmdBuf != NULL) {
#if defined(KVEBUFSIZ)
		while (pCmdBuf->rdp != pCmdBuf->wrp) {
			unsigned char pos	= pCmdBuf->kvebuf[pCmdBuf->rdp][0];
			unsigned char fx	= pCmdBuf->kvebuf[pCmdBuf->rdp][1];
			unsigned char para	= pCmdBuf->kvebuf[pCmdBuf->rdp][2];
			unsigned char val1	= pCmdBuf->kvebuf[pCmdBuf->rdp][3];
			unsigned char val2	= pCmdBuf->kvebuf[pCmdBuf->rdp][4];

			pCmdBuf->rdp++;
			KVEditorMessage(eDir, pos, fx, para, val1, val2);
			DebugCountUp(2);	// for check
		}
#else	//defined(KVEBUFSIZ)
		if (pos != 0x7f) {
			KVEditorMessage(eDir, pos, fx, parval & 0x7f, (parval>>7)&0x7f, (parval>>14)&0x7f);
			DebugCountUp(2);	// for check
		}
#endif	//defined(KVEBUFSIZ)
		if (pCmdBuf->buffer[0] == 0x7f) {	// MidiDebugTerminal
			MidiDebugMonitorIn(eDir, &(pCmdBuf->buffer[1]));
			pCmdBuf->buffer[0] = 0;
		}
		pCmdBuf->count = 0;
	}
	return;
}

/*	Receive data to command buffer	*/
int MidiDebugMonitorCommandIn(unsigned int uiData, ESource eSource)
{
	CCircular *pMidiInBuffer = NULL;
	struct CmdBuf *pCmdBuf = NULL;
#if defined(KVEBUFSIZ)
	int ret = false;
#else	//defined(KVEBUFSIZ)
	int ret = -3;
#endif	//defined(KVEBUFSIZ)

	if (eSource == CMIDIMessage::eSrcUartMIDI) {
		pMidiInBuffer = &g_oMIDIInBuffer;
		pCmdBuf = &cmdUartBuf;
	}
	else if (eSource == CMIDIMessage::eSrcUsbMIDI) {
		pMidiInBuffer = &g_oUSBMIDIInBuffer;
		pCmdBuf = &cmdUsbBuf;
	}
	if (pCmdBuf != NULL) {
#if !defined(KVEBUFSIZ)
		ret = pCmdBuf->count;
#endif	//!defined(KVEBUFSIZ)
		if (pCmdBuf->count < CMDBUFSIZ) {
			pCmdBuf->buffer[pCmdBuf->count++] = uiData;
		}
		if (pCmdBuf->buffer[0] == 0x7f) {	// MidiDebugTerminal
			if (uiData == 0) {
#if defined(KVEBUFSIZ)
				ret = true;
#else	//defined(KVEBUFSIZ)
				ret = -1;
#endif	//defined(KVEBUFSIZ)
				pCmdBuf->count = 0;
			}
		}
		else {	// KVE
			if (pCmdBuf->count == 5) {
#if defined(KVEBUFSIZ)
				unsigned char nxtwrp = pCmdBuf->wrp +1;

				if (nxtwrp != pCmdBuf->rdp) {
					for (int i = 0; i < 5; i++) {
						pCmdBuf->kvebuf[pCmdBuf->wrp][i] = pCmdBuf->buffer[i];
					}
					pCmdBuf->wrp++;
				}
				else {
					DebugCountUp(3);	// for check
				}
				ret = true;
#else	//defined(KVEBUFSIZ)
				ret = -2;
#endif	//defined(KVEBUFSIZ)
				pCmdBuf->count = 0;
				DebugCountUp(1);	// for check
			}
		}
	}
	return ret;
}
#endif

#undef dputbuf_flush
#undef dputc
#undef dputs
#undef dprintf

//}

#endif //defined(MIDIDEBUGMONITOR)
