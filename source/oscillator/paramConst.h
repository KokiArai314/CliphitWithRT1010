/**
  @file ParamConst.h
  @brief

  Copyright (C) 2021 Korg Inc. All rights reserved.
 */

#ifndef	PARAMCONST_H
#define	PARAMCONST_H

#include <stdint.h>

#ifdef 	__cplusplus
extern "C" {

namespace parameter
{

/**
 * @enum ObjectId
 * オブジェクトIDの番号の定義。
 */
typedef enum
{
	ObjectId_SAMPLE = 0,
	ObjectId_INSTRUMENT,
	ObjectId_KIT,
	ObjectId_GLOBAL,
	ObjectId_CURRENTINSTRUMENT,
	ObjectId_CURRENTKIT,
	ObjectId_FX,
	ObjectId_LOOPER,
	ObjectId_RECORDER,
	ObjectId_SAMPLECATEGORY,
	ObjectId_INSTRUMENTCATEGORY,

	NumOfObjectId,

	ObjectId_INVALID = -1,
} ObjectId;

/**
 * @enum ParamKind
 * SoundEditorにて、どのようなUIでエディットされるか。
 */
typedef enum
{
	ParamKind_SLIDER = 0, // default
	ParamKind_COMBOBOX,
	ParamKind_TEXTBUTTON,
	ParamKind_CHECKBOX,

	NumOfParamKind,
	ParamKind_INVALID = -1,
} ParamKind;

typedef enum
{
	ParameterId_Common = 100,
} ParameterId;

/**
 * @enum ObjectVersion
 * eMMC内のParameterFileで使用(バージョン違いはロードされない)。
 * 各オブジェクトの構造が変更になったら、バージョンを上げる。
 */
typedef enum
{
	ObjectVersion_SAMPLE		= 0,
	ObjectVersion_INSTRUMENT	= 2,
	ObjectVersion_KIT			= 2,
	ObjectVersion_GLOBAL		= 2,
	ObjectVersion_FX			= 0,
	ObjectVersion_LOOPER		= 0,
	ObjectVersion_RECORDER		= 0,
	ObjectVersion_STARTUPPARAM	= 0,
	ObjectVersion_CATEGORY		= 0,
} ObjectVersion;

#define	ObjectVersion_KITV1		(1)

} // namespace parameter
#endif	/* __cplusplus */

enum EParamConst {
	constNumOfNameByte			= 24,
	constNumOfFileNameByte		= 24,

	constNumOfPresetCategory	= 128,
	constNumOfUserCategory		= 128,
	constNumOfLocalizeCategory	= 128,
	constNumOfCategory			= (constNumOfPresetCategory + constNumOfUserCategory),	// caution ! without constNumOfLocalizeCategory

	constNumOfPresetKit			= 100,
	constNumOfUserKit			= 100,
	constNumOfLocalizeKit		= 100,
	constNumOfPresetUserKit		= constNumOfPresetKit + constNumOfUserKit,
	constNumOfKit				= constNumOfPresetKit + constNumOfUserKit + constNumOfLocalizeKit,

	constNumOfPresetInstrument	= 3000,
	constNumOfUserInstrument	= 2000,
	constNumOfLocalizeInstrument	= 2000,
	constNumOfPresetUserInstrument	= constNumOfPresetInstrument + constNumOfUserInstrument,
	constNumOfInstrument		= constNumOfPresetInstrument + constNumOfUserInstrument + constNumOfLocalizeInstrument,

	constNumOfPresetFxType		= 100,

	constNumOfMetronome			= 50,
	constNumOfPlayer			= 50,

	constMinOfTempo				= 30,
	constMaxOfTempo				= 300,

	constMinOfTempoX100			= 30 * 100,
	constMaxOfTempoX100			= 300 * 100,

	constNumOfAssignablePedal	= 1,

};

enum ESampleBankId {
	SampleBankId_Preset1,		/* for preset short sample */
	SampleBankId_Preset2,		/* for preset long sample */
	SampleBankId_User,			/* for user long sample */

	numofSampleBankId,

	constNumOfSampleBank		= numofSampleBankId,
};

enum ESampleParamConst {
	constSelectableStartNumSample1 = 100,
	constNumOfPresetSample1		= 8192,
	constNumOfPresetSample2		= 1024,
	constNumOfPresetSample		= constNumOfPresetSample1 + constNumOfPresetSample2,

	constNumOfUserSample		= 10000,
	constNumOfLocalizeSample	= 10000,

	constNumOfPresetUserSample	= constNumOfPresetSample + constNumOfUserSample,
	constNumOfSample			= constNumOfPresetSample + constNumOfUserSample + constNumOfLocalizeSample,

	constSamplingStartNum		= constNumOfSample,
};

enum EOscillatorConst {
	/* For Oscillator parameters */
	constNumOfSampleBlock		= 8,
	constNumOfVelocityLayer		= 8,
	constNumOfRoundRobbin		= 8
};

enum EInstParamConst {
	/* For Inst parameters */
	Inst_Osc1 = 0,
	Inst_Osc2,

	constNumOfInstOscillator,
};

enum EKitParamConst {
	/* For Kit parameters */
	constNumOfContinuousControlPad	= 4,
};

enum ECategoryId {
	CategoryId_Preset = (constNumOfPresetCategory - 1),
	CategoryId_User,

	numofCategoryId,
};

enum EAssignableSwId {
	AssignableSw_1,
	AssignableSw_2,

	numofAssignableSwId,

	constNumOfAssignableSwitch	= numofAssignableSwId,
};

enum EIfxId {
	Ifx_INVALID = -1,

	Ifx_1 = 0,
	Ifx_2,

	numofEIfxId,

	constNumOfInstFx = numofEIfxId,
};

enum ELfxId {
	Lfx_1 = 0,

	numofELfxId,

	constNumOfLooperFx = numofELfxId,
};

enum EIeqId {
	Ieq_INVALID = -1,

	Ieq_Fx1 = 0,
	Ieq_Fx2,
	Ieq_Main,

	numofEIeqId,

	constNumOfInstEq = numofEIeqId,
};

// Fx ストレージ内で特別扱いする必要のあるもののインデクス。
enum ESpecificFxIndex {
	X16800_FX_INDEX_DUMMY		= 0,
	X16800_FX_INDEX_IFXCOMP		= 1,
	X16800_FX_INDEX_IFXEQ		= 2,
};

enum EHHPosition {
	HHPos_Start,

	HHPos_Close		= HHPos_Start,
	HHPos_1,
	HHPos_2,
	HHPos_3,
	HHPos_4,
	HHPos_5,
	HHPos_6,
	HHPos_7,

	numofEHHPosition
};

enum EFxParamConst {
	/* For Effect */
	constNumOfFxParam		= 32,
	constNumOfFxUserEdit	= constNumOfFxParam,	///@NOTE UIで制限する場合は変更
};

enum EMasterFxList {
	masterFx_INVALID = -1,

	masterFx_MFX = 0,
	masterFx_Reverb,
	masterFx_Filter,

	constNumOfMasterFx,
};

enum EFxConstants {
	totalEq_ParamOffset = 23,		// MFx 内での TotalEQ パラメータの開始位置。
};

enum EEffectTypeConst {
	constNumOfIFX1Types		= 1,
	constNumOfIFX2Types		= 1,
	constNumMFXTypes 		= 121,
};

enum EKitInstId {
	kitinstINVALID = -1,

	kitinstSTART = 0,

	kitinstPad1 = kitinstSTART,
	kitinstPad2,
	kitinstPad3,
	kitinstPad4,
	kitinstPad5,
	kitinstPad6,
	kitinstPad7,
	kitinstPad8,
	kitinstPad9,
	kitinstPad10,
	kitinstPad11,
	kitinstPad12,
	kitinstPad13,
	kitinstPad14,
	kitinstPad15,

	numofEKitInstId,

	constNumOfKitInst = numofEKitInstId,

	constNumOfInternalKitInst = (kitinstPad10 + 1),
};

enum EParamCurve {
	ParamCurve_Linear = 0,
	ParamCurve_Exp1,
	ParamCurve_Exp2,
	ParamCurve_Exp3,
	ParamCurve_Log1,
	ParamCurve_Log2,
	ParamCurve_Log3,

	NumOfParamCurve,
};

enum EParamBeat{
	ParamBeat_INVALID = -1,

	ParamBeat_1_2 = 0,
	ParamBeat_2_2,
	ParamBeat_3_2,
	ParamBeat_4_2,
	ParamBeat_5_2,
	ParamBeat_6_2,
	ParamBeat_7_2,
	ParamBeat_8_2,
	ParamBeat_9_2,
	ParamBeat_10_2,
	ParamBeat_11_2,
	ParamBeat_12_2,
	ParamBeat_13_2,
	ParamBeat_14_2,
	ParamBeat_15_2,
	ParamBeat_16_2,
	ParamBeat_17_2,
	ParamBeat_18_2,
	ParamBeat_19_2,
	ParamBeat_20_2,
	ParamBeat_21_2,
	ParamBeat_22_2,
	ParamBeat_23_2,
	ParamBeat_24_2,

	ParamBeat_1_4,
	ParamBeat_2_4,
	ParamBeat_3_4,
	ParamBeat_4_4,
	ParamBeat_5_4,
	ParamBeat_6_4,
	ParamBeat_7_4,
	ParamBeat_8_4,
	ParamBeat_9_4,
	ParamBeat_10_4,
	ParamBeat_11_4,
	ParamBeat_12_4,
	ParamBeat_13_4,
	ParamBeat_14_4,
	ParamBeat_15_4,
	ParamBeat_16_4,
	ParamBeat_17_4,
	ParamBeat_18_4,
	ParamBeat_19_4,
	ParamBeat_20_4,
	ParamBeat_21_4,
	ParamBeat_22_4,
	ParamBeat_23_4,
	ParamBeat_24_4,

	ParamBeat_1_8,
	ParamBeat_2_8,
	ParamBeat_3_8,
	ParamBeat_4_8,
	ParamBeat_5_8,
	ParamBeat_6_8,
	ParamBeat_7_8,
	ParamBeat_8_8,
	ParamBeat_9_8,
	ParamBeat_10_8,
	ParamBeat_11_8,
	ParamBeat_12_8,
	ParamBeat_13_8,
	ParamBeat_14_8,
	ParamBeat_15_8,
	ParamBeat_16_8,
	ParamBeat_17_8,
	ParamBeat_18_8,
	ParamBeat_19_8,
	ParamBeat_20_8,
	ParamBeat_21_8,
	ParamBeat_22_8,
	ParamBeat_23_8,
	ParamBeat_24_8,

	ParamBeat_1_8t,
	ParamBeat_2_8t,
	ParamBeat_3_8t,
	ParamBeat_4_8t,
	ParamBeat_5_8t,
	ParamBeat_6_8t,
	ParamBeat_7_8t,
	ParamBeat_8_8t,
	ParamBeat_9_8t,
	ParamBeat_10_8t,
	ParamBeat_11_8t,
	ParamBeat_12_8t,
	ParamBeat_13_8t,
	ParamBeat_14_8t,
	ParamBeat_15_8t,
	ParamBeat_16_8t,
	ParamBeat_17_8t,
	ParamBeat_18_8t,
	ParamBeat_19_8t,
	ParamBeat_20_8t,
	ParamBeat_21_8t,
	ParamBeat_22_8t,
	ParamBeat_23_8t,
	ParamBeat_24_8t,

	ParamBeat_1_16,
	ParamBeat_2_16,
	ParamBeat_3_16,
	ParamBeat_4_16,
	ParamBeat_5_16,
	ParamBeat_6_16,
	ParamBeat_7_16,
	ParamBeat_8_16,
	ParamBeat_9_16,
	ParamBeat_10_16,
	ParamBeat_11_16,
	ParamBeat_12_16,
	ParamBeat_13_16,
	ParamBeat_14_16,
	ParamBeat_15_16,
	ParamBeat_16_16,
	ParamBeat_17_16,
	ParamBeat_18_16,
	ParamBeat_19_16,
	ParamBeat_20_16,
	ParamBeat_21_16,
	ParamBeat_22_16,
	ParamBeat_23_16,
	ParamBeat_24_16,

	NumOfParamBeat,
};


// Fx Attribute
enum EFxAttributeId {
	fxAttributeId_Invalid = 0,
	fxAttributeId_All,		// for All(Fx OFF)
	fxAttributeId_Misc,		// for MFx & IFx & Looper Fx
	fxAttributeId_CcPad,	// for MFx
	fxAttributeId_Reverb,	// for MFx Reverb & IFx & Looper Fx
	fxAttributeId_Filter,	// for MFx Filter & IFx & Looper Fx

	numofEFxAttributeId,
};

// Fx Algorithm
enum EFxAlgorithm
{
	fxAlgorithm_None = 0,
	fxAlgorithm_IfxEq,
	fxAlgorithm_DynaComp,
	fxAlgorithm_RedComp,
	fxAlgorithm_GadgetDistortion,
	fxAlgorithm_JamVoxOverb,
	fxAlgorithm_ChorusFlanger,
	fxAlgorithm_ChiangmaiChorus,
	fxAlgorithm_OrangePhaser,
	fxAlgorithm_GadgetRingMod,
	fxAlgorithm_BpmSyncModDelay,
	fxAlgorithm_BpmSyncModDelayLR,
	fxAlgorithm_ModDelay,
	fxAlgorithm_ModDelayLR,
	fxAlgorithm_LPF,
	fxAlgorithm_HPF,
	fxAlgorithm_BPF,
	fxAlgorithm_Decimator,
	fxAlgorithm_VoxWah,
	fxAlgorithm_GateReverb,
	fxAlgorithm_ValveReactor,
	fxAlgorithm_StereoCompressor,
	fxAlgorithm_TapeEcho,

	numofEFxAlgorithm,
};

/* ベロシティーカーブテーブルエディタアクセス用 */
enum trigger_veltbl {
	eVelCrvTblNr = 0,
	eVelCrvTblHi,
	eVelCrvTblLo,

	eVelCnvTblNumOf,

	eVelCrvTblOp1 = eVelCnvTblNumOf,
	eVelCrvTblOp2,
	eVelCrvTblOp3,

	eVelCrvTblOpMax = eVelCrvTblOp3,
};

enum SongPlayerAssign {
	SongPlayerAssign_None = 0,
	SongPlayerAssign_File1LR,
	SongPlayerAssign_File2LR,
	SongPlayerAssign_File1L,
	SongPlayerAssign_File1R,
	SongPlayerAssign_File2L,
	SongPlayerAssign_File2R,
	NumOFSongPlayerAssign,

	SongPlayerAssign_Invalid = -1,
};

#ifdef 	__cplusplus
}
#endif	/* __cplusplus */

#endif	/* PARAMCONST_H */
