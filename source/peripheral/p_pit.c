/*
 * p_pit.c
 *
 *  Created on: 2024/11/14
 *      Author: koki_arai
 */

#include "p_pit.h"

#include "fsl_pit.h"

/**
 * PIT : CYCLE Timer 100ms
 */
#define PIT_CYCLE_TIMER_HANDLER PIT_IRQHandler
#define PIT_IRQ_ID PIT_IRQn
#define PIT_SOURCE_CLOCK CLOCK_GetFreq(kCLOCK_OscClk)
#define CYCLE_TIMER_US_TIME 100000U // 100ms
#define NSEC_TO_COUNT(ns, clockFreqInHz) (uint64_t)(((uint64_t)(ns) * (clockFreqInHz)) / 1000000000U)
#define CYCLE_TIMER_NS_TIME 62500U // 62.5us

void pitInit() {
  /*
   * @note PITのサンプルプロジェクト参照.
   * これを入れると, PIT_SetTimerPeriodで
   * USEC_TO_COUNTをセットしたタイマでカウントしてくれるぽい
   */

  CLOCK_SetMux(kCLOCK_PerclkMux, 1U); /* Set PERCLK_CLK source to OSC_CLK*/
  CLOCK_SetDiv(kCLOCK_PerclkDiv, 0U); /* Set PERCLK_CLK divider to 1 */

  pit_config_t pitConfig;           /* Structure of initialize PIT */
  PIT_GetDefaultConfig(&pitConfig); /*Get Default setting */
  PIT_Init(PIT, &pitConfig);        /* Init pit module */

  /**
   * @note おまじない(とりあえずこのプロジェクトにのみ効く対策)
   * o
   * このプロジェクトは、MCUEpressoのPeripheral設定を介していないUSB_Speakerの
   * o サンプルプロジェクトを元に作成している. 下記はそこでの弊害が予想される
   * o おまじない  を入れないと, LTH/LTLの値がTSVの値にリセットされない模様
   * o => 0xFFFFFFFF / 0xFFFFFFFFからダウンカウンタしていくので
   * o 初回のPIT割り込みがかかるのが異常に遅くなる
   * o Channel1がEnableになってしまっているので, Disableにしないと
   * o LTHのカウンタが-1されていく => Channel1もStopする
   * o おまじない を入れると, LTH/LTLの値がきちんと設定されるみたい
   */

  PIT_StopTimer(PIT, kPIT_Chnl_0);
  PIT_StopTimer(PIT, kPIT_Chnl_1);

  /**
   * PIT0 CH0
   */
  PIT_SetTimerPeriod(PIT, kPIT_Chnl_0,
                     USEC_TO_COUNT(CYCLE_TIMER_US_TIME, PIT_SOURCE_CLOCK)); /* Set timer period for channel 0 */
  PIT_EnableInterrupts(PIT, kPIT_Chnl_0, kPIT_TimerInterruptEnable);        /* Enable timer interrupts for channel 0 */
  NVIC_SetPriority(PIT_IRQ_ID, PIT_INTERRUPT_PRIORITY);                     /* Enable at the NVIC */

  /**
   * PIT0 CH1
   */
  PIT_SetTimerPeriod(PIT, kPIT_Chnl_1,
                     NSEC_TO_COUNT(CYCLE_TIMER_NS_TIME, PIT_SOURCE_CLOCK)); /* Set timer period for channel 1 */
  PIT_SetTimerChainMode(PIT, kPIT_Chnl_1, false);                           /* Set timer chain mode for channel 1 */
  PIT_EnableInterrupts(PIT, kPIT_Chnl_1, kPIT_TimerInterruptEnable);        /* Enable timer interrupts for channel 1 */

  EnableIRQ(PIT_IRQ_ID);
  return;
}

void pitStart() {
  PIT_StartTimer(PIT, kPIT_Chnl_0); /* Start channel 0 */
  PIT_StartTimer(PIT, kPIT_Chnl_1); /* Start channel 1 */
}

void PIT_IRQHandler(void) {
  if (PIT_GetStatusFlags(PIT, kPIT_Chnl_0)) {
    PIT_ClearStatusFlags(PIT, kPIT_Chnl_0, kPIT_TimerFlag); /* Clear interrupt flag.*/
#if (defined(USB_DEVICE_CONFIG_EHCI) && (USB_DEVICE_CONFIG_EHCI > 0U))
    USB_DeviceEhciAttachedDevice(g_composite.deviceHandle);
#endif
    USB_setMidiInTimeoutCount(); /// @note MIDI IN Timeout (100ms)
  }
  if (PIT_GetStatusFlags(PIT, kPIT_Chnl_1)) {
    PIT_ClearStatusFlags(PIT, kPIT_Chnl_1, kPIT_TimerFlag); /* Clear interrupt flag.*/
    adcStartFillingOnelap();
    PIT_ClearStatusFlags(PIT, kPIT_Chnl_0, kPIT_TimerFlag); /* Clear interrupt flag.*/
#if (defined(USB_DEVICE_CONFIG_EHCI) && (USB_DEVICE_CONFIG_EHCI > 0U))
    USB_DeviceEhciAttachedDevice(g_composite.deviceHandle);
#endif
    USB_setMidiInTimeoutCount(); /// @note MIDI IN Timeout (100ms)
    __DSB();
  }
}
