/*
 * CLIPHIT2.c
 *
 *  Created on: 2024/10/03
 *      Author: koki_arai
 */

#include "board.h"
#include "audio_task/audio_player.h"
#include "audio_task/audio_task.h"
#include "midi/midi_player.h"
#include "midi/midi_if.h"
#include "midi_debug_monitor/midi_debug_monitor.h"
#include "definitions.h"
#include "usb/CLIPHIT2_usb.h"
#include "pin_mux.h"
#include "systick.h"

#include "fsl_lpuart.h"

#include "fsl_pit.h"

#ifdef ADC_ENABLE
#include "trigger/adc.h"
#include "trigger/trigger.h"
#endif	//ADC_ENABLE

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/**
 * PIT : CYCLE Timer 100ms
 */
#define PIT_CYCLE_TIMER_HANDLER PIT_IRQHandler
#define PIT_IRQ_ID PIT_IRQn
/* Get source clock for PIT driver */
#define PIT_SOURCE_CLOCK CLOCK_GetFreq(kCLOCK_OscClk)
#define CYCLE_TIMER_US_TIME	100000U	// 100ms

#ifdef ADC_ENABLE
#define NSEC_TO_COUNT(ns, clockFreqInHz) (uint64_t)(((uint64_t)(ns) * (clockFreqInHz)) / 1000000000U)
#define CYCLE_TIMER_NS_TIME	62500U	// 62.5us
//extern void AdcAudioDebugOn();
#endif	//ADC_ENABLE

void APPInit(void);
void composite_idle(void);
void MIDI_IF_IDLE(void);

extern usb_device_composite_struct_t g_composite;

void PIT_CYCLE_TIMER_HANDLER(void)
{
#ifdef ADC_ENABLE
	if (PIT_GetStatusFlags(PIT, kPIT_Chnl_0))
	{
	    /* Clear interrupt flag.*/
	    PIT_ClearStatusFlags(PIT, kPIT_Chnl_0, kPIT_TimerFlag);
	#if (defined(USB_DEVICE_CONFIG_EHCI) && (USB_DEVICE_CONFIG_EHCI > 0U))
	    USB_DeviceEhciAttachedDevice(g_composite.deviceHandle);
	#endif
		USB_setMidiInTimeoutCount();	/// @note MIDI IN Timeout (100ms)
	}
	if (PIT_GetStatusFlags(PIT, kPIT_Chnl_1))
	{
	    /* Clear interrupt flag.*/
	    PIT_ClearStatusFlags(PIT, kPIT_Chnl_1, kPIT_TimerFlag);
	    adc_start(0);

#else	//ADC_ENABLE
    /* Clear interrupt flag.*/
    PIT_ClearStatusFlags(PIT, kPIT_Chnl_0, kPIT_TimerFlag);
#if (defined(USB_DEVICE_CONFIG_EHCI) && (USB_DEVICE_CONFIG_EHCI > 0U))
    USB_DeviceEhciAttachedDevice(g_composite.deviceHandle);
#endif
	USB_setMidiInTimeoutCount();	/// @note MIDI IN Timeout (100ms)
	__DSB();
#endif	//ADC_ENABLE
	}
}

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
    //BOARD_InitDebugConsole();
    systick_init();	//systick on for JobTime

    /*Clock setting for LPI2C and SAI1 */
	BOARD_InitClockPinMux();

    /*Enable MCLK clock*/
    BOARD_EnableSaiMclkOutput(true);

    APPInit();

    /// @note [SAI] Implement Amp/Effects
	{
		void EditResume1stStatus(void);
		EditResume1stStatus();
	}

	while (1)
	{
#ifdef LOCAL_DEBUG_ENABLE
		composite_idle();
#ifdef ADC_ENABLE
		trigger_idle();
#endif	//ADC_ENABLE
	}
}


#ifdef LOCAL_DEBUG_ENABLE
void composite_idle(void);
extern void midi_hook_exec(void);
#endif	//LOCAL_DEBUG_ENABLE
void composite_idle(void)
{
	{
#endif	//LOCAL_DEBUG_ENABLE
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

#ifdef LOCAL_DEBUG_ENABLE
		else
		{	// for debug command
			MIDI_IF_IDLE();
		}
		midi_hook_exec();
#endif	//LOCAL_DEBUG_ENABLE
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

void APPInit(void)
{
		usb_init();
    Init_Board_Sai_Codec();
    audio_task_init();
    trigger_init();

    /* LPUART */
    {
        lpuart_config_t uartConfig;
        uint32_t uartClkSrcFreq = BOARD_DebugConsoleSrcFreq();	/// @note OK?

        /*
         * config.baudRate_Bps = 115200U;
         * config.parityMode = kLPUART_ParityDisabled;
         * config.stopBitCount = kLPUART_OneStopBit;
         * config.txFifoWatermark = 0;
         * config.rxFifoWatermark = 0;
         * config.enableTx = false;
         * config.enableRx = false;
         */
        LPUART_GetDefaultConfig(&uartConfig);
        uartConfig.baudRate_Bps = BOARD_UART_BAUDRATE;
        uartConfig.enableTx     = true;
        uartConfig.enableRx     = true;

        LPUART_Init(LPUART1, &uartConfig, uartClkSrcFreq);
        
        //@note UART Rx側の割込み許可は、USB SetConfigurationが来たら許可する
        //NVIC_SetPriority((IRQn_Type)BOARD_UART_IRQ, UART_INTERRUPT_PRIORITY);
    }
#ifdef LOCAL_DEBUG_ENABLE
        /* Enable RX interrupt. */
        midi_IF_RxInit();
        EnableIRQ(BOARD_UART_IRQ);
#endif

	/* PIT */
    {
    	/*
    	 * @note PITのサンプルプロジェクト参照.
    	 * これを入れると, PIT_SetTimerPeriodで
    	 * USEC_TO_COUNTをセットしたタイマでカウントしてくれるぽい
    	 */
        /* Set PERCLK_CLK source to OSC_CLK*/
        CLOCK_SetMux(kCLOCK_PerclkMux, 1U);
        /* Set PERCLK_CLK divider to 1 */
        CLOCK_SetDiv(kCLOCK_PerclkDiv, 0U);
    }
	{
        /* Structure of initialize PIT */
        pit_config_t pitConfig;

	    /*
	     * pitConfig.enableRunInDebug = false;
	     */
	    PIT_GetDefaultConfig(&pitConfig);

	    /* Init pit module */
	    PIT_Init(PIT, &pitConfig);

	    /**
	     * @note おまじない(とりあえずこのプロジェクトにのみ効く対策)
	     * o このプロジェクトは、MCUEpressoのPeripheral設定を介していないUSB_Speakerの
	     * o サンプルプロジェクトを元に作成している. 下記はそこでの弊害が予想される
	     * o おまじない  を入れないと, LTH/LTLの値がTSVの値にリセットされない模様
	     * o => 0xFFFFFFFF / 0xFFFFFFFFからダウンカウンタしていくので
	     * o 初回のPIT割り込みがかかるのが異常に遅くなる
	     * o Channel1がEnableになってしまっているので, Disableにしないと
	     * o LTHのカウンタが-1されていく => Channel1もStopする
	     * o おまじない を入れると, LTH/LTLの値がきちんと設定されるみたい
	     */
	    {
	    	PIT_StopTimer(PIT, kPIT_Chnl_0);
	    	PIT_StopTimer(PIT, kPIT_Chnl_1);
	    }

	    /* Set timer period for channel 0 */
	    PIT_SetTimerPeriod(PIT, kPIT_Chnl_0, USEC_TO_COUNT(CYCLE_TIMER_US_TIME, PIT_SOURCE_CLOCK));

	    /* Enable timer interrupts for channel 0 */
	    PIT_EnableInterrupts(PIT, kPIT_Chnl_0, kPIT_TimerInterruptEnable);

	    NVIC_SetPriority(PIT_IRQ_ID, PIT_INTERRUPT_PRIORITY);
	    /* Enable at the NVIC */
	    EnableIRQ(PIT_IRQ_ID);

	    /* Start channel 0 */
	    PIT_StartTimer(PIT, kPIT_Chnl_0);

#ifdef ADC_ENABLE

	    adc_init();

	    /* Set timer period for channel 1 */
	    PIT_SetTimerPeriod(PIT, kPIT_Chnl_1, NSEC_TO_COUNT(CYCLE_TIMER_NS_TIME, PIT_SOURCE_CLOCK));

	    /* Set timer chain mode for channel 1 */
	    PIT_SetTimerChainMode(PIT, kPIT_Chnl_1, false);

	    /* Enable timer interrupts for channel 1 */
	    PIT_EnableInterrupts(PIT, kPIT_Chnl_1, kPIT_TimerInterruptEnable);

	    /* Start channel 1 */
	    PIT_StartTimer(PIT, kPIT_Chnl_1);

	    //AdcAudioDebugOn();

#endif	//ADC_ENABLE
	}

	trigger_init();

}
