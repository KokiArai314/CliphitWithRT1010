/*
 * p_dac.c
 *
 *  Created on: 2024/11/14
 *      Author: koki_arai
 */
#include "p_dac.h"
#include "p_sai.h"
#include "peripheral/p_adc.h"

#include <stdio.h>
#include <stdlib.h>

#include "board.h"
#include "clock_config.h"
#include "fsl_device_registers.h"

#include "usb_device_config.h"
#include "usb_device.h"
#include "usb_device_class.h"
#include "usb_device_audio.h"
#include "usb_device_ch9.h"
#include "usb_device_descriptor.h"

//#include "p_usb.h"

#if defined(USB_DEVICE_CONFIG_EHCI) && (USB_DEVICE_CONFIG_EHCI > 0U)
#include "usb_phy.h"
#endif

#include "fsl_dmamux.h"
#include "fsl_sai.h"
#include "fsl_sai_edma.h"
#include "fsl_wm8960.h"
#include "pin_mux.h"
#define OVER_SAMPLE_RATE (256U)

/**
 * extern veriables from p_usb_audio
 */


codec_config_t boardCodecConfig = {.I2C_SendFunc = BOARD_Codec_I2C_Send,
                                   .I2C_ReceiveFunc = BOARD_Codec_I2C_Receive,
                                   .op.Init = WM8960_Init,
                                   .op.Deinit = WM8960_Deinit,
                                   .op.SetFormat = WM8960_ConfigDataFormat};

codec_handle_t codecHandle = {0};

void BOARD_Codec_Init() {
  CODEC_Init(&codecHandle, &boardCodecConfig);
  CODEC_SetFormat(&codecHandle, audioFormat.masterClockHz, audioFormat.sampleRate_Hz, audioFormat.bitWidth);
}

void WM8960_Config_Audio_Formats(uint32_t samplingRate) {
  /* Configure the audio audioFormat */
  /// @note support 24bit
  audioFormat.bitWidth = kSAI_WordWidth24bits;

  audioFormat.channel = 0U;
  audioFormat.sampleRate_Hz = samplingRate;

  audioFormat.masterClockHz = OVER_SAMPLE_RATE * audioFormat.sampleRate_Hz;
  audioFormat.protocol = saiTxConfig.protocol;
  audioFormat.stereo = kSAI_Stereo;
#if defined(FSL_FEATURE_SAI_FIFO_COUNT) && (FSL_FEATURE_SAI_FIFO_COUNT > 1)
#if 1 /// @note FSL_FEATURE_SAI_FIFO_COUNT == 2
  audioFormat.watermark = FSL_FEATURE_SAI_FIFO_COUNT / 4U;
#else
  audioFormat.watermark = FSL_FEATURE_SAI_FIFO_COUNT / 2U;
#endif
#endif
}

/// @note [SAI] enable RX
void BOARD_USB_Audio_TxRxInit(uint32_t samplingRate) { WM8960_Config_Audio_Formats(samplingRate); }
