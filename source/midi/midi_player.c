/*
 * midi_player.c
 *
 *  Created on: 2020/02/06
 *      Author: higuchi
 */

#include "usb_device_config.h"
#include "usb.h"
#include "usb_device.h"

#include "usb_device_class.h"
#include "usb_device_midi.h"

#include "usb_device_ch9.h"
#include "usb_device_descriptor.h"

#include "../composite.h"
#include "midi_player.h"

#include "board.h"

#if SUPPORT_MIDI_IF
#include "midi_if.h"
#else
#include "midi_device.h"
#endif

#include "fsl_device_registers.h"
#include <stdio.h>
#include <stdlib.h>

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/* USB MIDI config*/
/*buffer size for USB MIDI */
#if SUPPORT_USB_HIGH_SPEED
#define HS_USB_DEVICE_MIDI_RX_BUFF_SIZE (HS_USB_MIDI_ENDP_MAX_PACKET_SIZE)
#define HS_USB_DEVICE_MIDI_TX_BUFF_SIZE (HS_USB_MIDI_ENDP_MAX_PACKET_SIZE * 4U)
#define HS_USB_DEVICE_MIDI_TX_BUFF_DWORD_SIZE (HS_USB_DEVICE_MIDI_TX_BUFF_SIZE >> 2)
#define HS_USB_DEVICE_MIDI_TX_FIFO_DWORD_SIZE (HS_USB_MIDI_ENDP_MAX_PACKET_SIZE >> 2)
#endif

#define FS_USB_DEVICE_MIDI_RX_BUFF_SIZE (USB_MIDI_ENDP_MAX_PACKET_SIZE)
#define FS_USB_DEVICE_MIDI_TX_BUFF_SIZE (USB_MIDI_ENDP_MAX_PACKET_SIZE * 16U)
#define FS_USB_DEVICE_MIDI_TX_BUFF_DWORD_SIZE (FS_USB_DEVICE_MIDI_TX_BUFF_SIZE >> 2)
#define FS_USB_DEVICE_MIDI_TX_FIFO_DWORD_SIZE (USB_MIDI_ENDP_MAX_PACKET_SIZE >> 2)


/*******************************************************************************
 * Variables
 ******************************************************************************/
#if SUPPORT_USB_HIGH_SPEED
USB_DMA_NONINIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE) uint32_t g_midiRxDataBuf[HS_USB_DEVICE_MIDI_RX_BUFF_SIZE >> 2];

USB_DMA_NONINIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE) uint32_t g_midiTxDataBuf[HS_USB_DEVICE_MIDI_TX_BUFF_DWORD_SIZE];
USB_DMA_NONINIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE) uint32_t g_midiFifoTxDataBuf[HS_USB_DEVICE_MIDI_TX_FIFO_DWORD_SIZE];
#else
USB_DMA_NONINIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE) uint32_t g_midiRxDataBuf[FS_USB_DEVICE_MIDI_RX_BUFF_SIZE >> 2];

USB_DMA_NONINIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE) uint32_t g_midiTxDataBuf[FS_USB_DEVICE_MIDI_TX_BUFF_DWORD_SIZE];
USB_DMA_NONINIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE) uint32_t g_midiFifoTxDataBuf[FS_USB_DEVICE_MIDI_TX_FIFO_DWORD_SIZE];
#endif

usb_midi_rx_buffer_struct_t g_midiRxBuffer;
usb_midi_tx_buffer_struct_t g_midiTxBuffer;
usb_midi_fifo_tx_buffer_struct_t g_midiFifoTxBuffer;

extern usb_device_composite_struct_t *g_deviceComposite;

/* USB MIDI IN用変数 */
static uint8_t MidiInOpen;		/* 1:Open, 0:Close */
static int	MidiInTimeout;
#define USBMIDI_IN_TIMEOUT	3	/* 100ms x 3 = 300ms */


#if SUPPORT_MIDI_IF
#define	TYPE_IGNORE				0
#define	TYPE_SYSEX_CONTINUE		1
#define	TYPE_SYSEX_COMMON_END	2
#define	TYPE_SYSEX_END			3
#define	TYPE_PASS				4

static const struct UsbMidiDecodeInfo {
	uint8_t type;
	uint8_t size;
} usbMidiDecodeInfo[] = {
	{TYPE_IGNORE,			0},	/*	0x0:	Miscellaneous function codes. Reserved for future extensions.				*/
	{TYPE_IGNORE,			0},	/*	0x1:	Cable events.Reserved for future expansion.									*/
	{TYPE_PASS,				2},	/*	0x2:	Two-byte System Common messages like  MTC, SongSelect, etc.					*/
	{TYPE_PASS,				3},	/*	0x3:	Three-byte System Common messages like SPP, etc.							*/
	{TYPE_SYSEX_CONTINUE,	3},	/*	0x4:	SysEx starts or continues													*/
	{TYPE_SYSEX_COMMON_END,	1},	/*	0x5:	Single-byte System Common Message SysEx ends with following single byte.	*/
	{TYPE_SYSEX_END,		2},	/*	0x6:	SysEx ends with following two bytes.										*/
	{TYPE_SYSEX_END,		3},	/*	0x7:	SysEx ends with following three bytes.										*/

	{TYPE_PASS,				3},	/*	0x8:	Note-off																	*/
	{TYPE_PASS,				3},	/*	0x9:	Note-on																		*/
	{TYPE_PASS,				3},	/*	0xA:	Poly-KeyPress																*/
	{TYPE_PASS,				3},	/*	0xB:	Control Change																*/
	{TYPE_PASS,				2},	/*	0xC:	Program Change																*/
	{TYPE_PASS,				2},	/*	0xD:	Channel Pressure															*/
	{TYPE_PASS,				3},	/*	0xE:	PitchBend Change															*/
	{TYPE_PASS,				1},	/*	0xF:	Single Byte																	*/
};

#define	STATE_NORMAL	0
#define	STATE_SYSEX		1
static uint8_t cableStatus[16];
#endif	// #if SUPPORT_MIDI_IF

/*******************************************************************************
 * Code
 ******************************************************************************/
/**
 * ・受信割り込みで受信したデータをg_midiRxBufferにセット
 * - 受信したら、rxDataReceivedを1に
 * - idleで受信データ解析し、処理する
 * ・送信データはg_midiTxBuffer(Circular buffer)で管理(idle上)
 * ・idle監視で送信可能なデータができたら、g_midiFifoTxBufferにセットして、送信割り込みを有効に
 * - g_midiTxBufferでバッファFullになりそうな場合は, txDataBufferFullを1に
 * - g_midiFifoTxBufferの準備が出来た場合は, txDataTransmissionReadyを1に
 */

/*!
 * @brief device MIDI callback function.
 *
 * This function handle the disk class specified event.
 * @param handle          The USB class  handle.
 * @param event           The USB device event type.
 * @param param           The parameter of the class specific event.
 * @return kStatus_USB_Success or error.
 */
usb_status_t USB_DeviceMidiCallback(class_handle_t handle, uint32_t event, void *param)
{
    usb_status_t error = kStatus_USB_Success;
    usb_device_endpoint_callback_message_struct_t *ep_cb_param;
    ep_cb_param = (usb_device_endpoint_callback_message_struct_t *)param;

    switch (event) {
    case kUSB_DeviceMidiEventStreamRecvResponse:
    	if ((g_deviceComposite->midiPlayer.attach) && (ep_cb_param->length != (USB_UNINITIALIZED_VAL_32)))
    	{
    		/**
    		 * rxDataReceivedフラグをセット. length値だけ覚えておく
    		 */
    		g_deviceComposite->midiPlayer.rxDataReceived = USB_RXDATA_STATE_SEQ_DATA_RECEIVED;
    		g_deviceComposite->midiPlayer.rxBuffer->Size = ep_cb_param->length;
    		error = kStatus_USB_Success;
    	}
    	break;
    case kUSB_DeviceMidiEventStreamSendResponse:
    	if ((g_deviceComposite->midiPlayer.attach) && (ep_cb_param->length != (USB_UNINITIALIZED_VAL_32)))
    	{
    		/**
    		 * txDataTransmissionCompletedフラグをセット. 送信長のカウントをリセット
    		 */
			g_deviceComposite->midiPlayer.txDataTransmissionCompleted = 1;
			g_deviceComposite->midiPlayer.fifoTxBuffer->Cnt = 0;
			error = kStatus_USB_Success;
    	}
    	break;
    default:
    	break;
    }

    return error;
}

/*!
 * @brief MIDI device set configuration function.
 *
 * This function sets configuration for MIDI class.
 *
 * @param handle The MIDI class handle.
 * @param configure The MIDI class configure index.
 *
 * @return A USB error code or kStatus_USB_Success.
 */
usb_status_t USB_DeviceMidiSetConfigure(class_handle_t handle, uint8_t configure)
{
    if (USB_AUDIO_MIDI_CONFIGURE_INDEX == configure)
    {
    	g_deviceComposite->midiPlayer.attach = 1U;
    	g_deviceComposite->midiPlayer.currentConfiguration = configure;

#if SUPPORT_MIDI_IF
    	/**
    	 * USBがConfiguration状態になったらMIDI RX側の割込みを許可する
    	 */
        /* Enable RX interrupt. */
        midi_IF_RxInit();

        NVIC_SetPriority(LPUART1_IRQn, UART_INTERRUPT_PRIORITY);
        EnableIRQ(LPUART1_IRQn);
#endif
    }
    return kStatus_USB_Success;
}

/*!
 * @brief device MIDI init function.
 *
 * This function initializes the device with the composite device class information.
 *
 * @param deviceComposite          The pointer to the composite device structure.
 * @return kStatus_USB_Success .
 */
usb_status_t USB_DeviceMidiPlayerInit(usb_device_composite_struct_t *device_composite)
{
	{
		g_midiRxBuffer.Buffer = g_midiRxDataBuf;
		g_midiRxBuffer.Size = 0;

		g_midiTxBuffer.Buffer = g_midiTxDataBuf;
		g_midiTxBuffer.Rd = 0;
		g_midiTxBuffer.Wr = 0;
		g_midiTxBuffer.Cnt = 0;

		g_midiFifoTxBuffer.Buffer = g_midiFifoTxDataBuf;
		g_midiFifoTxBuffer.Cnt = 0;
	}

	/// @note Audio側で設定済み
//	g_deviceComposite = device_composite;
	/// @note midiHandleはmain初期化時に設定済み
	g_deviceComposite->midiPlayer.deviceHandle = NULL;
	g_deviceComposite->midiPlayer.rxBuffer = &g_midiRxBuffer;
	g_deviceComposite->midiPlayer.txBuffer = &g_midiTxBuffer;
	g_deviceComposite->midiPlayer.fifoTxBuffer = &g_midiFifoTxBuffer;
	g_deviceComposite->midiPlayer.attach = 0;
	g_deviceComposite->midiPlayer.currentConfiguration = 0;
	g_deviceComposite->midiPlayer.rxDataReceived = USB_RXDATA_STATE_SEQ_MIDIIF_FINISH_PROCESSING;
	g_deviceComposite->midiPlayer.txDataBufferFull = 0;
	g_deviceComposite->midiPlayer.txDataTransmissionCompleted = 1;	/// @note defaultは1
	g_deviceComposite->midiPlayer.currentStreamMaxPacketSize = USB_MIDI_ENDP_MAX_PACKET_SIZE;
	g_deviceComposite->midiPlayer.speed = USB_SPEED_FULL;

	{	/* MIDI IN Open/Close検出 */
		MidiInOpen = 1;
		MidiInTimeout = 0;
	}

	return kStatus_USB_Success;
}

/*******************************************************************************
 * USB MIDI RX
 ******************************************************************************/
static uint8_t setOutCommand(uint32_t UsbMidiData)
{
#if SUPPORT_MIDI_IF
	uint8_t ret;
	uint8_t codeID;

	ret = 1;
	codeID = (uint8_t)(UsbMidiData & 0x000000FF);
	{
		const struct UsbMidiDecodeInfo *info = usbMidiDecodeInfo + (codeID & 0x0F);
		int cn = (codeID >> 4) & 0x0F;
		int i;

		switch (info->type) {
		case TYPE_PASS:
			cableStatus[cn] = STATE_NORMAL;
			for (i = 1; i <= info->size; i++) {
				midi_IF_TxProc_ReceiveByte((uint8_t)(UsbMidiData >> (i*8)));
			}
			break;
		case TYPE_SYSEX_CONTINUE:
		case TYPE_SYSEX_END:
			cableStatus[cn] = ((info->type) == TYPE_SYSEX_CONTINUE) ? STATE_SYSEX : STATE_NORMAL;
			for (i = 1; i <= info->size; i++) {
				midi_IF_TxProc_SysEx((uint8_t)(UsbMidiData >> (i*8)));
			}
			break;
		case TYPE_SYSEX_COMMON_END:
			if (cableStatus[cn] == STATE_SYSEX) {
				cableStatus[cn] = STATE_NORMAL;
				for (i = 1; i <= info->size; i++) {
					midi_IF_TxProc_SysEx((uint8_t)(UsbMidiData >> (i*8)));
				}
			}
			else {
				for (i = 1; i <= info->size; i++) {
					midi_IF_TxProc_ReceiveByte((uint8_t)(UsbMidiData >> (i*8)));
				}
			}
			break;
		default:
			break;
		}
	}

 #if 0
	ret = 1;
	codeID = (uint8_t)(UsbMidiData & 0x000000FF);
	if ((codeID & 0xF0) == 0) {
		/* Cable Number is 0 */
		switch (codeID) {
		/*
		case 0x00:	Miscellaneous function codes. Reserved for future extensions
		case 0x01:	Cable events. Reserved for future expansion
			break;
		*/
		case 0x04:	/* SysEx starts or continues */
			midi_IF_TxProc_SysEx(	(uint8_t)(UsbMidiData >> 8),
									(uint8_t)(UsbMidiData >> 16),
									(uint8_t)(UsbMidiData >> 24));
			break;
		case 0x05:	/* Single-byte System Common Message or SysEx ends with following single byte */
		case 0x0F:	/* Single Byte */
			midi_IF_TxProc_ReceiveByte((uint8_t)(UsbMidiData >> 8));
			break;
		case 0x06:	/* SysEx ends with following two bytes */
			{
				midi_IF_TxProc_ReceiveByte((uint8_t)(UsbMidiData >> 8));
				midi_IF_TxProc_ReceiveByte((uint8_t)(UsbMidiData >> 16));
			}
			break;
		case 0x07:	/* SysEx ends with following three bytes */
			{
				midi_IF_TxProc_ReceiveByte((uint8_t)(UsbMidiData >> 8));
				midi_IF_TxProc_ReceiveByte((uint8_t)(UsbMidiData >> 16));
				midi_IF_TxProc_ReceiveByte((uint8_t)(UsbMidiData >> 24));
			}
			break;
		default:
			break;
		}
	}
	/*　ケーブルナンバーがゼロ以外は捨てる */
 #endif	// #if 0

#else	// #if SUPPORT_MIDI_IF (MIDI_DEVICE)
	uint8_t	ret;
	uint8_t	codeID;

	ret = 1;
	codeID = (uint8_t)(UsbMidiData & 0x000000FF);
	if ((codeID & 0xF0) == 0) {
		/* Cable Number is 0 */
		switch (codeID) {
		/*
		case 0x00:	Miscellaneous function codes. Reserved for future extensions
		case 0x01:	Cable events. Reserved for future expansion
			break;
		*/
		case 0x04:	/* SysEx starts or continues */
			midi_AMP_RxProc_SysEx(	(uint8_t)(UsbMidiData >> 8),
									(uint8_t)(UsbMidiData >> 16),
									(uint8_t)(UsbMidiData >> 24));
			break;
		case 0x05:	/* Single-byte System Common Message or SysEx ends with following single byte */
		case 0x0F:	/* Single Byte */
			midi_AMP_RxProc_ReceiveByte((uint8_t)(UsbMidiData >> 8));
			break;
		case 0x06:	/* SysEx ends with following two bytes */
			{
				midi_AMP_RxProc_ReceiveByte((uint8_t)(UsbMidiData >> 8));
				midi_AMP_RxProc_ReceiveByte((uint8_t)(UsbMidiData >> 16));
			}
			break;
		case 0x07:	/* SysEx ends with following three bytes */
			{
				midi_AMP_RxProc_ReceiveByte((uint8_t)(UsbMidiData >> 8));
				midi_AMP_RxProc_ReceiveByte((uint8_t)(UsbMidiData >> 16));
				midi_AMP_RxProc_ReceiveByte((uint8_t)(UsbMidiData >> 24));
			}
			break;
		default:
			break;
		}
	}
	/*　ケーブルナンバーがゼロ以外は捨てる */

#endif	// #if SUPPORT_MIDI_IF (MIDI_DEVICE)

	return ret;
}

/* Idle routine */
static void USB_MIDI_OUT_IDLE(void)
{
#if SUPPORT_MIDI_IF
	volatile uint8_t rxMode;
	volatile bool finished;

	finished = false;
	rxMode = g_deviceComposite->midiPlayer.rxDataReceived;
	switch (rxMode) {
	case USB_RXDATA_STATE_SEQ_DATA_RECEIVED:
		{
			if (g_deviceComposite->midiPlayer.rxBuffer->Size > 0) {
				uint32_t* rxPtr;
				int i;

				i = 0;
				/* Size分解析する */
				while (g_deviceComposite->midiPlayer.rxBuffer->Size > 0) {
					rxPtr = (g_deviceComposite->midiPlayer.rxBuffer->Buffer + i);
					if (!setOutCommand(*rxPtr)) {
						break;
					}
					i++;
					g_deviceComposite->midiPlayer.rxBuffer->Size -= 4;
				}
			}
			/// @note LPUART1 Txへデータを一つ送る & LPUART1 Tx割込み許可
			midi_IF_TxInit();
		}
		break;
#if 0
	case USB_RXDATA_STATE_SEQ_MIDIIF_ANALYZING:
			if (midi_IF_TxFinished() == true) {
				finished = true;
			}
		break;
#endif
	case USB_RXDATA_STATE_SEQ_MIDIIF_FINISH_PROCESSING:
		finished = true;
		break;
	default:
		break;
	}

	if (finished == true) {
		usb_status_t error;

		/// @note usb interrupt disable
		USB_DeviceEhciMaskUsbInterruptEnable(g_deviceComposite->deviceHandle, 0);

		/*　受信できる状態をセット */
		{
			error = USB_DeviceMidiRecv(
						g_deviceComposite->midiPlayer.midiHandle,
						(uint8_t*)g_deviceComposite->midiPlayer.rxBuffer->Buffer,
						g_deviceComposite->midiPlayer.currentStreamMaxPacketSize);
			if (error == kStatus_USB_Success) {
				/*　準備できたらフラグをクリア */
				g_deviceComposite->midiPlayer.rxDataReceived = USB_RXDATA_STATE_SEQ_IDLE;
			}
		}

		/// @note usb interrupt enable
   		USB_DeviceEhciMaskUsbInterruptEnable(g_deviceComposite->deviceHandle, 1);
	}
#else	// #if SUPPORT_MIDI_IF
	/**
	 * 初期化時/受信完了割り込み後はrxDataReceivedは"1"
	 * ->　初期化時:受信できる状態をセットする
	 * ->　受信完了割り込み後:受信したデータを解析.　解析後, 次のデータを受信できる状態をセット
	 */
	if (g_deviceComposite->midiPlayer.rxDataReceived == USB_RXDATA_STATE_SEQ_DATA_RECEIVED) {
		if (g_deviceComposite->midiPlayer.rxBuffer->Size > 0) {
			uint32_t* rxPtr;
			int i;

			i = 0;
			/* Size分解析する */
			while (g_deviceComposite->midiPlayer.rxBuffer->Size > 0) {
				rxPtr = (g_deviceComposite->midiPlayer.rxBuffer->Buffer + i);
				if (!setOutCommand(*rxPtr)) {
					break;
				}
				i++;
				g_deviceComposite->midiPlayer.rxBuffer->Size -= 4;
			}
		}
		{
		    usb_status_t error;

		    /// @note usb interrupt disable
		    USB_DeviceEhciMaskUsbInterruptEnable(g_deviceComposite->deviceHandle, 0);

		    /*　受信できる状態をセット */
		    {
		    	error = USB_DeviceMidiRecv(
    					g_deviceComposite->midiPlayer.midiHandle,
    					(uint8_t*)g_deviceComposite->midiPlayer.rxBuffer->Buffer,
						g_deviceComposite->midiPlayer.currentStreamMaxPacketSize);
		    	if (error == kStatus_USB_Success) {
		    		/*　準備できたらフラグをクリア */
		    		g_deviceComposite->midiPlayer.rxDataReceived = USB_RXDATA_STATE_SEQ_IDLE;
		    	}
		    }

		    /// @note usb interrupt enable
		    USB_DeviceEhciMaskUsbInterruptEnable(g_deviceComposite->deviceHandle, 1);
		}
	}
#endif	// #if SUPPORT_MIDI_IF
}

/*******************************************************************************
 * USB MIDI Tx
 ******************************************************************************/
static uint8_t SendFill0;
#if SUPPORT_USB_HIGH_SPEED
static uint8_t zeroData[HS_USB_MIDI_ENDP_MAX_PACKET_SIZE] = {0};
#else	// #if SUPPORT_USB_HIGH_SPEED
static uint8_t zeroData[USB_MIDI_ENDP_MAX_PACKET_SIZE] = {0};
#endif	// #if SUPPORT_USB_HIGH_SPEED


/**
 * 0 fill dataを送信する
 */
static void sendFill0Data(void)
{
	if (g_deviceComposite->midiPlayer.txDataTransmissionCompleted == 1) {
	    usb_status_t error;

	    /// @note usb interrupt disable
	    USB_DeviceEhciMaskUsbInterruptEnable(g_deviceComposite->deviceHandle, 0);

	    {
	    	error = USB_DeviceMidiSend(
				g_deviceComposite->midiPlayer.midiHandle,
				(uint8_t*)zeroData,
				g_deviceComposite->midiPlayer.currentStreamMaxPacketSize);
	    	if (error == kStatus_USB_Success) {
	    		/* 準備できたらフラグをクリア */
	    		g_deviceComposite->midiPlayer.txDataTransmissionCompleted = 0;
	    	}
	    }

	    /// @note usb interrupt enable
		USB_DeviceEhciMaskUsbInterruptEnable(g_deviceComposite->deviceHandle, 1);
	}
}

void USB_setMidiInTimeoutCount()
{
	if (MidiInOpen) {
		if (MidiInTimeout < USBMIDI_IN_TIMEOUT) {
			MidiInTimeout++;
		}
	}
}

static void WatchUsbMidiInOpen(void)
{
	uint8_t MidiInOpenCheck = 0;

	if (MidiInOpen) {
		if (SendFill0) {
			if (MidiInTimeout >= USBMIDI_IN_TIMEOUT) {	/* 0Fillデータを送信後そのまま来た場合 */
				MidiInTimeout = 0;
				MidiInOpenCheck = 1;
			}
			else {
				MidiInTimeout = 0;
				SendFill0 = 0;
			}
		}
		else {
			if ((MidiInTimeout) >= USBMIDI_IN_TIMEOUT) {
				/* Timeoutしたら0 fill dataを送信 */
				sendFill0Data();
				SendFill0 = 1;
			}
		}
	}
	else {	/* MIDI Close時 */
		MidiInOpenCheck = 1;
	}

	if (MidiInOpenCheck) {
		/// @note USB "Configured"状態じゃないとここにはこないことが前提
		if (g_deviceComposite->midiPlayer.txDataTransmissionCompleted == 1) {
			if (SendFill0) {
				// Timeoutで0FillデータをFIFOへ送って、現在FIFOがFullじゃない->データがPCへ送信された。つまりMIDI Openな状態
				SendFill0 = 0;
			}
			MidiInOpen = 1;
		}
		else {
			// 1)MIDI Close時にまだClose状態の時
			// 2)Timeoutで0Fillデータを送ったFIFOの中にまだ0Fillデータが残っている状態。=> MIDI Close状態と判断
			if (MidiInOpen == 1) {
				MidiInOpen = 0;
			}
		}
	}
}

static uint16_t INC_IN_BUF_POS(uint16_t a) {
	uint16_t ret;
	uint32_t size;

#if SUPPORT_USB_HIGH_SPEED
	size = (g_deviceComposite->midiPlayer.speed == USB_SPEED_HIGH) ? HS_USB_DEVICE_MIDI_TX_BUFF_DWORD_SIZE : FS_USB_DEVICE_MIDI_TX_BUFF_DWORD_SIZE;
#else
	size = FS_USB_DEVICE_MIDI_TX_BUFF_DWORD_SIZE;
#endif

	ret = (a != (size-1)) ? (a+1) : 0;
	return ret;
}

/**
 * g_midiTxDataBufに積んでいく
 * ret : Status(0=成功, 1=Bufferが空いていない, FF=USB Not Ready
 */
uint8_t WriteData4(uint32_t UsbMidiData)
{
	uint8_t ret = USB_RSLT_NOTRDY;
	uint32_t size;

#if SUPPORT_USB_HIGH_SPEED
	size = (g_deviceComposite->midiPlayer.speed == USB_SPEED_HIGH) ? HS_USB_DEVICE_MIDI_TX_BUFF_DWORD_SIZE : FS_USB_DEVICE_MIDI_TX_BUFF_DWORD_SIZE;
#else
	size = FS_USB_DEVICE_MIDI_TX_BUFF_DWORD_SIZE;
#endif
	if (g_deviceComposite->midiPlayer.attach) {
		if (MidiInOpen) {
			if (g_deviceComposite->midiPlayer.txBuffer->Cnt < size)
			{
				/*　貯蓄 */
				*(g_deviceComposite->midiPlayer.txBuffer->Buffer + g_deviceComposite->midiPlayer.txBuffer->Wr) = UsbMidiData;
				g_deviceComposite->midiPlayer.txBuffer->Wr = INC_IN_BUF_POS(g_deviceComposite->midiPlayer.txBuffer->Wr);
				g_deviceComposite->midiPlayer.txBuffer->Cnt++;
				ret = USB_RSLT_OK;
			}
			else {
				ret = USB_RSLT_BUFFULL;
			}
		}
	}

	return ret;
}

/**
 * g_midiTxDataBufのデータをg_midiFifoTxDataBufに移す
 * 途中F7があったりしたら, 0FillしてtxDataTransmissionReadyを1にして, 送信割り込みを有効にする
 */
uint8_t Send2USB(void)
{
	uint8_t ret = USB_RSLT_NOTRDY;
	uint32_t size;
	uint32_t fifoSize;

#if SUPPORT_USB_HIGH_SPEED
	size = (g_deviceComposite->midiPlayer.speed == USB_SPEED_HIGH) ? HS_USB_DEVICE_MIDI_TX_BUFF_DWORD_SIZE : FS_USB_DEVICE_MIDI_TX_BUFF_DWORD_SIZE;
	fifoSize = (g_deviceComposite->midiPlayer.speed == USB_SPEED_HIGH) ? HS_USB_DEVICE_MIDI_TX_FIFO_DWORD_SIZE : FS_USB_DEVICE_MIDI_TX_FIFO_DWORD_SIZE;
#else
	size = FS_USB_DEVICE_MIDI_TX_BUFF_DWORD_SIZE;
	fifoSize = FS_USB_DEVICE_MIDI_TX_FIFO_DWORD_SIZE;
#endif

	if (g_deviceComposite->midiPlayer.attach) {
		if (MidiInOpen) {
			/**
			 * 次の送信データの準備ができた場合, 送信用データをセットして送信準備
			 */
			if ((g_deviceComposite->midiPlayer.txDataTransmissionCompleted == 1) &&
				(g_deviceComposite->midiPlayer.txBuffer->Cnt > 0)) {
				uint8_t sendOK = 0;
				int i;

				i = 0;
				g_deviceComposite->midiPlayer.fifoTxBuffer->Cnt = size;
				while (g_deviceComposite->midiPlayer.fifoTxBuffer->Cnt > 0) {
					if (g_deviceComposite->midiPlayer.txBuffer->Cnt > 0) {
						g_deviceComposite->midiPlayer.fifoTxBuffer->Buffer[i++] =
								*(g_deviceComposite->midiPlayer.txBuffer->Buffer + g_deviceComposite->midiPlayer.txBuffer->Rd);
						g_deviceComposite->midiPlayer.txBuffer->Rd = INC_IN_BUF_POS(g_deviceComposite->midiPlayer.txBuffer->Rd);

						g_deviceComposite->midiPlayer.txBuffer->Cnt--;
						sendOK = 1;
						g_deviceComposite->midiPlayer.fifoTxBuffer->Cnt--;
					}
					else {
						break;
					}
				}
				if (sendOK) {
					if (g_deviceComposite->midiPlayer.fifoTxBuffer->Cnt > 0) {
						/* MaxPacketサイズ未満のデータの場合、残り容量は0 Fillして送信する */
						for (;i < fifoSize; i++) {
							g_deviceComposite->midiPlayer.fifoTxBuffer->Buffer[i] = 0;
						}
					}
					{
						usb_status_t error;

						/// @note usb interrupt disable
						USB_DeviceEhciMaskUsbInterruptEnable(g_deviceComposite->deviceHandle, 0);

						error = USB_DeviceMidiSend(
		        		 	 g_deviceComposite->midiPlayer.midiHandle,
							 (uint8_t*)g_deviceComposite->midiPlayer.fifoTxBuffer->Buffer,
							 g_deviceComposite->midiPlayer.currentStreamMaxPacketSize);
						if (error == kStatus_USB_Success) {
							g_deviceComposite->midiPlayer.txDataTransmissionCompleted = 0;
							MidiInTimeout = 0;
							ret = USB_RSLT_OK;
						}
						/* errorだった場合, 送信されずにデータが捨てられる形になる */

						/// @note usb interrupt enable
						USB_DeviceEhciMaskUsbInterruptEnable(g_deviceComposite->deviceHandle, 1);
					}
				}
			}
		}
	}

	return ret;
}

int isMidiInOpen(void)
{
	return MidiInOpen;
}

/* Idle routine */
static void USB_MIDI_IN_IDLE(void)
{
	/* MIDI IN Openチェック */
	WatchUsbMidiInOpen();

	/* MIDI TX Bufferの状態をチェック */
	Send2USB();
}

void USB_MIDI_IDLE()
{
	USB_MIDI_IN_IDLE();
	USB_MIDI_OUT_IDLE();
}

