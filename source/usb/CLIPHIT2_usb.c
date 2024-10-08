/*
 * usb.c
 *
 *  Created on: 2024/10/03
 *      Author: koki_arai
 */

#include "CLIPHIT2_usb.h"

#include "../definitions.h"
#include "board.h"
#include "pin_mux.h"
#include "clock_config.h"

#include "fsl_device_registers.h"
#include "fsl_debug_console.h"
#include "fsl_clock.h"
#include "fsl_pit.h"
#include <stdio.h>
#include <stdlib.h>

#include "usb.h"
#include "usb_device_class.h"
#include "usb_device_audio.h"
#include "usb_device_ch9.h"
#include "usb_device.h"
#include "usb_device_config.h"
#include "usb_device_descriptor.h"

#if (defined(FSL_FEATURE_SOC_SYSMPU_COUNT) && (FSL_FEATURE_SOC_SYSMPU_COUNT > 0U))
#include "fsl_sysmpu.h"
#endif /* FSL_FEATURE_SOC_SYSMPU_COUNT */


extern void USB_DeviceClockInit(void);
void USB_DeviceIsrEnable(void);
#if USB_DEVICE_CONFIG_USE_TASK
void USB_DeviceTaskFn(void *deviceHandle);
#endif

//Prototype
usb_status_t USB_DeviceCallback(usb_device_handle handle, uint32_t event, void *param);

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
void usb_init(void){

  USB_DeviceClockInit();
#if (defined(FSL_FEATURE_SOC_SYSMPU_COUNT) && (FSL_FEATURE_SOC_SYSMPU_COUNT > 0U))
  SYSMPU_Enable(SYSMPU, 0);
#endif /* FSL_FEATURE_SOC_SYSMPU_COUNT */

  g_composite.speed = USB_SPEED_FULL;
  g_composite.attach = 0U;
  g_composite.audioPlayer.audioHandle = (class_handle_t)NULL;
  g_composite.midiPlayer.midiHandle = (class_handle_t)NULL;
  g_composite.deviceHandle = NULL;
  
  if (kStatus_USB_Success != USB_DeviceClassInit(CONTROLLER_ID, &g_UsbDeviceCompositeConfigList, &g_composite.deviceHandle)){
    //usb_echo("USB device composite demo init failed\r\n");
    return;
  }else{
    //usb_echo("USB device composite demo\r\n");
    g_composite.audioPlayer.audioHandle = g_UsbDeviceCompositeConfigList.config[0].classHandle;
    g_composite.midiPlayer.midiHandle = g_UsbDeviceCompositeConfigList.config[1].classHandle;
    USB_DeviceAudioPlayerInit(&g_composite);
    USB_DeviceMidiPlayerInit(&g_composite);
  }

  /* Install isr, set priority, and enable IRQ. */
  USB_DeviceIsrEnable();
  USB_DeviceRun(g_composite.deviceHandle);

}

static uint8_t keepAttachedDevice;
void checkAttachedDevice(void)
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

void USB_OTG1_IRQHandler(void)
{
#if (defined(USB_DEVICE_CONFIG_EHCI) && (USB_DEVICE_CONFIG_EHCI > 0U))
    USB_DeviceEhciIsrFunction(g_composite.deviceHandle);
#endif
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

