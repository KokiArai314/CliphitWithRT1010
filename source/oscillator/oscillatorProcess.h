/*
	oscillatorProcess.h
	copy from x19850
*/

#ifndef OSCILLATORPROCESS_H_
#define OSCILLATORPROCESS_H_

#include "fader.h"
#include "oscillator.h"

//trans plant to CLIPHIT2
//#define MEMVOICEMAX	(32)
//#define DRVVOICEMAX	(16)
#define MEMVOICEMAX	(6)
#define DRVVOICEMAX	(0)

#define AMPEG_ZERO_CLIP		(0.000002f)	//-114dB!!

typedef enum {
	eAmpEgPhaseStop = 0,
	eAmpEgPhaseAttack,
	eAmpEgPhaseDecay,
	eAmpEgPhaseOff,
} eAmpEgPhase;

typedef struct sAmpEGCoef_ {
	eAmpEgPhase	phase;			// 0:stop,1:attack,2:decay,3:off
	float		EGMasterPhase;
	float		TimeIncValue;
	float		LPF_State;
	uint8_t		attack;
	uint8_t		decay;
	uint8_t		AmpEGCurve;
	uint32_t	decayStart;		// sample
	uint32_t	decayLength;	// sample
} sAmpEGCoef;

typedef struct oscFunction_ {
	void (*setup)(struct vcb_ *pVcb);
	void (*end)(struct vcb_ *pVcb);
	uint32_t (*tell)(struct vcb_ *pVcb);
	int (*seek)(struct vcb_ *pVcb, int32_t seekSamples, int (*osc)(struct vcb_ *, float **), void (*eg)(struct vcb_ *));
	int (*exec)(void *ps, float **ppfDst, int samples);
} OscFunction_t;

typedef struct oscsetup_ {
	SampleData_t *psSampleData;
	float fCentPitch;	// modify 参照用
	float fPitch;
	float fVerocity;
	float fLevel;
	float fPan;
	float fTempo;	// 0.5~2.0:倍率、20.0~300.0:即値
	unsigned long length;	// loop sample then set 0
	int egAttack;
	int egDecay;
	int egDecayCurve;
	int repeat;		// 0:one shot, 1:repeat, 2:repeat with masterTuneEnable
	int outCh;		// 0:main,1:fx1,2:fx2,3:sub
	uint32_t repeatCount;
	uint32_t checkCount;
	OscFunction_t	oscFunc;
	void *oscObject;
} OscSetup_t;

typedef struct vcb_ {
	union {
		struct {
			uint8_t active:1;	// 生きている
			uint8_t offReq:1;	// 停止要求
			uint8_t	egMute:1;	// EGミュート動作中
			uint8_t onReq:1;	// 開始要求
			uint8_t setupReq:1;	// セットアップ要求
			uint8_t pause:1;	// ポーズ中
			uint8_t pauseReq:1;	// ポーズ要求
		};
		uint8_t all;
	} flag;
	int8_t		vcbNum;		// 自身の番号(oscillatorStartでセットされる)
	float		*pfLevel;	// レベルの更新先をセットする
	float		*pfPan;		// パンの更新先をセットする
	float		oscLevel;	// ここに放り込むと msFader に取り込まれる
	Fader_t		msFader;
	sAmpEGCoef	ampEg;
	float		voiceLevel;	// velocity * other : eg で参照される
	uint32_t	fsCount;	// startでリセット

	OscSetup_t	runOscParam;
	OscSetup_t	nxtOscParam;
} Vcb_t;

float getEgTimeValueF(int value);
uint32_t chkFreeMemVcb(void);
uint32_t chkFreeDrvVcb(void);
void oscillatorProcess(float **ppfOut, int fs);
void oscillatorProcessAmpEgReset(Vcb_t *pVcb);
void oscillatorStart(uint8_t oscNum, OscSetup_t *psOscSetup);
void oscillatorStop(uint8_t oscNum);
bool oscillatorPauseTgl(uint8_t oscNum);
bool oscillatorPauseGet(uint8_t oscNum);
void oscillatorPauseSet(uint8_t oscNum, bool pause);
int oscillatorSeek(uint8_t oscNum, int32_t seekSample);
Vcb_t *getVcbPtr(int num);

#endif /* OSCILLATORPROCESS_H_ */
