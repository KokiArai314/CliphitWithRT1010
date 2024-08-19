/*
 * midi_message.h
 *
 *  Created on: 2020/02/06
 *      Author: higuchi
 */

#ifndef MIDI_MESSAGE_H_
#define MIDI_MESSAGE_H_

#include <stdint.h>
#include "midi_player.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/* SysExcl Status */
#define	EXCL_BEGIN		0xF0	/* Exclusive Status */
#define	EXCL_KORGID		0x42	/* Manufactures ID */
#define	EXCL_END		0xF7	/* End of exclusive */

/**
 * System Exclusive Message
 * ADIOのMIDI Implementation等を参考に移植
 */
/* System Exclusive Format ID */
#define SFID_DEVELOP	0x21	/* 3rd byte : for Develop */
#define SFID_TEST		0x22	/* 3rd byte : Test Mode */
#define SFID_USER		0x30	/* 3rd byte : for User (3n) */

/* System Exclusive Function Code */
#define	SFC_MODE_REQ	0x12	// MODE REQUEST
#define	SFC_CDUMP_REQ	0x10	// CURRENT PROGRAM DATA DUMP REQUEST
#define	SFC_CEQDUMP_REQ	0x19	// CURRENT AUDIO EQ DATA DUMP REQUEST
#define	SFC_PDUMP_REQ	0x1C	// PROGRAM DATA DUMP REQUEST
#define	SFC_EQDUMP_REQ	0x1D	// AUDIO EQ  DATA DUMP REQUEST
#define	SFC_GDUMP_REQ	0x0E	// GLOBAL DATA DUMP REQUEST
#define	SFC_WRITE_REQ	0x11	// PROGRAM WRITE REQUEST
#define	SFC_CDUMP		0x40	// CURRENT PROGRAM DATA DUMP
#define	SFC_CEQDUMP		0x49	// CURRENT AUDIO EQ DATA DUMP
#define	SFC_PDUMP		0x4C	// PROGRAM DATA DUMP
#define	SFC_EQDUMP		0x4D	// AUDIO EQ DATA DUMP
#define	SFC_GDUMP		0x51	// GLOBAL DATA DUMP
#define	SFC_MODE_CHG	0x4E	// MODE CHANGE
#define	SFC_PARA_CHG	0x41	// PARAMETER CHANGE
#define	SFC_MODE_DATA	0x42	// MODE DATA
#define	SFC_FORM_ERR	0x26	// DATA FORMAT ERROR
#define	SFC_LOAD_CMP	0x23	// DATA LOAD COMPLETED
#define	SFC_LOAD_ERR	0x24	// DATA LOAD ERROR
#define	SFC_WRITE_CMP	0x21	// WRITE COMPLETED
#define	SFC_WRITE_ERR	0x22	// WRITE ERROR

#define SFC_REC			0x80	// System Exclusive解析中
#define SFC_STANDBY		0xFF	// STANDBY

/* Inquiry Message */
#define	NONREALMSG		0x7E
#define	INQUIRYMSG		0x06
#define	INQUIRYREQ		0x01
#define	INQUIRYREP		0x02

/** Device Inquiry Requestの返事 */
typedef struct _tagMIDI_DeviceInquiryReply {
	uint8_t		ExclBegin;
	int8_t		NRealMsg;
	int8_t		MidiCh;
	int8_t		InqMsg;
	int8_t		InqReply;
	int8_t		KorgID;
	int8_t		ProdIdLsb;
	int8_t		ProdIdMsb;
	int8_t		MemCodeLsb;
	int8_t		MemCodeMsb;
	int8_t		MinorVersLsb;
	int8_t		MinorVersMsb;
	int8_t		MajorVersLsb;
	int8_t		MajorVersMsb;
	uint8_t		ExclEnd;	/* End of exclusive */
} MIDI_DevInquiryReply;

/** System Exclusive Header */
typedef struct _tagMIDI_SystemExclusiveHeader {
	uint8_t		ExclBegin;
	int8_t		KorgID;
	int8_t		MidiCh;
	int8_t		ProdId1st;
	int8_t		ProdId2nd;
	int8_t		ProdId3rd;
#ifdef USE_SUB_ID
	int8_t		SubID;
#endif
	int8_t		Cmmand;
} MIDI_SysExHeader;

/*******************************************************************************
 * API
 ******************************************************************************/
/*
 * USB MIDIメッセージ送信関連
 */
void midi_SendExclData(uint8_t* SrcPtr, uint32_t Size);
void midi_Send8BitTo7BitData(uint8_t* SrcPtr, uint32_t Size);

/*
 * USB MIDIメッセージ受信関連
 */
void midi_Cnv7BitTo8BitInitialize(uint8_t* DstPtr);
void midi_Cnv7BitTo8BitPutData(uint8_t Data);


#endif /* MIDI_MESSAGE_H_ */

