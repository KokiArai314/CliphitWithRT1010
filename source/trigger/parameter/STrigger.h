/*
	STrigger.h
*/
#ifndef STRIGGER_H_
#define STRIGGER_H_

typedef struct {
	uint16_t	smin;	// source calculate minimum
	uint16_t	smax;	// source calculate maximum
	uint16_t	tmin;	// target CC value minimum
	uint16_t	tmax;	// target CC value maximum
}MINMAXCONV_t;

typedef struct {
	uint8_t			id;
	uint8_t			onCnt;
	uint8_t			velWnd;	// sample
	uint8_t			mskTim;	// sample
	uint16_t		padOff;
	MINMAXCONV_t	vel;
}PAD_t;

typedef struct {
	uint8_t			id;
	uint8_t			onCnt;
	uint8_t			velWnd;	// sample
	uint8_t			mskTim;	// sample
	uint16_t		ribbonOff;
	MINMAXCONV_t	cnv;
	MINMAXCONV_t	vel;
}CCPAD_t;

typedef struct {
	uint8_t			id;
	uint8_t			onCnt;
	uint8_t			velWnd;	// sample
	uint8_t			mskTim;	// sample
	uint8_t			crsCan;
	uint16_t		onLvl;
	MINMAXCONV_t	vel;
}EXTPAD_t;

typedef struct {
	uint8_t			id;
	uint8_t			onCnt;	// sample
	uint8_t			offCnt;	// sample
	uint16_t		swOff;
}SWPEDAL_t;

typedef struct {
	uint8_t			id;
	uint8_t			pow;
	uint8_t			flt1;
	uint8_t			flt2;
	uint8_t			hysWnd;
	uint8_t			posA;
	uint8_t			posB;
	uint8_t			mskTim;	// msec
	MINMAXCONV_t	vol;
	MINMAXCONV_t	vel;
}VRPEDAL_t;

#define TRIGGER_CHANNEL	(10-1)

/* --- pad --- */
#define PAD_OFF		(3700)

#define PAD_ON_COUNT	(12)	// =0.75msec
#define PAD_VEL_WINDOW	(44)	// =2.75msec
#define PAD_MSK_TIME	(16*7/3)	// =7msec

#define PAD_36_VEL_MIN	  (30)
#define PAD_36_VEL_MAX	(4095)
#define PAD_37_VEL_MIN	  (30)
#define PAD_37_VEL_MAX	(4095)
#define PAD_38_VEL_MIN	  (30)
#define PAD_38_VEL_MAX	(4095)
#define PAD_39_VEL_MIN	  (30)
#define PAD_39_VEL_MAX	(4095)
#define PAD_40_VEL_MIN	  (30)
#define PAD_40_VEL_MAX	(4095)
#define PAD_41_VEL_MIN	  (30)
#define PAD_41_VEL_MAX	(4095)

#define PAD_NOTE_BASE	(36)

#define PAD_36_VEL_MAX50	(3433)
#define PAD_37_VEL_MAX50	(3319)
#define PAD_38_VEL_MAX50	(2651)
#define PAD_39_VEL_MAX50	(2903)
#define PAD_40_VEL_MAX50	(2805)
#define PAD_41_VEL_MAX50	(2697)

#define PAD_MSK_TIME_MAX	(16*45/3)	// =45msec
#define PAD_MSK_TIME_MIN	(0)

#define PAD_VMIN_VAL0		(30)
#define PAD_VMIN_VAL50		(540)
#define PAD_VMIN_VAL100		(1050)

/* --- ccPad --- */
#define RIBBON_OFF	(3700)

#define RIBBON_LEFT_MIN	(3300)
#define RIBBON_LEFT_MAX	(1000)
#define RIBBON_CLEFT_MIN	(3300)
#define RIBBON_CLEFT_MAX	(1000)
#define RIBBON_CRIGHT_MIN	(3300)
#define RIBBON_CRIGHT_MAX	(1000)
#define RIBBON_RIGHT_MIN	(3300)
#define RIBBON_RIGHT_MAX	(1000)

#define RIBBON_ON_COUNT	(12)	// =0.75msec
#define RIBBON_FILTER		(3)		// 3point median
#define RIBBON_VEL_WINDOW	(44)	// =2.75msec
#define RIBBON_MSK_TIME	(16*7/3)	// =7msec

#define RIBBON_LEFT_VEL_MIN	  (50)
#define RIBBON_LEFT_VEL_MAX	(4095)
#define RIBBON_CLEFT_VEL_MIN	  (50)
#define RIBBON_CLEFT_VEL_MAX	(4095)
#define RIBBON_CRIGHT_VEL_MIN	  (50)
#define RIBBON_CRIGHT_VEL_MAX	(4095)
#define RIBBON_RIGHT_VEL_MIN	  (50)
#define RIBBON_RIGHT_VEL_MAX	(4095)

#define CCPAD_NOTE_BASE	(42)
#define CCPAD_CTRL_BASE	(80)

#define RIBBON_LEFT_VEL_MAX50	(3144)
#define RIBBON_CLEFT_VEL_MAX50	(3275)
#define RIBBON_CRIGHT_VEL_MAX50	(2917)
#define RIBBON_RIGHT_VEL_MAX50	(2980)

#define RIBBON_MSK_TIME_MAX	(16*45/3)	// =45msec
#define RIBBON_MSK_TIME_MIN	(0)

#define CCPAD_VMIN_VAL0		(50)
#define CCPAD_VMIN_VAL50	(550)
#define CCPAD_VMIN_VAL100	(1050)

/* --- extpad --- */
#define EXTPAD_ON_COUNT		(32)	// =2msec
#define EXTPAD_ON_COUNT_MAX	(64)	// =4msec
#define EXTPAD_ON_LEVEL		(100)	//MPS-10 : 100
#define EXTPAD_VEL_WINDOW	(64)	// =4msec
#define EXTPAD_MSK_TIME		(16*7/3)	// =7msec
#define EXTPAD_CLS_CAN		(1)		// 0:off 1:on

#define EXTPAD_46_VEL_MIN	 (80) //MPS-10 : 400
#define EXTPAD_46_VEL_MAX	(500) //MPS-10 : 4095
#define EXTPAD_47_VEL_MIN	 (50)
#define EXTPAD_47_VEL_MAX	(4095)
#define EXTPAD_48_VEL_MIN	 (50)
#define EXTPAD_48_VEL_MAX	(4095)
#define EXTPAD_49_VEL_MIN	 (50)
#define EXTPAD_49_VEL_MAX	(4095)

#define EXTPAD_NOTE_BASE	(46)

#define EXTPAD_46_VEL_MAX50	(2822)
#define EXTPAD_47_VEL_MAX50	(2822)
#define EXTPAD_48_VEL_MAX50	(2822)
#define EXTPAD_49_VEL_MAX50	(2822)

#define EXTPAD_ON_LEVEL50	(150)
#define EXTPAD_MSK_TIME_MAX	(16*45/3)	// =45msec
#define EXTPAD_MSK_TIME_MIN	(0)

#define EXTPAD_ON_LEVEL0	(110)

/* --- swPedal --- */
#define SWPEDAL_OFF		(3700)

#define SWPEDAL_ON_COUNT		(16)	// =1msec
#define SWPEDAL_OFF_COUNT		(16)	// =1msec

#define SWPEDAL_CTRL_64	(64)
#define SWPEDAL_CTRL_69	(69)

/* --- volumePad --- */
#define VRPEDAL_POW			(0)	// 0:^0,1:^2,2:^4,3:^8,4:^16
#define VRPEDAL_POW_HHR		(3)

#define VRPEDAL_FILTER1		(6)
#define VRPEDAL_FILTER1_HHR	(0)
#define VRPEDAL_FILTER2		(8)
#define VRPEDAL_FILTER2_HHR	(8)
#define VRPEDAL_HYSWND			(18)
#define VRPEDAL_HYSWND_HHR		(24)

#define VRPEDAL_POSA			(117)
#define VRPEDAL_POSA_HHR		(50)
#define VRPEDAL_POSB			(127)
#define VRPEDAL_POSB_HHR		(122)

#define VRPEDAL_MASKTIME		(4)		// msec
#define VRPEDAL_MASKTIME_HHR	(4)		// msec

#define VRPEDAL_8_VOL_MIN		(68)
#define VRPEDAL_8_VOL_MIN_HHR	(4000)
#define VRPEDAL_8_VOL_MAX		(3916)
#define VRPEDAL_8_VOL_MAX_HHR	(400)

#define VRPEDAL_50_VEL_MIN		(50)
#define VRPEDAL_50_VEL_MIN_HHR	(25)
#define VRPEDAL_50_VEL_MAX		(500)
#define VRPEDAL_50_VEL_MAX_HHR	(50)

#define VRPEDAL_CTRL_BALANCE	(8)
#define VRPEDAL_NOTE_50		(50)

#define VRPEDAL_50_VEL_MAX50		(440)
#define VRPEDAL_50_VEL_MAX50_HHR	(44)

#define VRPEDAL_MASKTIME_MAX	(200)	// msec
#define VRPEDAL_MASKTIME_MIN	(0)		// msec

enum eTrig {
	eTrig_Pad1 = 0,
	eTrig_Pad2,
	eTrig_Pad3,
	eTrig_Pad4,
	eTrig_Pad5,
	eTrig_Pad6,
	eTrig_ccPad1,
	eTrig_ccPad2,
	eTrig_ccPad3,
	eTrig_ccPad4,
	eTrig_extPad1,
	eTrig_extPad2,
	eTrig_extPad3,
	eTrig_extPad4,
	eTrig_swPedal1,
	eTrig_swPedal2,
	eTrig_vrPedal1,
	eTrig_hhPedal1,

	eTrigNumOf,

	eTrig_PadNumOf		= 6,
	eTrig_CCPadNumOf	= 4,
	eTrig_ExtPadNumOf	= 4,
	eTrig_SwPedalNumOf	= 2,
	eTrig_VrPedalNumOf	= 2,
};

enum eTrigAd {

	eTrigAd_ExtPad46 = 1,
	eTrigAd_ExtPad48 = 2,
	eTrigAd_CCPadLeft42 = 3,
	eTrigAd_CCPadCRight44 = 4,
	eTrigAd_Pad38 = 5,
	eTrigAd_HihatPedal=6,
	eTrigAd_Pad40 = 7,
	eTrigAd_Pad36 = 8,

	eTrigAd_CCRibbonLeft = 10,

	eTrigAd_CCRibbonCRight = 12,
	eTrigAd_SwitchPedal1 = 13,

	eTrigAd_ExtPad47 = 15,
	eTrigAd_ExtPad49 = 16,
	eTrigAd_CCPadCLeft43 = 17,
	eTrigAd_CCPadRight45 = 18,
	eTrigAd_Pad39 = 19,
	eTrigAd_SwitchPedal2 = 20,
	eTrigAd_Pad41 = 21,
	eTrigAd_Pad37 = 22,

	eTrigAd_CCRibbonCLeft = 24,

	eTrigAd_CCRibbonRight = 26,
	eTrigAd_VolimePedal = 27,
	eTrigAd_PadSw38 = 28,
	eTrigAd_PadSw39 = 29,
	eTrigAd_PadSw40 = 30,
	eTrigAd_PadSw36 = 31,
	eTrigAd_PadSw41 = 32,
	eTrigAd_PadSw37 = 33,

	eTrigAdNumOf,
};

#endif /* STRIGGER_H_ */
