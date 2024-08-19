/*
 * param.c
 *
 *  Created on: 2020/02/06
 *      Author: higuchi
 */

#include	<string.h>
//#include	"edit.h"
//#include	"cpufx.h"
#include	"param.h"
//#include	"ad.h"

#if 0	//#include	"cpufx.h"
uint16_t LimitVal(int16_t val, uint16_t min, uint16_t max, bool fLoop)
{
	if(min <= max) {
		if(val < min) {
			val = (fLoop ? max : min);
		} else if(val > max) {
			val = (fLoop ? min : max);
		}
	} else {
		if(val < max) {
			val = (fLoop ? min : max);
		} else if(val > min) {
			val = (fLoop ? max : min);
		}
	}
	return(val);
}
#endif

//------------------------------------------------------------------------------
//	variables
//------------------------------------------------------------------------------

fbuf_t editflags;
editbuf_t editbuf;
globbuf_t globalbuf;

//------------------------------------------------------------------------------
// parameter structure
//------------------------------------------------------------------------------

// parameter structure type
//#define	PT_NCMP	B(00000001)	// 比較無し(常にspvchg_fg=true)
//#define	PT_WORD	B(00000010)	// WORDサイズパラメータ(PT_SFNC = trueのときのみ使用可)
//#define	PT_MWRT	B(00000100)	// Manual時は自動保存

// typedef parameter structure
//typedef struct {
//	uint8_t	type;
//	uint8_t	min, max;
//	void	(*getRangeFunc)(uint16_t*, uint16_t*);
//	uint16_t	(*calcParamFunc)(uint8_t*, uint8_t, uint16_t, uint16_t);
//	void	(*func)(void);
//} PARAMSTRC;

//extern const PARAMSTRC* ParamStrcTbl[];
//extern const uint8_t pid2ofs_tbl[];

//------------------------------------------------------------------------------
//	functions
//------------------------------------------------------------------------------
//uint16_t normalize7(uint8_t val, uint16_t min, uint16_t max);

//------------------------------------------------------------------------------
//	Parameter Change Proc
//------------------------------------------------------------------------------

bool ParamChangeProc(uint8_t paramid, uint8_t mode, int16_t value)
{
#if 1	//#include	"cpufx.h"
	return true;
#else
	// Parameterをeditbufにセット
	if(SetParam(paramid, mode, value)) {
		// Parameter変化に伴う処理を実行
		ParamStrcTbl[paramid]->func();
		if(ParamStrcTbl[paramid]->type & PT_MWRT) {
			BackUpEditParam(paramid);
		}
		return(true);
	} else {
		return(false);
	}
#endif
}

//------------------------------------------------------------------------------
//  Set Edit Parameter
//
//	ret	変化あり/なしフラグ(=1 : 変化あり)
//------------------------------------------------------------------------------

bool SetParam(uint8_t paramid, uint8_t mode, int16_t value)
{
#if 1	//#include	"cpufx.h"
	return true;
#else
	PARAMSTRC	*pstrc;
	uint8_t	type, *pparam;
	uint16_t	spvold, spvnew, spvmin, spvmax;
	bool	spvchg_fg;

	// parameter structure取得
	pstrc = (PARAMSTRC*)ParamStrcTbl[paramid];
	if(pstrc == NULL) {
		return(false);
	}
	// parameter type取得
	type = pstrc->type;
	// parameter address取得
	pparam = (uint8_t*)&editbuf + pid2ofs_tbl[paramid];
	// 現在のパラメータ値を退避
	if(type & PT_WORD) {
		spvold = RD16(pparam);
	} else {
		spvold = *pparam;
	}

	// parameter範囲取得
	if(pstrc->getRangeFunc != NULL) {
		pstrc->getRangeFunc(&spvmin, &spvmax);
	} else {
		spvmin = pstrc->min;
		spvmax = pstrc->max;
	}

	// 新parameter値取得
	spvnew = spvold;
	if(pstrc->calcParamFunc != NULL) {
		// 特殊関数で処理
		spvnew = pstrc->calcParamFunc(&mode, value, spvmin, spvmax);
	}
	if (mode != 0xFF) {
		// 特殊関数で処理しなかった場合
		if(mode == PMD_DIRECT) {
			// valueを直接保存
			spvnew = LimitVal(value, spvmin, spvmax, false);
		} else if(mode == PMD_ADDCLIP) {
			// add (clip to min or max)
			value += spvold;
			spvnew = LimitVal(value, spvmin, spvmax, false);
		} else if(mode == PMD_ADDLOOP) {
			// add & loop
			value += spvold;
			spvnew = LimitVal(value, spvmin, spvmax, true);
		} else if(mode == PMD_MS2BPM) {
			// msをbpmに変換
			value = 60000 / value;
			spvnew = LimitVal(value, spvmin, spvmax, false);
		} else {
			// その他はknob値をパラメータに変換して保存
			spvnew = normalize7((uint8_t)value, spvmin, spvmax);
		}
	}

	// 変化あり/なしフラグ
	if(type & PT_NCMP) {
		// 比較無し(常時変化ありとして処理)
		spvchg_fg = true;
	} else {
		// 旧値と比較
		spvchg_fg = (spvold != spvnew);
	}

	// param保存
	if(type & PT_WORD) {
		WR16(pparam, spvnew);
	} else {
		*pparam = (uint8_t)spvnew;
	}

	return(spvchg_fg);
#endif
}

//------------------------------------------------------------------------------
//  Get Edit Parameter
//------------------------------------------------------------------------------

uint16_t GetParam(uint8_t paramid)
{
#if 1	//#include	"cpufx.h"
	return 0;
#else
	PARAMSTRC	*pstrc;
	uint8_t	type, *pparam;
	uint16_t	ret;

	ret = 0;
	// parameter structure取得
	pstrc = (PARAMSTRC*)ParamStrcTbl[paramid];
	if(pstrc != NULL) {
		// parameter type取得
		type = pstrc->type;
		// parameter address取得
		pparam = (uint8_t*)&editbuf + pid2ofs_tbl[paramid];
		// 現在のパラメータ値を取得
		if(type & PT_WORD) {
			ret = RD16(pparam);
		} else {
			ret = (uint16_t)*pparam;
		}
	}
	return(ret);
#endif
}

//------------------------------------------------------------------------------
//  Get Edit Parameter Range
//------------------------------------------------------------------------------

void GetParamRange(uint8_t paramid, uint16_t *min, uint16_t *max)
{
#if 0	//#include	"cpufx.h"
	PARAMSTRC	*pstrc;
	uint16_t	pmin, pmax;

	// parameter structure取得
	pstrc = (PARAMSTRC*)ParamStrcTbl[paramid];
	if(pstrc == NULL) {
		return;
	}

	// parameter範囲取得
	if(pstrc->getRangeFunc != NULL) {
		pstrc->getRangeFunc(&pmin, &pmax);
	} else {
		pmin = pstrc->min;
		pmax = pstrc->max;
	}

	if(min != NULL) {
		*min = pmin;
	}
	if(max != NULL) {
		*max = pmax;
	}
#endif
}

//------------------------------------------------------------------------------
//	get fx type/param range
//------------------------------------------------------------------------------

#if 0	//#include	"cpufx.h"
static void getRange_f1t(uint16_t *min, uint16_t *max) { FxGetTypeRange(FXID_FX1, min, max); }
static void getRange_f1v1(uint16_t *min, uint16_t *max) { FxGetValRange(FXID_FX1, editbuf.fx[FXID_FX1].type, 0, min, max); }
static void getRange_f1v2(uint16_t *min, uint16_t *max) { FxGetValRange(FXID_FX1, editbuf.fx[FXID_FX1].type, 1, min, max); }
static void getRange_f2t(uint16_t *min, uint16_t *max) { FxGetTypeRange(FXID_FX2, min, max); }
static void getRange_f2v1(uint16_t *min, uint16_t *max) { FxGetValRange(FXID_FX2, editbuf.fx[FXID_FX2].type, 0, min, max); }
static void getRange_f2v2(uint16_t *min, uint16_t *max) { FxGetValRange(FXID_FX2, editbuf.fx[FXID_FX2].type, 1, min, max); }
static void getRange_f3t(uint16_t *min, uint16_t *max) { FxGetTypeRange(FXID_FX3, min, max); }
static void getRange_f3v1(uint16_t *min, uint16_t *max) { FxGetValRange(FXID_FX3, editbuf.fx[FXID_FX3].type, 0, min, max); }
static void getRange_f3v2(uint16_t *min, uint16_t *max) { FxGetValRange(FXID_FX3, editbuf.fx[FXID_FX3].type, 1, min, max); }
#endif

//------------------------------------------------------------------------------
//	calc fx param value
//------------------------------------------------------------------------------

#if 0	//#include	"cpufx.h"
static uint16_t calcParam_fv(uint8_t fxid, uint8_t valno, uint8_t *mode, uint8_t val, uint16_t min, uint16_t max)
{
	uint16_t retval;

	retval = val;
	if (*mode == PMD_NORM7) {
		if (!FxCalcValue(fxid, editbuf.fx[fxid].type, valno, &retval, min, max)) {
			retval = normalize7(val, min, max);
		}
		*mode = 0xFF;
	}
	return(retval);

}

static uint16_t calcParam_f1v1(uint8_t *mode, uint8_t val, uint16_t min, uint16_t max) { return(calcParam_fv(FXID_FX1, 0, mode, val, min, max)); }
static uint16_t calcParam_f1v2(uint8_t *mode, uint8_t val, uint16_t min, uint16_t max) { return(calcParam_fv(FXID_FX1, 1, mode, val, min, max)); }

static uint16_t calcParam_f2v1(uint8_t *mode, uint8_t val, uint16_t min, uint16_t max) { return(calcParam_fv(FXID_FX2, 0, mode, val, min, max)); }
static uint16_t calcParam_f2v2(uint8_t *mode, uint8_t val, uint16_t min, uint16_t max) { return(calcParam_fv(FXID_FX2, 1, mode, val, min, max)); }

static uint16_t calcParam_f3v1(uint8_t *mode, uint8_t val, uint16_t min, uint16_t max) { return(calcParam_fv(FXID_FX3, 0, mode, val, min, max)); }
static uint16_t calcParam_f3v2(uint8_t *mode, uint8_t val, uint16_t min, uint16_t max) { return(calcParam_fv(FXID_FX3, 1, mode, val, min, max)); }

#endif

//------------------------------------------------------------------------------
//	エディットパラメータをバックアップ/リストア
//------------------------------------------------------------------------------

void BackUpEditParam(uint8_t paramid)
{
//	uint8_t *dst, *src;
//	int ofs;
//	if ((paramid >= PID_FX_TOP) && (paramid <= PID_FX_END)) {
//		ofs = pid2ofs_tbl[paramid] - pid2ofs_tbl[PID_FX_TOP];
//		src = (uint8_t*)&editbuf.fx + ofs;
//		dst = (uint8_t*)&globalbuf.fx + ofs;
//		*dst = *src;
//		RequestToStoreGlobalData(GLBOFS_FX + ofs);
//		if (ParamStrcTbl[paramid]->type & PT_WORD) {
//			*(dst + 1) = *(src + 1);
//			RequestToStoreGlobalData(GLBOFS_FX + ofs + 1);
//		}
//	}
}

extern const globbuf_t globalbuf_ini;

void RestoreEditParam()
{
	int i;
//	editbuf.amptype = GetKnobParam(globalbuf.chno == 0 ? CKNBID_AMP1 : CKNBID_AMP2, NULL);
	for (i = 0; i < FXID_NUM; i++) {
		//CopyMemory((uint8_t*)&editbuf.fx[i], (uint8_t*)&globalbuf.fx[i], sizeof(fxsetting_t));
		//CopyMemory((uint8_t*)&editbuf.fx[i], (uint8_t*)&globalbuf_ini.fx[i], sizeof(fxsetting_t));
		memcpy((uint8_t*)&editbuf.fx[i], (uint8_t*)&globalbuf_ini.fx[i], sizeof(fxsetting_t));
	}
}

//------------------------------------------------------------------------------
//	editbuf, globalbufの正当性チェック
//------------------------------------------------------------------------------

void CheckEditbuf(void)
{
	uint8_t	pid;

	for (pid = 0; pid < NUM_PID; pid++) {
		// 範囲チェックして、範囲外なら修正
		SetParam(pid, PMD_DIRECT, GetParam(pid));
	}
}

void CheckGlobalbuf(void)
{
//	if (!HC_ModelGetAutoOffEna()) {
//		globalbuf.autooff = false;
//	}
//#if 1
//	if (globalbuf.chno >= NUM_CH) {
//		globalbuf.chno = GetInitialGlobalData(GLBOFS_CHNO);
//	}
//#else
//	if (HC_ModelGetId() == MODEL_ID_15) {
//		globalbuf.chno = 0;
//	} else if (globalbuf.chno >= NUM_CH) {
//		globalbuf.chno = GetInitialGlobalData(GLBOFS_CHNO);
//	}
//#endif

	#ifdef KNOB_CALIB_ENA
		CheckKnobRangeCalib();
	#endif

	#ifdef KAZA_CALIB_ENA
	// 風車ノブチェック/修正
		CheckKazaBorderCalib();
	#endif

}

//------------------------------------------------------------------------------
//	7bitデータ(0x00-0x7F)からパラメータ値を作る
//	in	val
//		param min
//		param max
//	ret	param
//------------------------------------------------------------------------------
#if 0	//#include	"cpufx.h"
uint16_t normalize7(uint8_t val, uint16_t min, uint16_t max)
{
	uint16_t ret;

	if(val == 0) {
		ret = min;
	} else if(val == 127) {
		ret = max;
	} else {
		if(min <= max) {
			ret = min + 1 + (uint16_t)((uint32_t)(max-min-1) * val / 128);
		} else {
			ret = min - 1 - (uint16_t)((uint32_t)(min-max-1) * val / 128);
		}
	}
	return(ret);
}
#endif

//------------------------------------------------------------------------------
//	InitFxValule
//	Fx Typeに応じて FX Valueを初期化する
//------------------------------------------------------------------------------

void InitFxValue(void)
{
//	// 仮で初期化
//	// → 本当は、Flash から読み込む
//	editbuf.fx[FXID_FX1].val1 = FxGetInitVal(FXID_FX1, editbuf.fx[FXID_FX1].type, 0);
//	editbuf.fx[FXID_FX1].val2 = FxGetInitVal(FXID_FX1, editbuf.fx[FXID_FX1].type, 1);
//	editbuf.fx[FXID_FX2].val1 = FxGetInitVal(FXID_FX2, editbuf.fx[FXID_FX2].type, 0);
//	editbuf.fx[FXID_FX2].val2 = FxGetInitVal(FXID_FX2, editbuf.fx[FXID_FX2].type, 1);
//	editbuf.fx[FXID_FX3].val1 = FxGetInitVal(FXID_FX3, editbuf.fx[FXID_FX3].type, 0);
//	editbuf.fx[FXID_FX3].val2 = FxGetInitVal(FXID_FX3, editbuf.fx[FXID_FX3].type, 1);
}

//------------------------------------------------------------------------------
//	Parameter Structure
//------------------------------------------------------------------------------

#if 0	//#include	"cpufx.h"
const PARAMSTRC pstrc_amptype = {	// PID_AMPTYPE
	PT_NCMP,			// flags
	0, 7,				// min, max
	NULL,				// special function to get range
	NULL,				// special function to calc param
	EditAmpSelector		// function
};

const PARAMSTRC pstrc_fx1onoff = {	// PID_FX1ONOFF
	PT_NCMP|PT_MWRT,	// flags
	0, 1,				// min, max
	NULL,				// special function to get range
	NULL,				// special function to calc param
	EditFx1OnOff		// function
};
const PARAMSTRC pstrc_fx1type = {	// PID_FX1TYPE
	PT_NCMP|PT_MWRT,	// flags
	0, 0,				// min, max
	getRange_f1t,		// special function to get range
	NULL,				// special function to calc param
	EditFx1Selector		// function
};
const PARAMSTRC pstrc_fx1val1 = {	// PID_FX1VAL1
	PT_MWRT|PT_WORD,	// flags
	0, 0,				// min, max
	getRange_f1v1,		// special function to get range
	calcParam_f1v1,		// special function to calc param
	EditFx1Val1			// function
};
const PARAMSTRC pstrc_fx1val2 = {	// PID_FX1VAL2
	PT_MWRT|PT_WORD,	// flags
	0, 0,				// min, max
	getRange_f1v2,		// special function to get range
	calcParam_f1v2,		// special function to calc param
	EditFx1Val2			// function
};

const PARAMSTRC pstrc_fx2onoff = {	// PID_FX2ONOFF
	PT_NCMP|PT_MWRT,	// flags
	0, 1,				// min, max
	NULL,				// special function to get range
	NULL,				// special function to calc param
	EditFx2OnOff		// function
};
const PARAMSTRC pstrc_fx2type = {	// PID_FX2TYPE
	PT_NCMP|PT_MWRT,	// flags
	0, 0,				// min, max
	getRange_f2t,		// special function to get range
	NULL,				// special function to calc param
	EditFx2Selector		// function
};
const PARAMSTRC pstrc_fx2val1 = {	// PID_FX2VAL1
	PT_MWRT|PT_WORD,	// flags
	0, 0,				// min, max
	getRange_f2v1,		// special function to get range
	calcParam_f2v1,		// special function to calc param
	EditFx2Val1			// function
};
const PARAMSTRC pstrc_fx2val2 = {	// PID_FX2VAL2
	PT_MWRT|PT_WORD,	// flags
	0, 0,				// min, max
	getRange_f2v2,		// special function to get range
	calcParam_f2v2,		// special function to calc param
	EditFx2Val2			// function
};

const PARAMSTRC pstrc_fx3onoff = {	// PID_FX3ONOFF
	PT_NCMP|PT_MWRT,	// flags
	0, 1,				// min, max
	NULL,				// special function to get range
	NULL,				// special function to calc param
	EditFx3OnOff		// function
};
const PARAMSTRC pstrc_fx3type = {	// PID_FX3TYPE
	PT_NCMP|PT_MWRT,	// flags
	0, 0,				// min, max
	getRange_f3t,		// special function to get range
	NULL,				// special function to calc param
	EditFx3Selector		// function
};
const PARAMSTRC pstrc_fx3val1 = {	// PID_FX3VAL1
	PT_MWRT|PT_WORD,	// flags
	0, 0,				// min, max
	getRange_f3v1,		// special function to get range
	calcParam_f3v1,		// special function to calc param
	EditFx3Val1			// function
};
const PARAMSTRC pstrc_fx3val2 = {	// PID_FX3VAL2
	PT_MWRT|PT_WORD,	// flags
	0, 0,				// min, max
	getRange_f3v2,		// special function to get range
	calcParam_f3v2,		// special function to calc param
	EditFx3Val2			// function
};


const PARAMSTRC* ParamStrcTbl[] = {
	&pstrc_amptype,	// PID_AMPTYPE
	&pstrc_fx1onoff,	// PID_FX1ONOFF
	&pstrc_fx1type,	// PID_FX1TYPE
	&pstrc_fx1val1,	// PID_FX1VAL1
	&pstrc_fx1val2,	// PID_FX1VAL2
	&pstrc_fx2onoff,	// PID_FX2ONOFF
	&pstrc_fx2type,	// PID_FX2TYPE
	&pstrc_fx2val1,	// PID_FX2VAL1
	&pstrc_fx2val2,	// PID_FX2VAL2
	&pstrc_fx3onoff,	// PID_FX3ONOFF
	&pstrc_fx3type,	// PID_FX3TYPE
	&pstrc_fx3val1,	// PID_FX3VAL1
	&pstrc_fx3val2,	// PID_FX3VAL2
};

const uint8_t pid2ofs_tbl[] = {
	offsetof(editbuf_t, amptype),
	offsetof(editbuf_t, fx[FXID_FX1].onoff),
	offsetof(editbuf_t, fx[FXID_FX1].type),
	offsetof(editbuf_t, fx[FXID_FX1].val1),
	offsetof(editbuf_t, fx[FXID_FX1].val2),
	offsetof(editbuf_t, fx[FXID_FX2].onoff),
	offsetof(editbuf_t, fx[FXID_FX2].type),
	offsetof(editbuf_t, fx[FXID_FX2].val1),
	offsetof(editbuf_t, fx[FXID_FX2].val2),
	offsetof(editbuf_t, fx[FXID_FX3].onoff),
	offsetof(editbuf_t, fx[FXID_FX3].type),
	offsetof(editbuf_t, fx[FXID_FX3].val1),
	offsetof(editbuf_t, fx[FXID_FX3].val2),
};
#endif

//---------------------------------------------------------------------
// Initial Global Data
//---------------------------------------------------------------------

const globbuf_t globalbuf_ini = {
	0,	// dummy
#if TEST_VXII_SYSEX
	0,
	0,
#endif	// #if TEST_VXII_SYSEX
	0,		// dumyy
	{		// fx
		{ 1, 0, 100, 300 }, // fx1 onoff, type, val1, val2
		{ 1, 0,  85,1000 }, // fx2 onoff, type, val1, val2
		{ 1, 0, 100,  65 }, // fx3 onoff, type, val1, val2
	},
	0,
#ifdef KNOB_CALIB_ENA
	{{HC_VK_MIN_LIM, HC_VK_MAX_LIM}},
#endif
};

uint8_t GetInitialGlobalData(uint8_t globalno)
{
	return(((uint8_t*)&globalbuf_ini)[globalno]);
}

