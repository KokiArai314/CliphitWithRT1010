/*
	trigger.h
*/
#ifndef TRIGGER_H_
#define TRIGGER_H_

#include "TriggerParameter/STrigger.h"

//namespace assigner {

#define TRGCHANNELL		(TRIGGER_CHANNEL)	// ch10 トリガーを鳴らすチャンネル
#define METROCHANNEL	(1-1)				// ch1 メトロノームを鳴らすチャンネル
#define PREVIEWCHANNEL	(2-1)				// ch2 プレヴューを鳴らすチャンネル
#define TRIGRSVCHANNEL	(3-1)				// ch3 トリガリザーブを鳴らすチャンネル
#define NOTEDLYCHANNEL	(9-1)				// ch9 ノートディレイを鳴らすチャンネル
#define EXTCHANNEL		(11-1)				// ch11 外部から鳴らすチャンネル
#define CTRLCHANNEL		(12-1)				// ch12 コントロール用チャンネル

#define TRGNOTENUM	(VRPEDAL_NOTE_50-PAD_NOTE_BASE+1)
#define TRGNOTEMIN	(PAD_NOTE_BASE)
#define TRGNOTEMAX	(PAD_NOTE_BASE+TRGNOTENUM-1)
#define TRGCCMIN	(CCPAD_CTRL_BASE)
#define TRGCCMAX	(CCPAD_CTRL_BASE+eTrig_CCPadNumOf-1)
#define TRGCCSW1	(SWPEDAL_CTRL_64)
#define TRGCCSW2	(SWPEDAL_CTRL_69)
#define TRGCCPEDAL	(VRPEDAL_CTRL_BALANCE)
#define CTRLMETONF	(80)
#define ALLSOUNDOFF	(120)

#define CCPADMIN	(eTrig_ccPad1)
#define CCPADMAX	(eTrig_ccPad4)

enum eSoundOffMode {
	eSoundOffMode_All = 0,	// 全サウンド
	eSoundOffMode_Previous,	// Kit change 前のサウンド
	eSoundOffMode_Enter,	// Pad Selected Mode Enter
	eSoundOffMode_Leave,	// Pad Selected Mode Leave
};

//}; //namespace assigner

#endif /* TRIGGER_H_ */
