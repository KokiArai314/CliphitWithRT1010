/*
 * midi_message.c
 *
 *  Created on: 2020/02/06
 *      Author: higuchi
 */

#include "midi_message.h"
#include "midi_player.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/


/*******************************************************************************
 * Variables
 ******************************************************************************/
/* MIDI 送信関連 */
static uint32_t	s_UsbMidiSendData;
static uint8_t	s_UsbMidiSendDataPos;

/*	エクスクルーシブ 7bit → 8bit変換	*/
static uint8_t*	s_7BitTo8BitDstPtr;
static uint8_t	s_7BitTo8BitCounter;
static uint8_t	s_7BitTo8BitPackedMsb;

/*******************************************************************************
 * Code
 ******************************************************************************/
/*
 * USB MIDIメッセージ送信関連
 */
static void	SendMidiData(uint32_t uldata)
{
	while (1) {
		if (WriteData4(uldata) == USB_RSLT_BUFFULL) {
			if (Send2USB() == USB_RSLT_NOTRDY) {
				/* USB Not Ready */
				break;
			}
			/*　正常終了かFIFO Full */
		}
		else {
			/*　正常終了かUSB Not Ready状態 */
			break;
		}
	}
}

void midi_SendExclData(uint8_t* SrcPtr, uint32_t Size)
{
	if (*SrcPtr == EXCL_BEGIN) {
		s_UsbMidiSendData = 0;
		s_UsbMidiSendDataPos = 8;
	}

	while (Size > 0) {

		s_UsbMidiSendData |= (uint32_t)(*SrcPtr) << s_UsbMidiSendDataPos;

		if (*SrcPtr == EXCL_END) {
			/* End of SysExclの処理 */
			if (s_UsbMidiSendDataPos == 24) {
				s_UsbMidiSendData |= 0x07UL;	/* SysEx ends with following three bytes */
			}
			else if (s_UsbMidiSendDataPos == 16) {
				s_UsbMidiSendData |= 0x06UL;	/* SysEx ends with following two bytes */
			}
			else {
				s_UsbMidiSendData |= 0x05UL;	/* SysEx ends with following single byte */
			}
			SendMidiData(s_UsbMidiSendData);
			s_UsbMidiSendData = 0;
			s_UsbMidiSendDataPos = 8;
		}
		else if (s_UsbMidiSendDataPos == 24) {
			/* USB MIDIパケットデータが出来上がった */
			s_UsbMidiSendData |= 0x04UL;		/* SysEx starts or continues */
			SendMidiData(s_UsbMidiSendData);
			s_UsbMidiSendData = 0;
			s_UsbMidiSendDataPos = 8;
		}
		else {
			s_UsbMidiSendDataPos += 8;
		}

		SrcPtr++;
		Size--;
	}
}

/*　エクスクルーシブ 8bit->7bit変換 */
void midi_Send8BitTo7BitData(uint8_t* SrcPtr, uint32_t Size)
{
	uint8_t	Counter, MsbData, Data;
	uint32_t	i;

	Counter = 0;
	while (Size > 0) {
		if (Counter == 0) {
			uint32_t	NumOfPack;

			Data = 0;
			if (Size >= 7) {
				NumOfPack = 7;
			}
			else {
				NumOfPack = Size;
			}
			for (i = 0; i < NumOfPack; i++) {
				MsbData = SrcPtr[i];

				MsbData = (uint8_t)((MsbData >> 7) & 1);
				MsbData <<= i;
				Data |= MsbData;
			}
		}
		else {
			Data = (uint8_t)((*SrcPtr) & 0x7f);
			SrcPtr++;
			Size--;
		}
		Counter++;
		Counter &= 0x07;
		midi_SendExclData(&Data, 1);
	}

	Data = EXCL_END;
	midi_SendExclData(&Data, 1);
}

/*
 * USB MIDIメッセージ受信関連
 */
/*　エクスクルーシブ 7bit->8bit変換 */
void midi_Cnv7BitTo8BitInitialize(uint8_t* DstPtr)
{
	s_7BitTo8BitDstPtr			= DstPtr;
	s_7BitTo8BitCounter			= 0;
	s_7BitTo8BitPackedMsb		= 0;
}

void midi_Cnv7BitTo8BitPutData(uint8_t Data)
{
	if (s_7BitTo8BitCounter == 0) {
		s_7BitTo8BitPackedMsb = Data;
	}
	else {
		*s_7BitTo8BitDstPtr = (uint8_t)(((s_7BitTo8BitPackedMsb & 1) << 7) | Data);
		s_7BitTo8BitDstPtr++;
		s_7BitTo8BitPackedMsb >>= 1;
	}
	s_7BitTo8BitCounter++;
	s_7BitTo8BitCounter &= 0x07;
}

