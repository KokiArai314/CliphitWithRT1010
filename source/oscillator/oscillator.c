/*
	oscillator.cpp to .c
	copy from x19850

	x19850 -> ClipHit2移植
	streamとstretch関連、assignerに関連するものを削除
*/

#include <stdint.h>
#include <stddef.h>
#include "oscillator.h"

/*
	オシレーター処理(ステレオソース、ステレオアウト)
	return 1:終端に達した(One Shot の場合は、pitchをクリア)
*/
static int OscStereo(Oscillator_t *ps, float **ppfDst, int samples)
{
	uint32_t	pitch = ps->Pitch;
	float *pfDstL = ppfDst[0];
	float *pfDstR = ppfDst[1];
	int ret = 0;

	if (pitch)
	{	// 有効
		int16_t		*baseAdr = (int16_t *)ps->DataPtr;

		if (baseAdr)
		{	// 有効
			uint32_t	curOfs = ps->CurOfs * 2;				// *2 for stereo
			uint32_t	curImg = ps->CurImg;
			uint32_t	endOfs = ps->EndOfs * 2;				// *2 for stereo
			uint32_t	loopOfs = ps->LoopOfs * 2;				// *2 for stereo
			uint32_t	pitchInt = (pitch >> OSCIMGBIT) * 2;	// *2 for stereo
			uint32_t	pitchImg = pitch & OSCIMGMSK;
			float		fPanL = ps->fLevel * (ps->fPan > 0.5f ? 1.0f - ps->fPan : 0.5f) * 2.0f;
			float		fPanR = ps->fLevel * (ps->fPan > 0.5f ? 0.5f : ps->fPan) * 2.0f;

			for (int i = 0; i < samples; i++)
			{
				int16_t dt1L, dt1R, dt2L,dt2R;
				float	fDtL, fDtR;

				// データ取得
				dt1L = baseAdr[curOfs];
				dt1R = baseAdr[curOfs+1];
				if (curOfs < endOfs)
				{
					dt2L = baseAdr[curOfs+2];
					dt2R = baseAdr[curOfs+3];
				}
				else
				{
					dt2L = baseAdr[curOfs-loopOfs];
					dt2R = baseAdr[curOfs-loopOfs+1];
				}

				// データ生成
				fDtL = dt2L - dt1L;
				fDtR = dt2R - dt1R;
				fDtL *= curImg;
				fDtR *= curImg;
				fDtL /= OSCEQPITCH;
				fDtR /= OSCEQPITCH;
				fDtL += dt1L;
				fDtR += dt1R;
				fDtL /= 32768.0f;
				fDtR /= 32768.0f;
				*pfDstL++ += fDtL * fPanL;
				*pfDstR++ += fDtR * fPanR;

				// アドレス更新
				curOfs += pitchInt;
				curImg += pitchImg;
				if (curImg >= OSCEQPITCH)
				{
					curImg -= OSCEQPITCH;
					curOfs += 2;								// +2 for stereo
				}

				// 終了チェック
				if (curOfs >= endOfs)
				{
					ret = 1;
					if (loopOfs)
					{	// loop
						curOfs -= loopOfs;
					}
					else
					{	// one shot
						ps->Pitch = 0;	// 停止
						break;
					}
				}
			}
			ps->CurOfs = curOfs / 2;							// /2 for stereo
			ps->CurImg = curImg;
		}
	}
	else
	{
		ret = 1;
	}

	return ret;
}

/*
	オシレーター処理(モノラルソース、ステレオアウト)
	return 1:終端に達した(One Shot の場合は、pitchをクリア)
*/
static int OscMono(Oscillator_t *ps, float **ppfDst, int samples)
{
	uint32_t	pitch = ps->Pitch;
	float *pfDstL = ppfDst[0]; 
	float *pfDstR = ppfDst[1]; 
	int ret = 0;

	if (pitch)
	{	// 有効
		int16_t		*baseAdr = (int16_t *)ps->DataPtr;

		if (baseAdr)
		{	// 有効
			uint32_t	curOfs = ps->CurOfs; //pitch操作のためのサンプル位置
			uint32_t	curImg = ps->CurImg; //pitch操作のためのサンプル位置の小数点分
			uint32_t	endOfs = ps->EndOfs;
			uint32_t	loopOfs = ps->LoopOfs;
			uint32_t	pitchInt = (pitch >> OSCIMGBIT);
			uint32_t	pitchImg = pitch & OSCIMGMSK;
			float		fPanL = ps->fLevel * (ps->fPan > 0.5f ? 1.0f - ps->fPan : 0.5f) * 2.0f;
			float		fPanR = ps->fLevel * (ps->fPan > 0.5f ? 0.5f : ps->fPan) * 2.0f;

			for (int i = 0; i < samples; i++)
			{
				int16_t dt1, dt2;
				float	fDt;

				// データ取得
				dt1 = baseAdr[curOfs];
				if (curOfs < endOfs)
				{
					dt2 = baseAdr[curOfs+1];
				}
				else
				{
					dt2 = baseAdr[curOfs-loopOfs];
				}

				// データ生成
				fDt = dt2 - dt1;
				fDt *= curImg;
				fDt /= OSCEQPITCH;
				fDt += dt1;
				fDt /= 32768.0f;
				*pfDstL++ += fDt * fPanL;
				*pfDstR++ += fDt * fPanR;

				// アドレス更新
				curOfs += pitchInt;
				curImg += pitchImg;
				if (curImg > OSCEQPITCH)
				{
					curImg -= OSCEQPITCH;
					curOfs++;
				}

				// 終了チェック
				if (curOfs >= endOfs)
				{
					ret = 1;
					if (loopOfs)
					{	// loop
						curOfs -= loopOfs;
					}
					else
					{	// one shot
						ps->Pitch = 0;	// 停止
						break;
					}
				}
			}
			ps->CurOfs = curOfs;
			ps->CurImg = curImg;
		}
	}
	else
	{
		ret = 1;
	}

	return ret;
}

/*
	オシレータセットアップ（オンメモリ）
*/
static void oscSetup(Oscillator_t *ps, SampleData_t *psSampleData, int sb, uint32_t ena, uint32_t lof, uint32_t pit, uint32_t cof, uint32_t cim, float flv, float fpn)
{
	int ms = 2 - (psSampleData->MonoStereo % 2);
	int16_t *sta = ((int16_t *)psSampleData->DataPtr) + psSampleData->StartOfs * ms;

	ps->DataPtr	= (void *)sta;
	ps->EndOfs	= ena;
	ps->LoopOfs	= lof;
	ps->Pitch	= pit;
	ps->CurOfs	= cof;
	ps->CurImg	= cim;
	ps->fLevel	= flv;
	ps->fPan	= fpn;

	return;
}


/*
	オシレータセットアップファンクションを得る
*/
static void (*GetOscillatorSetupFunc(int type))(Oscillator_t *, SampleData_t *, int, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, float, float)
{
	/*static void (* const tbl[])(Oscillator_t *, SampleData_t *, int, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, float, float) = {
		NULL, oscSetup, oscSetup, oscStreamSetup, oscStreamSetup, oscStreamSetup, oscStreamSetup,
	};*/
	static void (* const tbl[])(Oscillator_t *, SampleData_t *, int, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, float, float) = {
		NULL, oscSetup, oscSetup
	};

	if ((0 <= type) && (type < sizeof(tbl)/sizeof(tbl[0])))
	{
		return tbl[type];
	}
	else
	{
		return NULL;
	}
}

/*
	オシレータ実行ファンクションを得る
*/

static int (*GetOscillatorExecFunc(int ms))(Oscillator_t *sp, float **ppfDst, int samples)
{
	/*
	static int (* const tbl[])(Oscillator_t *, float **, int) = {
		NULL, OscMono, OscStereo, OscStreamMono, OscStreamStereo, OscStreamMonoF, OscStreamStereoF,
	};*/
	static int (* const tbl[])(Oscillator_t *, float **, int) = {
		NULL, OscMono, OscStereo
	};

	if ((0 <= ms) && (ms < sizeof(tbl)/sizeof(tbl[0])))
	{
		return tbl[ms];
	}
	else
	{
		return NULL;
	}
}


/*
	オンメモリオシレーターセットアップ
*/
void OnMemoryOscillatorSetup(OnMemOsc_t *ps, SampleData_t *psSampleData)
{
	int type = psSampleData ? psSampleData->MonoStereo : 0;

	ps->oscSetup	 = GetOscillatorSetupFunc(type);
	ps->oscExec		 = GetOscillatorExecFunc(type);

	return;
}

/**
 *以下x19850 -> ClipHit2移植時にstreamとstretch関連、assignerに関連するものをシュリンクした
 */

/*
	ストレッチ再生
	　＿＿＿
	／　　　＼
	　　　　　＿＿＿
	　　　　／　　　＼
	|<----->|
	 window

	注意！samplesは必ず4の倍数であること
*/
/*
int StretchOscillatorExecute(Stretcher_t *ps, float **ppfDst, int samples)
{
	const int workSamples = 4;
	float *ppfDWrk[2] = {ppfDst[0], ppfDst[1]};
	int ret = 0;

	while (samples > 0)
	{
		uint32_t pitch = ps->Pitch;

		if (pitch)
		{	// 有効
			uint32_t window = ps->Window;
			uint32_t winCnt = ps->WindowCnt;

			if (winCnt >= window)
			{	// start or cross
				if (ps->xfader.fRate == 0.0f)
				{	// now fade stop !
					Oscillator_t * pOsc = ps->Osc1or2 ? &(ps->osc1) : &(ps->osc2);
					float fs = ps->psSampleData->FsAdjust;
					uint32_t endOfs = ps->psSampleData->EndOfs;
					uint32_t loopOfs = ps->psSampleData->LoopOfs;
					uint64_t delay = winCnt;	// 経過時間

					if ((ps->osc1.Pitch == 0) && (ps->osc2.Pitch == 0))
					{
						delay = 0;
					}
					else if (delay)
					{
						uint32_t tempoX100 = ps->psSampleData->TempoX100;

						delay = delay << OSCIMGBIT;
						if (fs != 1.0f)
						{	// Fs補正
							delay = (uint64_t)((double)delay * fs);
						}
						if ((tempoX100) && (ps->TempoX100 != tempoX100))
						{	// テンポ補正
							delay = delay * ps->TempoX100 / tempoX100;
						}
						delay += ps->CurImg;
						ps->CurOfs += delay >> OSCIMGBIT;
						ps->CurImg = delay & OSCIMGMSK;
						if (ps->CurOfs >= endOfs)
						{
#if 1	// ここではループしない->してみる！
							if (loopOfs)
							{	// loop
								while (ps->CurOfs >= endOfs)
								{
									ps->CurOfs -= loopOfs;
								}
								ret = 1;	// あとでクリアされる
							}
							else
#endif
							{	// one shot then end
								pitch = 0;
								ret = 1;
							}
						}
					}
					if (pitch)
					{
						(ps->oscSetup)(pOsc,
									   ps->psSampleData,
									   ps->psSampleData->SampleByte,
									   endOfs,
									   loopOfs,
									   pitch,
									   ps->CurOfs,
									   ps->CurImg,
									   0.0f,	// 後でセットされる
									   ps->fPan);

						ps->Osc1or2 ^= 1;	// 0->1,1->0
						ps->xfader.fCur = 0.0f;
						ps->xfader.fRate = ps->fFadeRate;

						if (ps->Osc1or2 & 1)
						{
							ps->Window = STRETCHWINDOW*3/4 + (STRETCHWINDOW/2 * ps->Rnd8 / 255);
						}
						else
						{
							ps->Window = STRETCHWINDOW*7/4 - (STRETCHWINDOW/2 * ps->Rnd8 / 255);
						}
						ps->Rnd8 = ps->Rnd8 * 5 + 1;
					}
					winCnt = 0;
				}
			}
			// next
			winCnt += workSamples;
			// fade
			if (ps->xfader.fRate != 0.0f)
			{	// fade running !
				float fLev1, fLev2;

				xfaderExecute(&ps->xfader);
				fLev1 = ps->xfader.fCur;
				fLev2 = 1.0f - fLev1;
				fLev1 *= ps->fLevel;
				fLev2 *= ps->fLevel;
				if (ps->Osc1or2)
				{	// now 2nd
					ps->osc2.fLevel = fLev1;
					ps->osc1.fLevel = fLev2;
				}
				else
				{	// now 1st
					ps->osc1.fLevel = fLev1;
					ps->osc2.fLevel = fLev2;
				}
				ps->osc1.fPan = ps->fPan;
				ps->osc2.fPan = ps->fPan;
			}
			else
			{	// fader stop !
				if (ps->Osc1or2)
				{	// now 2nd
					ps->osc2.fLevel = ps->fLevel;
					ps->osc2.fPan = ps->fPan;
				}
				else
				{	// now 1st
					ps->osc1.fLevel = ps->fLevel;
					ps->osc1.fPan = ps->fPan;
				}
			}
			// osc 1st
			if (ps->osc1.fLevel != 0.0f)
			{	// active !
				int flag = (ps->oscExec)(&(ps->osc1), ppfDWrk, workSamples);

				if (flag && (ps->Osc1or2 == 0))
				{	// 対象オシレータ、終端に達した
					ret |= flag;
				}
			}
			// osc 2nd
			if (ps->osc2.fLevel != 0.0f)
			{	// active !
				int flag = (ps->oscExec)(&(ps->osc2), ppfDWrk, workSamples);

				if (flag && (ps->Osc1or2 == 1))
				{	// 対象オシレータ、終端に達した
					ret |= flag;
				}
			}
			// end check
#if 1	// オシレータとしてループさせるならここを有効にするが、今回は呼び出し元でリセット->ここで！
			if (ret)
			{
				if (ps->psSampleData->LoopOfs)
				{	// ループサンプル
					ret = 0;
				}
				else
				{
					ps->Pitch = 0;
				}
			}
#endif
			ps->WindowCnt = winCnt;
		}
		else
		{
			ret = 1;
		}
		samples -= workSamples;
		ppfDWrk[0] = &(ppfDWrk[0][workSamples]);
		ppfDWrk[1] = &(ppfDWrk[1][workSamples]);
	}

	return ret;
}*/

/*
	ストレッチオシレーターセットアップ
*/
/*
void StretchOscillatorSetup(Stretcher_t *ps, SampleData_t *psSampleData)
{
	int type = psSampleData ? psSampleData->MonoStereo : 0;

	ps->psSampleData = psSampleData;
	ps->TempoX100	 = assigner::tempoX100;
	ps->Pitch		 = 0;
	ps->CurOfs		 = 0;
	ps->CurImg		 = 0;
	ps->Window		 = STRETCHWINDOW;
	ps->WindowCnt	 = STRETCHWINDOW;
	ps->Osc1or2		 = 0;
	ps->fLevel		 = 1.0f;
	ps->fPan		 = 0.5f;
	ps->fFadeRate	 = STRETCHCROSFDf;
	ps->xfader		 = {1.0f, 0.0f, 1.0f};
	ps->oscSetup	 = GetOscillatorSetupFunc(type);
	ps->oscExec		 = GetOscillatorExecFunc(type);
	ps->osc1.Pitch	 = 0;
	ps->osc2.Pitch	 = 0;

	return;
}*/

/*
	ストリームオシレーターセットアップ
*/
/*
void StreamOscillatorSetup(Streamer_t *ps, SampleData_t *psSampleData)
{
	int type = psSampleData ? psSampleData->MonoStereo : 0;

	ps->psSampleData = psSampleData;
	ps->oscSetup	 = GetOscillatorSetupFunc(type);
	ps->oscExec		 = GetOscillatorExecFunc(type);

	return;
}*/


/*
	オシレーター処理(ステレオストリームソース、ステレオアウト)
	return 1:終端に達した(One Shot の場合は、pitchをクリア)
*/
/*
static int OscStreamStereo(Oscillator_t *ps, float **ppfDst, int samples)
{
	uint32_t	pitch = ps->Pitch;
	float *pfDstL = ppfDst[0];
	float *pfDstR = ppfDst[1];
	int ret = 0;

	if (pitch)
	{	// 有効
		StreamData_t *pStreamData = (StreamData_t *)ps->DataPtr;

		if (pStreamData)
		{	// 有効
			uint32_t	curOfs = ps->CurOfs * 2;				// *2 for stereo
			uint32_t	curImg = ps->CurImg;
			uint32_t	endOfs = ps->EndOfs * 2;				// *2 for stereo
			uint32_t	loopOfs = ps->LoopOfs * 2;				// *2 for stereo
			uint32_t	pitchInt = (pitch >> OSCIMGBIT) * 2;	// *2 for stereo
			uint32_t	pitchImg = pitch & OSCIMGMSK;
			int32_t		sb = ps->SampleByte;
			float		fPanL = ps->fLevel * (ps->fPan > 0.5f ? 1.0f - ps->fPan : 0.5f) * 2.0f;
			float		fPanR = ps->fLevel * (ps->fPan > 0.5f ? 0.5f : ps->fPan) * 2.0f;

			for (int i = 0; i < samples; i++)
			{
				int32_t dt[4];
				int32_t dt1L, dt1R, dt2L,dt2R;
				float	fDtL, fDtR;

				// データ取得
				if (curOfs < endOfs)
				{
					getStreamSample(pStreamData, curOfs, sb, dt, 4);
				}
				else
				{
					getStreamSample(pStreamData, curOfs, sb, dt, 2);
					getStreamSample(pStreamData, curOfs-loopOfs, sb, &dt[2], 2);
				}
				dt1L = dt[0];
				dt1R = dt[1];
				dt2L = dt[2];
				dt2R = dt[3];

				// データ生成
				fDtL = dt2L - dt1L;
				fDtR = dt2R - dt1R;
				fDtL *= curImg;
				fDtR *= curImg;
				fDtL /= OSCEQPITCH;
				fDtR /= OSCEQPITCH;
				fDtL += dt1L;
				fDtR += dt1R;
				fDtL /= 8388608.0f;
				fDtR /= 8388608.0f;
				*pfDstL++ += fDtL * fPanL;
				*pfDstR++ += fDtR * fPanR;

				// アドレス更新
				curOfs += pitchInt;
				curImg += pitchImg;
				if (curImg >= OSCEQPITCH)
				{
					curImg -= OSCEQPITCH;
					curOfs += 2;								// +2 for stereo
				}

				// 終了チェック
				if (curOfs >= endOfs)
				{
					ret = 1;
					if (loopOfs)
					{	// loop
						curOfs -= loopOfs;
					}
					else
					{	// one shot
						ps->Pitch = 0;	// 停止
						break;
					}
				}
			}
			ps->CurOfs = curOfs / 2;							// /2 for stereo
			ps->CurImg = curImg;
		}
	}
	else
	{
		ret = 1;
	}

	return ret;
}*/

/*
	オシレーター処理(モノラルストリームソース、ステレオアウト)
	return 1:終端に達した(One Shot の場合は、pitchをクリア)
*/
/*
static int OscStreamMono(Oscillator_t *ps, float **ppfDst, int samples)
{
	uint32_t	pitch = ps->Pitch;
	float *pfDstL = ppfDst[0];
	float *pfDstR = ppfDst[1];
	int ret = 0;

	if (pitch)
	{	// 有効
		StreamData_t *pStreamData = (StreamData_t *)ps->DataPtr;

		if (pStreamData)
		{	// 有効
			uint32_t	curOfs = ps->CurOfs;
			uint32_t	curImg = ps->CurImg;
			uint32_t	endOfs = ps->EndOfs;
			uint32_t	loopOfs = ps->LoopOfs;
			uint32_t	pitchInt = (pitch >> OSCIMGBIT);
			uint32_t	pitchImg = pitch & OSCIMGMSK;
			int32_t		sb = ps->SampleByte;
			float		fPanL = ps->fLevel * (ps->fPan > 0.5f ? 1.0f - ps->fPan : 0.5f) * 2.0f;
			float		fPanR = ps->fLevel * (ps->fPan > 0.5f ? 0.5f : ps->fPan) * 2.0f;

			for (int i = 0; i < samples; i++)
			{
				int32_t dt[2];
				int32_t dt1, dt2;
				float	fDt;

				// データ取得
				if (curOfs < endOfs)
				{
					getStreamSample(pStreamData, curOfs, sb, dt, 2);
				}
				else
				{
					getStreamSample(pStreamData, curOfs, sb, dt, 1);
					getStreamSample(pStreamData, curOfs-loopOfs, sb, &dt[1], 1);
				}
				dt1 = dt[0];
				dt2 = dt[1];

				// データ生成
				fDt = dt2 - dt1;
				fDt *= curImg;
				fDt /= OSCEQPITCH;
				fDt += dt1;
				fDt /= 8388608.0f;
				*pfDstL++ += fDt * fPanL;
				*pfDstR++ += fDt * fPanR;

				// アドレス更新
				curOfs += pitchInt;
				curImg += pitchImg;
				if (curImg >= OSCEQPITCH)
				{
					curImg -= OSCEQPITCH;
					curOfs++;
				}

				// 終了チェック
				if (curOfs >= endOfs)
				{
					ret = 1;
					if (loopOfs)
					{	// loop
						curOfs -= loopOfs;
					}
					else
					{	// one shot
						ps->Pitch = 0;	// 停止
						break;
					}
				}
			}
			ps->CurOfs = curOfs;
			ps->CurImg = curImg;
		}
	}
	else
	{
		ret = 1;
	}

	return ret;
}*/

/*
	オシレーター処理(フロートステレオストリームソース、ステレオアウト)
	return 1:終端に達した(One Shot の場合は、pitchをクリア)
*/
/*
static int OscStreamStereoF(Oscillator_t *ps, float **ppfDst, int samples)
{
	uint32_t	pitch = ps->Pitch;
	float *pfDstL = ppfDst[0];
	float *pfDstR = ppfDst[1];
	int ret = 0;

	if (pitch)
	{	// 有効
		StreamData_t *pStreamData = (StreamData_t *)ps->DataPtr;

		if (pStreamData)
		{	// 有効
			uint32_t	curOfs = ps->CurOfs * 2;				// *2 for stereo
			uint32_t	curImg = ps->CurImg;
			uint32_t	endOfs = ps->EndOfs * 2;				// *2 for stereo
			uint32_t	loopOfs = ps->LoopOfs * 2;				// *2 for stereo
			uint32_t	pitchInt = (pitch >> OSCIMGBIT) * 2;	// *2 for stereo
			uint32_t	pitchImg = pitch & OSCIMGMSK;
			int32_t		sb = ps->SampleByte;
			float		fPanL = ps->fLevel * (ps->fPan > 0.5f ? 1.0f - ps->fPan : 0.5f) * 2.0f;
			float		fPanR = ps->fLevel * (ps->fPan > 0.5f ? 0.5f : ps->fPan) * 2.0f;

			for (int i = 0; i < samples; i++)
			{
				float dt[4];
				float dt1L, dt1R, dt2L,dt2R;
				float fDtL, fDtR;
				float mulImg = (float)curImg / (float)OSCEQPITCH;

				// データ取得
				if (curOfs < endOfs)
				{
					getStreamSample(pStreamData, curOfs, sb, (int32_t *)dt, 4);
				}
				else
				{
					getStreamSample(pStreamData, curOfs, sb, (int32_t *)dt, 2);
					getStreamSample(pStreamData, curOfs-loopOfs, sb, (int32_t *)&dt[2], 2);
				}
				dt1L = dt[0];
				dt1R = dt[1];
				dt2L = dt[2];
				dt2R = dt[3];

				// データ生成
				fDtL = dt2L - dt1L;
				fDtR = dt2R - dt1R;
				fDtL *= mulImg;
				fDtR *= mulImg;
				fDtL += dt1L;
				fDtR += dt1R;
				*pfDstL++ += fDtL * fPanL;
				*pfDstR++ += fDtR * fPanR;

				// アドレス更新
				curOfs += pitchInt;
				curImg += pitchImg;
				if (curImg >= OSCEQPITCH)
				{
					curImg -= OSCEQPITCH;
					curOfs += 2;								// +2 for stereo
				}

				// 終了チェック
				if (curOfs >= endOfs)
				{
					ret = 1;
					if (loopOfs)
					{	// loop
						curOfs -= loopOfs;
					}
					else
					{	// one shot
						ps->Pitch = 0;	// 停止
						break;
					}
				}
			}
			ps->CurOfs = curOfs / 2;							// /2 for stereo
			ps->CurImg = curImg;
		}
	}
	else
	{
		ret = 1;
	}

	return ret;
}
*/

/*
	オシレーター処理(フロートモノラルストリームソース、ステレオアウト)
	return 1:終端に達した(One Shot の場合は、pitchをクリア)
*/
/*
static int OscStreamMonoF(Oscillator_t *ps, float **ppfDst, int samples)
{
	uint32_t	pitch = ps->Pitch;
	float *pfDstL = ppfDst[0];
	float *pfDstR = ppfDst[1];
	int ret = 0;

	if (pitch)
	{	// 有効
		StreamData_t *pStreamData = (StreamData_t *)ps->DataPtr;

		if (pStreamData)
		{	// 有効
			uint32_t	curOfs = ps->CurOfs;
			uint32_t	curImg = ps->CurImg;
			uint32_t	endOfs = ps->EndOfs;
			uint32_t	loopOfs = ps->LoopOfs;
			uint32_t	pitchInt = (pitch >> OSCIMGBIT);
			uint32_t	pitchImg = pitch & OSCIMGMSK;
			int32_t		sb = ps->SampleByte;
			float		fPanL = ps->fLevel * (ps->fPan > 0.5f ? 1.0f - ps->fPan : 0.5f) * 2.0f;
			float		fPanR = ps->fLevel * (ps->fPan > 0.5f ? 0.5f : ps->fPan) * 2.0f;

			for (int i = 0; i < samples; i++)
			{
				float dt[2];
				float dt1, dt2;
				float fDt;
				float mulImg = (float)curImg / (float)OSCEQPITCH;

				// データ取得
				if (curOfs < endOfs)
				{
					getStreamSample(pStreamData, curOfs, sb, (int32_t *)dt, 2);
				}
				else
				{
					getStreamSample(pStreamData, curOfs, sb, (int32_t *)dt, 1);
					getStreamSample(pStreamData, curOfs-loopOfs, sb, (int32_t *)&dt[1], 1);
				}
				dt1 = dt[0];
				dt2 = dt[1];

				// データ生成
				fDt = dt2 - dt1;
				fDt *= mulImg;
				fDt += dt1;
				*pfDstL++ += fDt * fPanL;
				*pfDstR++ += fDt * fPanR;

				// アドレス更新
				curOfs += pitchInt;
				curImg += pitchImg;
				if (curImg >= OSCEQPITCH)
				{
					curImg -= OSCEQPITCH;
					curOfs++;
				}

				// 終了チェック
				if (curOfs >= endOfs)
				{
					ret = 1;
					if (loopOfs)
					{	// loop
						curOfs -= loopOfs;
					}
					else
					{	// one shot
						ps->Pitch = 0;	// 停止
						break;
					}
				}
			}
			ps->CurOfs = curOfs;
			ps->CurImg = curImg;
		}
	}
	else
	{
		ret = 1;
	}

	return ret;
}*/

/*
	オシレータセットアップ（ストリーム）
*/
/*
static void oscStreamSetup(Oscillator_t *ps, SampleData_t *, int sb, uint32_t eof, uint32_t lof, uint32_t pit, uint32_t cof, uint32_t cim, float flv, float fpn)
{
	ps->SampleByte = sb;
	ps->EndOfs	= eof;
	ps->LoopOfs	= lof;
	ps->Pitch	= pit;
	ps->CurOfs	= cof;
	ps->CurImg	= cim;
	ps->fLevel	= flv;
	ps->fPan	= fpn;

	return;
}*/
