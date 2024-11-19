/*
#include <p_codec.h>
 * p_dac.c
 *
 *  Created on: 2024/11/14
 *      Author: koki_arai
 */
#include "p_codec.h"
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

codec_config_t boardCodecConfig = {.I2C_SendFunc = BOARD_Codec_I2C_Send,
                                   .I2C_ReceiveFunc = BOARD_Codec_I2C_Receive,
                                   .op.Init = WM8960_Init,
                                   .op.Deinit = WM8960_Deinit,
                                   .op.SetFormat = WM8960_ConfigDataFormat};

codec_handle_t codecHandle = {0};

/**
 * CodecをsaiTransferFormatの値を使いながら初期化
 * codecHandleに値を入れていく
 * WM8960の初期化
 */
void codecInit() {
    //codecHandle のリセット
  CODEC_Init(&codecHandle, &boardCodecConfig);
  //WM8960_ConfigDataFormatでcodecHanleの値とsaiTransferFormatの値を用いて初期化
  CODEC_SetFormat(&codecHandle, saiTransferFormat.masterClockHz, saiTransferFormat.sampleRate_Hz,
                  saiTransferFormat.bitWidth);
  WM8960_SetProtocol(&codecHandle, kWM8960_BusI2S);
  WM8960_SetMasterSlave(&codecHandle, false);
  WM8960_ModifyReg(&codecHandle, WM8960_IFACE1, 0b11110011, WM8960_IFACE1_WL_24BITS);
}
