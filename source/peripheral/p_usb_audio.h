/*
 * p_usb_audio.h
 *
 *  Created on: 2024/11/14
 *      Author: koki_arai
 */

#ifndef PERIPHERAL_P_USB_AUDIO_H_
#define PERIPHERAL_P_USB_AUDIO_H_

#include <stdint.h>

#include "p_usb.h"
#include "usb_device.h"
#include "usb_device_class.h"
#include "usb_device_config.h"
#include "usb_device_descriptor.h"

/// @note support 44100Hz, support SOF

#define AUDIO_SAMPLING_RATE (44100)
#define AUDIO_RECORDER_DATA_WHOLE_BUFFER_LENGTH (4 * 2)
#define AUDIO_SPEAKER_DATA_WHOLE_BUFFER_LENGTH (4 * 2)

#define TSAMFREQ2BYTES(f) (f & 0xFFU), ((f >> 8U) & 0xFFU), ((f >> 16U) & 0xFFU)
#define MUTE_CODEC_TASK (1UL << 0U)
#define UNMUTE_CODEC_TASK (1UL << 1U)
#define VOLUME_CHANGE_TASK (1UL << 2U)

#define AUDIO_UPDATE_FEEDBACK_DATA(m, n) \
  {                                      \
    m[0] = (n & 0xFFU);                  \
    m[1] = ((n >> 8U) & 0xFFU);          \
    m[2] = ((n >> 16U) & 0xFFU);         \
  }

#define USB_AUDIO_ENTER_CRITICAL() \
                                   \
  USB_OSA_SR_ALLOC();              \
                                   \
  USB_OSA_ENTER_CRITICAL()

#define USB_AUDIO_EXIT_CRITICAL() USB_OSA_EXIT_CRITICAL()

/// @note support 24bit 44100Hz
#define AUDIO_DATA_BUFF_32BIT_ALIGN_SIZE (AUDIO_SAMPLING_RATE_KHZ * AUDIO_FORMAT_CHANNELS * 4) // 4=32bit align
#define AUDIO_DATA_BUFF_32BIT_ALIGN_SIZE_45SAMPLES (((AUDIO_SAMPLING_RATE_KHZ + 1) * AUDIO_FORMAT_CHANNELS * 4))
//#define AUDIO_DATA_BUFF_FORMAT_SIZE_45SAMPLES (((AUDIO_SAMPLING_RATE_KHZ+1) * AUDIO_FORMAT_CHANNELS * AUDIO_FORMAT_SIZE))

#define AUDIO_IN_DATA_BUFF_FORMAT_SIZE_45SAMPLES \
  (((AUDIO_SAMPLING_RATE_KHZ + 1) * AUDIO_IN_FORMAT_CHANNELS * AUDIO_FORMAT_SIZE))

#define PLAY_DATA_BUFF_SIZE (AUDIO_SPEAKER_DATA_WHOLE_BUFFER_LENGTH * AUDIO_DATA_BUFF_32BIT_ALIGN_SIZE_45SAMPLES)
#define REC_DATA_BUFF_SIZE (AUDIO_RECORDER_DATA_WHOLE_BUFFER_LENGTH * AUDIO_IN_DATA_BUFF_FORMAT_SIZE_45SAMPLES)



void usbAudioIncLRClockCount(void);

#endif /* PERIPHERAL_P_USB_AUDIO_H_ */
