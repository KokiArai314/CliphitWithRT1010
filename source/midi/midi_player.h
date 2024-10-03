/*
 * midi_player.h
 *
 *  Created on: 2020/02/06
 *      Author: higuchi
 */

#ifndef MIDI_PLAYER_H_
#define MIDI_PLAYER_H_

#include <stdint.h>
#include "usb.h"

#include "usb_device_config.h"
#include "usb_device.h"
#include "usb_device_class.h"
#include "usb_device_descriptor.h"

/*******************************************************************************
* Definitions
******************************************************************************/

typedef struct _usb_midi_rx_buffer_struct
{
	uint32_t*	Buffer;
	uint16_t	Size;
} usb_midi_rx_buffer_struct_t;

typedef struct _usb_midi_tx_buffer_struct
{
	uint32_t*	Buffer;
	uint16_t	Wr;
	uint16_t	Rd;
	uint16_t	Cnt;
} usb_midi_tx_buffer_struct_t;

typedef struct _usb_midi_fifo_tx_buffer_struct
{
	uint32_t*	Buffer;
	uint16_t	Cnt;
} usb_midi_fifo_tx_buffer_struct_t;

enum usbRxDataStateSequence {
	USB_RXDATA_STATE_SEQ_IDLE = 0,
	USB_RXDATA_STATE_SEQ_DATA_RECEIVED,

	/* MIDI I/F */
	USB_RXDATA_STATE_SEQ_MIDIIF_ANALYZING,
	USB_RXDATA_STATE_SEQ_MIDIIF_FINISH_PROCESSING
};

typedef struct _usb_midi_player_struct
{
	usb_device_handle deviceHandle;	/* USB device handle.                   */
	class_handle_t midiHandle;		/* USB MIDI GENERATOR class handle.    */
	usb_midi_rx_buffer_struct_t* rxBuffer;
	usb_midi_tx_buffer_struct_t* txBuffer;
	usb_midi_fifo_tx_buffer_struct_t* fifoTxBuffer;
    uint8_t currentConfiguration;
    uint8_t attach;
    uint8_t speed;
    uint32_t currentStreamMaxPacketSize;

    /**
     * rxDataReceived
     * case of
     * 1. MIDIDevice : rxBuffer->received => rxDataReceived=1
     *                 rxBuffer Data finished processing => rxdataReceived=0
     * 2. MIDI IF :    rxBuffer->received => rxDataReceived=1
     *                 rxBuffer Data : analyzing => rxDataReceived = 2
     *                 rxBuffer Data finished processing => rxDataReceived=3
     *                 USB MIDI Out prepared OK => rxDataReceived=0
     */
    uint8_t rxDataReceived;
    uint8_t txDataBufferFull;				/* txBuffer->Full:1, else 0 */
    uint8_t txDataTransmissionCompleted;	/* fifoTxBuffer->TxCompleted:1, else 0 */
} usb_midi_player_struct_t;

/*******************************************************************************
* API
******************************************************************************/

/* Idle routine */
void USB_MIDI_IDLE(void);

/* MIDI送信関連 */
#define	USB_RSLT_OK			(0x00)
#define	USB_RSLT_BUFFULL	(0x01)
#define	USB_RSLT_FIFOFULL	(0x01)
#define	USB_RSLT_NOTRDY		(0xFF)

uint8_t WriteData4(uint32_t UsbMidiData);
uint8_t Send2USB(void);

#endif /* MIDI_PLAYER_H_ */

