/*
 * extpadrev2.h
 *
 *  Created on: 2021/06/03
 *      Author: akino
 */
//#include "extpad.h"

#ifndef APPLICATION_TRIGGER_EXTPADREV2_H_
#define APPLICATION_TRIGGER_EXTPADREV2_H_

/*     ___ ____ __
 *   ,'   |    |
 * 	<  46 | 47 |
 * 	 ',___|____|__
 *     ___ ____ __
 *   ,'   |    |
 * 	<  48 | 49 |
 * 	 ',___|____|__
 *
 *
 *  -----^v^v^v^v^v^v-----
 * 	    |<->|			on window
 * 	    |<---------->|	velocity detect time
 *
 */
#if 0	// defined extpad.h
enum {
	EXTPAD_46_ID = 0,
//	EXTPAD_47_ID,
//	EXTPAD_48_ID,
//	EXTPAD_49_ID,

	EXTPAD_NUM_OF,
};

#define EXTPAD_46_AD_CH	(1)
#define EXTPAD_46_SW_CH	(-1)
//#define EXTPAD_47_AD_CH	(eTrigAd_ExtPad47)
//#define EXTPAD_47_SW_CH	(-1)
//#define EXTPAD_48_AD_CH	(eTrigAd_ExtPad48)
//#define EXTPAD_48_SW_CH	(-1)
//#define EXTPAD_49_AD_CH	(eTrigAd_ExtPad49)
//#define EXTPAD_49_SW_CH	(-1)
#endif	// #if 0

void extPadRev2(TRIGSCN_t *ptrigscn);

#endif /* APPLICATION_TRIGGER_EXTPADREV2_H_ */
