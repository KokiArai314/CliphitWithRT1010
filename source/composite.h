/*
 * composite.h
 *
 *  Created on: 2020/02/06
 *      Author: higuchi
 */

#ifndef COMPOSITE_H_
#define COMPOSITE_H_

#include "audio_player.h"
#include "midi_player.h"
#include "midi_debug_monitor/midi_debug_monitor.h"

/*******************************************************************************
* Definitions
******************************************************************************/
#if defined(USB_DEVICE_CONFIG_EHCI) && (USB_DEVICE_CONFIG_EHCI > 0U)
#define CONTROLLER_ID kUSB_ControllerEhci0
#endif

#define PIT_INTERRUPT_PRIORITY (6U)
#define SAI_INTERRUPT_PRIORITY (4U)
#define USB_DEVICE_INTERRUPT_PRIORITY (5U)
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

#endif /* COMPOSITE_H_ */
