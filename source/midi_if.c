/*
 * midi_if.c
 *
 *  Created on: 2020/05/07
 *      Author: higuchi
 */

#include <stddef.h>
#include "fsl_lpuart.h"
#include "midi_if.h"

#include "board.h"
#include "midi_player.h"
#include "composite.h"
#include "usb_device_descriptor.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
extern usb_device_composite_struct_t *g_deviceComposite;

/**
 * MIDI I/F TX
 */
#if SUPPORT_USB_HIGH_SPEED
static uint8_t	s_MidiIfTxBuff[384];	/// @note 512 * 3/4
#else
static uint8_t	s_MidiIfTxBuff[48];		/// @note 64 * 3/4
#endif
static uint16_t	s_MidiIfTxBuffPos;
static uint16_t	s_MidiIfTxNumOfData;

static uint8_t receiveSysExData;


/**
 * MIDI I/F RX
 * CN(Cable Number)=0固定
 */
typedef struct UsbMidiEventPacket {
	uint8_t cin;
	uint8_t midi0;
	uint8_t midi1;
	uint8_t midi2;
} UsbMidiEventPacket_t;

typedef struct UsbMidiSysExEventPacket {
	uint8_t* bufPtr;
	uint16_t size;
} UsbMidiSysExEventPacket_t;


#define NUM_OF_USB_MIDI_SHORT_EVENT_BUFFER 16

#define NUM_OF_USB_MIDI_SYSEX_DATA_BUFFER 2
#define NUM_OF_USB_MIDI_SYSEX_EVENT_BUFFER 16
#if SUPPORT_USB_HIGH_SPEED
#define NUM_OF_USB_MIDI_SYSEX_EVENT_DATA_SIZE (NUM_OF_USB_MIDI_SYSEX_DATA_BUFFER * HS_USB_MIDI_ENDP_MAX_PACKET_SIZE)
#else
#define NUM_OF_USB_MIDI_SYSEX_EVENT_DATA_SIZE (NUM_OF_USB_MIDI_SYSEX_DATA_BUFFER * USB_MIDI_ENDP_MAX_PACKET_SIZE)
#endif

#define INC_DATA_POS(pos, num) pos = ((pos + 1) & (num-1))
#define DEC_DATA_POS(pos, num) pos = ((pos == 0) ? (num-1) : (pos - 1))


 /* Short Message Event */
static UsbMidiEventPacket_t UsbShortEventPacketBuf[NUM_OF_USB_MIDI_SHORT_EVENT_BUFFER];
static uint8_t setMidiShortEvent;
static uint8_t midiShortEventRdPtr;
static uint8_t midiShortEventWtPtr;

/* SysEX Message Event */
static UsbMidiSysExEventPacket_t UsbSysExEventPacketBuf[NUM_OF_USB_MIDI_SYSEX_EVENT_BUFFER];
static uint8_t setMidiSysExEvent;
static uint8_t midiSysExEventRdPtr;
static uint8_t midiSysExEventWtPtr;

/* F0を受信したらtrue */
static bool forwarding_sysex;

/* UART RXから受信したデータをanalyzeする際に必要な変数群(Short Message) */
static uint8_t shortStopCount;
static uint8_t shortMsgCount;
static uint8_t runningStatusData;
static uint8_t oneShortMessageBuf[4];

/* UART RXから受信したデータをanalyzeする際に必要な変数群(SysEX Message) */
static uint16_t midiSysExMessageCurrentWriteBufPtr;
static uint16_t midiSysExMessageBufWtPtr;
static uint16_t midiSysExCurrentDataSize;
static uint8_t UsbSysExMessageBuf[NUM_OF_USB_MIDI_SYSEX_EVENT_DATA_SIZE];


/*******************************************************************************
 * Code
 ******************************************************************************/
static void clearMidiRxEventParameter(void);
static void real_midi_received(uint8_t data);

/**
 * @note startから1byteしかないデータなら割り込み許可せず終了. それ以外は割り込み内で送信を続ける
 * a送信し終わったら, 送信割り込みを止める
 */
static void startSendtoUartTx(void)
{
	g_deviceComposite->midiPlayer.rxDataReceived = USB_RXDATA_STATE_SEQ_MIDIIF_ANALYZING;
	LPUART_WriteByte(LPUART1, s_MidiIfTxBuff[s_MidiIfTxBuffPos++]);
	s_MidiIfTxNumOfData--;
	/* Enable TX interrupt. */
	LPUART_EnableInterrupts(LPUART1, kLPUART_TransmissionCompleteFlag);
}

static void sendtoUartTx(void)
{
	if (s_MidiIfTxNumOfData == 0) {
		g_deviceComposite->midiPlayer.rxDataReceived = USB_RXDATA_STATE_SEQ_MIDIIF_FINISH_PROCESSING;

	    /* Disable TX interrupt. */
	    LPUART_DisableInterrupts(LPUART1, kLPUART_TransmissionCompleteFlag);
	}
	else {
		LPUART_WriteByte(LPUART1, s_MidiIfTxBuff[s_MidiIfTxBuffPos++]);
		s_MidiIfTxNumOfData--;
	}
}

/*******************************************************************************
 * LPUART1 Interrupt Handler
 ******************************************************************************/
void LPUART1_IRQHandler() {
	volatile uint32_t uart_stat;
	volatile uint32_t uart_ctrl;

	uart_stat = LPUART_GetStatusFlags(LPUART1);
	uart_ctrl = LPUART1->CTRL;
	uart_stat &= uart_ctrl;

#if 1	// エラー対応..
    /* If RX overrun. */
    if ((uint32_t)kLPUART_RxOverrunFlag == ((uint32_t)kLPUART_RxOverrunFlag & uart_stat))
    {
        /* Clear overrun flag, otherwise the RX does not work. */
    	LPUART1->STAT = ((LPUART1->STAT & 0x3FE00000U) | LPUART_STAT_OR_MASK);

    	/// @note 何かする？
    }
#endif	// ..エラー対応

	/// @note RX側が先
	if ((uart_stat & kLPUART_RxDataRegFullFlag) != 0)
	{
#if 1	// FIFO消化..
	    uint8_t count = ((uint8_t)((LPUART1->WATER & LPUART_WATER_RXCOUNT_MASK) >> LPUART_WATER_RXCOUNT_SHIFT));

	    while (count > 0)
	    {	/* If new data arrived. */
			uint8_t data = (uint8_t)LPUART1->DATA;

			real_midi_received(data);
	    	count--;
	    }
#else	// ..FIFO消化
		LPUART_ClearStatusFlags(LPUART1, kLPUART_RxDataRegFullFlag);	// clear status flag
		{
			/* If new data arrived. */
			uint8_t data;

			data = LPUART_ReadByte(LPUART1);
			real_midi_received(data);
		}
#endif	// FIFO消化
	}
	if ((uart_stat & kLPUART_TransmissionCompleteFlag) != 0)
	{
		LPUART_ClearStatusFlags(LPUART1, kLPUART_TransmissionCompleteFlag);	// clear status flag
		/**
		 * Rx割り込み時にも、uart_statにkLPUART_TransmissionCompleteFlagが立っているケースがある
		 * -> rxDataReceivedがANALYZING時だけ送信処理をして, FINISH_PROCESSING時は割り込みを止めておく
		 */
		switch (g_deviceComposite->midiPlayer.rxDataReceived) {
		case USB_RXDATA_STATE_SEQ_MIDIIF_ANALYZING:
			sendtoUartTx();
			break;
		case USB_RXDATA_STATE_SEQ_MIDIIF_FINISH_PROCESSING:
			LPUART_DisableInterrupts(LPUART1, kLPUART_TransmissionCompleteFlag);
			break;
		default:
			break;
		}
	}
}

/**
 * clearMidiRxEventParameter
 */
static void clearMidiRxEventParameter()
{
	{	 /* Short Message Event */
		setMidiShortEvent = 0;
		midiShortEventRdPtr = 0;
		midiShortEventWtPtr = 0;
	}
	{	/* SysEX Message Event */
		setMidiSysExEvent = 0;
		midiSysExEventRdPtr = 0;
		midiSysExEventWtPtr = 0;
	}

	forwarding_sysex = false;

	{	/* UART RXから受信したデータをanalyzeする際に必要な変数群(Short Message) */
		shortStopCount = 0;
		shortMsgCount = 0;
		runningStatusData = 0;
	}
	{	/* UART RXから受信したデータをanalyzeする際に必要な変数群(SysEX Message) */
		midiSysExMessageCurrentWriteBufPtr = 0;
		midiSysExMessageBufWtPtr = 0;
		midiSysExCurrentDataSize = 0;
	}
}

/**
 * real_midi_received
 */
static void setRealTimeMessage(uint8_t data);
static void setShortMessage(void);

static void setSysExMessage(uint16_t setNextBufWtPtr);

static void analyzeShortMessage(uint8_t data) {
	if (data >= 0xF8) {	/* F8~FF */
		setRealTimeMessage(data);	// Real Time Messageを作成
	}
	else if (data > 0xF0) {
		if (shortStopCount == 0) {
			if ((data == 0xF1) || (data == 0xF3)) {
				shortStopCount = 2;
			}
			else {
				shortStopCount = 3;
			}
			oneShortMessageBuf[0] = 0x0F;
			oneShortMessageBuf[1] = data;
			shortMsgCount = 1;
		}
		/* else : short messageの途中で来た場合は捨てる */
	}
	else {	/* data < 0xF0 */
		volatile uint8_t command;

		command = (data & 0xF0);
		if (command >= 0x80) {
			/*
			 * status byte受信 -> short message開始
			 * short message受信中にstatus buteが来た場合はそれまでの
			 * データを捨てて, 新しくshort messageを作り始める
			 */
			if ((command == 0xC0) || (command == 0xD0)) {
				shortStopCount = 2;
				oneShortMessageBuf[3] = 0x00;
			}
			else {
				shortStopCount = 3;
			}
			runningStatusData = data;	/* backup */
			oneShortMessageBuf[0] = (command >> 4);
			oneShortMessageBuf[1] = data;

			shortMsgCount = 1;
		}
		else {	/* data: 0x00 ~ 0x7F */
			if (shortStopCount == 0) {
				if (runningStatusData != 0) {
					/* running statusと認識 */
					oneShortMessageBuf[0] = (runningStatusData >> 4);
					oneShortMessageBuf[1] = runningStatusData;
					oneShortMessageBuf[2] = data;
					command = runningStatusData & 0xF0;
					if ((command == 0xC0) || (command == 0xD0)) {
						/* 2byte messageのRunningStatusはここでセット */
						oneShortMessageBuf[3] = 0x00;
						setShortMessage();
					}
					shortMsgCount = 2;
					shortStopCount = 3;	/* 2->3byteでメッセージを作り直す */
				}
				/* else : dataが壊れたみたいなので捨てる */
			}
			else {
				shortMsgCount++;
				oneShortMessageBuf[shortMsgCount] = data;
				if (shortStopCount <= shortMsgCount) {
					setShortMessage();
				}
			}
		}
	}
}

static void analyzeSysExMessage(uint8_t data) {
	if (forwarding_sysex == true) {
		volatile int type;

		/* type : 0,1,2の何れかとなる(3は飛ばすので出てこない) */
		type = (midiSysExCurrentDataSize % 0x04);

		if (data > 0xF7) {
			/* real time messageはSysEx中に混ざることがあり得るが, 全てdropする */
			return;
		}
		/// @note B2のソースの間違い部分を改定
		if (data == 0xF0) {
			/**
			 * F0->F0と来たけれども, 今まで貯めていたものをとりあえず出す
			 */
			{
				int i;

				*(UsbSysExMessageBuf + midiSysExMessageBufWtPtr) = 0;	/// @note dummy
				INC_DATA_POS(midiSysExMessageBufWtPtr, NUM_OF_USB_MIDI_SYSEX_EVENT_DATA_SIZE);
				midiSysExCurrentDataSize++;

				for (i = 0; i < (2 - type); i++) {
					*(UsbSysExMessageBuf + midiSysExMessageBufWtPtr) = 0;
					INC_DATA_POS(midiSysExMessageBufWtPtr, NUM_OF_USB_MIDI_SYSEX_EVENT_DATA_SIZE);
					midiSysExCurrentDataSize++;
				}
				{	/* 4つ前のデータ位置にセット */
					uint16_t decPtr;

					decPtr = midiSysExMessageBufWtPtr;
					for (i = 0; i < 4;i++) {
						DEC_DATA_POS(decPtr, NUM_OF_USB_MIDI_SYSEX_EVENT_DATA_SIZE);
					}
					*(UsbSysExMessageBuf + decPtr) = 0x04;			/// @note dummy
					midiSysExCurrentDataSize++;
				}
				{	/* set message */
					setSysExMessage(midiSysExMessageBufWtPtr);	/// @note Next:4byte mesageの先頭
				}
				{
					/**
					 * [ ][F0][ ][ ]
					 */
					{	/* F0をセット */
						INC_DATA_POS(midiSysExMessageBufWtPtr, NUM_OF_USB_MIDI_SYSEX_EVENT_DATA_SIZE);
						*(UsbSysExMessageBuf + midiSysExMessageBufWtPtr) = data;	// F0
						INC_DATA_POS(midiSysExMessageBufWtPtr, NUM_OF_USB_MIDI_SYSEX_EVENT_DATA_SIZE);
						midiSysExCurrentDataSize++;
					}
				}
			}
		}
		else {
			*(UsbSysExMessageBuf + midiSysExMessageBufWtPtr) = data;
			INC_DATA_POS(midiSysExMessageBufWtPtr, NUM_OF_USB_MIDI_SYSEX_EVENT_DATA_SIZE);
			midiSysExCurrentDataSize++;
			if (0x80 & data) {
				/* EOXかreal time messageを除く任意のstatus byteで終了する */
				int i;

				for (i = 0; i < (2 - type); i++) {
					*(UsbSysExMessageBuf + midiSysExMessageBufWtPtr) = 0;
					INC_DATA_POS(midiSysExMessageBufWtPtr, NUM_OF_USB_MIDI_SYSEX_EVENT_DATA_SIZE);
					midiSysExCurrentDataSize++;
				}
				{	/* 4つ前のデータ位置にセット */
					uint16_t decPtr;

					decPtr = midiSysExMessageBufWtPtr;
					for (i = 0; i < 4; i++) {
						DEC_DATA_POS(decPtr, NUM_OF_USB_MIDI_SYSEX_EVENT_DATA_SIZE);
					}
					*(UsbSysExMessageBuf + decPtr) = (0x05 + type);
					midiSysExCurrentDataSize++;
				}
				{	/* set message */
					setSysExMessage(midiSysExMessageBufWtPtr);	/// @note Next:4byte mesageの先頭
					forwarding_sysex = false;
				}
			}
			else {
				if (type == 2) {
					/**
					 * USB 1 event packetが埋まるので先頭に0x04を入力
					 * next event packeの先頭を空けて次のバイトから書くようにポインタを1進める
					 */
					volatile uint16_t nextMessageBufWtPtr;	/// @note USB FIFO Size以上の場合の対策

					nextMessageBufWtPtr = midiSysExMessageBufWtPtr;
					{	/* 4つ前のデータ位置にセット */
						uint16_t decPtr;
						int i;

						decPtr = midiSysExMessageBufWtPtr;
						for (i = 0; i < 4; i++) {
							DEC_DATA_POS(decPtr, NUM_OF_USB_MIDI_SYSEX_EVENT_DATA_SIZE);
						}
						*(UsbSysExMessageBuf + decPtr) = 0x04;
						midiSysExCurrentDataSize++;
						/// @note　[cin][data1][data2][data3] 今[cin]を指してるので[data1]を指すようにptrを1進める
						INC_DATA_POS(midiSysExMessageBufWtPtr, NUM_OF_USB_MIDI_SYSEX_EVENT_DATA_SIZE);
					}
					/**
					 * USB FIFO SIZE以上のデータが来た場合, 一度出力する必要あり
					 */
					if (midiSysExCurrentDataSize >= g_deviceComposite->midiPlayer.currentStreamMaxPacketSize) {
						setSysExMessage((uint16_t)nextMessageBufWtPtr);
					}
				}
			}
		}
	}
	else if (data == 0xF0) {
		forwarding_sysex = true;
		/**
		 * [cin][F0][data1][data2]
		 * F0を取得したので, midiSysExCurrentDataSize++
		 */
		INC_DATA_POS(midiSysExMessageBufWtPtr, NUM_OF_USB_MIDI_SYSEX_EVENT_DATA_SIZE);	// ->F0
		*(UsbSysExMessageBuf + midiSysExMessageBufWtPtr) = data;
		INC_DATA_POS(midiSysExMessageBufWtPtr, NUM_OF_USB_MIDI_SYSEX_EVENT_DATA_SIZE);	// [F0]->[ ]
		midiSysExCurrentDataSize++;
	}
}

static void real_midi_received(uint8_t data)
{
	if (data == 0xFE) {
		return;	/// @note 捨てる
	}

	if (forwarding_sysex) {
		/* SysExデータ受信中 */
		if (data > 0xF7) {
			// drop data
		}
		else {
			analyzeSysExMessage(data);
		}
	}
	else if (data == 0xF0) {
		analyzeSysExMessage(data);
	}
	else {	/* short message対応 */
		analyzeShortMessage(data);
	}
}

/* set RealTimeMessage */
static void setRealTimeMessage(uint8_t data)
{
	UsbShortEventPacketBuf[midiShortEventWtPtr].cin = 0x0F;
	UsbShortEventPacketBuf[midiShortEventWtPtr].midi0 = data;
	UsbShortEventPacketBuf[midiShortEventWtPtr].midi1 = 0;
	UsbShortEventPacketBuf[midiShortEventWtPtr].midi2 = 0;
	INC_DATA_POS(midiShortEventWtPtr, NUM_OF_USB_MIDI_SHORT_EVENT_BUFFER);

	setMidiShortEvent++;
	/**
	 * @note Overrunした場合
	 * => WARNING : UART RX割込み処理が, idle処理を上回っている..
	 */
	if (setMidiShortEvent > NUM_OF_USB_MIDI_SHORT_EVENT_BUFFER) {
		setMidiShortEvent -= NUM_OF_USB_MIDI_SHORT_EVENT_BUFFER;	// support overrun
	}
}

/* set ShortMessage */
static void setShortMessage()
{
	UsbShortEventPacketBuf[midiShortEventWtPtr].cin = oneShortMessageBuf[0];
	UsbShortEventPacketBuf[midiShortEventWtPtr].midi0 = oneShortMessageBuf[1];
	UsbShortEventPacketBuf[midiShortEventWtPtr].midi1 = oneShortMessageBuf[2];
	UsbShortEventPacketBuf[midiShortEventWtPtr].midi2 = oneShortMessageBuf[3];
	INC_DATA_POS(midiShortEventWtPtr, NUM_OF_USB_MIDI_SHORT_EVENT_BUFFER);

	shortMsgCount = 0;
	shortStopCount = 0;
	setMidiShortEvent++;
	/**
	 * @note Overrunした場合
	 * => WARNING : UART RX割込み処理が, idle処理を上回っている..
	 */
	if (setMidiShortEvent > NUM_OF_USB_MIDI_SHORT_EVENT_BUFFER) {
		setMidiShortEvent -= NUM_OF_USB_MIDI_SHORT_EVENT_BUFFER;	// support overrun
	}
}

static void setSysExMessage(uint16_t setNextBufWtPtr)
{
	UsbSysExEventPacketBuf[midiSysExEventWtPtr].bufPtr = (UsbSysExMessageBuf + midiSysExMessageCurrentWriteBufPtr);
	UsbSysExEventPacketBuf[midiSysExEventWtPtr].size = midiSysExCurrentDataSize;
	INC_DATA_POS(midiSysExEventWtPtr, NUM_OF_USB_MIDI_SYSEX_EVENT_BUFFER);

	/* Reset CurrentWriteBufPtr */
	midiSysExMessageCurrentWriteBufPtr = setNextBufWtPtr;
	midiSysExCurrentDataSize = 0;

	setMidiSysExEvent++;
	/**
	 * @note Overrunした場合
	 * => WARNING : UART RX割込み処理が, idle処理を上回っている
	 */
	if (setMidiSysExEvent > NUM_OF_USB_MIDI_SYSEX_EVENT_BUFFER) {
		setMidiSysExEvent -= NUM_OF_USB_MIDI_SYSEX_EVENT_BUFFER;	// support overrun
	}
}


/**
 * midi_IF_TxInit
 * TX pointer初期化
 * LPUART1 Txへデータを一つ送る & LPUART1 Tx割込み許可
 */
void midi_IF_TxInit()
{
	bool thruMidi;

	thruMidi = false;
	if (s_MidiIfTxNumOfData > 0) {
		thruMidi = true;
	}

	if (thruMidi) {
		s_MidiIfTxBuffPos = 0;
		startSendtoUartTx();
	}
	else {
		/**
		 * @note LPUARTへ送信するデータが無かった場合
		 */
		g_deviceComposite->midiPlayer.rxDataReceived = USB_RXDATA_STATE_SEQ_MIDIIF_FINISH_PROCESSING;
	}

	receiveSysExData = 0;	// reset
}

void midi_IF_TxProc_SysEx(uint8_t Data)
{
	receiveSysExData = 1;
	s_MidiIfTxBuff[s_MidiIfTxNumOfData++] = Data;
}

void midi_IF_TxProc_ReceiveByte(uint8_t Data)
{
	s_MidiIfTxBuff[s_MidiIfTxNumOfData++] = Data;
}

void midi_IF_RxInit()
{
	clearMidiRxEventParameter();
	LPUART_EnableInterrupts(LPUART1, kLPUART_RxDataRegFullFlag);
}

/**
 * MIDI IN => USB MIDI IN
 * check : 送信用バッファが用意されていたら送信
 * short -> sysexの順番. 同時には出さない
 * half fifo size未満だった場合, 送信用データを連続で出す可能性あり
 */
void MIDI_IF_IDLE()
{
	{
		int totalSize;
		int halfFifoSize;
		uint32_t usbMidiData;
		volatile uint8_t midiShortEventCount;
		volatile uint8_t midiSysExEventCount;
		volatile uint8_t setCount;

		 DisableIRQ(LPUART1_IRQn);
		 midiShortEventCount = setMidiShortEvent;
		 midiSysExEventCount = setMidiSysExEvent;
		 EnableIRQ(LPUART1_IRQn);

		halfFifoSize = (g_deviceComposite->midiPlayer.currentStreamMaxPacketSize >> 1);
		totalSize = 0;
		if (midiShortEventCount > 0) {
			setCount = 0;
			while (totalSize < halfFifoSize) {
				usbMidiData = UsbShortEventPacketBuf[midiShortEventRdPtr].cin;
				usbMidiData |= (UsbShortEventPacketBuf[midiShortEventRdPtr].midi0 << 8);
				usbMidiData |= (UsbShortEventPacketBuf[midiShortEventRdPtr].midi1 << 16);
				usbMidiData |= (UsbShortEventPacketBuf[midiShortEventRdPtr].midi2 << 24);
				WriteData4(usbMidiData);
				INC_DATA_POS(midiShortEventRdPtr, NUM_OF_USB_MIDI_SHORT_EVENT_BUFFER);
				setCount++;
				if (midiShortEventCount == setCount) {
					break;
				}
				totalSize += 4;
			}
			Send2USB();
			DisableIRQ(LPUART1_IRQn);
			setMidiShortEvent -= setCount;
			EnableIRQ(LPUART1_IRQn);
		}
		else if (midiSysExEventCount > 0) {
			int i;
			int bufSize;
			uint8_t* curBuf;

			setCount = 0;
			{
				bufSize = (UsbSysExEventPacketBuf[midiSysExEventRdPtr].size >> 2);
				curBuf = UsbSysExEventPacketBuf[midiSysExEventRdPtr].bufPtr;
				for (i = 0; i < bufSize; i++) {
					usbMidiData = (*curBuf);
					usbMidiData |= ((*(curBuf + 1)) << 8);
					usbMidiData |= ((*(curBuf + 2)) << 16);
					usbMidiData |= ((*(curBuf + 3)) << 24);
					curBuf = ((curBuf + 4) >= (UsbSysExMessageBuf + NUM_OF_USB_MIDI_SYSEX_EVENT_DATA_SIZE)) ? UsbSysExMessageBuf : curBuf + 4;
					WriteData4(usbMidiData);
				}
				INC_DATA_POS(midiSysExEventRdPtr, NUM_OF_USB_MIDI_SYSEX_EVENT_BUFFER);
				setCount++;
			}
			Send2USB();
			DisableIRQ(LPUART1_IRQn);
			setMidiSysExEvent -= setCount;
			EnableIRQ(LPUART1_IRQn);
		}
	}
}
