/*
#include <p_codec.h>
 * CLIPHIT2.c
 *
 *  Created on: 2024/10/03
 *      Author: koki_arai
 */

#include "board.h"
#include "definitions.h"

#include "audio_task/audio_task.h"
#include "p_usb.h"

#include "p_adc.h"
#include "p_pit.h"
#include "p_sai.h"

#include "pin_mux.h"
#include "systick.h"
#include "utilities/RTT/rtt_debugger.h"
#include "fsl_lpuart.h"
#include "midi/midi_if.h"
#include "midi/midi_player.h"
#include "midi_debug_monitor/midi_debug_monitor.h"

#ifdef ADC_ENABLE
#include "peripheral/p_adc.h"
#include "trigger/trigger.h"
#endif // ADC_ENABLE

/*******************************************************************************
 * Definitions
 ******************************************************************************/

void APPInit(void);
void composite_idle(void);
void MIDI_IF_IDLE(void);
void composite_idle(void);

extern void midi_hook_exec(void);
extern usb_device_composite_struct_t g_composite;

/*!
 * @brief Application main routine
 * @return None.
 */
#if defined(__CC_ARM) || defined(__GNUC__)
int main(void)
#else
void main(void)
#endif
{
  BOARD_ConfigMPU();
  BOARD_InitPins();
  BOARD_BootClockRUN();
  BOARD_AudioInitPllClock();
  BOARD_InitClockPinMux(); /*Clock setting for LPI2C and SAI1 */

  // BOARD_InitDebugConsole();
  // systick_init();	//systick on for JobTime
  // rtt_debugger_init();

  APPInit();

  while (1) {
    composite_idle();
    trigger_idle();
  }
}

void composite_idle(void) {
  {
    /* USBケーブルのconnect/disconnect状態を監視 */
    checkAttachedDevice();

    if (g_composite.audioPlayer.attach == 1) {
      USB_AudioCodecTask();
      USB_AudioSpeakerResetTask();
    }
    if (g_composite.midiPlayer.attach == 1) {
      MIDI_IF_IDLE();
      USB_MIDI_IDLE();
    }

    else { // for debug command
      MIDI_IF_IDLE();
    }
    midi_hook_exec();
#if USB_DEVICE_CONFIG_USE_TASK
    USB_DeviceTaskFn(g_composite.deviceHandle);
#endif

    trigger_idle();
  }
}

/*!
 * @brief Application initialization function.
 *
 * This function initializes the application.
 *
 * @return None.
 */

void APPInit(void) {
  usbInit();
  saiInit();
  // trigger_init();

  /* LPUART */
  {
    // lpuart_config_t uartConfig;
    // uint32_t uartClkSrcFreq = BOARD_DebugConsoleSrcFreq();	///
    // @note OK?

    /*
     * config.baudRate_Bps = 115200U;
     * config.parityMode = kLPUART_ParityDisabled;
     * config.stopBitCount = kLPUART_OneStopBit;
     * config.txFifoWatermark = 0;
     * config.rxFifoWatermark = 0;
     * config.enableTx = false;
     * config.enableRx = false;
     */
    // LPUART_GetDefaultConfig(&uartConfig);
    // uartConfig.baudRate_Bps = BOARD_UART_BAUDRATE;
    // uartConfig.enableTx     = true;
    // uartConfig.enableRx     = true;

    // LPUART_Init(LPUART1, &uartConfig, uartClkSrcFreq);

    //@note UART Rx側の割込み許可は、USB SetConfigurationが来たら許可する
    // NVIC_SetPriority((IRQn_Type)BOARD_UART_IRQ, UART_INTERRUPT_PRIORITY);
  }
#ifdef LOCAL_DEBUG_ENABLE
  /* Enable RX interrupt. */
  // midi_IF_RxInit();
  // EnableIRQ(BOARD_UART_IRQ);
#endif

#ifdef ADC_ENABLE

  adcInit();
  pitInit();
  pitStart();

  // AdcAudioDebugOn();

#endif // ADC_ENABLE

  triggerInit();
  audio_task_init();
}
