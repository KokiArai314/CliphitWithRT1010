/*
 * Copyright (c) 2015 - 2016, Freescale Semiconductor, Inc.
 * Copyright 2016 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __USB_DEVICE_DESCRIPTOR_H__
#define __USB_DEVICE_DESCRIPTOR_H__

/*******************************************************************************
* Definitions
******************************************************************************/

#define USB_DEVICE_SPECIFIC_BCD_VERSION (0x0200U)
#define USB_DEVICE_DEMO_BCD_VERSION (0x0100U)	//(0x0101U)

#define USB_DEVICE_MAX_POWER (0x00)	//(0x32U)

/* usb descritpor length */
#define USB_DESCRIPTOR_LENGTH_CONFIGURATION_ALL (sizeof(g_UsbDeviceConfigurationDescriptor))

#define USB_ENDPOINT_AUDIO_DESCRIPTOR_LENGTH (9)
#define USB_AUDIO_CLASS_SPECIFIC_ENDPOINT_LENGTH (8)
#define USB_AUDIO_STANDARD_AS_ISO_DATA_ENDPOINT_LENGTH (7)
#define USB_AUDIO_STANDARD_AS_ISO_FEEDBACK_ENDPOINT_LENGTH (7)
#if 1	/// @note AudioRec + Play
#define USB_AUDIO_CONTROL_INTERFACE_HEADER_LENGTH (8+2)
#endif
#if 0	/// @note AudioPlay Only
#define USB_AUDIO_CONTROL_INTERFACE_HEADER_LENGTH (9)
#endif
#define USB_DESCRIPTOR_LENGTH_AC_INTERRUPT_ENDPOINT (9)
#define USB_AUDIO_INPUT_TERMINAL_ONLY_DESC_SIZE (12)
#define USB_AUDIO_OUTPUT_TERMINAL_ONLY_DESC_SIZE (9)
#define USB_AUDIO_FEATURE_UNIT_ONLY_DESC_SIZE(ch, n) (0x07 + (ch + 1) * n)
#define USB_AUDIO_STREAMING_IFACE_DESC_SIZE (7)
#define USB_AUDIO_STREAMING_ENDP_DESC_SIZE (7)
#define USB_AUDIO_STREAMING_TYPE_I_DESC_SIZE (11)

/* Configuration, interface and endpoint. */
#define USB_DEVICE_CONFIGURATION_COUNT (1)	/// @note memo: 常に1
#define USB_DEVICE_STRING_COUNT (6)
#define USB_DEVICE_LANGUAGE_COUNT (1)

#define USB_AUDIO_MIDI_CONFIGURE_INDEX (1)	/// @note memo:ずっと1でOK

/// @note memo:USB_AUDIO_PLAYER_INTERFACE_COUNTのIndexの並びと思ってよい
#define USB_AUDIO_CONTROL_INTERFACE_INDEX (0)
#define USB_AUDIO_STREAM_INTERFACE_INDEX (1)
/// @note add AudioRec
#define USB_AUDIO_IN_STREAM_INTERFACE_INDEX (2)

#define USB_AUDIO_OFFSET_ENDPOINT_INTERFACE_INDEX	(USB_AUDIO_STREAM_INTERFACE_INDEX - USB_AUDIO_CONTROL_INTERFACE_INDEX)

#define USB_AUDIO_STREAM_ENDPOINT_COUNT (2)
/// @note add AudioRec
#define USB_AUDIO_IN_STREAM_ENDPOINT_COUNT (1)

/// @note memo:各EndpointのEP番号
#define USB_AUDIO_SPEAKER_STREAM_ENDPOINT (1)
#define USB_AUDIO_SPEAKER_FEEDBACK_ENDPOINT (1)

/// @note add AudioRec
#define USB_AUDIO_RECORDER_STREAM_ENDPOINT (3)

/// @note support composite MIDI
#define USB_MIDI_STREAM_ENDPOINT 	(2)
#define USB_MIDI_STREAM_RX_ENDPOINT (USB_MIDI_STREAM_ENDPOINT)
#define USB_MIDI_STREAM_TX_ENDPOINT (USB_MIDI_STREAM_ENDPOINT)

/// @note AudioPlay + Rec
#define USB_AUDIO_PLAYER_INTERFACE_COUNT 5
	/*
    	(USB_AUDIO_SPEAKER_CONTROL_INTERFACE_COUNT +
    	 USB_AUDIO_SPEAKER_STREAM_INTERFACE_COUNT +
    	 USB_AUDIO_RECORDER_STREAM_INTERFACE_COUNT +
    	 USB_MIDI_CONTROL_INTERFACE_COUNT +
    	 USB_MIDI_STREAMING_INTERFACE_COUNT)
    */

#if 0	/// @note AudioPlay Only
#define USB_AUDIO_PLAYER_INTERFACE_COUNT \
    (USB_AUDIO_SPEAKER_CONTROL_INTERFACE_COUNT + USB_AUDIO_SPEAKER_STREAM_INTERFACE_COUNT)
#endif

#define USB_AUDIO_SPEAKER_CONTROL_INTERFACE_COUNT (1)
#define USB_AUDIO_SPEAKER_STREAM_INTERFACE_COUNT (1)
/// @note add AudioRec
#define USB_AUDIO_RECORDER_STREAM_INTERFACE_COUNT (1)

/* Audio data format */
#define AUDIO_FORMAT_CHANNELS (0x02)
#define AUDIO_IN_FORMAT_CHANNELS (0x02)
#if 1	/// @note support 24bit
#define AUDIO_FORMAT_BITS (24)
#define AUDIO_FORMAT_SIZE (0x03)
#else
#define AUDIO_FORMAT_BITS (16)
#define AUDIO_FORMAT_SIZE (0x02)
#endif

/* Packet size and interval. */
#define HS_ISO_OUT_ENDP_PACKET_SIZE                    \
    (AUDIO_SAMPLING_RATE_KHZ * AUDIO_FORMAT_CHANNELS * \
     AUDIO_FORMAT_SIZE) /* This should be changed to 192 if sampling rate is 48k */
#define FS_ISO_OUT_ENDP_PACKET_SIZE (AUDIO_SAMPLING_RATE_KHZ * AUDIO_FORMAT_CHANNELS * AUDIO_FORMAT_SIZE)

/// @note add AudioRec
#define HS_ISO_IN_ENDP_PACKET_SIZE	HS_ISO_OUT_ENDP_PACKET_SIZE
#define FS_ISO_IN_ENDP_PACKET_SIZE	FS_ISO_OUT_ENDP_PACKET_SIZE

#if 1	/// @note Audio Class 1.0
#define HS_ISO_FEEDBACK_ENDP_PACKET_SIZE (3)
#else	/// @note Audio class 2.0(USB High Speed)
#define HS_ISO_FEEDBACK_ENDP_PACKET_SIZE (4)
#endif
#define FS_ISO_FEEDBACK_ENDP_PACKET_SIZE (3)
#define HS_ISO_OUT_ENDP_INTERVAL (0x04)
#define HS_ISO_IN_ENDP_INTERVAL (0x04)
#define FS_ISO_OUT_ENDP_INTERVAL (0x01)
#define FS_ISO_IN_ENDP_INTERVAL (0x01)
#define ISO_OUT_ENDP_INTERVAL (0x01)

/* String descriptor length. */
#define USB_DESCRIPTOR_LENGTH_STRING0 (sizeof(g_UsbDeviceString0))
#define USB_DESCRIPTOR_LENGTH_STRING1 (sizeof(g_UsbDeviceString1))
#define USB_DESCRIPTOR_LENGTH_STRING2 (sizeof(g_UsbDeviceString2))
#define USB_DESCRIPTOR_LENGTH_STRING3 (sizeof(g_UsbDeviceString3))
#define USB_DESCRIPTOR_LENGTH_STRING4 (sizeof(g_UsbDeviceString4))
#define USB_DESCRIPTOR_LENGTH_STRING5 (sizeof(g_UsbDeviceString5))

#define USBD_IDX_LANGID_STR		(0x00)
#define USBD_IDX_MFC_STR		(0x01)
#define USBD_IDX_PRODUCT_STR	(0x02)
#define USBD_IDX_AUDIO_OUT_STR	(0x03)
#define USBD_IDX_AUDIO_IN_STR	(0x04)
#define USBD_IDX_MIDI_STR		(0x05)

/* AUDIO_STR, MIDI_OUT_STR, MIDI_IN_STR */
#define USBD_IDX_AUDIO_STR		(0x03)
#define USBD_IDX_MIDI_OUT_STR	(0x04)
#define USBD_IDX_MIDI_IN_STR	(0x05)

/* Class code. */
#define USB_DEVICE_CLASS (0x00)
#define USB_DEVICE_SUBCLASS (0x00)
#define USB_DEVICE_PROTOCOL (0x00)

#define USB_DEVICE_CLASS_MISCELLANEOUS_DC (0xEF)	// Miscellaneous Device Class
#define USB_DEVICE_SUBCLASS_COMMON	(0x02)			// Common Class
#define USB_DEVICE_PROTOCOL_IAD	(0x01)				// Interface Association Descriptor

#define USB_AUDIO_CLASS (0x01)
#define USB_SUBCLASS_AUDIOCONTROL (0x01)
#define USB_SUBCLASS_AUDIOSTREAM (0x02)
#define USB_AUDIO_PROTOCOL (0x00)

#define USB_AUDIO_FORMAT_TYPE_I (0x01)
#define USB_AUDIO_STREAM_ENDPOINT_DESCRIPTOR (0x25)
#define USB_AUDIO_EP_GENERAL_DESCRIPTOR_SUBTYPE (0x01)

#define USB_AUDIO_SPEAKER_CONTROL_INPUT_TERMINAL_ID (0x01)
#define USB_AUDIO_SPEAKER_CONTROL_OUTPUT_TERMINAL_ID (0x02)
/// @note add AudioRec
#define USB_AUDIO_RECORDER_CONTROL_INPUT_TERMINAL_ID (0x03)
#define USB_AUDIO_RECORDER_CONTROL_OUTPUT_TERMINAL_ID (0x04)

/* USB MIDI Streaming Class */
#define USB_MIDI_ENDP_MAX_PACKET_SIZE		(64)
#define HS_USB_MIDI_ENDP_MAX_PACKET_SIZE	(512)

#define USB_MIDI_PLAYER_INTERFACE_COUNT 2

#define USB_MIDI_STREAM_ENDPOINT_COUNT	(2)

#define USB_MIDI_OUT_STREAM_ENDPOINT	(2)
#define USB_MIDI_IN_STREAM_ENDPOINT		(2)

#define USB_MIDI_CONTROL_INTERFACE_INDEX (3)
#define USB_MIDI_STREAM_INTERFACE_INDEX (4)
#define USB_MIDI_OFFSET_ENDPOINT_INTERFACE_INDEX	(USB_MIDI_STREAM_INTERFACE_INDEX - USB_MIDI_CONTROL_INTERFACE_INDEX)

#define USB_SUBCLASS_MIDISTREAM (0x03)

#define USB_MIDI_CONTROL_INTERFACE_HEADER_LENGTH (8+1)

#define	TYPE_MS_HEADER							0x01
#define	TYPE_MIDI_IN_JACK						0x02
#define	TYPE_MIDI_OUT_JACK						0x03
#define	TYPE_ELEMENT							0x04
/* MS Class-Specific Endpoint Descriptor Subtypes */
#define	TYPE_MS_GENERAL							0x01
/* MS MIDI In and OUT Jack Types */
#define	TYPE_MS_EMBEDDED						0x01
#define	TYPE_MS_EXTERNAL						0x02

#define	SIZE_OF_STD_MS_IF_DESC					9
#define SIZE_OF_CS_MS_IF_HDR_DESC				7
#define	SIZE_OF_MIDI_IN_JACK_DESC				6
#define	SIZE_OF_MIDI_OUT_JACK_DESC				(7 + 2)

#define	SIZE_OF_STD_MS_BD_EP_DESC				9
#define	SIZE_OF_CS_MS_BD_EP_DESC				(4 + 1)

#define	SIZE_OF_MS_HEADER_CONTENT																			\
	  SIZE_OF_CS_MS_IF_HDR_DESC				/* Class-Specific MS Interface Header Descriptor			*/	\
	+ 2 * SIZE_OF_MIDI_IN_JACK_DESC 			/* MIDI IN (from host) Jack Descriptor 					*/	\
	+ 2 * SIZE_OF_MIDI_OUT_JACK_DESC			/* MIDI OUT (to host) Jack Descriptor					*/


/* Total Size of Configuration Descriptor */
/*
 * 		USB_DESCRIPTOR_LENGTH_CONFIGURE +
		//Interface Assosiation Desc
		USB_DESCRIPTOR_LENGTH_INTERFACE_ASSOSIATION +
		// Interface 0
		USB_DESCRIPTOR_LENGTH_INTERFACE +
		USB_AUDIO_CONTROL_INTERFACE_HEADER_LENGTH +
		USB_AUDIO_INPUT_TERMINAL_ONLY_DESC_SIZE + USB_AUDIO_OUTPUT_TERMINAL_ONLY_DESC_SIZE +
		USB_AUDIO_INPUT_TERMINAL_ONLY_DESC_SIZE + USB_AUDIO_OUTPUT_TERMINAL_ONLY_DESC_SIZE +
		// Interface 1
		USB_DESCRIPTOR_LENGTH_INTERFACE + USB_DESCRIPTOR_LENGTH_INTERFACE +
		USB_AUDIO_STREAMING_IFACE_DESC_SIZE + USB_AUDIO_STREAMING_TYPE_I_DESC_SIZE +
		USB_ENDPOINT_AUDIO_DESCRIPTOR_LENGTH + USB_AUDIO_STREAMING_ENDP_DESC_SIZE +
		USB_ENDPOINT_AUDIO_DESCRIPTOR_LENGTH +
		// Interface 2
		USB_DESCRIPTOR_LENGTH_INTERFACE + USB_DESCRIPTOR_LENGTH_INTERFACE +
		USB_AUDIO_STREAMING_IFACE_DESC_SIZE + USB_AUDIO_STREAMING_TYPE_I_DESC_SIZE +
		USB_ENDPOINT_AUDIO_DESCRIPTOR_LENGTH + USB_AUDIO_STREAMING_ENDP_DESC_SIZE +
		// Interface Assosiation Desc
		USB_DESCRIPTOR_LENGTH_INTERFACE_ASSOSIATION +
		// Interface 3
		USB_DESCRIPTOR_LENGTH_INTERFACE + USB_MIDI_CONTROL_INTERFACE_HEADER_LENGTH +
		// Interface 4
		USB_DESCRIPTOR_LENGTH_INTERFACE +
		SIZE_OF_CS_MS_IF_HDR_DESC +
		SIZE_OF_MIDI_IN_JACK_DESC + SIZE_OF_MIDI_OUT_JACK_DESC +
		SIZE_OF_MIDI_OUT_JACK_DESC + SIZE_OF_MIDI_IN_JACK_DESC +
		SIZE_OF_STD_MS_BD_EP_DESC + SIZE_OF_CS_MS_BD_EP_DESC +
		SIZE_OF_STD_MS_BD_EP_DESC + SIZE_OF_CS_MS_BD_EP_DESC
 */
#define SIZE_OF_TOTAL_CONFIG_DESC																			\
		USB_DESCRIPTOR_LENGTH_CONFIGURE																		\
		+ USB_DESCRIPTOR_LENGTH_INTERFACE_ASSOSIATION														\
		+ USB_DESCRIPTOR_LENGTH_INTERFACE																	\
		+ USB_AUDIO_CONTROL_INTERFACE_HEADER_LENGTH															\
		+ USB_AUDIO_INPUT_TERMINAL_ONLY_DESC_SIZE + USB_AUDIO_OUTPUT_TERMINAL_ONLY_DESC_SIZE				\
		+ USB_AUDIO_INPUT_TERMINAL_ONLY_DESC_SIZE + USB_AUDIO_OUTPUT_TERMINAL_ONLY_DESC_SIZE				\
		+ USB_DESCRIPTOR_LENGTH_INTERFACE + USB_DESCRIPTOR_LENGTH_INTERFACE									\
		+ USB_AUDIO_STREAMING_IFACE_DESC_SIZE + USB_AUDIO_STREAMING_TYPE_I_DESC_SIZE						\
		+ USB_ENDPOINT_AUDIO_DESCRIPTOR_LENGTH + USB_AUDIO_STREAMING_ENDP_DESC_SIZE							\
		+ USB_ENDPOINT_AUDIO_DESCRIPTOR_LENGTH																\
		+ USB_DESCRIPTOR_LENGTH_INTERFACE + USB_DESCRIPTOR_LENGTH_INTERFACE									\
		+ USB_AUDIO_STREAMING_IFACE_DESC_SIZE + USB_AUDIO_STREAMING_TYPE_I_DESC_SIZE						\
		+ USB_ENDPOINT_AUDIO_DESCRIPTOR_LENGTH + USB_AUDIO_STREAMING_ENDP_DESC_SIZE							\
		+ USB_DESCRIPTOR_LENGTH_INTERFACE_ASSOSIATION														\
		+ USB_DESCRIPTOR_LENGTH_INTERFACE + USB_MIDI_CONTROL_INTERFACE_HEADER_LENGTH						\
		+ USB_DESCRIPTOR_LENGTH_INTERFACE																	\
		+ SIZE_OF_CS_MS_IF_HDR_DESC																			\
		+ SIZE_OF_MIDI_IN_JACK_DESC + SIZE_OF_MIDI_OUT_JACK_DESC											\
		+ SIZE_OF_MIDI_OUT_JACK_DESC + SIZE_OF_MIDI_IN_JACK_DESC											\
		+ SIZE_OF_STD_MS_BD_EP_DESC + SIZE_OF_CS_MS_BD_EP_DESC												\
		+ SIZE_OF_STD_MS_BD_EP_DESC + SIZE_OF_CS_MS_BD_EP_DESC


/* Offset (different between USB High Speed and USB Full Speed) */
#define SIZE_OF_INTERFACE0																					\
		USB_DESCRIPTOR_LENGTH_INTERFACE + USB_AUDIO_CONTROL_INTERFACE_HEADER_LENGTH							\
		+ USB_AUDIO_INPUT_TERMINAL_ONLY_DESC_SIZE + USB_AUDIO_OUTPUT_TERMINAL_ONLY_DESC_SIZE				\
		+ USB_AUDIO_INPUT_TERMINAL_ONLY_DESC_SIZE + USB_AUDIO_OUTPUT_TERMINAL_ONLY_DESC_SIZE

#define SIZE_OF_INTERFACE1																					\
		USB_DESCRIPTOR_LENGTH_INTERFACE + USB_AUDIO_CONTROL_INTERFACE_HEADER_LENGTH							\
		+ USB_AUDIO_INPUT_TERMINAL_ONLY_DESC_SIZE + USB_AUDIO_OUTPUT_TERMINAL_ONLY_DESC_SIZE				\
		+ USB_AUDIO_INPUT_TERMINAL_ONLY_DESC_SIZE + USB_AUDIO_OUTPUT_TERMINAL_ONLY_DESC_SIZE

#define SIZE_OF_INTERFACE2																					\
		USB_DESCRIPTOR_LENGTH_INTERFACE + USB_DESCRIPTOR_LENGTH_INTERFACE									\
		+ USB_AUDIO_STREAMING_IFACE_DESC_SIZE + USB_AUDIO_STREAMING_TYPE_I_DESC_SIZE						\
		+ USB_ENDPOINT_AUDIO_DESCRIPTOR_LENGTH + USB_AUDIO_STREAMING_ENDP_DESC_SIZE

#define SIZE_OF_INTERFACE3																					\
		USB_DESCRIPTOR_LENGTH_INTERFACE + USB_MIDI_CONTROL_INTERFACE_HEADER_LENGTH

/* bInterval */
#define OFFSET_EP1_OUT_BINTERVAL																			\
		USB_DESCRIPTOR_LENGTH_CONFIGURE																		\
		+ USB_DESCRIPTOR_LENGTH_INTERFACE_ASSOSIATION														\
		+ SIZE_OF_INTERFACE0																				\
		+ USB_DESCRIPTOR_LENGTH_INTERFACE + USB_DESCRIPTOR_LENGTH_INTERFACE									\
		+ USB_AUDIO_STREAMING_IFACE_DESC_SIZE + USB_AUDIO_STREAMING_TYPE_I_DESC_SIZE						\
		+ 6

#define OFFSET_EP1_IN_BINTERVAL																				\
		USB_DESCRIPTOR_LENGTH_CONFIGURE																		\
		+ USB_DESCRIPTOR_LENGTH_INTERFACE_ASSOSIATION														\
		+ SIZE_OF_INTERFACE0																				\
		+ USB_DESCRIPTOR_LENGTH_INTERFACE + USB_DESCRIPTOR_LENGTH_INTERFACE									\
		+ USB_AUDIO_STREAMING_IFACE_DESC_SIZE + USB_AUDIO_STREAMING_TYPE_I_DESC_SIZE						\
		+ USB_ENDPOINT_AUDIO_DESCRIPTOR_LENGTH + USB_AUDIO_STREAMING_ENDP_DESC_SIZE							\
		+ 6

#define OFFSET_EP3_IN_BINTERVAL																				\
		USB_DESCRIPTOR_LENGTH_CONFIGURE																		\
		+ USB_DESCRIPTOR_LENGTH_INTERFACE_ASSOSIATION														\
		+ SIZE_OF_INTERFACE0																				\
		+ SIZE_OF_INTERFACE1																				\
		+ USB_DESCRIPTOR_LENGTH_INTERFACE + USB_DESCRIPTOR_LENGTH_INTERFACE									\
		+ USB_AUDIO_STREAMING_IFACE_DESC_SIZE + USB_AUDIO_STREAMING_TYPE_I_DESC_SIZE						\
		+ 6

/* Max Packet SIZE */
#define OFFSET_EP2_OUT_MAX_PACKET_SIZE																			\
		USB_DESCRIPTOR_LENGTH_CONFIGURE																		\
		+ USB_DESCRIPTOR_LENGTH_INTERFACE_ASSOSIATION														\
		+ SIZE_OF_INTERFACE0																				\
		+ SIZE_OF_INTERFACE1																				\
		+ SIZE_OF_INTERFACE2																				\
		+ USB_DESCRIPTOR_LENGTH_INTERFACE_ASSOSIATION														\
		+ SIZE_OF_INTERFACE3																				\
		+ USB_DESCRIPTOR_LENGTH_INTERFACE																	\
		+ SIZE_OF_CS_MS_IF_HDR_DESC																			\
		+ SIZE_OF_MIDI_IN_JACK_DESC + SIZE_OF_MIDI_OUT_JACK_DESC											\
		+ SIZE_OF_MIDI_OUT_JACK_DESC + SIZE_OF_MIDI_IN_JACK_DESC											\
		+ 4

#define OFFSET_EP2_IN_MAX_PACKET_SIZE																				\
		USB_DESCRIPTOR_LENGTH_CONFIGURE																		\
		+ USB_DESCRIPTOR_LENGTH_INTERFACE_ASSOSIATION														\
		+ SIZE_OF_INTERFACE0																				\
		+ SIZE_OF_INTERFACE1																				\
		+ SIZE_OF_INTERFACE2																				\
		+ USB_DESCRIPTOR_LENGTH_INTERFACE_ASSOSIATION														\
		+ SIZE_OF_INTERFACE3																				\
		+ USB_DESCRIPTOR_LENGTH_INTERFACE																	\
		+ SIZE_OF_CS_MS_IF_HDR_DESC																			\
		+ SIZE_OF_MIDI_IN_JACK_DESC + SIZE_OF_MIDI_OUT_JACK_DESC											\
		+ SIZE_OF_MIDI_OUT_JACK_DESC + SIZE_OF_MIDI_IN_JACK_DESC											\
		+ SIZE_OF_STD_MS_BD_EP_DESC + SIZE_OF_CS_MS_BD_EP_DESC												\
		+ 4


/*******************************************************************************
* API
******************************************************************************/
/*!
 * @brief USB device set speed function.
 *
 * This function sets the speed of the USB device.
 *
 * @param handle The USB device handle.
 * @param speed Speed type. USB_SPEED_HIGH/USB_SPEED_FULL/USB_SPEED_LOW.
 *
 * @return A USB error code or kStatus_USB_Success.
 */
extern usb_status_t USB_DeviceSetSpeed(usb_device_handle handle, uint8_t speed);
/*!
 * @brief USB device get device descriptor function.
 *
 * This function gets the device descriptor of the USB device.
 *
 * @param handle The USB device handle.
 * @param deviceDescriptor The pointer to the device descriptor structure.
 *
 * @return A USB error code or kStatus_USB_Success.
 */
usb_status_t USB_DeviceGetDeviceDescriptor(usb_device_handle handle,
                                           usb_device_get_device_descriptor_struct_t *deviceDescriptor);
/*!
 * @brief USB device get configuration descriptor function.
 *
 * This function gets the configuration descriptor of the USB device.
 *
 * @param handle The USB device handle.
 * @param configurationDescriptor The pointer to the configuration descriptor structure.
 *
 * @return A USB error code or kStatus_USB_Success.
 */
usb_status_t USB_DeviceGetConfigurationDescriptor(
    usb_device_handle handle, usb_device_get_configuration_descriptor_struct_t *configurationDescriptor);

usb_status_t USB_DeviceGetStringDescriptor(usb_device_handle handle,
                                           usb_device_get_string_descriptor_struct_t *stringDescriptor);

/*--------- USB Device Qualifier Descriptor -----------*/
usb_status_t USB_DeviceGetDeviceQualifierDescriptor(
    usb_device_handle handle, usb_device_get_device_qualifier_descriptor_struct_t *deviceQualifierDescriptor);

/*--------- USB Other Speed Configuration  Descriptor -----------*/
usb_status_t USB_DeviceGetOtherSpeedConfigurationDescriptor(
    usb_device_handle handle, usb_device_get_device_other_speed_configuration_descriptor_struct_t *deviceOtherSpeedConfigDescriptor);

#endif /* __USB_DESCRIPTOR_H__ */
