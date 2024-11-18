/*
 * p_usb_audio.c
 *
 *  Created on: 2024/11/14
 *      Author: koki_arai
 */

#include "p_usb_audio.h"

#include <stdio.h>
#include <stdlib.h>

//#include "audio_task.h"
#include "board.h"
#include "clock_config.h"
#include "definitions.h"
#include "fsl_device_registers.h"

#include "peripheral/p_adc.h"

#include "usb_device_config.h"
#include "usb_device.h"
#include "usb_device_class.h"
#include "usb_device_audio.h"
#include "usb_device_ch9.h"
#include "usb_device_descriptor.h"

#if defined(USB_DEVICE_CONFIG_EHCI) && (USB_DEVICE_CONFIG_EHCI > 0U)
#include "usb_phy.h"
#endif

#include "fsl_dmamux.h"
#include "fsl_sai.h"
#include "fsl_sai_edma.h"
#include "fsl_wm8960.h"
#include "pin_mux.h"

extern usb_device_composite_struct_t *g_deviceComposite;

static uint8_t audioRecDataBufRdPtr; // 0 - AUDIO_REC_DATA_BUF_NUM-1
static uint8_t audioRecDataBufWrPtr; // 0 - AUDIO_REC_DATA_BUF_NUM-1

/// @note support SOF
uint8_t startLRClockCount; /// @note Play/RecのどちらかでSetInterface==1のとき1, Play/RecどちらもSetInterface==0のとき0
static uint32_t beforeLRClockCount;
static uint32_t LRClockCount; /// @note SAIの1sample割り込み時にカウント+1

/* Parameter */
static uint32_t output_samples_per_feedback_span;
static uint32_t feedback_value; /* Q10.14 */
//static int feedback_total;

static int8_t feedback_adjustment_sample; /// @note  1 or 0 or -1

#define HISTORY_SIZE 64
#define HISTORY_SIZE_LOG2 6

static uint16_t feedback_history[HISTORY_SIZE];
static int history_index;

/// @note add AudioRec
/* Feedback情報を元に録音側のSample数を決める為の変数 */
static uint32_t feedback_rec_value;
static uint8_t feedback_rec_value_count;
static uint32_t feedback_rec_sub_value;
static uint32_t feedback_rec_mod_value;
static uint32_t feedback_rec_current_value;

static uint8_t initialize_feedback_count_play;
static uint8_t initialize_feedback_count_rec;

/// @note audioPlayDataBuff 32bit align
USB_DMA_NONINIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE)
uint8_t audioPlayDataBuff[PLAY_DATA_BUFF_SIZE] __attribute__((aligned(4)));

USB_DMA_NONINIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE)
uint8_t audioPlayPacket[FS_ISO_OUT_ENDP_PACKET_SIZE + AUDIO_FORMAT_CHANNELS * AUDIO_FORMAT_SIZE];

USB_DMA_NONINIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE)
uint8_t audioFeedBackBuffer[3];

/// @note add AudioRec
USB_DMA_NONINIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE)
/// @note memo: 24bit L/R data
uint8_t audioRecDataBuff[REC_DATA_BUFF_SIZE] __attribute__((aligned(4)));

#define AUDIO_REC_DATA_BUF_NUM 2
USB_DMA_NONINIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE)
uint8_t audioRecDataBuf1[FS_ISO_IN_ENDP_PACKET_SIZE + AUDIO_IN_FORMAT_CHANNELS * AUDIO_FORMAT_SIZE]
    __attribute__((aligned(4)));
USB_DMA_NONINIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE)
uint8_t audioRecDataBuf2[FS_ISO_IN_ENDP_PACKET_SIZE + AUDIO_IN_FORMAT_CHANNELS * AUDIO_FORMAT_SIZE]
    __attribute__((aligned(4)));
USB_DMA_NONINIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE)
uint16_t audioRecDataBufSize[AUDIO_REC_DATA_BUF_NUM];

/// @note support SOF
static uint32_t getLRClockCount(void) { return LRClockCount; }
void usbAudioIncLRClockCount(void) { LRClockCount++; }

static void initializeFeedbackValue(void) {
  { startLRClockCount = 0; }
#if 1 /// @note support 44100Hz
  {
    volatile uint32_t mod;
    volatile uint32_t cnt;

    mod = 0;
    for (int i = 1; i <= HISTORY_SIZE; i++) {
      feedback_history[i - 1] = 704; /* 44 * 16*/ /// @note Feedback_EP_16ms_Interval
      {
        cnt = (i * 16) / 10;
        cnt -= mod;
        mod += cnt;
        feedback_history[i - 1] += cnt;
      }
    }
    feedback_value = 0x0B0666; /* 44.1 * 16384 = 722534.4 => 0B0666 */
    history_index = 0;
  }
#endif
  AUDIO_UPDATE_FEEDBACK_DATA(audioFeedBackBuffer, feedback_value);
}

/// @note add AudioRec
static uint8_t *getUsbAudioInBufPtr(uint8_t select) {
  uint8_t *ret;

  ret = NULL;
  switch (select) {
    case 0:
      ret = audioRecDataBuf1;
      break;
    case 1:
      ret = audioRecDataBuf2;
      break;
    default:
      break;
  }
  return ret;
}

void BOARD_EnableSaiMclkOutput(bool enable) {
  if (enable) {
    IOMUXC_GPR->GPR1 |= IOMUXC_GPR_GPR1_SAI1_MCLK_DIR_MASK;
  } else {
    IOMUXC_GPR->GPR1 &= (~IOMUXC_GPR_GPR1_SAI1_MCLK_DIR_MASK);
  }
}

/*!
 * @brief Audio class specific request function.
 *
 * This function handles the Audio class specific requests.
 *
 * @param handle		  The Audio class handle.
 * @param event 		  The Audio class event type.
 * @param param 		  The parameter of the class specific request.
 *
 * @return A USB error code or kStatus_USB_Success.
 */
usb_status_t USB_DeviceAudioRequest(class_handle_t handle, uint32_t event, void *param) {
  usb_device_control_request_struct_t *request = (usb_device_control_request_struct_t *)param;
  usb_status_t error = kStatus_USB_Success;

  switch (event) {
    case USB_DEVICE_AUDIO_GET_CUR_MUTE_CONTROL:
      request->buffer = &g_deviceComposite->audioPlayer.curMute;
      request->length = sizeof(g_deviceComposite->audioPlayer.curMute);
      break;
    case USB_DEVICE_AUDIO_GET_CUR_VOLUME_CONTROL:
      request->buffer = g_deviceComposite->audioPlayer.curVolume;
      request->length = sizeof(g_deviceComposite->audioPlayer.curVolume);
      break;
    case USB_DEVICE_AUDIO_GET_CUR_BASS_CONTROL:
      request->buffer = &g_deviceComposite->audioPlayer.curBass;
      request->length = sizeof(g_deviceComposite->audioPlayer.curBass);
      break;
    case USB_DEVICE_AUDIO_GET_CUR_MID_CONTROL:
      request->buffer = &g_deviceComposite->audioPlayer.curMid;
      request->length = sizeof(g_deviceComposite->audioPlayer.curMid);
      break;
    case USB_DEVICE_AUDIO_GET_CUR_TREBLE_CONTROL:
      request->buffer = &g_deviceComposite->audioPlayer.curTreble;
      request->length = sizeof(g_deviceComposite->audioPlayer.curTreble);
      break;
    case USB_DEVICE_AUDIO_GET_CUR_AUTOMATIC_GAIN_CONTROL:
      request->buffer = &g_deviceComposite->audioPlayer.curAutomaticGain;
      request->length = sizeof(g_deviceComposite->audioPlayer.curAutomaticGain);
      break;
    case USB_DEVICE_AUDIO_GET_CUR_DELAY_CONTROL:
      request->buffer = g_deviceComposite->audioPlayer.curDelay;
      request->length = sizeof(g_deviceComposite->audioPlayer.curDelay);
      break;
    case USB_DEVICE_AUDIO_GET_CUR_SAMPLING_FREQ_CONTROL:
      request->buffer = g_deviceComposite->audioPlayer.curSamplingFrequency;
      request->length = sizeof(g_deviceComposite->audioPlayer.curSamplingFrequency);
      break;
    case USB_DEVICE_AUDIO_GET_MIN_VOLUME_CONTROL:
      request->buffer = g_deviceComposite->audioPlayer.minVolume;
      request->length = sizeof(g_deviceComposite->audioPlayer.minVolume);
      break;
    case USB_DEVICE_AUDIO_GET_MIN_BASS_CONTROL:
      request->buffer = &g_deviceComposite->audioPlayer.minBass;
      request->length = sizeof(g_deviceComposite->audioPlayer.minBass);
      break;
    case USB_DEVICE_AUDIO_GET_MIN_MID_CONTROL:
      request->buffer = &g_deviceComposite->audioPlayer.minMid;
      request->length = sizeof(g_deviceComposite->audioPlayer.minMid);
      break;
    case USB_DEVICE_AUDIO_GET_MIN_TREBLE_CONTROL:
      request->buffer = &g_deviceComposite->audioPlayer.minTreble;
      request->length = sizeof(g_deviceComposite->audioPlayer.minTreble);
      break;
    case USB_DEVICE_AUDIO_GET_MIN_DELAY_CONTROL:
      request->buffer = g_deviceComposite->audioPlayer.minDelay;
      request->length = sizeof(g_deviceComposite->audioPlayer.minDelay);
      break;
    case USB_DEVICE_AUDIO_GET_MIN_SAMPLING_FREQ_CONTROL:
      request->buffer = g_deviceComposite->audioPlayer.minSamplingFrequency;
      request->length = sizeof(g_deviceComposite->audioPlayer.minSamplingFrequency);
      break;
    case USB_DEVICE_AUDIO_GET_MAX_VOLUME_CONTROL:
      request->buffer = g_deviceComposite->audioPlayer.maxVolume;
      request->length = sizeof(g_deviceComposite->audioPlayer.maxVolume);
      break;
    case USB_DEVICE_AUDIO_GET_MAX_BASS_CONTROL:
      request->buffer = &g_deviceComposite->audioPlayer.maxBass;
      request->length = sizeof(g_deviceComposite->audioPlayer.maxBass);
      break;
    case USB_DEVICE_AUDIO_GET_MAX_MID_CONTROL:
      request->buffer = &g_deviceComposite->audioPlayer.maxMid;
      request->length = sizeof(g_deviceComposite->audioPlayer.maxMid);
      break;
    case USB_DEVICE_AUDIO_GET_MAX_TREBLE_CONTROL:
      request->buffer = &g_deviceComposite->audioPlayer.maxTreble;
      request->length = sizeof(g_deviceComposite->audioPlayer.maxTreble);
      break;
    case USB_DEVICE_AUDIO_GET_MAX_DELAY_CONTROL:
      request->buffer = g_deviceComposite->audioPlayer.maxDelay;
      request->length = sizeof(g_deviceComposite->audioPlayer.maxDelay);
      break;
    case USB_DEVICE_AUDIO_GET_MAX_SAMPLING_FREQ_CONTROL:
      request->buffer = g_deviceComposite->audioPlayer.maxSamplingFrequency;
      request->length = sizeof(g_deviceComposite->audioPlayer.maxSamplingFrequency);
      break;
    case USB_DEVICE_AUDIO_GET_RES_VOLUME_CONTROL:
      request->buffer = g_deviceComposite->audioPlayer.resVolume;
      request->length = sizeof(g_deviceComposite->audioPlayer.resVolume);
      break;
    case USB_DEVICE_AUDIO_GET_RES_BASS_CONTROL:
      request->buffer = &g_deviceComposite->audioPlayer.resBass;
      request->length = sizeof(g_deviceComposite->audioPlayer.resBass);
      break;
    case USB_DEVICE_AUDIO_GET_RES_MID_CONTROL:
      request->buffer = &g_deviceComposite->audioPlayer.resMid;
      request->length = sizeof(g_deviceComposite->audioPlayer.resMid);
      break;
    case USB_DEVICE_AUDIO_GET_RES_TREBLE_CONTROL:
      request->buffer = &g_deviceComposite->audioPlayer.resTreble;
      request->length = sizeof(g_deviceComposite->audioPlayer.resTreble);
      break;
    case USB_DEVICE_AUDIO_GET_RES_DELAY_CONTROL:
      request->buffer = g_deviceComposite->audioPlayer.resDelay;
      request->length = sizeof(g_deviceComposite->audioPlayer.resDelay);
      break;
    case USB_DEVICE_AUDIO_GET_RES_SAMPLING_FREQ_CONTROL:
      request->buffer = g_deviceComposite->audioPlayer.resSamplingFrequency;
      request->length = sizeof(g_deviceComposite->audioPlayer.resSamplingFrequency);
      break;
#if USBCFG_AUDIO_CLASS_2_0
    case USB_DEVICE_AUDIO_GET_CUR_SAM_FREQ_CONTROL:
      request->buffer = (uint8_t *)&g_deviceComposite->audioPlayer.curSampleFrequency;
      request->length = sizeof(g_deviceComposite->audioPlayer.curSampleFrequency);
      break;
    case USB_DEVICE_AUDIO_GET_RANGE_SAM_FREQ_CONTROL:
      request->buffer = (uint8_t *)&g_deviceComposite->audioPlayer.controlRange;
      request->length = sizeof(g_deviceComposite->audioPlayer.controlRange);
      break;
    case USB_DEVICE_AUDIO_GET_CUR_CLOCK_VALID_CONTROL:
      request->buffer = &g_deviceComposite->audioPlayer.curClockValid;
      request->length = sizeof(g_deviceComposite->audioPlayer.curClockValid);
      break;
#endif

    case USB_DEVICE_AUDIO_SET_CUR_VOLUME_CONTROL:
      if (request->isSetup == 1U) {
        request->buffer = g_deviceComposite->audioPlayer.curVolume;
      } else {
        uint16_t volume = (uint16_t)((uint16_t)g_deviceComposite->audioPlayer.curVolume[1] << 8U);
        volume |= (uint8_t)(g_deviceComposite->audioPlayer.curVolume[0]);
        g_deviceComposite->audioPlayer.codecTask |= VOLUME_CHANGE_TASK;
      }
      break;
    case USB_DEVICE_AUDIO_SET_CUR_MUTE_CONTROL:
      if (request->isSetup == 1U) {
        request->buffer = &g_deviceComposite->audioPlayer.curMute;
      } else {
        if (g_deviceComposite->audioPlayer.curMute) {
          g_deviceComposite->audioPlayer.codecTask |= MUTE_CODEC_TASK;
        } else {
          g_deviceComposite->audioPlayer.codecTask |= UNMUTE_CODEC_TASK;
        }
      }
      break;
    case USB_DEVICE_AUDIO_SET_CUR_BASS_CONTROL:
      if (request->isSetup == 1U) {
        request->buffer = &g_deviceComposite->audioPlayer.curBass;
      }
      break;
    case USB_DEVICE_AUDIO_SET_CUR_MID_CONTROL:
      if (request->isSetup == 1U) {
        request->buffer = &g_deviceComposite->audioPlayer.curMid;
      }
      break;
    case USB_DEVICE_AUDIO_SET_CUR_TREBLE_CONTROL:
      if (request->isSetup == 1U) {
        request->buffer = &g_deviceComposite->audioPlayer.curTreble;
      }
      break;
    case USB_DEVICE_AUDIO_SET_CUR_AUTOMATIC_GAIN_CONTROL:
      if (request->isSetup == 1U) {
        request->buffer = &g_deviceComposite->audioPlayer.curAutomaticGain;
      }
      break;
    case USB_DEVICE_AUDIO_SET_CUR_DELAY_CONTROL:
      if (request->isSetup == 1U) {
        request->buffer = g_deviceComposite->audioPlayer.curDelay;
      }
      break;
    case USB_DEVICE_AUDIO_SET_CUR_SAMPLING_FREQ_CONTROL:
      if (request->isSetup == 1U) {
        request->buffer = g_deviceComposite->audioPlayer.curSamplingFrequency;
      }
      break;
    case USB_DEVICE_AUDIO_SET_MIN_VOLUME_CONTROL:
      if (request->isSetup == 1U) {
        request->buffer = g_deviceComposite->audioPlayer.minVolume;
      }
      break;
    case USB_DEVICE_AUDIO_SET_MIN_BASS_CONTROL:
      if (request->isSetup == 1U) {
        request->buffer = &g_deviceComposite->audioPlayer.minBass;
      }
      break;
    case USB_DEVICE_AUDIO_SET_MIN_MID_CONTROL:
      if (request->isSetup == 1U) {
        request->buffer = &g_deviceComposite->audioPlayer.minMid;
      }
      break;
    case USB_DEVICE_AUDIO_SET_MIN_TREBLE_CONTROL:
      if (request->isSetup == 1U) {
        request->buffer = &g_deviceComposite->audioPlayer.minTreble;
      }
      break;
    case USB_DEVICE_AUDIO_SET_MIN_DELAY_CONTROL:
      if (request->isSetup == 1U) {
        request->buffer = g_deviceComposite->audioPlayer.minDelay;
      }
      break;
    case USB_DEVICE_AUDIO_SET_MIN_SAMPLING_FREQ_CONTROL:
      if (request->isSetup == 1U) {
        request->buffer = g_deviceComposite->audioPlayer.minSamplingFrequency;
      }
      break;
    case USB_DEVICE_AUDIO_SET_MAX_VOLUME_CONTROL:
      if (request->isSetup == 1U) {
        request->buffer = g_deviceComposite->audioPlayer.maxVolume;
      }
      break;
    case USB_DEVICE_AUDIO_SET_MAX_BASS_CONTROL:
      if (request->isSetup == 1U) {
        request->buffer = &g_deviceComposite->audioPlayer.maxBass;
      }
      break;
    case USB_DEVICE_AUDIO_SET_MAX_MID_CONTROL:
      if (request->isSetup == 1U) {
        request->buffer = &g_deviceComposite->audioPlayer.maxMid;
      }
      break;
    case USB_DEVICE_AUDIO_SET_MAX_TREBLE_CONTROL:
      if (request->isSetup == 1U) {
        request->buffer = &g_deviceComposite->audioPlayer.maxTreble;
      }
      break;
    case USB_DEVICE_AUDIO_SET_MAX_DELAY_CONTROL:
      if (request->isSetup == 1U) {
        request->buffer = g_deviceComposite->audioPlayer.maxDelay;
      }
      break;
    case USB_DEVICE_AUDIO_SET_MAX_SAMPLING_FREQ_CONTROL:
      if (request->isSetup == 1U) {
        request->buffer = g_deviceComposite->audioPlayer.maxSamplingFrequency;
      }
      break;
    case USB_DEVICE_AUDIO_SET_RES_VOLUME_CONTROL:
      if (request->isSetup == 1U) {
        request->buffer = g_deviceComposite->audioPlayer.resVolume;
      }
      break;
    case USB_DEVICE_AUDIO_SET_RES_BASS_CONTROL:
      if (request->isSetup == 1U) {
        request->buffer = &g_deviceComposite->audioPlayer.resBass;
      }
      break;
    case USB_DEVICE_AUDIO_SET_RES_MID_CONTROL:
      if (request->isSetup == 1U) {
        request->buffer = &g_deviceComposite->audioPlayer.resMid;
      }
      break;
    case USB_DEVICE_AUDIO_SET_RES_TREBLE_CONTROL:
      if (request->isSetup == 1U) {
        request->buffer = &g_deviceComposite->audioPlayer.resTreble;
      }
      break;
    case USB_DEVICE_AUDIO_SET_RES_DELAY_CONTROL:
      if (request->isSetup == 1U) {
        request->buffer = g_deviceComposite->audioPlayer.resDelay;
      }
      break;
    case USB_DEVICE_AUDIO_SET_RES_SAMPLING_FREQ_CONTROL:
      if (request->isSetup == 1U) {
        request->buffer = g_deviceComposite->audioPlayer.resSamplingFrequency;
      }
      break;
#if USBCFG_AUDIO_CLASS_2_0
    case USB_DEVICE_AUDIO_SET_CUR_SAM_FREQ_CONTROL:
      if (request->isSetup == 1U) {
        request->buffer = (uint8_t *)&g_deviceComposite->audioPlayer.curSampleFrequency;
      }
      break;
    case USB_DEVICE_AUDIO_SET_CUR_CLOCK_VALID_CONTROL: {
      request->buffer = &g_deviceComposite->audioPlayer.curClockValid;
    } break;

#endif
    default:
      error = kStatus_USB_InvalidRequest;
      break;
  }
  return error;
}

/* The USB_AudioSpeakerBufferSpaceUsed() function gets the used speaker ringbuffer size */
uint32_t USB_AudioSpeakerBufferSpaceUsed(void) {
  if (g_deviceComposite->audioPlayer.tdReadNumberPlay > g_deviceComposite->audioPlayer.tdWriteNumberPlay) {
    g_deviceComposite->audioPlayer.speakerReservedSpace =
        g_deviceComposite->audioPlayer.tdReadNumberPlay - g_deviceComposite->audioPlayer.tdWriteNumberPlay;
  } else {
#if 1 /// @note support 24bit
    g_deviceComposite->audioPlayer.speakerReservedSpace = g_deviceComposite->audioPlayer.tdReadNumberPlay +
                                                          PLAY_DATA_BUFF_SIZE -
                                                          g_deviceComposite->audioPlayer.tdWriteNumberPlay;
#else // 16bit
    g_deviceComposite->audioPlayer.speakerReservedSpace =
        g_deviceComposite->audioPlayer.tdReadNumberPlay +
        AUDIO_SPEAKER_DATA_WHOLE_BUFFER_LENGTH * FS_ISO_OUT_ENDP_PACKET_SIZE -
        g_deviceComposite->audioPlayer.tdWriteNumberPlay;
#endif
  }
  return g_deviceComposite->audioPlayer.speakerReservedSpace;
}

/* The USB_AudioFeedbackDataUpdate() function calculates the feedback data */
void USB_AudioFeedbackDataUpdate() {
  {
    volatile uint32_t currentLRClockCount;

    currentLRClockCount = getLRClockCount();
    if (startLRClockCount == 0) {
      /* start */
      startLRClockCount = 1;
      beforeLRClockCount = currentLRClockCount; /* keep count before once */
    } else {
      if (currentLRClockCount >= beforeLRClockCount) {
        output_samples_per_feedback_span = currentLRClockCount - beforeLRClockCount;
      } else {
        output_samples_per_feedback_span = (0xFFFFFFFF - beforeLRClockCount) + currentLRClockCount + 1;
      }
      beforeLRClockCount = currentLRClockCount; /* keep count before once */

      { /// @note 再生用バッファが溢れそうだった場合、ここで調整する
        if (feedback_adjustment_sample > 0) {
          output_samples_per_feedback_span++;
        } else if (feedback_adjustment_sample < 0) {
          output_samples_per_feedback_span--;
        }
        feedback_adjustment_sample = 0; // reset
      }

      /// @note support 44100Hz
      /* @note なんか大きくおかしい場合の対応を入れておく:平均取りなので、影響を小さくするようにしてある */
      /// @note test (SAI DMA) 基本的には704,705,706		/// @note Feedback_EP_16ms_Interval
      if ((output_samples_per_feedback_span > 800) || (output_samples_per_feedback_span < 600)) {
        /* over threshold */
        //				usb_echo("Feedback Info data: out of range\r\n");
        return;
      }

      feedback_history[history_index] = output_samples_per_feedback_span;
      history_index = (history_index + 1) & (HISTORY_SIZE - 1);

      {
        feedback_value = 0;
        for (int i = 0; i < HISTORY_SIZE; i++) {
          feedback_value += feedback_history[i];
        }
        feedback_value *= (1 << 4); // (14 - HISTORY_SIZE_LOG2 - FEEDBACK_REFRESH);	/// @note 16ms間隔
      }
      AUDIO_UPDATE_FEEDBACK_DATA(audioFeedBackBuffer, feedback_value);
      /// @note add AudioRec
      { /* feedback_rec_valueの更新 */
        feedback_rec_value = feedback_value;
      }
    }
  }
}

/* The USB_AudioSpeakerPutBuffer() function fills the audioRecDataBuff with audioPlayPacket in every callback*/
#define HALF_PLAY_DATA_BUFF_SIZE (PLAY_DATA_BUFF_SIZE >> 1)
#define HALF_REC_DATA_BUFF_SIZE (REC_DATA_BUFF_SIZE >> 1)
//static uint32_t Buf_Space_Reset_Count;
void USB_AudioSpeakerPutBuffer(uint8_t *buffer, uint32_t size) {
  /// @note support 24bit
  if (g_deviceComposite->audioPlayer.startPlayHalfFull == 1) {
    uint32_t currentWritePtr;
    uint32_t currentReadPtr;
    uint8_t isOverflow;

    isOverflow = 0;
    USB_AUDIO_ENTER_CRITICAL();
    currentWritePtr = g_deviceComposite->audioPlayer.tdReadNumberPlay;
    USB_AUDIO_EXIT_CRITICAL();

    currentReadPtr = g_deviceComposite->audioPlayer.tdWriteNumberPlay;
    if (currentReadPtr >= currentWritePtr) {
      if ((currentReadPtr + AUDIO_DATA_BUFF_32BIT_ALIGN_SIZE_45SAMPLES) >= PLAY_DATA_BUFF_SIZE) {
        currentReadPtr = (currentReadPtr + AUDIO_DATA_BUFF_32BIT_ALIGN_SIZE_45SAMPLES - PLAY_DATA_BUFF_SIZE);
        if (currentReadPtr >= currentWritePtr) { /// @note sizeを加算した値がWritePtrを超える場合
          isOverflow = 1;
          /// @note Write側が遅れているので、もっとサンプルを送るようなFeedback情報を仕掛ける
          feedback_adjustment_sample = 1;
        }
      } else {
        currentWritePtr += AUDIO_DATA_BUFF_32BIT_ALIGN_SIZE_45SAMPLES;
        if (currentWritePtr >= currentReadPtr) {
          isOverflow = 2;
          /// @note Read側が遅れているので、サンプルを減らすようなFeedback情報を仕掛ける
          feedback_adjustment_sample = -1;
        }
      }
    } else {
      if ((currentWritePtr + AUDIO_DATA_BUFF_32BIT_ALIGN_SIZE_45SAMPLES) >= PLAY_DATA_BUFF_SIZE) {
        currentWritePtr = (currentWritePtr + AUDIO_DATA_BUFF_32BIT_ALIGN_SIZE_45SAMPLES - PLAY_DATA_BUFF_SIZE);
        if (currentWritePtr >= currentReadPtr) { /// @note sizeを加算した値がReadPtrを超える場合
          isOverflow = 2;
          /// @note Read側が遅れているので、サンプルを減らすようなFeedback情報を仕掛ける
          feedback_adjustment_sample = -1;
        }
      } else {
        currentReadPtr += AUDIO_DATA_BUFF_32BIT_ALIGN_SIZE_45SAMPLES;
        if (currentReadPtr >= currentWritePtr) {
          isOverflow = 1;
          /// @note Write側が遅れているので、もっとサンプルを送るようなFeedback情報を仕掛ける
          feedback_adjustment_sample = 1;
        }
      }
    }
    if (isOverflow == 1) {
      //			g_deviceComposite->audioPlayer.startPlayHalfFull = 0;
      //			g_deviceComposite->audioPlayer.tdWriteNumberPlay = 0;
      //			g_deviceComposite->audioPlayer.tdReadNumberPlay = 0;
      //    		usb_echo("USB Buffer Space ReadPtr Overflow\r\n");
    }
    if (isOverflow == 2) {
      //    		usb_echo("USB Buffer Space WritePtr Overflow\r\n");
    }
  }

  while (size != 0) {
    /// @note USB Audio
    audioPlayDataBuff[g_deviceComposite->audioPlayer.tdReadNumberPlay] = 0;
    audioPlayDataBuff[g_deviceComposite->audioPlayer.tdReadNumberPlay + 1] = *buffer++;
    audioPlayDataBuff[g_deviceComposite->audioPlayer.tdReadNumberPlay + 2] = *buffer++;
    audioPlayDataBuff[g_deviceComposite->audioPlayer.tdReadNumberPlay + 3] = *buffer++;
    g_deviceComposite->audioPlayer.tdReadNumberPlay += 4;
    size -= 3;
    if (g_deviceComposite->audioPlayer.tdReadNumberPlay >= PLAY_DATA_BUFF_SIZE) {
      g_deviceComposite->audioPlayer.tdReadNumberPlay = 0;
    }
  }
}

/// @note add AudioRec (Adio style)
//__attribute__((section(".ramfunc.$SRAM_ITC")))
void USB_prepareAudioRecData() {
  if (g_deviceComposite->audioPlayer.currentInterfaceAlternateSetting[USB_AUDIO_IN_STREAM_INTERFACE_INDEX] == 1U) {
    /// @note 24bit 44100Hz
    {
      volatile uint16_t currentRecSamples;

      /**
			 * Feedback : 10.14 format 2^14 = 16384
			 *　feedback_rec_value : 最新のfeedback情報(固定値) <= feedback_valueをコピー
			 * feedback_rec_mod_value : feedback情報を16384で割った余り(固定値)
			 * feedback_rec_current_value : 現在のmod値の加算状況を表す
			 * feedback_rec_value_count : feedback_rec_mod_valueを乗算するカウント。SOF毎に+1増加。 16384を超えたら0に戻る
			 * feedback_rec_sub_value : 16384と小数点分の値を引いた値は、次の16384との差分を計算する部分へ持ち越す
			 */
      { /* currentRecSamplesの取得 : 44.1kHz対応オンリー */
        currentRecSamples = 44;

        /* (startWrite==1)開始時のみ(SOF Packetが先なので、まだIN Packetが出ていない) */
        if (feedback_rec_value_count == 0) {
          //					feedback_rec_mod_value = (feedback_rec_value % 16384);
          feedback_rec_current_value = 0;
          feedback_rec_value_count = 1;
        } else {
          /* その都度のfeedback_rec_value値から算出 */
          feedback_rec_mod_value = (feedback_rec_value % 16384);
          feedback_rec_current_value += feedback_rec_mod_value;
          if (feedback_rec_current_value >= 16384) {
            currentRecSamples++;
            feedback_rec_current_value -= 16384;
            //						feedback_rec_sub_value = feedback_rec_current_value;
            {                               /* feedback_rec_mod_valueの更新 */
              feedback_rec_value_count = 1; /* sub分からスタートなので1 */
                                            //							feedback_rec_mod_value = (feedback_rec_value % 16384);
                                            //							feedback_rec_current_value = 0;
            }
          }
        }
        audioRecDataBufSize[audioRecDataBufWrPtr] = currentRecSamples * (AUDIO_IN_FORMAT_CHANNELS * AUDIO_FORMAT_SIZE);
      }
      audioRecDataBufWrPtr = ((audioRecDataBufWrPtr + 1) % AUDIO_REC_DATA_BUF_NUM);
    }
  }
}

/*!
 * @brief Audio class specific callback function.
 *
 * This function handles the Audio class specific requests.
 *
 * @param handle		  The Audio class handle.
 * @param event 		  The Audio class event type.
 * @param param 		  The parameter of the class specific request.
 *
 * @return A USB error code or kStatus_USB_Success.
 */
usb_status_t USB_DeviceAudioCallback(class_handle_t handle, uint32_t event, void *param) {
  usb_status_t error = kStatus_USB_Error;
  usb_device_endpoint_callback_message_struct_t *ep_cb_param;
  ep_cb_param = (usb_device_endpoint_callback_message_struct_t *)param;

  switch (event) {
    case kUSB_DeviceAudioOutEventStreamSendResponse:
      if ((g_deviceComposite->audioPlayer.attach) && (ep_cb_param->length != (USB_UNINITIALIZED_VAL_32))) {
        if (ep_cb_param->length == g_deviceComposite->audioPlayer.currentFeedbackMaxPacketSize) {
          error = USB_DeviceAudioFeedbackInfoSend(handle, USB_AUDIO_SPEAKER_FEEDBACK_ENDPOINT, audioFeedBackBuffer,
                                                  g_deviceComposite->audioPlayer.currentFeedbackMaxPacketSize);
        }
      }
      break;
    case kUSB_DeviceAudioOutEventStreamRecvResponse:
      if ((g_deviceComposite->audioPlayer.attach) && (ep_cb_param->length != (USB_UNINITIALIZED_VAL_32))) {
        if (g_deviceComposite->audioPlayer.startPlay == 0) {
          g_deviceComposite->audioPlayer.startPlay = 1;
        }
        USB_AudioSpeakerPutBuffer(audioPlayPacket, ep_cb_param->length);
        g_deviceComposite->audioPlayer.usbRecvCount += ep_cb_param->length;
        g_deviceComposite->audioPlayer.usbRecvTimes++;

        if ((g_deviceComposite->audioPlayer.tdReadNumberPlay >= HALF_PLAY_DATA_BUFF_SIZE) &&
            (g_deviceComposite->audioPlayer.startPlayHalfFull == 0)) {
          g_deviceComposite->audioPlayer.startPlayHalfFull = 1;
        }

        error = USB_DeviceAudioRecv(handle, USB_AUDIO_SPEAKER_STREAM_ENDPOINT, audioPlayPacket,
                                    g_deviceComposite->audioPlayer.currentStreamOutMaxPacketSize);
      }
      break;
    case kUSB_DeviceAudioInEventStreamSendResponse:
      if ((g_deviceComposite->audioPlayer.attach) && (ep_cb_param->length != (USB_UNINITIALIZED_VAL_32))) {
        if (g_deviceComposite->audioPlayer.startRec == 0) {
          /// @note  RdPtrを合わせる 今のWrPtrから2つ前の位置を指定
          audioRecDataBufRdPtr = (audioRecDataBufWrPtr + 1) % AUDIO_REC_DATA_BUF_NUM;
          audioRecDataBufRdPtr = (audioRecDataBufRdPtr + 1) % AUDIO_REC_DATA_BUF_NUM;

          g_deviceComposite->audioPlayer.startRec = 1;
        }
        if ((g_deviceComposite->audioPlayer.tdWriteNumberRec >= HALF_REC_DATA_BUFF_SIZE) &&
            (g_deviceComposite->audioPlayer.startRecHalfFull == 0)) {
          g_deviceComposite->audioPlayer.startRecHalfFull = 1;
        }
        {
          int i;
          uint32_t bufSize;
          uint8_t *addr;

          bufSize = audioRecDataBufSize[audioRecDataBufRdPtr];
          addr = getUsbAudioInBufPtr(audioRecDataBufRdPtr);
          for (i = 0; i < bufSize; i++) {
            if (g_deviceComposite->audioPlayer.startRecHalfFull == 1) {
              *(addr + i) = audioRecDataBuff[g_deviceComposite->audioPlayer.tdReadNumberRec];

              g_deviceComposite->audioPlayer.tdReadNumberRec++;
              if (g_deviceComposite->audioPlayer.tdReadNumberRec >= REC_DATA_BUFF_SIZE) {
                g_deviceComposite->audioPlayer.tdReadNumberRec = 0;
              }
            } else { /// @note 半分うまるまでは0を返す
              *(addr + i) = 0;
            }
          }
          error = USB_DeviceAudioSend(handle, USB_AUDIO_RECORDER_STREAM_ENDPOINT, addr, bufSize);
          audioRecDataBufRdPtr = (audioRecDataBufRdPtr + 1) % AUDIO_REC_DATA_BUF_NUM;
        }
      }
      break;
    default:
      if (param && (event > 0xFF)) {
        error = USB_DeviceAudioRequest(handle, event, param);
      }
      break;
  }
  return error;
}

/* The USB_DeviceAudioSpeakerStatusReset() function resets the audio speaker status to the initialized status */
void USB_DeviceAudioSpeakerStatusReset(void) {
  g_deviceComposite->audioPlayer.startPlay = 0;
  g_deviceComposite->audioPlayer.startPlayHalfFull = 0;
  g_deviceComposite->audioPlayer.tdReadNumberPlay = 0;
  g_deviceComposite->audioPlayer.tdWriteNumberPlay = 0;
  g_deviceComposite->audioPlayer.audioSendCount = 0;
  g_deviceComposite->audioPlayer.usbRecvCount = 0;
  g_deviceComposite->audioPlayer.lastAudioSendCount = 0;
  g_deviceComposite->audioPlayer.audioSendTimes = 0;
  g_deviceComposite->audioPlayer.usbRecvTimes = 0;
  g_deviceComposite->audioPlayer.speakerIntervalCount = 0;
  g_deviceComposite->audioPlayer.speakerReservedSpace = 0;
  g_deviceComposite->audioPlayer.timesFeedbackCalculate = 0;
  g_deviceComposite->audioPlayer.speakerDetachOrNoInput = 0;
}

/*!
 * @brief Audio set configuration function.
 *
 * This function sets configuration for msc class.
 *
 * @param handle The Audio class handle.
 * @param configure The Audio class configure index.
 *
 * @return A USB error code or kStatus_USB_Success.
 */
usb_status_t USB_DeviceAudioSetConfigure(class_handle_t handle, uint8_t configure) {
  if (USB_AUDIO_MIDI_CONFIGURE_INDEX == configure) {
    g_deviceComposite->audioPlayer.attach = 1U;
    g_deviceComposite->audioPlayer.currentConfiguration = configure;
  }
  return kStatus_USB_Success;
}

usb_status_t USB_DeviceAudioSetInterface(class_handle_t handle, uint8_t interface, uint8_t alternateSetting) {
  if (g_deviceComposite->audioPlayer.currentInterfaceAlternateSetting[interface] != alternateSetting) {
    g_deviceComposite->audioPlayer.currentInterfaceAlternateSetting[interface] = alternateSetting;

    switch (interface) {
      case USB_AUDIO_STREAM_INTERFACE_INDEX: {
        if (g_deviceComposite->audioPlayer.currentInterfaceAlternateSetting[interface] == 1U) {
          /* play:alt1 */
          USB_DeviceAudioSpeakerStatusReset();
          {
            if (initialize_feedback_count_rec == 0) {
              initializeFeedbackValue();
            }
            initialize_feedback_count_play = 1;
          }
          USB_DeviceAudioRecv(g_deviceComposite->audioPlayer.audioHandle, USB_AUDIO_SPEAKER_STREAM_ENDPOINT,
                              audioPlayPacket, g_deviceComposite->audioPlayer.currentStreamOutMaxPacketSize);
          USB_DeviceAudioFeedbackInfoSend(g_deviceComposite->audioPlayer.audioHandle,
                                          USB_AUDIO_SPEAKER_FEEDBACK_ENDPOINT, audioFeedBackBuffer,
                                          g_deviceComposite->audioPlayer.currentFeedbackMaxPacketSize);
        } else {
          /* paly:alt0 */
          g_deviceComposite->audioPlayer.startPlay = 0;
          g_deviceComposite->audioPlayer.startPlayHalfFull = 0;
          g_deviceComposite->audioPlayer.tdReadNumberPlay = 0;
          g_deviceComposite->audioPlayer.tdWriteNumberPlay = 0;
          initialize_feedback_count_play = 0;
          {
            int i;

            for (i = 0; i < PLAY_DATA_BUFF_SIZE; i++) {
              audioPlayDataBuff[i] = 0;
            }
          }
        }
      } break;
      case USB_AUDIO_IN_STREAM_INTERFACE_INDEX: {
        if (g_deviceComposite->audioPlayer.currentInterfaceAlternateSetting[interface] == 1U) {
          /* rec:alt1 */
          {
            {
              int i;

              for (i = 0; i < (FS_ISO_IN_ENDP_PACKET_SIZE + AUDIO_IN_FORMAT_CHANNELS * AUDIO_FORMAT_SIZE); i++) {
                audioRecDataBuf1[i] = 0;
                audioRecDataBuf2[i] = 0;
              }
              for (i = 0; i < AUDIO_REC_DATA_BUF_NUM; i++) {
                audioRecDataBufSize[i] = FS_ISO_IN_ENDP_PACKET_SIZE;
              }
              audioRecDataBufRdPtr = 0;
              audioRecDataBufWrPtr = 0;

              g_deviceComposite->audioPlayer.tdReadNumberRec = 0;
              g_deviceComposite->audioPlayer.tdWriteNumberRec = 0;
            }
            if (initialize_feedback_count_play == 0) {
              initializeFeedbackValue();
            }
            initialize_feedback_count_rec = 1;
            { /* feedbackInfoを元にサイズを算出するための変数 */
              feedback_rec_value = feedback_value;
              feedback_rec_value_count = 0;
              feedback_rec_mod_value = 0;
              feedback_rec_sub_value = 0;
              feedback_rec_current_value = 0;
            }
          }
          //                	USB_PrepareData();	/// @note SOF割り込みのほうでこれをやる
          USB_DeviceAudioSend(g_deviceComposite->audioPlayer.audioHandle, USB_AUDIO_RECORDER_STREAM_ENDPOINT,
                              audioRecDataBuf1, FS_ISO_IN_ENDP_PACKET_SIZE);
        } else {
          /* rec:alt0 */
          g_deviceComposite->audioPlayer.startRec = 0;
          g_deviceComposite->audioPlayer.startRecHalfFull = 0;
          initialize_feedback_count_rec = 0;
        }
      } break;
      default:
        break;
    }
  }
  return kStatus_USB_Success;
}

/*!
 * @brief Audio init function.
 *
 * This function initializes the device with the composite device class information.
 *
 * @param device_composite          The pointer to the composite device structure.
 * @return kStatus_USB_Success .
 */
usb_status_t USB_DeviceAudioPlayerInit(usb_device_composite_struct_t *device_composite) {
  g_deviceComposite = device_composite;
  /// @note audioHandleはmain初期化時に設定済み
  g_deviceComposite->audioPlayer.deviceHandle = NULL;
  g_deviceComposite->audioPlayer.currentStreamOutMaxPacketSize =
      (FS_ISO_OUT_ENDP_PACKET_SIZE + AUDIO_FORMAT_CHANNELS * AUDIO_FORMAT_SIZE);
  g_deviceComposite->audioPlayer.currentFeedbackMaxPacketSize = FS_ISO_FEEDBACK_ENDP_PACKET_SIZE;
  g_deviceComposite->audioPlayer.speed = USB_SPEED_FULL;
  g_deviceComposite->audioPlayer.attach = 0U;
  g_deviceComposite->audioPlayer.copyProtect = 0x01U;
  g_deviceComposite->audioPlayer.curMute = 0x00U;
  g_deviceComposite->audioPlayer.curVolume[0] = 0x00;
  g_deviceComposite->audioPlayer.curVolume[1] = 0x1F;
  g_deviceComposite->audioPlayer.minVolume[0] = 0x00;
  g_deviceComposite->audioPlayer.minVolume[1] = 0x00;
  g_deviceComposite->audioPlayer.maxVolume[0] = 0x00;
  g_deviceComposite->audioPlayer.maxVolume[1] = 0x43;
  g_deviceComposite->audioPlayer.resVolume[0] = 0x01;
  g_deviceComposite->audioPlayer.resVolume[1] = 0x00;
  g_deviceComposite->audioPlayer.curBass = 0x00U;
  g_deviceComposite->audioPlayer.minBass = 0x80U;
  g_deviceComposite->audioPlayer.maxBass = 0x7FU;
  g_deviceComposite->audioPlayer.resBass = 0x01U;
  g_deviceComposite->audioPlayer.curMid = 0x00U;
  g_deviceComposite->audioPlayer.minMid = 0x80U;
  g_deviceComposite->audioPlayer.maxMid = 0x7FU;
  g_deviceComposite->audioPlayer.resMid = 0x01U;
  g_deviceComposite->audioPlayer.curTreble = 0x01U;
  g_deviceComposite->audioPlayer.minTreble = 0x80U;
  g_deviceComposite->audioPlayer.maxTreble = 0x7FU;
  g_deviceComposite->audioPlayer.resTreble = 0x01U;
  g_deviceComposite->audioPlayer.curAutomaticGain = 0x01U;
  g_deviceComposite->audioPlayer.curDelay[0] = 0x00;
  g_deviceComposite->audioPlayer.curDelay[1] = 0x40;
  g_deviceComposite->audioPlayer.minDelay[0] = 0x00;
  g_deviceComposite->audioPlayer.minDelay[1] = 0x00;
  g_deviceComposite->audioPlayer.maxDelay[0] = 0xFF;
  g_deviceComposite->audioPlayer.maxDelay[1] = 0xFF;
  g_deviceComposite->audioPlayer.resDelay[0] = 0x00;
  g_deviceComposite->audioPlayer.resDelay[1] = 0x01;
  g_deviceComposite->audioPlayer.curLoudness = 0x01U;
  g_deviceComposite->audioPlayer.curSamplingFrequency[0] = 0x00;
  g_deviceComposite->audioPlayer.curSamplingFrequency[1] = 0x00;
  g_deviceComposite->audioPlayer.curSamplingFrequency[2] = 0x01;
  g_deviceComposite->audioPlayer.minSamplingFrequency[0] = 0x00;
  g_deviceComposite->audioPlayer.minSamplingFrequency[1] = 0x00;
  g_deviceComposite->audioPlayer.minSamplingFrequency[2] = 0x01;
  g_deviceComposite->audioPlayer.maxSamplingFrequency[0] = 0x00;
  g_deviceComposite->audioPlayer.maxSamplingFrequency[1] = 0x00;
  g_deviceComposite->audioPlayer.maxSamplingFrequency[2] = 0x01;
  g_deviceComposite->audioPlayer.resSamplingFrequency[0] = 0x00;
  g_deviceComposite->audioPlayer.resSamplingFrequency[1] = 0x00;
  g_deviceComposite->audioPlayer.resSamplingFrequency[2] = 0x01;
#if USBCFG_AUDIO_CLASS_2_0
  g_deviceComposite->audioPlayer.curSampleFrequency =
      48000U; /* This should be changed to 48000 if sampling rate is 48k */
  g_deviceComposite->audioPlayer.curClockValid = 1U;
  g_deviceComposite->audioPlayer.controlRange[0] = 1;
  g_deviceComposite->audioPlayer.controlRange[1] = 48000;
  g_deviceComposite->audioPlayer.controlRange[2] = 48000;
  g_deviceComposite->audioPlayer.controlRange[3] = 0;
#endif
  g_deviceComposite->audioPlayer.speed = USB_SPEED_FULL;
  g_deviceComposite->audioPlayer.tdReadNumberPlay = 0;
  g_deviceComposite->audioPlayer.tdWriteNumberPlay = 0;
  g_deviceComposite->audioPlayer.audioSendCount = 0;
  g_deviceComposite->audioPlayer.lastAudioSendCount = 0;
  g_deviceComposite->audioPlayer.usbRecvCount = 0;
  g_deviceComposite->audioPlayer.audioSendTimes = 0;
  g_deviceComposite->audioPlayer.usbRecvTimes = 0;
  g_deviceComposite->audioPlayer.startPlay = 0;
  g_deviceComposite->audioPlayer.startPlayHalfFull = 0;
  g_deviceComposite->audioPlayer.speakerIntervalCount = 0;
  g_deviceComposite->audioPlayer.speakerReservedSpace = 0;
  g_deviceComposite->audioPlayer.timesFeedbackCalculate = 0;
  g_deviceComposite->audioPlayer.speakerDetachOrNoInput = 0;
  /// @note add AudioRec
  g_deviceComposite->audioPlayer.currentStreamInMaxPacketSize =
      (FS_ISO_IN_ENDP_PACKET_SIZE + AUDIO_IN_FORMAT_CHANNELS * AUDIO_FORMAT_SIZE);
  g_deviceComposite->audioPlayer.startRec = 0;
  g_deviceComposite->audioPlayer.startRecHalfFull = 0;
  g_deviceComposite->audioPlayer.tdReadNumberRec = 0;
  g_deviceComposite->audioPlayer.tdWriteNumberRec = 0;

  return kStatus_USB_Success;
}

void USB_AudioCodecTask(void) {
  if (g_deviceComposite->audioPlayer.codecTask & MUTE_CODEC_TASK) {
    usb_echo("Set Cur Mute : %x\r\n", g_deviceComposite->audioPlayer.curMute);
    g_deviceComposite->audioPlayer.codecTask &= ~MUTE_CODEC_TASK;
  }
  if (g_deviceComposite->audioPlayer.codecTask & UNMUTE_CODEC_TASK) {
    usb_echo("Set Cur Mute : %x\r\n", g_deviceComposite->audioPlayer.curMute);
    g_deviceComposite->audioPlayer.codecTask &= ~UNMUTE_CODEC_TASK;
  }
  if (g_deviceComposite->audioPlayer.codecTask & VOLUME_CHANGE_TASK) {
    usb_echo("Set Cur Volume : %x\r\n", (uint16_t)(g_deviceComposite->audioPlayer.curVolume[1] << 8U) |
                                            g_deviceComposite->audioPlayer.curVolume[0]);
    g_deviceComposite->audioPlayer.codecTask &= ~VOLUME_CHANGE_TASK;
  }
}

void USB_AudioSpeakerResetTask(void) {
  if (g_deviceComposite->audioPlayer.speakerDetachOrNoInput) {
    USB_DeviceAudioSpeakerStatusReset();
  }
}
