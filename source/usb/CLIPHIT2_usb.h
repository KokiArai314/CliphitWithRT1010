/*
 * usb.h
 *
 *  Created on: 2024/10/03
 *      Author: koki_arai
 */

#include "audio_task/audio_player.h"
#include "midi/midi_player.h"
#include "midi_debug_monitor/midi_debug_monitor.h"

#ifndef CLIPHIT2_USB_H_
#define CLIPHIT2_USB_H_
/*******************************************************************************
* Definitions
******************************************************************************/
#if defined(USB_DEVICE_CONFIG_EHCI) && (USB_DEVICE_CONFIG_EHCI > 0U)
#define CONTROLLER_ID kUSB_ControllerEhci0
#endif

#define PIT_INTERRUPT_PRIORITY (0U)	//Trigger(ADC)
#define SAI_INTERRUPT_PRIORITY (5U)	//Audio
#define USB_DEVICE_INTERRUPT_PRIORITY (3U)	
#define UART_INTERRUPT_PRIORITY (3U)

#define USB_DEVICE_INTERFACE_COUNT (2U)	/// @note USB Audio Play/Rec

typedef struct _usb_device_composite_struct
{
    usb_device_handle deviceHandle;
    usb_audio_player_struct_t audioPlayer;
    usb_midi_player_struct_t midiPlayer;
    uint8_t speed;
    uint8_t attach;
    uint8_t currentConfiguration;
    uint8_t currentInterfaceAlternateSetting[USB_DEVICE_INTERFACE_COUNT];
} usb_device_composite_struct_t;

/*******************************************************************************
* API
******************************************************************************/
/* AUDIO */
usb_status_t USB_DeviceAudioCallback(class_handle_t handle, uint32_t event, void *param);

usb_status_t USB_DeviceAudioSetConfigure(class_handle_t handle, uint8_t configure);

usb_status_t USB_DeviceAudioSetInterface(class_handle_t handle, uint8_t interface,
                                                        uint8_t alternateSetting);

usb_status_t USB_DeviceAudioPlayerInit(usb_device_composite_struct_t *device_composite);

/* MIDI */
usb_status_t USB_DeviceMidiCallback(class_handle_t handle, uint32_t event, void *param);

usb_status_t USB_DeviceMidiSetConfigure(class_handle_t handle, uint8_t configure);

usb_status_t USB_DeviceMidiPlayerInit(usb_device_composite_struct_t *device_composite);

void checkAttachedDevice(void);
void usb_init(void);


#endif /* CLIPHIT2_USB_H_ */
