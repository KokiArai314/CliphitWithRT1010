/*
 * p_usb.h
 *
 *  Created on: 2024/10/03
 *      Author: koki_arai
 */

#ifndef PERIPHERAL_P_USB_H_
#define PERIPHERAL_P_USB_H_

#include "usb_device_config.h"
#include "usb.h"
#include "usb_device.h"

#include "usb_device_class.h"
#include "usb_device_audio.h"

#include <usb_device_ch9.h>
#include "usb_device_descriptor.h"

#include <usb_device_midi.h>

#include "midi/midi_player.h"
#include "p_usb_audio.h"

/*******************************************************************************
* Definitions
******************************************************************************/

#define USB_DEVICE_INTERFACE_COUNT (2U) /// @note USB Audio Play/Rec

#if defined(USB_DEVICE_CONFIG_EHCI) && (USB_DEVICE_CONFIG_EHCI > 0U)
#define CONTROLLER_ID kUSB_ControllerEhci0
#endif
/**
 * USB Audio
 */
typedef struct _usb_audio_player_struct {
  usb_device_handle deviceHandle; /* USB device handle.                   */
  class_handle_t audioHandle;     /* USB AUDIO GENERATOR class handle.    */
  uint32_t currentStreamOutMaxPacketSize;
  uint32_t currentFeedbackMaxPacketSize;
  uint8_t copyProtect;
  uint8_t curMute;
  uint8_t curVolume[2]; /* need to consider the endians */
  uint8_t minVolume[2]; /* need to consider the endians */
  uint8_t maxVolume[2]; /* need to consider the endians */
  uint8_t resVolume[2]; /* need to consider the endians */
  uint8_t curBass;
  uint8_t minBass;
  uint8_t maxBass;
  uint8_t resBass;
  uint8_t curMid;
  uint8_t minMid;
  uint8_t maxMid;
  uint8_t resMid;
  uint8_t curTreble;
  uint8_t minTreble;
  uint8_t maxTreble;
  uint8_t resTreble;
  uint8_t curAutomaticGain;
  uint8_t curDelay[2]; /* need to consider the endians */
  uint8_t minDelay[2]; /* need to consider the endians */
  uint8_t maxDelay[2]; /* need to consider the endians */
  uint8_t resDelay[2]; /* need to consider the endians */
  uint8_t curLoudness;
  uint8_t curSamplingFrequency[3]; /* need to consider the endians */
  uint8_t minSamplingFrequency[3]; /* need to consider the endians */
  uint8_t maxSamplingFrequency[3]; /* need to consider the endians */
  uint8_t resSamplingFrequency[3]; /* need to consider the endians */
#if USBCFG_AUDIO_CLASS_2_0
  uint32_t curSampleFrequency;
  uint8_t curClockValid;
  usb_device_control_range_struct_t controlRange;

#endif
  uint8_t currentConfiguration;
  uint8_t currentInterfaceAlternateSetting[USB_AUDIO_PLAYER_INTERFACE_COUNT];
  uint8_t speed;
  uint8_t attach;
  volatile uint8_t startPlay;
  volatile uint8_t startPlayHalfFull;
  volatile uint32_t tdReadNumberPlay;
  volatile uint32_t tdWriteNumberPlay;
  volatile uint32_t audioSendCount;
  volatile uint32_t lastAudioSendCount;
  volatile uint32_t usbRecvCount;
  volatile uint32_t audioSendTimes;
  volatile uint32_t usbRecvTimes;
  volatile uint32_t speakerIntervalCount;
  volatile uint32_t speakerReservedSpace;
  volatile uint32_t timesFeedbackCalculate;
  volatile uint32_t speakerDetachOrNoInput;
  volatile uint32_t codecTask;

  /// @note support AudioRec
  uint32_t currentStreamInMaxPacketSize;
  volatile uint8_t startRec;
  volatile uint8_t startRecHalfFull;
  volatile uint32_t tdReadNumberRec;
  volatile uint32_t tdWriteNumberRec;
} usb_audio_player_struct_t;

/**
 * USB Midi
 */

typedef struct _usb_midi_rx_buffer_struct {
  uint32_t* Buffer;
  uint16_t Size;
} usb_midi_rx_buffer_struct_t;

typedef struct _usb_midi_tx_buffer_struct {
  uint32_t* Buffer;
  uint16_t Wr;
  uint16_t Rd;
  uint16_t Cnt;
} usb_midi_tx_buffer_struct_t;

typedef struct _usb_midi_fifo_tx_buffer_struct {
  uint32_t* Buffer;
  uint16_t Cnt;
} usb_midi_fifo_tx_buffer_struct_t;

enum usbRxDataStateSequence {
  USB_RXDATA_STATE_SEQ_IDLE = 0,
  USB_RXDATA_STATE_SEQ_DATA_RECEIVED,

  /* MIDI I/F */
  USB_RXDATA_STATE_SEQ_MIDIIF_ANALYZING,
  USB_RXDATA_STATE_SEQ_MIDIIF_FINISH_PROCESSING
};

typedef struct _usb_midi_player_struct {
  usb_device_handle deviceHandle; /* USB device handle.                   */
  class_handle_t midiHandle;      /* USB MIDI GENERATOR class handle.    */
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
  uint8_t txDataBufferFull;            /* txBuffer->Full:1, else 0 */
  uint8_t txDataTransmissionCompleted; /* fifoTxBuffer->TxCompleted:1, else 0 */
} usb_midi_player_struct_t;

/**
 * Composite Midi and Audio
 */

typedef struct _usb_device_composite_struct {
  usb_device_handle deviceHandle;
  usb_audio_player_struct_t audioPlayer;
  usb_midi_player_struct_t midiPlayer;
  uint8_t speed;
  uint8_t attach;
  uint8_t currentConfiguration;
  uint8_t currentInterfaceAlternateSetting[USB_DEVICE_INTERFACE_COUNT];
} usb_device_composite_struct_t;

extern usb_device_class_struct_t g_UsbDeviceAudioClass;
extern usb_device_class_struct_t g_UsbDeviceMidiClass;

/*******************************************************************************
* API
******************************************************************************/
/* AUDIO */
usb_status_t USB_DeviceAudioCallback(class_handle_t handle, uint32_t event, void* param);

usb_status_t USB_DeviceAudioSetConfigure(class_handle_t handle, uint8_t configure);

usb_status_t USB_DeviceAudioSetInterface(class_handle_t handle, uint8_t interface, uint8_t alternateSetting);

usb_status_t USB_DeviceAudioPlayerInit(usb_device_composite_struct_t* device_composite);

/* MIDI */
usb_status_t USB_DeviceMidiCallback(class_handle_t handle, uint32_t event, void* param);

usb_status_t USB_DeviceMidiSetConfigure(class_handle_t handle, uint8_t configure);

usb_status_t USB_DeviceMidiPlayerInit(usb_device_composite_struct_t* device_composite);

void checkAttachedDevice(void);
void usbInit(void);

#endif /* PERIPHERAL_P_USB_H */
