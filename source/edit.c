/*
 * edit.c
 *
 *  Created on: 2020/02/06
 *      Author: higuchi
 */

//#include	"board.h"
//#include	"common.h"

//#include	"audio.h"
//#include	"cpufx.h"

#include	"edit.h"
#include	"param.h"
//#include	"ad.h"
//#include	"swled.h"


//------------------------------------------------------------------------------
//	Prototype, local variables
//------------------------------------------------------------------------------

/**
 * 暫定(VX II)
 */
#if TEST_VXII_SYSEX
static uint8_t presnobuf;
#endif	// #if TEST_VXII_SYSEX

//------------------------------------------------------------------------------
//	global variables
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//	global functions
//------------------------------------------------------------------------------

/**
 * 暫定(VX II)
 */
#if TEST_VXII_SYSEX
//-------------------------------
// Preset Number を取得
//-------------------------------
uint8_t GetPresetNo(void)
{
	return(presnobuf);
}
#endif	// #if TEST_VXII_SYSEX


void EditIdle(void)
{
}

//// CH, MODE 制御信号を更新
////	CH Switch, Foot Sw, Mode Sw の状態に応じて、
////	CH, MODE 信号を切替える
//void ChangeChModeEx(void)
//{
//	HC_ChannelSelect(GetChannelSetting());
//	HC_ModeSelect(GetModeSetting());
//}

//bool CheckChModeChange(void)
//{
//	return ((GetChannelSetting() != HC_ChannelGetCurrent()) || (GetModeSetting() != HC_ModeGetCurrent()));
//}

//void ChangeChMode(void)
//{
//	if (CheckChModeChange()) {
//
//		// Pre Amp / Fx Mute
//		HC_InOutMute(true, false);
////		FxChangeInputMute(true);
//		while (HC_InOutMuteGetStatus());
//		// Channel, Mode 切り替え
//		UpdateDispCh();
////		ChangeChModeEx();
//		while (HC_ChannelSelectGetStatus() || HC_ModeSelectGetStatus());
//		// Pre Amp / Fx Mute 解除
//		HC_InOutMute(false, false);
////		FxChangeMute(false);
//	}
//}

// Reverb, Send/Return, Master1/2 制御信号を更新

//void ChangeReverbOnOff(void)
//{
//	HC_ReverbOnOff(GetReverbSwStatus());
//}
//
//void ChangeSendReturnOnOff(void)
//{
//	HC_SendReturnOnOff(GetSendReturnSwStatus());
//}
//
//void ChangeMaster12(void)
//{
//	HC_MasterSelect(GetMasterSwStatus());
//}

//void ChangeChannel(uint8_t chno)
//{
//	if (chno >= ADSWCHNUM_MAX) {
//		chno = 0;
//	}
//	globalbuf.chno = chno;
//#if 0	// X-15705/6 : 必要なし
//	editbuf.amptype = (chno == 0 ? bSB_FX1 : bSB_FX2);
//#endif
//#if 0	// X-15705-8 : do not backup
//	RequestToStoreGlobalData(GLBOFS_CHNO);
//#endif
//
//	// Amp Model / Fx Mute
//	HC_InOutMute(true);
//	FxChangeMute(true);
//	while (HC_InOutMuteGetStatus());
//	// Chanel / Amp Model Select
////	HC_ModeSelect(editbuf.amptype);
////	HC_ChannelSelect(0xFF);	// ノイズ防止のため一旦すべて有効にする
////	while (HC_ChannelSelectGetStatus());
//#if 1	// X-15705-6
//	if (chno <= ADSWCHNUM_CHANNEL2) {
//		HC_ChannelSelect(chno);	// 改めてChannel設定
//		UpdateDispCh();
//		while (HC_ChannelSelectGetStatus());
//	}
//	/// @note X-15705-8 : FSWの場合、一旦何もしない(globalbuf.chnoだけ決める)
//#else
//	HC_ChannelSelect(chno);	// 改めてChannel設定
////	while (HC_ChannelSelectGetStatus() || HC_ModeSelectGetStatus());
//	while (HC_ChannelSelectGetStatus());
//#endif
//	// Amp Model / Fx Mute解除
//	HC_InOutMute(false);
////	FxChangeTypeAll();
//	FxChangeMute(false);
//}

void EditResume1stStatus(void)
{
//	int i;

	// editbuf初期化
	RestoreEditParam();
	CheckEditbuf();
	InitFxValue();

	//　表示更新、CH,AMP設定
//	HC_ChannelSelect(globalbuf.chno);
//	UpdateDispAllForPowerOn();
//	ChangeChModeEx();
//	ChangeReverbOnOff();
//	ChangeSendReturnOnOff();
//	ChangeMaster12();
//	while (HC_ChannelSelectGetStatus() || HC_ModeSelectGetStatus());
//	HC_InOutMute(false);

	// fx初期化(fs処理開始)
//	FxInit();

#if 1	// X-15705-8 : bEditFxBypass -> falseに設定, FX3ONOFFをOnに設定
//	EditFxBypass(false);
#endif

	// fxタイプ、パラメータ初期化
//	for (i = 0; i < FXID_NUM; i++) {
//		FxSetTypeSetting(i, &editbuf.fx[i]);
//		FxSetOnOffSetting(i, editbuf.fx[i].onoff && !bEditFxBypass);
//	}

	// fx動作開始
//	FxChangeTypeAll();
//	FxChangeMute(false);

//	ResetTap();

}

void EditAmpSelector()
{
//	// Amp Model / Fx Mute
//	HC_AmpModelMute(true);
//	FxChangeMute(true);
//	while (HC_AmpModelMuteGetStatus());
//	// Amp Model Select
//	HC_AmpSelect(editbuf.amptype);
//	while (HC_AmpSelectGetStatus());
//	// Amp Model / Fx Mute解除
//	HC_AmpModelMute(false);
//	FxChangeTypeAll();
//	FxChangeMute(false);
}

void EditFx1OnOff(void)    {
//	FxChangeOnOff(FXID_FX1, editbuf.fx[FXID_FX1].onoff && !bEditFxBypass);
}
void EditFx1Selector(void) {
//	FxChangeType(FXID_FX1, &editbuf.fx[FXID_FX1]);
}
void EditFx1Val1(void)     {
//	FxChangeVal1(FXID_FX1, editbuf.fx[FXID_FX1].val1);
}
void EditFx1Val2(void)     {
//	FxChangeVal2(FXID_FX1, editbuf.fx[FXID_FX1].val2);
}

void EditFx2OnOff(void)    {
//	FxChangeOnOff(FXID_FX2, editbuf.fx[FXID_FX2].onoff && !bEditFxBypass);
}
void EditFx2Selector(void) {
//	FxChangeType(FXID_FX2, &editbuf.fx[FXID_FX2]);
}
void EditFx2Val1(void)     {
//	FxChangeVal1(FXID_FX2, editbuf.fx[FXID_FX2].val1);
}
void EditFx2Val2(void)     {
//	FxChangeVal2(FXID_FX2, editbuf.fx[FXID_FX2].val2);
}

void EditFx3OnOff(void)    {
//	FxChangeOnOff(FXID_FX3, editbuf.fx[FXID_FX3].onoff && !bEditFxBypass);
}
void EditFx3Selector(void) {
//	FxChangeType(FXID_FX3, &editbuf.fx[FXID_FX3]);
}
void EditFx3Val1(void)     {
//	FxChangeVal1(FXID_FX3, editbuf.fx[FXID_FX3].val1);
}
void EditFx3Val2(void)     {
//	FxChangeVal2(FXID_FX3, editbuf.fx[FXID_FX3].val2);
}

void EditSetFxBypass(bool bBypass)
{
	bEditFxBypass = bBypass;
}
bool EditGetFxBypass(void)
{
	return(bEditFxBypass);
}

void EditFxBypass(bool bBypass)
{
	bEditFxBypass = bBypass;
	EditFx1OnOff();
	EditFx2OnOff();
	EditFx3OnOff();
}
