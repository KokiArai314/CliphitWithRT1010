/*
	oscillator.h
	copy from x19850
*/

#ifndef OSCILLATOR_H_
#define OSCILLATOR_H_

#define OSCIMGBIT	(28)
#define OSCEQPITCH	(1<<OSCIMGBIT)
#define OSCIMGMSK	(OSCEQPITCH-1)

#define STREAMBUFSIZE	(128*1024)	// 743msec@44.1kHz:16bit:stereo
//#define STREAMBUFSIZE	(256*1024)	// 1486msec@44.1kHz:16bit:stereo

#define STREAMMEMCPYENABLE	// 定義すると、非キャッシュ領域で受けて、キャッシュ領域へmemcpyする。

//#define STREAMLOOPSUPPORT	// 定義すると、ストリーミングでループポイント再生をサポートする。

#include "fader.h"
#include <stdint.h>

/*	 
[Memory]
	Stretcher_t
	+--------------+
	|SampleData_t *|>--->SampleData_t
	|              |     +------------+
	|Fader_t       |  ,-<|DataPtr *   |
	|+------------+|  |  |            |
	||            ||  |  +------------+
	|+------------+|  |
	|              |  |
	|Oscillator_t  |  |
	|+------------+|  |
	||DataPtr *   ||>-*->{Sample Data}
	||            ||  |  +------------+
	|+------------+|  |  |            |
	|Oscillator_t  |  |  +------------+
	|+------------+|  |
	||DataPtr *   ||>-'
	||            ||
	|+------------+|
	+--------------+

[Stream]
	Stretcher_t
	+--------------+
	|SampleData_t *|>--->SampleData_t
	|              |     +------------+
	|Fader_t       |     |DataPtr *   |>--->WaveSampleFile_t
	|+------------+|     |            |     +----------------+
	||            ||     +------------+     |                |
	|+------------+|                        +----------------+
	|              |
	|Oscillator_t  |
	|+------------+|        
	||DataPtr *   ||>-*->StreamData_t
	||            ||  |  +------------+
	|+------------+|  |  |            |
	|Oscillator_t  |  |  +------------+
	|+------------+|  |
	||DataPtr *   ||>-'
	||            ||
	|+------------+|
	+--------------+

	Streamer_t
	+--------------+
	|SampleData_t *|>--->SampleData_t
	|              |     +------------+
	|Oscillator_t  |     |DataPtr *   |>--->WaveSampleFile_t
	|+------------+|     |            |     +----------------+
	||DataPtr *   ||>-,  +------------+     |                |
	||            ||  |                     +----------------+
	|+------------+|  `->StreamData_t  
	+--------------+     +------------+
						 |            |
						 +------------+
*/

typedef struct streamdata_t_ {
	uint32_t	dataofs;	// ファイルの先頭からのデータ先頭位置
	uint32_t	buftop0;	// buf0の先頭データのファイル位置
	uint32_t	buftop1;	// buf1の先頭データのファイル位置
	uint32_t	buftop2;	// buf2の先頭データのファイル位置
#ifdef STREAMLOOPSUPPORT
	uint32_t	buftop3;	// buf3の先頭データのファイル位置
	uint32_t	buftop4;	// buf4の先頭データのファイル位置
#endif //STREAMLOOPSUPPORT
	uint32_t	buftop1req;	// buf1の読み出しリクエストのファイル位置
	uint32_t	buftop2req;	// buf2の読み出しリクエストのファイル位置
	uint8_t		*buf0;		// buf0の実体を指す
	uint8_t		*buf1;		// buf1の実体を指す
	uint8_t		*buf2;		// buf2の実体を指す
#ifdef STREAMLOOPSUPPORT
	uint8_t		*buf3;		// buf3の実体を指す
	uint8_t		*buf4;		// buf4の実体を指す
#endif //STREAMLOOPSUPPORT
} StreamData_t;

typedef struct sampledata_t_ {
	void		*DataPtr;	// StartAdrPointer or etc.
	int			SampleByte;	// byte per sample
	uint32_t	Length;
	uint32_t	MonoStereo;	// 1:mono,2:stereo,3:mono stream,4:stereo stream
	float		FsAdjust;	// 44100=1.00, 48000=1.0884 or other
	uint32_t	TempoX100;	// original tempo *100
	uint32_t	StartOfs;	//  |<->|--------|-----|
	uint32_t	EndOfs;		//	|   |<------>|     |
	uint32_t	LoopOfs;	//	|     |<---->|     |
} SampleData_t;

typedef struct oscillator_t_ {
	void		*DataPtr;	// StartAdrPointer or etc.
	int			SampleByte;	// byte per sample
	uint32_t	EndOfs;		// (=length)
	uint32_t	LoopOfs;	// 0:oneshot,other:loop(=loop length)
	uint32_t	Pitch;		// 4:Real.28:Image
	uint32_t	CurOfs;		//
	uint32_t	CurImg;		// 28bit
	float		fLevel;
	float		fPan;		// 0.0f:Left~0.5f:Center~1.0f:Right
} Oscillator_t;

typedef struct stretcher_t_ {
	SampleData_t	*psSampleData;
	uint32_t		TempoX100;	// replay tempo *100
	uint32_t		Pitch;		// replay pitch
	uint32_t		CurOfs;		// sample base
	uint32_t		CurImg;		// 28bit
	uint32_t		Window;		// sample base
	uint32_t		WindowCnt;	// sample base
	uint16_t		Osc1or2;
	uint8_t			Rnd8;		// N*5+1
	float			fLevel;
	float			fPan;
	float			fFadeRate;
	Fader_t			xfader;
	void			(*oscSetup)(Oscillator_t *sp, SampleData_t *psSampleData, int sb, uint32_t ena, uint32_t lof, uint32_t pit, uint32_t cof, uint32_t cim, float flv, float fpn);
	int				(*oscExec)(Oscillator_t *sp, float **ppfDst, int samples);
	Oscillator_t	osc1;
	Oscillator_t	osc2;
} Stretcher_t;

#define STRETCHWINDOW	(((441*4)/4)*4)					// 40msec@44.1kHz
#define STRETCHCROSFDf	(1.0f / (44.1f * 5.0f) * 4.0f)	// 5msec@44.1kHz execute/4fs

typedef struct streamer_t_ {
	SampleData_t	*psSampleData;
	void			(*oscSetup)(Oscillator_t *sp, SampleData_t *psSampleData, int sb, uint32_t ena, uint32_t lof, uint32_t pit, uint32_t cof, uint32_t cim, float flv, float fpn);
	int				(*oscExec)(Oscillator_t *sp, float **ppfDst, int samples);
	Oscillator_t	osc;
} Streamer_t;

typedef struct {
	void			(*oscSetup)(Oscillator_t *sp, SampleData_t *psSampleData, int sb, uint32_t ena, uint32_t lof, uint32_t pit, uint32_t cof, uint32_t cim, float flv, float fpn);
	int				(*oscExec)(Oscillator_t *sp, float **ppfDst, int samples);
	Oscillator_t	osc;
} OnMemOsc_t;

int StretchOscillatorExecute(Stretcher_t *ps, float **ppfDst, int samples);
void StretchOscillatorSetup(Stretcher_t *ps, SampleData_t *psSampleData);
void StreamOscillatorSetup(Streamer_t *ps, SampleData_t *psSampleData);
void OnMemoryOscillatorSetup(OnMemOsc_t *ps, SampleData_t *psSampleData);

#endif	//OSCILLATOR_H_
