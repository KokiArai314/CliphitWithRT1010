/*
 * p_dac.h
 *
 *  Created on: 2024/11/14
 *      Author: koki_arai
 */

#ifndef PERIPHERAL_P_DAC_H_
#define PERIPHERAL_P_DAC_H_

#include <stdint.h>
#include "p_sai.h"
#include "p_adc.h"
#include <fsl_sai.h>
#include <fsl_codec_common.h>

extern codec_config_t boardCodecConfig;
extern sai_transfer_format_t audioFormat;
extern sai_config_t saiTxConfig;

void BOARD_Codec_Init();
void WM8960_Config_Audio_Formats(uint32_t samplingRate);
void BOARD_USB_Audio_TxRxInit(uint32_t samplingRate);

#endif /* PERIPHERAL_P_DAC_H_ */
