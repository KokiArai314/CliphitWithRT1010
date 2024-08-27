/*
 * composite.c
 *
 *  Created on: 2020/02/06
 *      Author: higuchi
 */

#include "usb_device_config.h"
#include "usb.h"
#include "usb_device.h"

#include "usb_device_class.h"
#include "usb_device_audio.h"
#include "usb_device_ch9.h"
#include "usb_device_descriptor.h"

#include "composite.h"
#include "pin_mux.h"

#include "fsl_device_registers.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_debug_console.h"
#include <stdio.h>
#include <stdlib.h>
#if (defined(FSL_FEATURE_SOC_SYSMPU_COUNT) && (FSL_FEATURE_SOC_SYSMPU_COUNT > 0U))
#include "fsl_sysmpu.h"
#endif /* FSL_FEATURE_SOC_SYSMPU_COUNT */

#include "fsl_clock.h"
#include "fsl_pit.h"

#if 0	/// @note RT1020-EVK debugPin (board J18:4pin)
#include "fsl_iomuxc.h"
#endif

#include "fsl_lpuart.h"
#include "midi_if.h"
#include "midi_player.h"
#include "audio_task/audio_task.h"

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


/*******************************************************************************
 * Prototypes
 ******************************************************************************/
extern void BOARD_InitHardware(void);
extern void USB_DeviceClockInit(void);
void USB_DeviceIsrEnable(void);
#if USB_DEVICE_CONFIG_USE_TASK
void USB_DeviceTaskFn(void *deviceHandle);
#endif

extern usb_status_t USB_DeviceAudioCallback(class_handle_t handle, uint32_t event, void *param);
extern usb_status_t USB_DeviceMidiCallback(class_handle_t handle, uint32_t event, void *param);
usb_status_t USB_DeviceCallback(usb_device_handle handle, uint32_t event, void *param);

extern void BOARD_USB_Audio_TxInit(uint32_t samplingRate);
extern void BOARD_Codec_Init(void);

extern void BOARD_DMA_EDMA_Config(void);
extern void BOARD_Create_Audio_DMA_EDMA_Handle(void);
extern void BOARD_DMA_EDMA_Set_AudioFormat(void);
extern void BOARD_DMA_EDMA_Enable_Audio_Interrupts(void);
extern void BOARD_DMA_EDMA_Start(void);

extern void Init_Board_Sai_Codec(void);
extern void USB_AudioCodecTask(void);
extern void USB_AudioSpeakerResetTask(void);

// main()
extern void BOARD_AudioInitPllClock(void);
extern void BOARD_InitClockPinMux(void);
extern void BOARD_EnableSaiMclkOutput(bool enable);

// PIT_Handler
extern void USB_setMidiInTimeoutCount(void);

/*******************************************************************************
* Variables
******************************************************************************/
usb_device_composite_struct_t g_composite;

extern usb_device_class_struct_t g_UsbDeviceAudioClass;
extern usb_device_class_struct_t g_UsbDeviceMidiClass;

/* USB device class information */
static usb_device_class_config_struct_t g_CompositeClassConfig[2] = {
    {
        USB_DeviceAudioCallback, (class_handle_t)NULL, &g_UsbDeviceAudioClass,
    },
    {
        USB_DeviceMidiCallback, (class_handle_t)NULL, &g_UsbDeviceMidiClass,
    }
};

/* USB device class configuraion information */
static usb_device_class_config_list_struct_t g_UsbDeviceCompositeConfigList = {
    g_CompositeClassConfig, USB_DeviceCallback, 2,
};

/*******************************************************************************
 * Code
 ******************************************************************************/
void USB_OTG1_IRQHandler(void)
{
#if (defined(USB_DEVICE_CONFIG_EHCI) && (USB_DEVICE_CONFIG_EHCI > 0U))
    USB_DeviceEhciIsrFunction(g_composite.deviceHandle);
#endif
}

void PIT_CYCLE_TIMER_HANDLER(void)
{
    /* Clear interrupt flag.*/
    PIT_ClearStatusFlags(PIT, kPIT_Chnl_0, kPIT_TimerFlag);
#if (defined(USB_DEVICE_CONFIG_EHCI) && (USB_DEVICE_CONFIG_EHCI > 0U))
    USB_DeviceEhciAttachedDevice(g_composite.deviceHandle);
#endif
	USB_setMidiInTimeoutCount();	/// @note MIDI IN Timeout (100ms)
	__DSB();
}

void USB_DeviceIsrEnable(void)
{
    uint8_t irqNumber;
#if defined(USB_DEVICE_CONFIG_EHCI) && (USB_DEVICE_CONFIG_EHCI > 0U)
    uint8_t usbDeviceEhciIrq[] = USBHS_IRQS;
    irqNumber = usbDeviceEhciIrq[CONTROLLER_ID - kUSB_ControllerEhci0];
#endif
/* Install isr, set priority, and enable IRQ. */
    NVIC_SetPriority((IRQn_Type)irqNumber, USB_DEVICE_INTERRUPT_PRIORITY);
    EnableIRQ((IRQn_Type)irqNumber);
}
#if USB_DEVICE_CONFIG_USE_TASK
void USB_DeviceTaskFn(void *deviceHandle)
{
#if defined(USB_DEVICE_CONFIG_EHCI) && (USB_DEVICE_CONFIG_EHCI > 0U)
    USB_DeviceEhciTaskFunction(deviceHandle);
#endif
}
#endif

/*!
 * @brief USB device callback function.
 *
 * This function handles the usb device specific requests.
 *
 * @param handle		  The USB device handle.
 * @param event 		  The USB device event type.
 * @param param 		  The parameter of the device specific request.
 *
 * @return A USB error code or kStatus_USB_Success.
 */
usb_status_t USB_DeviceCallback(usb_device_handle handle, uint32_t event, void *param)
{
    usb_status_t error = kStatus_USB_Error;
    uint8_t *temp8 = (uint8_t *)param;
    uint16_t *temp16 = (uint16_t *)param;

    switch (event)
    {
        case kUSB_DeviceEventBusReset:
        {
        	g_composite.audioPlayer.attach = 0U;
        	g_composite.midiPlayer.attach = 0U;
        	{
                /* Disable RX interrupt. */
                DisableIRQ(LPUART1_IRQn);
        	}
            error = kStatus_USB_Success;
#if (defined(USB_DEVICE_CONFIG_EHCI) && (USB_DEVICE_CONFIG_EHCI > 0U)) || \
    (defined(USB_DEVICE_CONFIG_LPCIP3511HS) && (USB_DEVICE_CONFIG_LPCIP3511HS > 0U))
            /* Get USB speed to configure the device, including max packet size and interval of the endpoints. */
            if (kStatus_USB_Success == USB_DeviceClassGetSpeed(CONTROLLER_ID, &g_composite.speed))
            {
                USB_DeviceSetSpeed(handle, g_composite.speed);
                g_composite.audioPlayer.speed = g_composite.speed;
                g_composite.midiPlayer.speed = g_composite.speed;
            }
            if (USB_SPEED_HIGH == g_composite.audioPlayer.speed)
            {
            	g_composite.audioPlayer.currentStreamOutMaxPacketSize =
                    (HS_ISO_OUT_ENDP_PACKET_SIZE + AUDIO_FORMAT_CHANNELS * AUDIO_FORMAT_SIZE);
            	g_composite.audioPlayer.currentFeedbackMaxPacketSize = HS_ISO_FEEDBACK_ENDP_PACKET_SIZE;
            }
            if (USB_SPEED_HIGH == g_composite.midiPlayer.speed) {
            	g_composite.midiPlayer.currentStreamMaxPacketSize = HS_USB_MIDI_ENDP_MAX_PACKET_SIZE;
            }

#endif
        }
        break;
        case kUSB_DeviceEventSetConfiguration:
#if 1	/// @note support composite MIDI
        	if (USB_AUDIO_MIDI_CONFIGURE_INDEX == (*temp8))
        	{
        		USB_DeviceAudioSetConfigure(g_composite.audioPlayer.audioHandle, *temp8);
				USB_DeviceMidiSetConfigure(g_composite.midiPlayer.midiHandle, *temp8);
				error = kStatus_USB_Success;
        	}
#else
            if (USB_AUDIO_SPEAKER_CONFIGURE_INDEX == (*temp8))
            {
            	g_composite.audioPlayer.attach = 1U;
            	g_composite.audioPlayer.currentConfiguration = *temp8;
            }
#endif
            break;
        case kUSB_DeviceEventSetInterface:
            if (g_composite.audioPlayer.attach)
            {
                uint8_t interface = (uint8_t)((*temp16 & 0xFF00U) >> 0x08U);
                uint8_t alternateSetting = (uint8_t)(*temp16 & 0x00FFU);

                USB_DeviceAudioSetInterface(g_composite.audioPlayer.audioHandle,
                							interface,
											alternateSetting);
                error = kStatus_USB_Success;
            }
            break;
        case kUSB_DeviceEventGetConfiguration:
            if (param)
            {
                *temp8 = g_composite.audioPlayer.currentConfiguration;
                error = kStatus_USB_Success;
            }
            break;
        case kUSB_DeviceEventGetInterface:
            if (param)
            {
                /* Get current alternate setting of the interface request */
                uint8_t interface = (uint8_t)((*temp16 & 0xFF00U) >> 0x08U);
                if (interface < USB_AUDIO_PLAYER_INTERFACE_COUNT)
                {
                    *temp16 = (*temp16 & 0xFF00U) | g_composite.audioPlayer.currentInterfaceAlternateSetting[interface];
                    error = kStatus_USB_Success;
                }
                else
                {
                    error = kStatus_USB_InvalidRequest;
                }
            }
            break;
        case kUSB_DeviceEventGetDeviceDescriptor:
            if (param)
            {
                /* Get device descriptor request */
                error = USB_DeviceGetDeviceDescriptor(handle, (usb_device_get_device_descriptor_struct_t *)param);
            }
            break;
        case kUSB_DeviceEventGetConfigurationDescriptor:
            if (param)
            {
                /* Get device configuration descriptor request */
                error = USB_DeviceGetConfigurationDescriptor(handle,
                                                             (usb_device_get_configuration_descriptor_struct_t *)param);
            }
            break;
        case kUSB_DeviceEventGetStringDescriptor:
            if (param)
            {
                /* Get device string descriptor request */
                error = USB_DeviceGetStringDescriptor(handle, (usb_device_get_string_descriptor_struct_t *)param);
            }
            break;
        case kUSB_DeviceEventGetDeviceQualifierDescriptor:
        	if (param) {
                /* Get device qualifier descriptor request */
                error = USB_DeviceGetDeviceQualifierDescriptor(handle, (usb_device_get_device_qualifier_descriptor_struct_t *)param);
        	}
        	break;
        case kUSB_DeviceEventGetOtherSpeedConfigurationDescriptor:
        	if (param) {
                /* Get device other speed configuration descriptor request */
                error = USB_DeviceGetOtherSpeedConfigurationDescriptor(handle, (usb_device_get_device_other_speed_configuration_descriptor_struct_t *)param);
        	}
        	break;
        default:
            break;
    }

    return error;
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
    USB_DeviceClockInit();
#if (defined(FSL_FEATURE_SOC_SYSMPU_COUNT) && (FSL_FEATURE_SOC_SYSMPU_COUNT > 0U))
    SYSMPU_Enable(SYSMPU, 0);
#endif /* FSL_FEATURE_SOC_SYSMPU_COUNT */

    {
    	g_composite.speed = USB_SPEED_FULL;
    	g_composite.attach = 0U;
    	g_composite.audioPlayer.audioHandle = (class_handle_t)NULL;
    	g_composite.midiPlayer.midiHandle = (class_handle_t)NULL;
    	g_composite.deviceHandle = NULL;
    }

    if (kStatus_USB_Success !=
        USB_DeviceClassInit(CONTROLLER_ID, &g_UsbDeviceCompositeConfigList, &g_composite.deviceHandle))
    {
        usb_echo("USB device composite demo init failed\r\n");
        return;
    }
    else
    {
        usb_echo("USB device composite demo\r\n");
        g_composite.audioPlayer.audioHandle = g_UsbDeviceCompositeConfigList.config[0].classHandle;
        g_composite.midiPlayer.midiHandle = g_UsbDeviceCompositeConfigList.config[1].classHandle;

        USB_DeviceAudioPlayerInit(&g_composite);
        USB_DeviceMidiPlayerInit(&g_composite);
    }

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
   //     NVIC_SetPriority(((IRQn_Type)LPUART1_IRQn, UART_INTERRUPT_PRIORITY);

        /**
         * @note UART Rx側の割込み許可は、USB SetConfigurationが来たら許可する
         */
    }

    /* Install isr, set priority, and enable IRQ. */
    USB_DeviceIsrEnable();

    USB_DeviceRun(g_composite.deviceHandle);

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
	}

	trigger_init();

}

static uint8_t keepAttachedDevice;
static void checkAttachedDevice(void)
{
	uint8_t attachedDevice;

	attachedDevice = 0;
#if ((defined(USB_DEVICE_CONFIG_EHCI)) && (USB_DEVICE_CONFIG_EHCI > 0U))
	attachedDevice = USB_DeviceEhciGetAttachedDeviceStatus();
#endif
	/**
	 * attach=1はSetConfig受信で得る. attach=0時のみ, audioPlayer/midiPlayerのattach情報に反映
	 */
	if ((attachedDevice == 0) && (attachedDevice != keepAttachedDevice)) {
		/*　再初期化 */
        USB_DeviceAudioPlayerInit(&g_composite);
        USB_DeviceMidiPlayerInit(&g_composite);
	}
	keepAttachedDevice = attachedDevice;
}

/*!
 * @brief Application task function.
 *
 * This function runs the task for application.
 *
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
//    BOARD_InitDebugConsole();

#if 0	/// @note RT1020-EVK debugPin (board J18:4pin) => RT1010-EVK???
    IOMUXC_SetPinMux(
  	  IOMUXC_GPIO_AD_B1_13_GPIO1_IO29,        /* GPIO_AD_B1_13 is configured as GPIO */
        0U);                                    /* Software Input On Field: Force input path of pad GPIO_AD_B1_13 */
    GPIO_PinWrite(GPIO1, 29U, 1);	// GPIO1_IO29 Output:1
    GPIO1->GDIR |= (1U << 29U); /*!< Enable target USER_LED */
//	GPIO_PortSet(GPIO1, 1U << 29);
	GPIO_PortClear(GPIO1, 1U << 29U);
#endif
#if 0	/// @note RT1010-EVK USER_LED
    GPIO_PinWrite(BOARD_USER_LED_GPIO, BOARD_USER_LED_GPIO_PIN, 1);
    BOARD_USER_LED_GPIO->GDIR |= (1U << BOARD_USER_LED_GPIO_PIN);
    GPIO_PortClear(BOARD_USER_LED_GPIO, 1U << BOARD_USER_LED_GPIO_PIN);
#endif

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
#if USB_DEVICE_CONFIG_USE_TASK
		USB_DeviceTaskFn(g_composite.deviceHandle);
#endif

		trigger_idle();
	}
}
