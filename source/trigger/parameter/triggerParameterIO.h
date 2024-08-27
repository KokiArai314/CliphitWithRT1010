/*
	triggerParameterIO.h
*/
#ifndef TRIGGERPARAMETERIO_H_
#define TRIGGERPARAMETERIO_H_

#ifdef __cplusplus
extern "C" {
#endif	/* __cplusplus */

#define HEADCMD	(0xf8)
#define HEADPAD	(0xf9)
#define HEADCCPAD	(0xfa)
#define HEADEXTPAD	(0xfb)
#define HEADSWITCH	(0xfc)
#define HEADVOLUME	(0xfd)
#define HEADCMDANS	(0xfe)
#define HEADRESET	(0xff)

#define CMDVERREQ	(0x00)
#define CMDALLDMP	(0x01)
#define CMDVELCRV	(0x10)
#define CMDRAWDMP	(0x40)

void triggerParameterIOEntry(uint8_t data);

#ifdef __cplusplus
}
#endif	/* __cplusplus */

#endif /* TRIGGERPARAMETERIO_H_ */
