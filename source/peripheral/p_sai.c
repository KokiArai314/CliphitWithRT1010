/*
 * p_sai.c
 *
 *  Created on: 2024/11/14
 *      Author: koki_arai
 */
#include "p_sai.h"

#include <stdio.h>
#include <stdlib.h>

#include "board.h"
#include "clock_config.h"
#include "definitions.h"
#include "fsl_device_registers.h"
#include "p_adc.h"
#include "p_usb.h"
#include "p_usb_audio.h"
#include "usb_device.h"
#include "usb_device_audio.h"
#include "usb_device_ch9.h"
#include "usb_device_class.h"
#include "usb_device_config.h"
#include "usb_device_descriptor.h"

#if defined(USB_DEVICE_CONFIG_EHCI) && (USB_DEVICE_CONFIG_EHCI > 0U)
#include "usb_phy.h"
#endif

#include "fsl_dmamux.h"
#include "fsl_sai.h"
#include "fsl_sai_edma.h"
#include "fsl_wm8960.h"
#include "pin_mux.h"

#define BOARD_DEMO_SAI SAI1
#define DEMO_SAI_IRQ_TX SAI1_IRQn
/// @note [SAI] enable RX
#define DEMO_SAI_CHANNEL (0)
#define DEMO_SAI_IRQ SAI1_IRQn
#define SAI_UserIRQHandler SAI1_IRQHandler

/// @note support 44100Hz
#define DEMO_SAI1_CLOCK_SOURCE_SELECT (2U)      /* Select Audio/Video PLL (1.12896 GHz) as sai1 clock source */
#define DEMO_SAI1_CLOCK_SOURCE_PRE_DIVIDER (4U) /* Clock pre divider for sai1 clock source */
#define DEMO_SAI1_CLOCK_SOURCE_DIVIDER (4U)     /* Clock divider for sai1 clock source */

/* Get frequency of sai1 clock */
#define DEMO_SAI_CLK_FREQ                                                      \
  (CLOCK_GetFreq(kCLOCK_AudioPllClk) / (DEMO_SAI1_CLOCK_SOURCE_DIVIDER + 1U) / \
   (DEMO_SAI1_CLOCK_SOURCE_PRE_DIVIDER + 1U))

sai_config_t saiTxConfig;
/// @note [SAI] enable RX
sai_config_t saiRxConfig;
sai_transfer_format_t audioFormat;

static float saidata[FSL_FEATURE_SAI_FIFO_COUNT];
static int32_t out;
static int32_t *pOut = &out;
static float *pSaidata[2] = {&(saidata[0]), &(saidata[1])};
int32_t saiRxdata[FSL_FEATURE_SAI_FIFO_COUNT];

static inline float convertInput(const int32_t iData) { return ((float)iData / 0x80000000ul); }

static inline int32_t convertOutput(float fData) {
  // float to uint32_t
  fData = fData < -1.0f ? -1.0f : fData > 1.0f ? 1.0f : fData;
  // fData = (fData + 1.0f)/2.0f;
  int32_t iData = (int32_t)(fData * 0x007ffffful) & 0x00ffffff;
  return iData;
}

static void SAI_USB_Audio_TxInit(I2S_Type *SAIBase) {
  SAI_TxGetDefaultConfig(&saiTxConfig);

  SAI_TxInit(SAIBase, &saiTxConfig);
}

/// @note [SAI] enable RX
static void SAI_USB_Audio_RxInit(I2S_Type *SAIBase) {
  SAI_RxGetDefaultConfig(&saiRxConfig);

  SAI_RxInit(SAIBase, &saiRxConfig);
}

static uint8_t hcCritSectCount = 0;

static void HC_CRITICAL_SECTION_ENTER(void) {
  if (hcCritSectCount == 0) {
    __disable_irq();
  }
  ++hcCritSectCount;
}

static void HC_CRITICAL_SECTION_EXIT(void) {
  if (hcCritSectCount != 0) {
    if (--hcCritSectCount == 0) {
      __enable_irq();
    }
  }
}

static void (*hcAudioCallback)(int32_t *data_l, int32_t *data_r) = NULL;

void HC_AudioSetCallback(void (*callback)(int32_t *data_l, int32_t *data_r)) {
  HC_CRITICAL_SECTION_ENTER();
  hcAudioCallback = callback;
  HC_CRITICAL_SECTION_EXIT();
}

/* Initialize the structure information for sai. */
void saiInit(void) {
  SAI_USB_Audio_TxInit(BOARD_DEMO_SAI);
  SAI_USB_Audio_RxInit(BOARD_DEMO_SAI);

  BOARD_USB_Audio_TxRxInit(AUDIO_SAMPLING_RATE);
  BOARD_Codec_I2C_Init();
  BOARD_Codec_Init();

  /*Clock setting for SAI1*/
  CLOCK_SetMux(kCLOCK_Sai1Mux, DEMO_SAI1_CLOCK_SOURCE_SELECT);
  CLOCK_SetDiv(kCLOCK_Sai1PreDiv, DEMO_SAI1_CLOCK_SOURCE_PRE_DIVIDER);
  CLOCK_SetDiv(kCLOCK_Sai1Div, DEMO_SAI1_CLOCK_SOURCE_DIVIDER);

  uint32_t mclkSourceClockHz = 0U;
  uint32_t delayCycle = 500000;

#if defined(CODEC_CYCLE)
  delayCycle = CODEC_CYCLE;
#endif
  while (delayCycle) {
    __ASM("nop");
    delayCycle--;
  }

  mclkSourceClockHz = DEMO_SAI_CLK_FREQ;
  SAI_TxSetFormat(BOARD_DEMO_SAI, &audioFormat, mclkSourceClockHz, audioFormat.masterClockHz);
  SAI_RxSetFormat(BOARD_DEMO_SAI, &audioFormat, mclkSourceClockHz, audioFormat.masterClockHz);

  NVIC_SetPriority(DEMO_SAI_IRQ, SAI_INTERRUPT_PRIORITY);
  EnableIRQ(DEMO_SAI_IRQ);

  SAI_TxEnableInterrupts(BOARD_DEMO_SAI, kSAI_FIFOWarningInterruptEnable | kSAI_FIFOErrorInterruptEnable);
  SAI_RxEnableInterrupts(BOARD_DEMO_SAI, kSAI_FIFOWarningInterruptEnable | kSAI_FIFOErrorInterruptEnable);
  SAI_TxEnable(BOARD_DEMO_SAI, true);
  SAI_RxEnable(BOARD_DEMO_SAI, true);
}

void SAI_UserIRQHandler(void) {
  int i;

  /* Clear the TX FIFO error flag */
  if (SAI_TxGetStatusFlag(BOARD_DEMO_SAI) & kSAI_FIFOErrorFlag) {
    SAI_TxClearStatusFlags(BOARD_DEMO_SAI, kSAI_FIFOErrorFlag);
  }

  if (SAI_TxGetStatusFlag(BOARD_DEMO_SAI) & kSAI_FIFOWarningFlag) {
    /* Read from USB audio rx buffer and mix to SAI tx buffer */
    usbAudioIncLRClockCount();

    for (i = 0; i < FSL_FEATURE_SAI_FIFO_COUNT; i++) {
      saidata[i] = 0;
    }

    /* SAI out */
    if (hcAudioCallback != NULL) {
      int32_t outL, outR;
      // hcAudioCallback(&outL, &outR);
      // SAI_WriteData(BOARD_DEMO_SAI, DEMO_SAI_CHANNEL,(uint32_t)outL);
      // SAI_WriteData(BOARD_DEMO_SAI, DEMO_SAI_CHANNEL,(uint32_t)outR);
    } else {
      audio_task(pOut);
      /**
       * I2S通信
       * 32bit singed を 24bit singedに変換する必要がある
       * MSB 1bitが符号ビット
       * 32bit singedの入れ物に入れるので下位8bitは0になる
       */
      //  clip as 24bit signed
      if (out > 8388607) out = 8388607;
      if (out < -8388608) out = -8388608;
      out = out * 2;
      // int32_t ret = out >> 8;

      // 左詰め signed
      uint32_t sign = out & 0x80000000;  // 符号bit
      uint32_t value = out & 0x007fffff; // 値(23bit)
      uint32_t ret = (value | sign) >> 8;

      // 左詰め signed
      // uint32_t sign   = out & 0x80000000; //符号bit
      // uint32_t value  = out & 0x007fffff; //値(23bit)
      // uint32_t ret = (uint32_t)((value << 8) | sign);

      // 右詰め
      // uint32_t sign   = (out & (0x80000000)) >> 8; //符号bit
      // uint32_t value  = out & 0x007fffff; //値(23bit)
      // <-負の値の場合これが補数になるので変なことになるかも uint32_t ret =
      // (uint32_t)((value ) | sign);

      // 右詰め unsigned
      // uint32_t ret = out + 8388607;

      // 左詰め unsigned
      // uint32_t ret = (out + 8388607) << 8;
      // ret = ret * 4;

      for (i = 0; i < FSL_FEATURE_SAI_FIFO_COUNT; i++) {
        SAI_WriteData(BOARD_DEMO_SAI, DEMO_SAI_CHANNEL, (uint32_t)ret);
      }
    }
  }

  /// @note [SAI] enable RX
  for (i = 0; i < FSL_FEATURE_SAI_FIFO_COUNT; i++) {
    saiRxdata[i] = 0;
  }

  /* Clear the RX FIFO error flag */
  if (SAI_RxGetStatusFlag(BOARD_DEMO_SAI) & kSAI_FIFOErrorFlag) {
    SAI_RxClearStatusFlags(BOARD_DEMO_SAI, kSAI_FIFOErrorFlag);
  }

  if (SAI_RxGetStatusFlag(BOARD_DEMO_SAI) & kSAI_FIFOWarningFlag) {
    for (i = 0; i < FSL_FEATURE_SAI_FIFO_COUNT; i++) {
      saiRxdata[i] = (int32_t)SAI_ReadData(BOARD_DEMO_SAI, DEMO_SAI_CHANNEL) << 8;
      saiRxdata[i] >>= 8;
    }

    // @note add AudioRec

    if (g_deviceComposite->audioPlayer.startRec) {
      int i;

      /* 24bit L/R dataをUSB用バッファに格納  */
      for (i = 0; i < FSL_FEATURE_SAI_FIFO_COUNT; i++) {
        audioRecDataBuff[g_deviceComposite->audioPlayer.tdWriteNumberRec + (i * 3) + 0] =
            (uint8_t)(saiRxdata[i] & 0x0000FF);
        audioRecDataBuff[g_deviceComposite->audioPlayer.tdWriteNumberRec + (i * 3) + 1] =
            (uint8_t)((saiRxdata[i] & 0x00FF00) >> 8);
        audioRecDataBuff[g_deviceComposite->audioPlayer.tdWriteNumberRec + (i * 3) + 2] =
            (uint8_t)((saiRxdata[i] & 0xFF0000) >> 16);
      }
      g_deviceComposite->audioPlayer.tdWriteNumberRec += (FSL_FEATURE_SAI_FIFO_COUNT * AUDIO_FORMAT_SIZE);
      if (g_deviceComposite->audioPlayer.tdWriteNumberRec >= REC_DATA_BUFF_SIZE) {
        g_deviceComposite->audioPlayer.tdWriteNumberRec = 0;
      }
      /// @note SAI通信でOverflowしてしまったらリセットする

      if (g_deviceComposite->audioPlayer.tdWriteNumberRec == g_deviceComposite->audioPlayer.tdReadNumberRec) {
        g_deviceComposite->audioPlayer.startRec = 0;
        g_deviceComposite->audioPlayer.startRecHalfFull = 0;
        g_deviceComposite->audioPlayer.tdReadNumberRec = 0;
        g_deviceComposite->audioPlayer.tdWriteNumberRec = 0;
      }
    }
  }
  /* Add for ARM errata 838869, affects Cortex-M4, Cortex-M4F Store immediate
     overlapping exception return operation might vector to incorrect interrupt
   */
  __DSB();
}
