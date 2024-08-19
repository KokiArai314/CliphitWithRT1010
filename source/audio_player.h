/*
 * audio_player.h
 *
 *  Created on: 2020/02/06
 *      Author: higuchi
 */

#ifndef AUDIO_PLAYER_H_
#define AUDIO_PLAYER_H_

#include <stdint.h>
#include "usb.h"

#include "usb_device_config.h"
#include "usb_device.h"
#include "usb_device_class.h"
#include "usb_device_descriptor.h"


/*******************************************************************************
* Definitions
******************************************************************************/
#if 0	/// @note support composite MIDI
 #if defined(USB_DEVICE_CONFIG_EHCI) && (USB_DEVICE_CONFIG_EHCI > 0U)
#define CONTROLLER_ID kUSB_ControllerEhci0
 #endif
 #if defined(USB_DEVICE_CONFIG_KHCI) && (USB_DEVICE_CONFIG_KHCI > 0U)
#define CONTROLLER_ID kUSB_ControllerKhci0
 #endif
 #if defined(USB_DEVICE_CONFIG_LPCIP3511FS) && (USB_DEVICE_CONFIG_LPCIP3511FS > 0U)
#define CONTROLLER_ID kUSB_ControllerLpcIp3511Fs0
 #endif
 #if defined(USB_DEVICE_CONFIG_LPCIP3511HS) && (USB_DEVICE_CONFIG_LPCIP3511HS > 0U)
#define CONTROLLER_ID kUSB_ControllerLpcIp3511Hs0
 #endif
#endif

/// @note support 44100Hz, support SOF
#define AUDIO_SAMPLING_RATE_KHZ (44)
#define AUDIO_SAMPLING_RATE_16KHZ (16)
#define AUDIO_SAMPLING_RATE (44100)

#define AUDIO_RECORDER_DATA_WHOLE_BUFFER_LENGTH (4 * 2)
#define AUDIO_SPEAKER_DATA_WHOLE_BUFFER_LENGTH (4 * 2)

#define AUDIO_BUFFER_UPPER_LIMIT(x) (((x)*5) / 8)
#define AUDIO_BUFFER_LOWER_LIMIT(x) (((x)*3) / 8)
#define AUDIO_CALCULATE_Ff_INTERVAL (1024)
#define TSAMFREQ2BYTES(f) (f & 0xFFU), ((f >> 8U) & 0xFFU), ((f >> 16U) & 0xFFU)
#define TSAMFREQ2BYTESHS(f) (f & 0xFFU), ((f >> 8U) & 0xFFU), ((f >> 16U) & 0xFFU), ((f >> 24U) & 0xFFU)
#define AUDIO_ADJUST_MIN_STEP (0x10)

#define MUTE_CODEC_TASK (1UL << 0U)
#define UNMUTE_CODEC_TASK (1UL << 1U)
#define VOLUME_CHANGE_TASK (1UL << 2U)

#if 1	/// @note support composite MIDI
#else
#define USB_DEVICE_INTERRUPT_PRIORITY (3U)
#endif

typedef struct _usb_audio_player_struct
{
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

#endif /* AUDIO_PLAYER_H_ */
