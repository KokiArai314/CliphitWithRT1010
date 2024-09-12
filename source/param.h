/*
 * param.h
 *
 *  Created on: 2020/02/06
 *      Author: higuchi
 */

#ifndef PARAM_H_
#define PARAM_H_

#include	"base_types.h"
//#include	"cpufx.h"

#include	"midi/midi_device.h"	// #define TEST_VXII_SYSEX 1

//------------------------------------------------------------------------------
//	Spec
//------------------------------------------------------------------------------

#define	NUM_CH		2	/* X-15705-8 : CH1,CH2 */
/* X-15705-8 : AD_SW_CH (CH1, CH2, FootSW) */
enum adSwChannelNumber {
	ADSWCHNUM_CHANNEL1,
	ADSWCHNUM_CHANNEL2,
	ADSWCHNUM_FOOTSW,

	ADSWCHNUM_MAX
};

//#define	KNOB_CALIB_ENA
//#define	KAZA_CALIB_ENA

//------------------------------------------------------------------------------
//	Parameter ID
//------------------------------------------------------------------------------

enum {

	PID_AMPTYPE,

	PID_FX_TOP,

	PID_FX1ONOFF = PID_FX_TOP,
	PID_FX1TYPE,
	PID_FX1VAL1,
	PID_FX1VAL2,

	PID_FX2ONOFF,
	PID_FX2TYPE,
	PID_FX2VAL1,
	PID_FX2VAL2,

	PID_FX3ONOFF,
	PID_FX3TYPE,
	PID_FX3VAL1,
	PID_FX3VAL2,

	PID_FX_END = PID_FX3VAL2,

	NUM_PID,

	// Parameter ID (not for editbuf)
	PID_FXVALUE

};


//------------------------------------------------------------------------------
//	Edit Buffer, Global Parameter Buffer
//------------------------------------------------------------------------------

#if 1	/// @note cpufx.h省略系
enum {
	FXID_FX1,
	FXID_FX2,
	FXID_FX3,

	FXID_NUM
};
typedef struct {
	bool	onoff;
	uint8_t		type;
	uint16_t	val1;
	uint16_t	val2;
} fxsetting_t;
#endif

typedef struct {
	uint8_t	amptype;
	fxsetting_t	fx[FXID_NUM];
} editbuf_t;

#define	EDITBUF_SIZE	sizeof(editbuf_t)

extern editbuf_t editbuf;


typedef struct {
// for user
	uint8_t	autooff;
#if TEST_VXII_SYSEX
	uint8_t	modeid;
	uint8_t	progno;
#endif	// #if TEST_VXII_SYSEX
	uint8_t	chno;
	fxsetting_t	fx[FXID_NUM];
// for system
	uint8_t	temp;
#ifdef KNOB_CALIB_ENA
	range16_t	knobrng[1];	// 1 : Knob数
#endif
} globbuf_t;

#define	GLOBALBUF_SIZE	sizeof(globbuf_t)

extern globbuf_t globalbuf;

#define	GLBOFS_AUTOOFF		offsetof(globbuf_t, autooff)
#if TEST_VXII_SYSEX
#define	GLBOFS_MODEID		offsetof(globbuf_t, modeid)
#define	GLBOFS_PROGNO		offsetof(globbuf_t, progno)
#endif	// #if TEST_VXII_SYSEX
#define	GLBOFS_CHNO			offsetof(globbuf_t, chno)
#define	GLBOFS_FX			offsetof(globbuf_t, fx)
#define	GLBOFS_TEMP			offsetof(globbuf_t, temp)
#ifdef KNOB_CALIB_ENA
#define	GLBOFS_KNOB_RNG		offsetof(globbuf_t, knobrng)
#endif

#define	GLBOFS_SYSTEM	GLBOFS_TEMP

//#define	GLOBALBUF_OFS_MODE	0
//#define	GLOBALBUF_OFS_FXONOFF	1
//#define	GLOBALBUF_OFS_FXTYPE	2
//#define	GLOBALBUF_OFS_FXVAL1	3
//#define	GLOBALBUF_OFS_FXVAL2	4	// 2 byte Parameter
////
//#define	GLOBALBUF_OFS_DACADJ	6
//#define	GLOBALBUF_OFS_FSADJ	7
//#define	GLOBALBUF_OFS_FXGROUP	8
//#define	GLOBALBUF_OFS_FXVARI1	9
//#define	GLOBALBUF_OFS_FXVARI2	10
//#define	GLOBALBUF_OFS_FXVARI3	11

//**************************************************************************
//	for param.c
//**************************************************************************

//------------------------------------------------------------------------------
//	Param Change Mode for SetParam/ParamChangeProc
//------------------------------------------------------------------------------

#define	PMD_DIRECT	0	//　直値
#define	PMD_NORM7	1	// 0x00-0x7Fをmin-maxに正規化
#define	PMD_ADDCLIP	2	// add (clip to min or max)
#define	PMD_ADDLOOP	3	// add & loop

#define	PMD_MS2BPM	4	// msをbpmに変換

//------------------------------------------------------------------------------
//	function declaration
//------------------------------------------------------------------------------

bool ParamChangeProc(uint8_t paramid, uint8_t mode, int16_t value);
bool SetParam(uint8_t paramid, uint8_t mode, int16_t value);

void GetParamRange(uint8_t paramid, uint16_t *min, uint16_t *max);

void RestoreEditParam(void);
void BackUpEditParam(uint8_t paramid);

uint8_t GetInitialGlobalData(uint8_t globalno);

//extern void SetManuParam(void);
//extern void InitAmpValue(void);
extern void InitFxValue(void);
//
//extern void StoreFx2Value2(void);

extern void CheckEditbuf(void);
extern void CheckGlobalbuf(void);

//extern const WORD t_fx2val2Range[];

//#define	MIN_DLYTIME	23
//#define	MAX_DLYTIME	1175
//#define	MIN_REVTIME	400/2	//(800/2)
//#define	MAX_REVTIME	2000/2	//(8000/2)
//
//#define	INI_DLYTIME		500	// DELAY TIME (ANALOG, ECHO)
//#define	INI_REV1TIME	825/2	//1358	// REVERB DECAY (SPRING)
//#define	INI_REV2TIME	1500/2	// REVERB DECAY (HALL)

//**************************************************************************
//	for param_storage.c
//**************************************************************************
//
//#include "flash.h"
//
////------------------------------------------------------------------------------
////	function declaration
////------------------------------------------------------------------------------
//
//void InitStorageAccess(void);
//void InitStorageAll(void);
//void InitStorageAllEx(bool fKeepSystemParam);
//bool VerifyStorageAll(void);
//bool VerifyStorageAllEx(bool fUseCurrentSystemParam);
//bool CheckStorageStatus(void);
//bool GetStorageStatus(void);
//
//#ifdef USE_PROGRAM_RECORD
//void ReadProgramData(uint8_t progno);
//void ReadProgramDataEx(uint8_t progno, editbuf_t *dst);
//bool StoreProgramData(uint8_t progno);
//bool StoreProgramDataEx(uint8_t progno, editbuf_t *src);
//#endif
//
//#ifdef USE_GLOBAL_RECORD
//void ReadGlobalData(uint8_t globalno);
//void ReadGlobalDataAll(void);
//void StoreGlobalData(uint8_t globalno);
//void RequestToStoreGlobalData(uint8_t globalno);
//bool IdleForStoreGlobalData(bool fStoreNow);
//#endif
//

#endif /* PARAM_H_ */
