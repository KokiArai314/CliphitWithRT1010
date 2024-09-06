/*
 * Copyright (c) 2015 - 2016, Freescale Semiconductor, Inc.
 * Copyright 2016 - 2017, 2019 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "../audio_task/audio_player.h"
#include "usb_device/usb_device_config.h"
#include "usb.h"
#include "usb_device.h"

#include "usb_device_class.h"
#include "usb_device_audio.h"
#include "usb_device_descriptor.h"

#include "board.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
/************************************
 * AUDIO
 ************************************/
/* Audio generator stream endpoint information */
usb_device_endpoint_struct_t g_UsbDeviceAudioSpeakerEndpoints[USB_AUDIO_STREAM_ENDPOINT_COUNT] = {
    /* Audio generator ISO OUT pipe */
    {
        USB_AUDIO_SPEAKER_STREAM_ENDPOINT | (USB_OUT << USB_DESCRIPTOR_ENDPOINT_ADDRESS_DIRECTION_SHIFT),
        USB_ENDPOINT_ISOCHRONOUS,
        FS_ISO_OUT_ENDP_PACKET_SIZE +
            AUDIO_FORMAT_CHANNELS
                *AUDIO_FORMAT_SIZE, /* The max packet size should be increased otherwise if host send data larger
than max packet size will cause DMA error. */
    },
    {
        USB_AUDIO_SPEAKER_FEEDBACK_ENDPOINT | (USB_IN << USB_DESCRIPTOR_ENDPOINT_ADDRESS_DIRECTION_SHIFT),
        USB_ENDPOINT_ISOCHRONOUS, FS_ISO_FEEDBACK_ENDP_PACKET_SIZE,
    }
};

/// @note add AudioRec
/* Audio generator stream endpoint information */
usb_device_endpoint_struct_t g_UsbDeviceAudioRecorderEndpoints[USB_AUDIO_IN_STREAM_ENDPOINT_COUNT] = {
    /* Audio generator ISO IN pipe */
    {
    		USB_AUDIO_RECORDER_STREAM_ENDPOINT | (USB_IN << USB_DESCRIPTOR_ENDPOINT_ADDRESS_DIRECTION_SHIFT),
        USB_ENDPOINT_ISOCHRONOUS, FS_ISO_IN_ENDP_PACKET_SIZE + AUDIO_IN_FORMAT_CHANNELS * AUDIO_FORMAT_SIZE,
    }
};

/* Audio speaker stream interface information */
usb_device_interface_struct_t g_UsbDeviceAudioStreamInterface[] = {
    {
        0U,
        {
            0U, NULL,
        },
        NULL,
    },
    {
        1U,
        {
            USB_AUDIO_STREAM_ENDPOINT_COUNT, g_UsbDeviceAudioSpeakerEndpoints,
        },
        NULL,
    },
};

/// @note add AudioRec
/* Audio generator stream interface information */
usb_device_interface_struct_t g_UsbDeviceAudioInStreamInterface[] = {
    {
        0U,
        {
            0U, NULL,
        },
        NULL,
    },
    {
        1U,
        {
        	USB_AUDIO_IN_STREAM_ENDPOINT_COUNT, g_UsbDeviceAudioRecorderEndpoints,
        },
        NULL,
    },
};

/// @note AudioRec + Play
/* Define interfaces for audio speaker & recorder */
usb_device_interfaces_struct_t g_UsbDeviceAudioInterfaces[USB_AUDIO_PLAYER_INTERFACE_COUNT - USB_MIDI_PLAYER_INTERFACE_COUNT] =
{
    {
        USB_AUDIO_CLASS,                   /* Audio class code */
        USB_SUBCLASS_AUDIOCONTROL,         /* Audio control subclass code */
        USB_AUDIO_PROTOCOL,                /* Audio protocol code */
        USB_AUDIO_CONTROL_INTERFACE_INDEX, /* The interface number of the Audio control */
		NULL,
		0,
    },
    {
        USB_AUDIO_CLASS,                  /* Audio class code */
        USB_SUBCLASS_AUDIOSTREAM,         /* Audio stream subclass code */
        USB_AUDIO_PROTOCOL,               /* Audio protocol code */
        USB_AUDIO_STREAM_INTERFACE_INDEX, /* The interface number of the Audio control */
        g_UsbDeviceAudioStreamInterface,  /* The handle of Audio stream */
        sizeof(g_UsbDeviceAudioStreamInterface) / sizeof(usb_device_interfaces_struct_t),
    },
    {
        USB_AUDIO_CLASS,                  /* Audio class code */
        USB_SUBCLASS_AUDIOSTREAM,         /* Audio stream subclass code */
        USB_AUDIO_PROTOCOL,               /* Audio protocol code */
		USB_AUDIO_IN_STREAM_INTERFACE_INDEX, /* The interface number of the Audio control */
        g_UsbDeviceAudioInStreamInterface,  /* The handle of Audio stream */
        sizeof(g_UsbDeviceAudioInStreamInterface) / sizeof(usb_device_interfaces_struct_t),
    }
};

/* Define configurations for audio player */
usb_device_interface_list_t g_UsbDeviceAudioInterfaceList[USB_DEVICE_CONFIGURATION_COUNT] = {
    {
        USB_AUDIO_PLAYER_INTERFACE_COUNT, g_UsbDeviceAudioInterfaces,
    },
};

/* Define class information for audio player */
usb_device_class_struct_t g_UsbDeviceAudioClass = {
    g_UsbDeviceAudioInterfaceList, kUSB_DeviceClassTypeAudio, USB_DEVICE_CONFIGURATION_COUNT,
};


/************************************
 * MIDI
 ************************************/
/* MIDI stream endpoint information */
usb_device_endpoint_struct_t g_UsbDeviceMidiEndpoints[USB_MIDI_STREAM_ENDPOINT_COUNT] = {
    /* MIDI Out pipe */
    {
   		USB_MIDI_OUT_STREAM_ENDPOINT | (USB_OUT << USB_DESCRIPTOR_ENDPOINT_ADDRESS_DIRECTION_SHIFT),
        USB_ENDPOINT_BULK, USB_MIDI_ENDP_MAX_PACKET_SIZE,
    },
    {
   		USB_MIDI_IN_STREAM_ENDPOINT | (USB_IN << USB_DESCRIPTOR_ENDPOINT_ADDRESS_DIRECTION_SHIFT),
        USB_ENDPOINT_BULK, USB_MIDI_ENDP_MAX_PACKET_SIZE,
    }
};

/* Define interface for MIDI class */
usb_device_interface_struct_t g_UsbDeviceMidiStreamInterface[] = {
	{
		0,
		{
			USB_MIDI_STREAM_ENDPOINT_COUNT, g_UsbDeviceMidiEndpoints,
		},
		NULL
	}
};

/* Define interfaces for MSC disk */
usb_device_interfaces_struct_t g_MidiInterfaces[USB_MIDI_PLAYER_INTERFACE_COUNT] = {
    {
    	USB_AUDIO_CLASS,                   /* Audio class code */
        USB_SUBCLASS_AUDIOCONTROL,         /* Audio control subclass code */
        USB_AUDIO_PROTOCOL,                /* Audio protocol code */
		USB_MIDI_CONTROL_INTERFACE_INDEX, /* The interface number of the Audio control */
		NULL,
		0,
    },
	{
		USB_AUDIO_CLASS,
		USB_SUBCLASS_MIDISTREAM,
		USB_AUDIO_PROTOCOL,
		USB_MIDI_STREAM_INTERFACE_INDEX,
		g_UsbDeviceMidiStreamInterface,
		sizeof(g_UsbDeviceMidiStreamInterface) / sizeof(usb_device_interfaces_struct_t),
	}
};

/* Define configurations for midi player */
usb_device_interface_list_t g_UsbDeviceMidinterfaceList[USB_DEVICE_CONFIGURATION_COUNT] = {
    {
    	USB_MIDI_PLAYER_INTERFACE_COUNT, g_MidiInterfaces,
    },
};

/* Define class information for midi player */
usb_device_class_struct_t g_UsbDeviceMidiClass = {
		g_UsbDeviceMidinterfaceList, kUSB_DeviceClassTypeMidi, USB_DEVICE_CONFIGURATION_COUNT,
};

/* Define device descriptor */
USB_DMA_INIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE)
#if DESCRIPTOR_X19730
 #if SUPPORT_USB_HIGH_SPEED
uint8_t g_UsbDeviceDescriptor[] = {
    USB_DESCRIPTOR_LENGTH_DEVICE, /* Size of this descriptor in bytes */
    USB_DESCRIPTOR_TYPE_DEVICE,   /* DEVICE Descriptor Type */
    USB_SHORT_GET_LOW(USB_DEVICE_SPECIFIC_BCD_VERSION),
    USB_SHORT_GET_HIGH(USB_DEVICE_SPECIFIC_BCD_VERSION), /* USB Specification Release Number in
                                                            Binary-Coded Decimal (i.e., 2.10 is 210H). */
	USB_DEVICE_CLASS_MISCELLANEOUS_DC,					 /* Class code (assigned by the USB-IF). */
	USB_DEVICE_SUBCLASS_COMMON,							 /* Subclass code (assigned by the USB-IF). */
	USB_DEVICE_PROTOCOL_IAD,							 /* Protocol code (assigned by the USB-IF). */
    USB_CONTROL_MAX_PACKET_SIZE,                         /* Maximum packet size for endpoint zero
                                                            (only 8, 16, 32, or 64 are valid) */
    0x44U, 0x09U,                                        /* Vendor ID (assigned by the USB-IF) */

    0x16U, 0x02U,                                        /* Product ID (assigned by the manufacturer) */
    USB_SHORT_GET_LOW(USB_DEVICE_DEMO_BCD_VERSION),
    USB_SHORT_GET_HIGH(USB_DEVICE_DEMO_BCD_VERSION), /* Device release number in binary-coded decimal */
    0x01U,                                           /* Index of string descriptor describing manufacturer */
    0x02U,                                           /* Index of string descriptor describing product */
    0x00U,                                           /* Index of string descriptor describing the
                                                        device's serial number */
    USB_DEVICE_CONFIGURATION_COUNT,                  /* Number of possible configurations */
};
 #else	// #if SUPPORT_USB_HIGH_SPEED
uint8_t g_UsbDeviceDescriptor[] = {
    USB_DESCRIPTOR_LENGTH_DEVICE, /* Size of this descriptor in bytes */
    USB_DESCRIPTOR_TYPE_DEVICE,   /* DEVICE Descriptor Type */
    USB_SHORT_GET_LOW(USB_DEVICE_SPECIFIC_BCD_VERSION),
    USB_SHORT_GET_HIGH(USB_DEVICE_SPECIFIC_BCD_VERSION), /* USB Specification Release Number in
                                                            Binary-Coded Decimal (i.e., 2.10 is 210H). */
    USB_DEVICE_CLASS,                                    /* Class code (assigned by the USB-IF). */
    USB_DEVICE_SUBCLASS,                                 /* Subclass code (assigned by the USB-IF). */
    USB_DEVICE_PROTOCOL,                                 /* Protocol code (assigned by the USB-IF). */
    USB_CONTROL_MAX_PACKET_SIZE,                         /* Maximum packet size for endpoint zero
                                                            (only 8, 16, 32, or 64 are valid) */
    0x44U, 0x09U,                                        /* Vendor ID (assigned by the USB-IF) */

    0x16U, 0x02U,                                        /* Product ID (assigned by the manufacturer) */
    USB_SHORT_GET_LOW(USB_DEVICE_DEMO_BCD_VERSION),
    USB_SHORT_GET_HIGH(USB_DEVICE_DEMO_BCD_VERSION), /* Device release number in binary-coded decimal */
    0x01U,                                           /* Index of string descriptor describing manufacturer */
    0x02U,                                           /* Index of string descriptor describing product */
    0x00U,                                           /* Index of string descriptor describing the
                                                        device's serial number */
    USB_DEVICE_CONFIGURATION_COUNT,                  /* Number of possible configurations */
};
 #endif
#endif	// DESCRIPTOR_X19730
#if DESCRIPTOR_X19850
uint8_t g_UsbDeviceDescriptor[] = {
    USB_DESCRIPTOR_LENGTH_DEVICE, /* Size of this descriptor in bytes */
    USB_DESCRIPTOR_TYPE_DEVICE,   /* DEVICE Descriptor Type */
    USB_SHORT_GET_LOW(USB_DEVICE_SPECIFIC_BCD_VERSION),
    USB_SHORT_GET_HIGH(USB_DEVICE_SPECIFIC_BCD_VERSION), /* USB Specification Release Number in
                                                            Binary-Coded Decimal (i.e., 2.10 is 210H). */
	USB_DEVICE_CLASS_MISCELLANEOUS_DC,					 /* Class code (assigned by the USB-IF). */
	USB_DEVICE_SUBCLASS_COMMON,							 /* Subclass code (assigned by the USB-IF). */
	USB_DEVICE_PROTOCOL_IAD,							 /* Protocol code (assigned by the USB-IF). */
    USB_CONTROL_MAX_PACKET_SIZE,                         /* Maximum packet size for endpoint zero
                                                            (only 8, 16, 32, or 64 are valid) */
    0x44U, 0x09U,                                        /* Vendor ID (assigned by the USB-IF) */

    0x16U, 0x02U,                                        /* Product ID (assigned by the manufacturer) */
    USB_SHORT_GET_LOW(USB_DEVICE_DEMO_BCD_VERSION),
    USB_SHORT_GET_HIGH(USB_DEVICE_DEMO_BCD_VERSION), /* Device release number in binary-coded decimal */
    0x01U,                                           /* Index of string descriptor describing manufacturer */
    0x02U,                                           /* Index of string descriptor describing product */
    0x00U,                                           /* Index of string descriptor describing the
                                                        device's serial number */
    USB_DEVICE_CONFIGURATION_COUNT,                  /* Number of possible configurations */
};
#endif

USB_DMA_INIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE)
#if DESCRIPTOR_X19730
 #if SUPPORT_USB_HIGH_SPEED
uint8_t g_UsbDeviceConfigurationDescriptor[] = {
	/* Configuration Descriptor Size - always 9 bytes*/
	USB_DESCRIPTOR_LENGTH_CONFIGURE, /* Size of this descriptor in bytes */
	USB_DESCRIPTOR_TYPE_CONFIGURE,   /* CONFIGURATION Descriptor Type */
	USB_SHORT_GET_LOW(
		SIZE_OF_TOTAL_CONFIG_DESC
	),
	USB_SHORT_GET_HIGH(
		SIZE_OF_TOTAL_CONFIG_DESC
	),	/* Total length of data returned for this configuration. */
	USB_AUDIO_PLAYER_INTERFACE_COUNT,         /* Number of interfaces supported by this configuration */
	USB_AUDIO_MIDI_CONFIGURE_INDEX,           /* Value to use as an argument to the
                                                    SetConfiguration() request to select this configuration */
    0x00U,                                     /* Index of string descriptor describing this configuration */
    (USB_DESCRIPTOR_CONFIGURE_ATTRIBUTE_D7_MASK) |
        (USB_DEVICE_CONFIG_SELF_POWER << USB_DESCRIPTOR_CONFIGURE_ATTRIBUTE_SELF_POWERED_SHIFT) |
        (USB_DEVICE_CONFIG_REMOTE_WAKEUP << USB_DESCRIPTOR_CONFIGURE_ATTRIBUTE_REMOTE_WAKEUP_SHIFT),
    /* Configuration characteristics
       D7: Reserved (set to one)
       D6: Self-powered
       D5: Remote Wakeup
       D4...0: Reserved (reset to zero)
    */
    USB_DEVICE_MAX_POWER, /* Maximum power consumption of the USB
                           * device from the bus in this specific
                           * configuration when the device is fully
                           * operational. Expressed in 2 mA units
                           *  (i.e., 50 = 100 mA).
                           */

	USB_DESCRIPTOR_LENGTH_INTERFACE_ASSOSIATION,
	USB_DESCRIPTOR_TYPE_INTERFACE_ASSOCIATION,
	USB_AUDIO_CONTROL_INTERFACE_INDEX,		/* bFirstInterface */
	0x03,									/* bInterfaceCount */	// Audio Control, Audio Streaming, Audio Streaming
	0x01,									/* bFunctionClass */
	0x00,									/* bFuctionSubClass */
	0x00,									/* bFunctionProtocol */
	0x00,									/* iFunction */

    USB_DESCRIPTOR_LENGTH_INTERFACE,   /* Size of this descriptor in bytes */
    USB_DESCRIPTOR_TYPE_INTERFACE,     /* INTERFACE Descriptor Type */
    USB_AUDIO_CONTROL_INTERFACE_INDEX, /* Number of this interface. */
    0x00U,                             /* Value used to select this alternate setting
                                          for the interface identified in the prior field */
	0x00U,
    USB_AUDIO_CLASS,                   /*The interface implements the Audio Interface class  */
    USB_SUBCLASS_AUDIOCONTROL,         /*The interface implements the AUDIOCONTROL Subclass  */
    0x00U,                             /*The interface doesn't use any class-specific protocols  */
    0x00U,                             /* The device doesn't have a string descriptor describing this iInterface  */

    /* Audio Class Specific type of INTERFACE Descriptor */
    USB_AUDIO_CONTROL_INTERFACE_HEADER_LENGTH,   /* Size of the descriptor, in bytes  */
    USB_DESCRIPTOR_TYPE_AUDIO_CS_INTERFACE,      /* CS_INTERFACE Descriptor Type   */
    USB_DESCRIPTOR_SUBTYPE_AUDIO_CONTROL_HEADER, /* HEADER descriptor subtype   */
    0x00U, 0x01U, /* Audio Device compliant to the USB Audio specification version 1.00  */
	0x34U, 0x00U,	// 10 + 12 + 9 + 12 + 9
	0x02U,									/* bInCollection */
	USB_AUDIO_STREAM_INTERFACE_INDEX,		/* baInterfaceNr(1) */
	USB_AUDIO_IN_STREAM_INTERFACE_INDEX,	/* baInterfaceNr(2) */

    /* Audio Class Specific type of Input Terminal*/
    USB_AUDIO_INPUT_TERMINAL_ONLY_DESC_SIZE, /* Size of the descriptor, in bytes  */
    USB_DESCRIPTOR_TYPE_AUDIO_CS_INTERFACE,  /* CS_INTERFACE Descriptor Type   */
    USB_DESCRIPTOR_SUBTYPE_AUDIO_CONTROL_INPUT_TERMINAL,
    /* INPUT_TERMINAL descriptor subtype  */
    USB_AUDIO_SPEAKER_CONTROL_INPUT_TERMINAL_ID, /* Constant uniquely identifying the Terminal within the audio
              function. This value is used in all requests
              to address this Terminal.  */
    0x01U,
    0x01,         /* A generic microphone that does not fit under any of the other classifications.  */
    0x00U,        /* This Input Terminal has no association  */
    0x02U,        /* This Terminal's output audio channel cluster has 1 logical output channels  */
    0x03U, 0x00U, /* Spatial locations present in the cluster */
    0x00U,        /* Index of a string descriptor, describing the name of the first logical channel.   */
    0x00U,        /* Index of a string descriptor, describing the Input Terminal.   */

    /* Audio Class Specific type of  Output Terminal */
    USB_AUDIO_OUTPUT_TERMINAL_ONLY_DESC_SIZE, /* Size of the descriptor, in bytes  */
    USB_DESCRIPTOR_TYPE_AUDIO_CS_INTERFACE,   /* CS_INTERFACE Descriptor Type   */
    USB_DESCRIPTOR_SUBTYPE_AUDIO_CONTROL_OUTPUT_TERMINAL,
    /* OUTPUT_TERMINAL descriptor subtype  */
    USB_AUDIO_SPEAKER_CONTROL_OUTPUT_TERMINAL_ID, /* Constant uniquely identifying the Terminal within the audio
                                                     function*/
    0x03U,
    0x06U, /* A Terminal dealing with a signal carried over an endpoint in an AudioStreaming interface */
    0x00U, /*  This Output Terminal has no association   */
	USB_AUDIO_SPEAKER_CONTROL_INPUT_TERMINAL_ID, /* ID of the Unit or Terminal to which this Terminal is connected.   */
    0x00U,                                     /* Index of a string descriptor, describing the Output Terminal.  */

	/* Audio Class Specific type of Input Terminal [ID3] */
	USB_AUDIO_INPUT_TERMINAL_ONLY_DESC_SIZE, /* Size of the descriptor, in bytes  */
	USB_DESCRIPTOR_TYPE_AUDIO_CS_INTERFACE,  /* CS_INTERFACE Descriptor Type   */
	USB_DESCRIPTOR_SUBTYPE_AUDIO_CONTROL_INPUT_TERMINAL,
	/* INPUT_TERMINAL descriptor subtype  */
	USB_AUDIO_RECORDER_CONTROL_INPUT_TERMINAL_ID, /* Constant uniquely identifying the Terminal within the audio
             function. This value is used in all requests
             to address this Terminal.  */
	0x03U,
	0x06U,        /* A generic microphone that does not fit under any of the other classifications.  */
	0x00U,        /* This Input Terminal has no association  */
	0x02U,        /* This Terminal's output audio channel cluster has 1 logical output channels  */
	0x03U, 0x00U, /* Spatial locations present in the cluster */
	0x00U,        /* Index of a string descriptor, describing the name of the first logical channel.   */
	0x00U,        /* Index of a string descriptor, describing the Input Terminal.   */

	/* Audio Class Specific type of  Output Terminal [ID4] */
	USB_AUDIO_OUTPUT_TERMINAL_ONLY_DESC_SIZE, /* Size of the descriptor, in bytes  */
	USB_DESCRIPTOR_TYPE_AUDIO_CS_INTERFACE,   /* CS_INTERFACE Descriptor Type   */
	USB_DESCRIPTOR_SUBTYPE_AUDIO_CONTROL_OUTPUT_TERMINAL,
	/* OUTPUT_TERMINAL descriptor subtype  */
	USB_AUDIO_RECORDER_CONTROL_OUTPUT_TERMINAL_ID, /* Constant uniquely identifying the Terminal within the audio
                                                    function*/
	0x01U,
	0x01U, /* A Terminal dealing with a signal carried over an endpoint in an AudioStreaming interface */
	0x00U, /*  This Output Terminal has no association   */
	USB_AUDIO_RECORDER_CONTROL_INPUT_TERMINAL_ID, /* ID of the Unit or Terminal to which this Terminal is connected.   */
	0x00U,                                     /* Index of a string descriptor, describing the Output Terminal.  */

    /* Audio Class Specific INTERFACE Descriptor, alternative interface 0  */
    USB_DESCRIPTOR_LENGTH_INTERFACE,  /* Descriptor size is 9 bytes  */
    USB_DESCRIPTOR_TYPE_INTERFACE,    /* INTERFACE Descriptor Type   */
    USB_AUDIO_STREAM_INTERFACE_INDEX, /* The number of this interface is 1.  */
    0x00U,                            /* The value used to select the alternate setting for this interface is 0   */
    0x00U,                    /* The number of endpoints used by this interface is 0 (excluding endpoint zero)   */
    USB_AUDIO_CLASS,          /* The interface implements the Audio Interface class   */
    USB_SUBCLASS_AUDIOSTREAM, /* The interface implements the AUDIOSTREAMING Subclass   */
    0x00U,                    /* The interface doesn't use any class-specific protocols   */
	USBD_IDX_AUDIO_STR,   /* The device doesn't have a string descriptor describing this iInterface  */

    /* Audio Class Specific INTERFACE Descriptor, alternative interface 1 */
    USB_DESCRIPTOR_LENGTH_INTERFACE,  /* Descriptor size is 9 bytes  */
    USB_DESCRIPTOR_TYPE_INTERFACE,    /* INTERFACE Descriptor Type  */
    USB_AUDIO_STREAM_INTERFACE_INDEX, /*The number of this interface is 1.  */
    0x01U,                            /* The value used to select the alternate setting for this interface is 1  */
    0x02U,                    /* The number of endpoints used by this interface is 2 (excluding endpoint zero)    */
    USB_AUDIO_CLASS,          /* The interface implements the Audio Interface class   */
    USB_SUBCLASS_AUDIOSTREAM, /* The interface implements the AUDIOSTREAMING Subclass   */
    0x00U,                    /* The interface doesn't use any class-specific protocols  */
	USBD_IDX_AUDIO_STR,   /* The device doesn't have a string descriptor describing this iInterface  */

    /* Audio Class Specific CS INTERFACE Descriptor*/
    USB_AUDIO_STREAMING_IFACE_DESC_SIZE,            /* Size of the descriptor, in bytes  */
    USB_DESCRIPTOR_TYPE_AUDIO_CS_INTERFACE,         /* CS_INTERFACE Descriptor Type  */
    USB_DESCRIPTOR_SUBTYPE_AUDIO_STREAMING_GENERAL, /* AS_GENERAL descriptor subtype  */
    USB_AUDIO_SPEAKER_CONTROL_INPUT_TERMINAL_ID,    /* The Terminal ID of the Terminal to which the endpoint of this
                                                       interface is connected. */
	0x00U,
    0x01U,
    0x00U, /* PCM  */

    /* Audio Class Specific type I format INTERFACE Descriptor */
    USB_AUDIO_STREAMING_TYPE_I_DESC_SIZE,               /* bLength (11) */
    USB_DESCRIPTOR_TYPE_AUDIO_CS_INTERFACE,             /* bDescriptorType (CS_INTERFACE) */
    USB_DESCRIPTOR_SUBTYPE_AUDIO_STREAMING_FORMAT_TYPE, /* DescriptorSubtype: AUDIO STREAMING FORMAT TYPE */
    USB_AUDIO_FORMAT_TYPE_I,                            /* Format Type: Type I */
    AUDIO_FORMAT_CHANNELS,                              /* Number of Channels: one channel */
    AUDIO_FORMAT_SIZE,                                  /* SubFrame Size: one byte per audio subframe */
    AUDIO_FORMAT_BITS,                                  /* Bit Resolution: 8 bits per sample */
    0x01U,                                              /* One frequency supported */
	TSAMFREQ2BYTES(44100),

    /* ENDPOINT Descriptor */
    USB_ENDPOINT_AUDIO_DESCRIPTOR_LENGTH, /* Descriptor size is 9 bytes  */
    USB_DESCRIPTOR_TYPE_ENDPOINT,         /* Descriptor type (endpoint descriptor) */
    USB_AUDIO_SPEAKER_STREAM_ENDPOINT |
        (USB_OUT << USB_DESCRIPTOR_ENDPOINT_ADDRESS_DIRECTION_SHIFT), /* OUT endpoint address 1 */
    USB_ENDPOINT_ISOCHRONOUS | 0x04,                                  /* Isochronous endpoint */
    USB_SHORT_GET_LOW(FS_ISO_OUT_ENDP_PACKET_SIZE + AUDIO_FORMAT_CHANNELS * AUDIO_FORMAT_SIZE),
    USB_SHORT_GET_HIGH(FS_ISO_OUT_ENDP_PACKET_SIZE + AUDIO_FORMAT_CHANNELS * AUDIO_FORMAT_SIZE), /* 16 bytes  */
    FS_ISO_OUT_ENDP_INTERVAL, /* bInterval(0x01U): x ms */
    0x00U,                    /* Unused */
    USB_AUDIO_SPEAKER_FEEDBACK_ENDPOINT |
        (USB_IN
         << USB_DESCRIPTOR_ENDPOINT_ADDRESS_DIRECTION_SHIFT), /* Synchronization Endpoint (if used) is endpoint 0x81  */

    /* Audio Class Specific ENDPOINT Descriptor  */
    USB_AUDIO_STREAMING_ENDP_DESC_SIZE,      /*  Size of the descriptor, in bytes  */
    USB_AUDIO_STREAM_ENDPOINT_DESCRIPTOR,    /* CS_ENDPOINT Descriptor Type  */
    USB_AUDIO_EP_GENERAL_DESCRIPTOR_SUBTYPE, /* AUDIO_EP_GENERAL descriptor subtype  */
    0x01U,                                   /* Bit 0: Sampling Frequency 0
                                                Bit 1: Pitch 0
                                                Bit 7: MaxPacketsOnly 0   */
    0x00U,                                   /* Indicates the units used for the wLockDelay field: 0: Undefined  */
    0x00U,
    0x00U, /* Indicates the time it takes this endpoint to reliably lock its internal clock recovery circuitry */

    /* Endpoint 1 Feedback ENDPOINT */
    USB_ENDPOINT_AUDIO_DESCRIPTOR_LENGTH, /* bLength */
    USB_DESCRIPTOR_TYPE_ENDPOINT,         /* bDescriptorType */
    USB_AUDIO_SPEAKER_FEEDBACK_ENDPOINT |
        (USB_IN << USB_DESCRIPTOR_ENDPOINT_ADDRESS_DIRECTION_SHIFT), /* This is an IN endpoint with endpoint number 3 */
    USB_ENDPOINT_ISOCHRONOUS,                          				 /*  Types -
                                                                         Transfer: ISOCHRONOUS
                                                                         Sync: Async
                                                                         Usage: Feedback EP   */
    FS_ISO_FEEDBACK_ENDP_PACKET_SIZE,
    0x00, /* wMaxPacketSize */
    0x01, /* interval polling(2^x ms) */
	FEEDBACK_REFRESH, /* bRefresh(8ms)  */
    0x00, /* unused */

	/* Audio Class Specific INTERFACE Descriptor, alternative interface 0  */
	USB_DESCRIPTOR_LENGTH_INTERFACE,  /* Descriptor size is 9 bytes  */
	USB_DESCRIPTOR_TYPE_INTERFACE,    /* INTERFACE Descriptor Type   */
	USB_AUDIO_IN_STREAM_INTERFACE_INDEX, /* The number of this interface is 2.  */
	0x00U,                            /* The value used to select the alternate setting for this interface is 0   */
	0x00U,                    /* The number of endpoints used by this interface is 0 (excluding endpoint zero)   */
	USB_AUDIO_CLASS,          /* The interface implements the Audio Interface class   */
	USB_SUBCLASS_AUDIOSTREAM, /* The interface implements the AUDIOSTREAMING Subclass   */
	0x00U,                    /* The interface doesn't use any class-specific protocols   */
	USBD_IDX_AUDIO_STR,	  /* The device doesn't have a string descriptor describing this iInterface  */

	/* Audio Class Specific INTERFACE Descriptor, alternative interface 1 */
	USB_DESCRIPTOR_LENGTH_INTERFACE,  /* Descriptor size is 9 bytes  */
	USB_DESCRIPTOR_TYPE_INTERFACE,    /* INTERFACE Descriptor Type  */
	USB_AUDIO_IN_STREAM_INTERFACE_INDEX, /*The number of this interface is 2.  */
	0x01U,                            /* The value used to select the alternate setting for this interface is 1  */
	0x01U,                    /* The number of endpoints used by this interface is 1 (excluding endpoint zero)    */
	USB_AUDIO_CLASS,          /* The interface implements the Audio Interface class   */
	USB_SUBCLASS_AUDIOSTREAM, /* The interface implements the AUDIOSTREAMING Subclass   */
	0x00U,                    /* The interface doesn't use any class-specific protocols  */
	USBD_IDX_AUDIO_STR,    /* The device doesn't have a string descriptor describing this iInterface  */

	/* Audio Class Specific CS INTERFACE Descriptor*/
	USB_AUDIO_STREAMING_IFACE_DESC_SIZE,            /* Size of the descriptor, in bytes  */
	USB_DESCRIPTOR_TYPE_AUDIO_CS_INTERFACE,         /* CS_INTERFACE Descriptor Type  */
	USB_DESCRIPTOR_SUBTYPE_AUDIO_STREAMING_GENERAL, /* AS_GENERAL descriptor subtype  */
	USB_AUDIO_RECORDER_CONTROL_OUTPUT_TERMINAL_ID,    /* The Terminal ID of the Terminal to which the endpoint of this
                                                      interface is connected. */
	0x00U, /* Delay introduced by the data path. Expressed in number of frames.  */
	0x01U,
	0x00U, /* PCM  */

	/* Audio Class Specific type I format INTERFACE Descriptor */
	USB_AUDIO_STREAMING_TYPE_I_DESC_SIZE,               /* bLength (11) */
	USB_DESCRIPTOR_TYPE_AUDIO_CS_INTERFACE,             /* bDescriptorType (CS_INTERFACE) */
	USB_DESCRIPTOR_SUBTYPE_AUDIO_STREAMING_FORMAT_TYPE, /* DescriptorSubtype: AUDIO STREAMING FORMAT TYPE */
	USB_AUDIO_FORMAT_TYPE_I,                            /* Format Type: Type I */
	AUDIO_FORMAT_CHANNELS,                              /* Number of Channels: one channel */
	AUDIO_FORMAT_SIZE,                                  /* SubFrame Size: one byte per audio subframe */
	AUDIO_FORMAT_BITS,                                  /* Bit Resolution: 8 bits per sample */
	0x01U,                                              /* One frequency supported */
	TSAMFREQ2BYTES(44100),

	/* ENDPOINT Descriptor */
	USB_ENDPOINT_AUDIO_DESCRIPTOR_LENGTH, /* Descriptor size is 9 bytes  */
	USB_DESCRIPTOR_TYPE_ENDPOINT,         /* Descriptor type (endpoint descriptor) */
	USB_AUDIO_RECORDER_STREAM_ENDPOINT |
		(USB_IN << USB_DESCRIPTOR_ENDPOINT_ADDRESS_DIRECTION_SHIFT), /* OUT endpoint address 1 */
	USB_ENDPOINT_ISOCHRONOUS | 0x04,                                  /* Isochronous endpoint */
	USB_SHORT_GET_LOW(FS_ISO_IN_ENDP_PACKET_SIZE + AUDIO_IN_FORMAT_CHANNELS * AUDIO_FORMAT_SIZE),
	USB_SHORT_GET_HIGH(FS_ISO_IN_ENDP_PACKET_SIZE + AUDIO_IN_FORMAT_CHANNELS * AUDIO_FORMAT_SIZE), /* 16 bytes  */
	FS_ISO_IN_ENDP_INTERVAL, /* bInterval(0x01U): x ms */
	0x00U,                    /* Unused */
	0x00U,					/* bSynchAddress */

	/* Audio Class Specific ENDPOINT Descriptor  */
	USB_AUDIO_STREAMING_ENDP_DESC_SIZE,      /*  Size of the descriptor, in bytes  */
	USB_AUDIO_STREAM_ENDPOINT_DESCRIPTOR,    /* CS_ENDPOINT Descriptor Type  */
	USB_AUDIO_EP_GENERAL_DESCRIPTOR_SUBTYPE, /* AUDIO_EP_GENERAL descriptor subtype  */
	0x01U,                                   /* Bit 0: Sampling Frequency 0
                                               Bit 1: Pitch 0
                                               Bit 7: MaxPacketsOnly 0   */
	0x00U,                                   /* Indicates the units used for the wLockDelay field: 0: Undefined  */
	0x00U,
	0x00U, /* Indicates the time it takes this endpoint to reliably lock its internal clock recovery circuitry */

	USB_DESCRIPTOR_LENGTH_INTERFACE_ASSOSIATION,
	USB_DESCRIPTOR_TYPE_INTERFACE_ASSOCIATION,
	USB_MIDI_CONTROL_INTERFACE_INDEX,		/* bFirstInterface */
	0x02,									/* bInterfaceCount */	// Audio Control, MIDI Streaming
	0x01,									/* bFunctionClass */
	0x00,									/* bFuctionSubClass */
	0x00,									/* bFunctionProtocol */
	0x00,									/* iFunction */

	/// @note MIDI Streaming Class
	/* MIDI Class Specific INTERFACE Descriptor, alternative interface 0  */
	USB_DESCRIPTOR_LENGTH_INTERFACE,  /* Descriptor size is 9 bytes  */
	USB_DESCRIPTOR_TYPE_INTERFACE,    /* INTERFACE Descriptor Type   */
	USB_MIDI_CONTROL_INTERFACE_INDEX, /* The number of this interface is 1.  */
	0x00U,                            /* The value used to select the alternate setting for this interface is 0   */
	0x00U,                    /* The number of endpoints used by this interface is 0 (excluding endpoint zero)   */
	USB_AUDIO_CLASS,          /* The interface implements the Audio Interface class   */
	USB_SUBCLASS_AUDIOCONTROL,  /*The interface implements the AUDIOCONTROL Subclass  */
	0x00U,                    /* The interface doesn't use any class-specific protocols   */
	0x00U,                    /* The device doesn't have a string descriptor describing this iInterface  */

	/* MIDI Class Specific type of INTERFACE Descriptor */
	USB_MIDI_CONTROL_INTERFACE_HEADER_LENGTH,   /* Size of the descriptor, in bytes  */
	USB_DESCRIPTOR_TYPE_AUDIO_CS_INTERFACE,      /* CS_INTERFACE Descriptor Type   */
	USB_DESCRIPTOR_SUBTYPE_AUDIO_CONTROL_HEADER, /* HEADER descriptor subtype   */
	0x00U, 0x01U, /* Audio Device compliant to the USB MIDI specification version 1.00  */
	USB_MIDI_CONTROL_INTERFACE_HEADER_LENGTH, 0x00U,
	0x01U,									/* bInCollection */
	USB_MIDI_STREAM_INTERFACE_INDEX,		/* baInterfaceNr(1) */

	/* Audio Class Specific INTERFACE Descriptor, alternative interface 0  */
	USB_DESCRIPTOR_LENGTH_INTERFACE,  /* Descriptor size is 9 bytes  */
	USB_DESCRIPTOR_TYPE_INTERFACE,    /* INTERFACE Descriptor Type   */
	USB_MIDI_STREAM_INTERFACE_INDEX, /* The number of this interface is 1.  */
	0x00U,                            /* The value used to select the alternate setting for this interface is 0   */
	0x02U,                    /* The number of endpoints used by this interface is 0 (excluding endpoint zero)   */
	USB_AUDIO_CLASS,          /* The interface implements the Audio Interface class   */
	USB_SUBCLASS_MIDISTREAM,  /* The interface implements the AUDIOSTREAMING Subclass   */
	0x00U,                    /* The interface doesn't use any class-specific protocols   */
	0x00U,                    /* The device doesn't have a string descriptor describing this iInterface  */

	/* ---------- Class-Specific MS Interface Header Descriptor (midi 1.0 section 6.1.2) ---------- */
	SIZE_OF_CS_MS_IF_HDR_DESC,					/* bLength							*/
	USB_DESCRIPTOR_TYPE_AUDIO_CS_INTERFACE,		/* bDescriptorType (CS_INTERFACE)	*/
	TYPE_MS_HEADER,								/* bDescriptorSubtype (MS_HEADER)	*/
	0x00U, 0x01U, /* Audio Device compliant to the USB MIDI specification version 1.00  */
	SIZE_OF_MS_HEADER_CONTENT, 0x00U,

	/* ---------- MIDI IN (host to device, EMBEDDED) Jack Descriptor (midi 1.0 section 6.1.2.2) ---------- */
	SIZE_OF_MIDI_IN_JACK_DESC,				/* bLength 							*/
	USB_DESCRIPTOR_TYPE_AUDIO_CS_INTERFACE,		/* bDescriptorType */
	TYPE_MIDI_IN_JACK,						/* bDescriptorSubtype */
	TYPE_MS_EMBEDDED,						/* bJackType	*/
	0x10,									/* bJackID */
	USBD_IDX_MIDI_IN_STR,                   /* iJack	*/

	/* ---------- MIDI OUT (device to host, External) Jack Descriptor (midi 1.0 section 6.1.2.3) ---------- */
	SIZE_OF_MIDI_OUT_JACK_DESC,				/* bLength */
	USB_DESCRIPTOR_TYPE_AUDIO_CS_INTERFACE,		/* bDescriptorType */
	TYPE_MIDI_OUT_JACK,						/* bDescriptorSubType */
	TYPE_MS_EXTERNAL,						/* bJackType */
	0x40,									/* bJackID */
	0x01,									/* bNrInputPins */
	0x10,									/* baSourceID(1): ID of Entity to which the first Input Pin of this MIDI OUT Jack is connected */
	0x01,									/* baSourcePin(1): Output pin number of the Entity to which the first Input Pin of this MIDI OUT Jack is connected */
	0x00,									/* iJack */

	/* ---------- MIDI OUT (device to host, EMBEDDED) Jack Descriptor (midi 1.0 section 6.1.2.3) ---------- */
	SIZE_OF_MIDI_OUT_JACK_DESC,				/* bLength */
	USB_DESCRIPTOR_TYPE_AUDIO_CS_INTERFACE,		/* bDescriptorType */
	TYPE_MIDI_OUT_JACK,						/* bDescriptorSubType */
	TYPE_MS_EMBEDDED,						/* bJackType */
	0x30,									/* bJackID */
	0x01,									/* bNrInputPins */
	0x20,									/* baSourceID(1): ID of Entity to which the first Input Pin of this MIDI OUT Jack is connected */
	0x01,									/* baSourcePin(1): Output pin number of the Entity to which the first Input Pin of this MIDI OUT Jack is connected */
	USBD_IDX_MIDI_OUT_STR,					/* iJack */

	/* ---------- MIDI IN (host to device, External) Jack Descriptor (midi 1.0 section 6.1.2.2) ---------- */
	SIZE_OF_MIDI_IN_JACK_DESC,				/* bLength 							*/
	USB_DESCRIPTOR_TYPE_AUDIO_CS_INTERFACE,		/* bDescriptorType */
	TYPE_MIDI_IN_JACK,						/* bDescriptorSubtype */
	TYPE_MS_EXTERNAL,						/* bJackType	*/
	0x20,									/* bJackID */
	0x00,									/* iJack	*/

	/* ---------- Standard MS Bulk Data Endpoint Descriptor (midi 1.0 section 6.2.1) ---------- */
	SIZE_OF_STD_MS_BD_EP_DESC,				/* bLength */
	USB_DESCRIPTOR_TYPE_ENDPOINT,			/* Descriptor type: Endpopint descriptor */
	USB_MIDI_STREAM_RX_ENDPOINT,							/* bEndpointAddress (bulk Out endpoint 4) */
	0x02,									/* bmAttribute: Bulk Transfer */
	(USB_MIDI_ENDP_MAX_PACKET_SIZE & 0xFF),
	(USB_MIDI_ENDP_MAX_PACKET_SIZE >> 8),		/* wMaxPacketSize */
	0x00,									/* bInterval: must be 0 */
	0x00,									/* bRefresh: must be 0 */
	0x00,									/* bSyncAddress: must be 0 */

	/* ---------- Class-Specific MS Bulk Data Endpoint Descriptor (midi 1.0 section 6.2.2) ---------- */
	SIZE_OF_CS_MS_BD_EP_DESC,				/* bLength */
	USB_AUDIO_STREAM_ENDPOINT_DESCRIPTOR,			/* bDescriptorType (CS_ENDPOINT) */
	TYPE_MS_GENERAL,						/* bDescriptorSubType (MS_GENERAL) */
	0x01,									/* bNumEmbMIDIJack	*/
	0x10,									/* baAssocJackID(1): to Embedded MIDI In Jack	*/

	/* ---------- Standard MS Bulk Data Endpoint Descriptor (midi 1.0 section 6.2.1) ---------- */
	SIZE_OF_STD_MS_BD_EP_DESC,				/* Descriptor size */
	USB_DESCRIPTOR_TYPE_ENDPOINT,					/* Descriptor type: Endpopint descriptor */
	USB_MIDI_STREAM_TX_ENDPOINT |
		(USB_IN << USB_DESCRIPTOR_ENDPOINT_ADDRESS_DIRECTION_SHIFT),	/* bEndpointAddress (Bulk In endpoint 5) */
	0x02,									/* bmAttribute: Bulk Transfer */
	(USB_MIDI_ENDP_MAX_PACKET_SIZE & 0xFF),
	(USB_MIDI_ENDP_MAX_PACKET_SIZE >> 8),		/* wMaxPacketSize */
	0x00,									/* bInterval: must be 0	*/
	0x00,									/* bRefresh: must be 0	*/
	0x00,									/* bSyncAddress: must be 0 */

	/* ---------- Class-Specific MS Bulk Data Endpoint Descriptor (midi 1.0 section 6.2.2) ---------- */
	SIZE_OF_CS_MS_BD_EP_DESC,				/* bLength */
	USB_AUDIO_STREAM_ENDPOINT_DESCRIPTOR,			/* bDescriptorType (CS_ENDPOINT) */
	TYPE_MS_GENERAL,						/* bDescriptorSubType (MS_GENERAL) */
	0x01,									/* bNumEmbMIDIJack	*/
	0x30									/* baAssocJackID(1): to Embedded MIDI OUT Jack	*/
};
 #else	// #if SUPPORT_USB_HIGH_SPEED
uint8_t g_UsbDeviceConfigurationDescriptor[] = {
	/* Configuration Descriptor Size - always 9 bytes*/
	USB_DESCRIPTOR_LENGTH_CONFIGURE, /* Size of this descriptor in bytes */
	USB_DESCRIPTOR_TYPE_CONFIGURE,   /* CONFIGURATION Descriptor Type */
	USB_SHORT_GET_LOW(
		USB_DESCRIPTOR_LENGTH_CONFIGURE + USB_DESCRIPTOR_LENGTH_INTERFACE +
		USB_AUDIO_CONTROL_INTERFACE_HEADER_LENGTH +
		USB_AUDIO_INPUT_TERMINAL_ONLY_DESC_SIZE + USB_AUDIO_OUTPUT_TERMINAL_ONLY_DESC_SIZE +
		USB_AUDIO_INPUT_TERMINAL_ONLY_DESC_SIZE + USB_AUDIO_OUTPUT_TERMINAL_ONLY_DESC_SIZE +
		/* Interface 1 */
		USB_DESCRIPTOR_LENGTH_INTERFACE + USB_DESCRIPTOR_LENGTH_INTERFACE +
		USB_AUDIO_STREAMING_IFACE_DESC_SIZE + USB_AUDIO_STREAMING_TYPE_I_DESC_SIZE +
		USB_ENDPOINT_AUDIO_DESCRIPTOR_LENGTH + USB_AUDIO_STREAMING_ENDP_DESC_SIZE +
		USB_ENDPOINT_AUDIO_DESCRIPTOR_LENGTH +
		/* Interface 2 */
		USB_DESCRIPTOR_LENGTH_INTERFACE + USB_DESCRIPTOR_LENGTH_INTERFACE +
		USB_AUDIO_STREAMING_IFACE_DESC_SIZE + USB_AUDIO_STREAMING_TYPE_I_DESC_SIZE +
		USB_ENDPOINT_AUDIO_DESCRIPTOR_LENGTH + USB_AUDIO_STREAMING_ENDP_DESC_SIZE +
		/* Interface 3 */
		USB_DESCRIPTOR_LENGTH_INTERFACE + USB_MIDI_CONTROL_INTERFACE_HEADER_LENGTH +
		/* Interface 4 */
		USB_DESCRIPTOR_LENGTH_INTERFACE +
		SIZE_OF_CS_MS_IF_HDR_DESC +
		SIZE_OF_MIDI_IN_JACK_DESC + SIZE_OF_MIDI_OUT_JACK_DESC +
		SIZE_OF_MIDI_OUT_JACK_DESC + SIZE_OF_MIDI_IN_JACK_DESC +
		SIZE_OF_STD_MS_BD_EP_DESC + SIZE_OF_CS_MS_BD_EP_DESC +
		SIZE_OF_STD_MS_BD_EP_DESC + SIZE_OF_CS_MS_BD_EP_DESC
	),
	USB_SHORT_GET_HIGH(
		USB_DESCRIPTOR_LENGTH_CONFIGURE + USB_DESCRIPTOR_LENGTH_INTERFACE +
		USB_AUDIO_CONTROL_INTERFACE_HEADER_LENGTH +
		USB_AUDIO_INPUT_TERMINAL_ONLY_DESC_SIZE + USB_AUDIO_OUTPUT_TERMINAL_ONLY_DESC_SIZE +
		USB_AUDIO_INPUT_TERMINAL_ONLY_DESC_SIZE + USB_AUDIO_OUTPUT_TERMINAL_ONLY_DESC_SIZE +
		/* Interface 1 */
		USB_DESCRIPTOR_LENGTH_INTERFACE + USB_DESCRIPTOR_LENGTH_INTERFACE +
		USB_AUDIO_STREAMING_IFACE_DESC_SIZE + USB_AUDIO_STREAMING_TYPE_I_DESC_SIZE +
		USB_ENDPOINT_AUDIO_DESCRIPTOR_LENGTH + USB_AUDIO_STREAMING_ENDP_DESC_SIZE +
		USB_ENDPOINT_AUDIO_DESCRIPTOR_LENGTH +
		/* Interface 2 */
		USB_DESCRIPTOR_LENGTH_INTERFACE + USB_DESCRIPTOR_LENGTH_INTERFACE +
		USB_AUDIO_STREAMING_IFACE_DESC_SIZE + USB_AUDIO_STREAMING_TYPE_I_DESC_SIZE +
		USB_ENDPOINT_AUDIO_DESCRIPTOR_LENGTH + USB_AUDIO_STREAMING_ENDP_DESC_SIZE +
		/* Interface 3 */
		USB_DESCRIPTOR_LENGTH_INTERFACE + USB_MIDI_CONTROL_INTERFACE_HEADER_LENGTH +
		/* Interface 4 */
		USB_DESCRIPTOR_LENGTH_INTERFACE +
		SIZE_OF_CS_MS_IF_HDR_DESC +
		SIZE_OF_MIDI_IN_JACK_DESC + SIZE_OF_MIDI_OUT_JACK_DESC +
		SIZE_OF_MIDI_OUT_JACK_DESC + SIZE_OF_MIDI_IN_JACK_DESC +
		SIZE_OF_STD_MS_BD_EP_DESC + SIZE_OF_CS_MS_BD_EP_DESC +
		SIZE_OF_STD_MS_BD_EP_DESC + SIZE_OF_CS_MS_BD_EP_DESC
	),	/* Total length of data returned for this configuration. */

	USB_AUDIO_PLAYER_INTERFACE_COUNT,         /* Number of interfaces supported by this configuration */
	USB_AUDIO_MIDI_CONFIGURE_INDEX,           /* Value to use as an argument to the
                                                    SetConfiguration() request to select this configuration */
    0x00U,                                     /* Index of string descriptor describing this configuration */
    (USB_DESCRIPTOR_CONFIGURE_ATTRIBUTE_D7_MASK) |
        (USB_DEVICE_CONFIG_SELF_POWER << USB_DESCRIPTOR_CONFIGURE_ATTRIBUTE_SELF_POWERED_SHIFT) |
        (USB_DEVICE_CONFIG_REMOTE_WAKEUP << USB_DESCRIPTOR_CONFIGURE_ATTRIBUTE_REMOTE_WAKEUP_SHIFT),
    /* Configuration characteristics
       D7: Reserved (set to one)
       D6: Self-powered
       D5: Remote Wakeup
       D4...0: Reserved (reset to zero)
    */
    USB_DEVICE_MAX_POWER, /* Maximum power consumption of the USB
                           * device from the bus in this specific
                           * configuration when the device is fully
                           * operational. Expressed in 2 mA units
                           *  (i.e., 50 = 100 mA).
                           */

    USB_DESCRIPTOR_LENGTH_INTERFACE,   /* Size of this descriptor in bytes */
    USB_DESCRIPTOR_TYPE_INTERFACE,     /* INTERFACE Descriptor Type */
    USB_AUDIO_CONTROL_INTERFACE_INDEX, /* Number of this interface. */
    0x00U,                             /* Value used to select this alternate setting
                                          for the interface identified in the prior field */
	0x00U,
    USB_AUDIO_CLASS,                   /*The interface implements the Audio Interface class  */
    USB_SUBCLASS_AUDIOCONTROL,         /*The interface implements the AUDIOCONTROL Subclass  */
    0x00U,                             /*The interface doesn't use any class-specific protocols  */
    0x00U,                             /* The device doesn't have a string descriptor describing this iInterface  */

    /* Audio Class Specific type of INTERFACE Descriptor */
    USB_AUDIO_CONTROL_INTERFACE_HEADER_LENGTH,   /* Size of the descriptor, in bytes  */
    USB_DESCRIPTOR_TYPE_AUDIO_CS_INTERFACE,      /* CS_INTERFACE Descriptor Type   */
    USB_DESCRIPTOR_SUBTYPE_AUDIO_CONTROL_HEADER, /* HEADER descriptor subtype   */
    0x00U, 0x01U, /* Audio Device compliant to the USB Audio specification version 1.00  */

	/// @note add AudioRec
	0x34U, 0x00U,	// 10 + 12 + 9 + 12 + 9

	/// @note add AudioRec
	0x02U,									/* bInCollection */
	USB_AUDIO_STREAM_INTERFACE_INDEX,		/* baInterfaceNr(1) */
	USB_AUDIO_IN_STREAM_INTERFACE_INDEX,	/* baInterfaceNr(2) */

    /* Audio Class Specific type of Input Terminal*/
    USB_AUDIO_INPUT_TERMINAL_ONLY_DESC_SIZE, /* Size of the descriptor, in bytes  */
    USB_DESCRIPTOR_TYPE_AUDIO_CS_INTERFACE,  /* CS_INTERFACE Descriptor Type   */
    USB_DESCRIPTOR_SUBTYPE_AUDIO_CONTROL_INPUT_TERMINAL,
    /* INPUT_TERMINAL descriptor subtype  */
    USB_AUDIO_SPEAKER_CONTROL_INPUT_TERMINAL_ID, /* Constant uniquely identifying the Terminal within the audio
              function. This value is used in all requests
              to address this Terminal.  */
    0x01U,
    0x01,         /* A generic microphone that does not fit under any of the other classifications.  */
    0x00U,        /* This Input Terminal has no association  */
    0x02U,        /* This Terminal's output audio channel cluster has 1 logical output channels  */
    0x03U, 0x00U, /* Spatial locations present in the cluster */
    0x00U,        /* Index of a string descriptor, describing the name of the first logical channel.   */
    0x00U,        /* Index of a string descriptor, describing the Input Terminal.   */

    /* Audio Class Specific type of  Output Terminal */
    USB_AUDIO_OUTPUT_TERMINAL_ONLY_DESC_SIZE, /* Size of the descriptor, in bytes  */
    USB_DESCRIPTOR_TYPE_AUDIO_CS_INTERFACE,   /* CS_INTERFACE Descriptor Type   */
    USB_DESCRIPTOR_SUBTYPE_AUDIO_CONTROL_OUTPUT_TERMINAL,
    /* OUTPUT_TERMINAL descriptor subtype  */
    USB_AUDIO_SPEAKER_CONTROL_OUTPUT_TERMINAL_ID, /* Constant uniquely identifying the Terminal within the audio
                                                     function*/
    0x01U,
    0x03U, /* A Terminal dealing with a signal carried over an endpoint in an AudioStreaming interface */
    0x00U, /*  This Output Terminal has no association   */
	USB_AUDIO_SPEAKER_CONTROL_INPUT_TERMINAL_ID, /* ID of the Unit or Terminal to which this Terminal is connected.   */
    0x00U,                                     /* Index of a string descriptor, describing the Output Terminal.  */

	/// @note add AudioRec
	/* Audio Class Specific type of Input Terminal [ID3] */
	USB_AUDIO_INPUT_TERMINAL_ONLY_DESC_SIZE, /* Size of the descriptor, in bytes  */
	USB_DESCRIPTOR_TYPE_AUDIO_CS_INTERFACE,  /* CS_INTERFACE Descriptor Type   */
	USB_DESCRIPTOR_SUBTYPE_AUDIO_CONTROL_INPUT_TERMINAL,
	/* INPUT_TERMINAL descriptor subtype  */
	USB_AUDIO_RECORDER_CONTROL_INPUT_TERMINAL_ID, /* Constant uniquely identifying the Terminal within the audio
             function. This value is used in all requests
             to address this Terminal.  */
	0x03U,
	0x06U,        /* A generic microphone that does not fit under any of the other classifications.  */
	0x00U,        /* This Input Terminal has no association  */
	0x02U,        /* This Terminal's output audio channel cluster has 1 logical output channels  */
	0x03U, 0x00U, /* Spatial locations present in the cluster */
	0x00U,        /* Index of a string descriptor, describing the name of the first logical channel.   */
	0x00U,        /* Index of a string descriptor, describing the Input Terminal.   */

	/* Audio Class Specific type of  Output Terminal [ID4] */
	USB_AUDIO_OUTPUT_TERMINAL_ONLY_DESC_SIZE, /* Size of the descriptor, in bytes  */
	USB_DESCRIPTOR_TYPE_AUDIO_CS_INTERFACE,   /* CS_INTERFACE Descriptor Type   */
	USB_DESCRIPTOR_SUBTYPE_AUDIO_CONTROL_OUTPUT_TERMINAL,
	/* OUTPUT_TERMINAL descriptor subtype  */
	USB_AUDIO_RECORDER_CONTROL_OUTPUT_TERMINAL_ID, /* Constant uniquely identifying the Terminal within the audio
                                                    function*/
	0x01U,
	0x01U, /* A Terminal dealing with a signal carried over an endpoint in an AudioStreaming interface */
	0x00U, /*  This Output Terminal has no association   */
	USB_AUDIO_RECORDER_CONTROL_INPUT_TERMINAL_ID, /* ID of the Unit or Terminal to which this Terminal is connected.   */
	0x00U,                                     /* Index of a string descriptor, describing the Output Terminal.  */

    /* Audio Class Specific INTERFACE Descriptor, alternative interface 0  */
    USB_DESCRIPTOR_LENGTH_INTERFACE,  /* Descriptor size is 9 bytes  */
    USB_DESCRIPTOR_TYPE_INTERFACE,    /* INTERFACE Descriptor Type   */
    USB_AUDIO_STREAM_INTERFACE_INDEX, /* The number of this interface is 1.  */
    0x00U,                            /* The value used to select the alternate setting for this interface is 0   */
    0x00U,                    /* The number of endpoints used by this interface is 0 (excluding endpoint zero)   */
    USB_AUDIO_CLASS,          /* The interface implements the Audio Interface class   */
    USB_SUBCLASS_AUDIOSTREAM, /* The interface implements the AUDIOSTREAMING Subclass   */
    0x00U,                    /* The interface doesn't use any class-specific protocols   */
	USBD_IDX_AUDIO_OUT_STR,   /* The device doesn't have a string descriptor describing this iInterface  */

    /* Audio Class Specific INTERFACE Descriptor, alternative interface 1 */
    USB_DESCRIPTOR_LENGTH_INTERFACE,  /* Descriptor size is 9 bytes  */
    USB_DESCRIPTOR_TYPE_INTERFACE,    /* INTERFACE Descriptor Type  */
    USB_AUDIO_STREAM_INTERFACE_INDEX, /*The number of this interface is 1.  */
    0x01U,                            /* The value used to select the alternate setting for this interface is 1  */
    0x02U,                    /* The number of endpoints used by this interface is 2 (excluding endpoint zero)    */
    USB_AUDIO_CLASS,          /* The interface implements the Audio Interface class   */
    USB_SUBCLASS_AUDIOSTREAM, /* The interface implements the AUDIOSTREAMING Subclass   */
    0x00U,                    /* The interface doesn't use any class-specific protocols  */
	USBD_IDX_AUDIO_OUT_STR,   /* The device doesn't have a string descriptor describing this iInterface  */

    /* Audio Class Specific CS INTERFACE Descriptor*/
    USB_AUDIO_STREAMING_IFACE_DESC_SIZE,            /* Size of the descriptor, in bytes  */
    USB_DESCRIPTOR_TYPE_AUDIO_CS_INTERFACE,         /* CS_INTERFACE Descriptor Type  */
    USB_DESCRIPTOR_SUBTYPE_AUDIO_STREAMING_GENERAL, /* AS_GENERAL descriptor subtype  */
    USB_AUDIO_SPEAKER_CONTROL_INPUT_TERMINAL_ID,    /* The Terminal ID of the Terminal to which the endpoint of this
                                                       interface is connected. */
	0x00U,
    0x01U,
    0x00U, /* PCM  */

    /* Audio Class Specific type I format INTERFACE Descriptor */
    USB_AUDIO_STREAMING_TYPE_I_DESC_SIZE,               /* bLength (11) */
    USB_DESCRIPTOR_TYPE_AUDIO_CS_INTERFACE,             /* bDescriptorType (CS_INTERFACE) */
    USB_DESCRIPTOR_SUBTYPE_AUDIO_STREAMING_FORMAT_TYPE, /* DescriptorSubtype: AUDIO STREAMING FORMAT TYPE */
    USB_AUDIO_FORMAT_TYPE_I,                            /* Format Type: Type I */
    AUDIO_FORMAT_CHANNELS,                              /* Number of Channels: one channel */
    AUDIO_FORMAT_SIZE,                                  /* SubFrame Size: one byte per audio subframe */
    AUDIO_FORMAT_BITS,                                  /* Bit Resolution: 8 bits per sample */
    0x01U,                                              /* One frequency supported */
	TSAMFREQ2BYTES(44100),

    /* ENDPOINT Descriptor */
    USB_ENDPOINT_AUDIO_DESCRIPTOR_LENGTH, /* Descriptor size is 9 bytes  */
    USB_DESCRIPTOR_TYPE_ENDPOINT,         /* Descriptor type (endpoint descriptor) */
    USB_AUDIO_SPEAKER_STREAM_ENDPOINT |
        (USB_OUT << USB_DESCRIPTOR_ENDPOINT_ADDRESS_DIRECTION_SHIFT), /* OUT endpoint address 1 */
    USB_ENDPOINT_ISOCHRONOUS | 0x04,                                  /* Isochronous endpoint */
    USB_SHORT_GET_LOW(FS_ISO_OUT_ENDP_PACKET_SIZE + AUDIO_FORMAT_CHANNELS * AUDIO_FORMAT_SIZE),
    USB_SHORT_GET_HIGH(FS_ISO_OUT_ENDP_PACKET_SIZE + AUDIO_FORMAT_CHANNELS * AUDIO_FORMAT_SIZE), /* 16 bytes  */
    FS_ISO_OUT_ENDP_INTERVAL, /* bInterval(0x01U): x ms */
    0x00U,                    /* Unused */
    USB_AUDIO_SPEAKER_FEEDBACK_ENDPOINT |
        (USB_IN
         << USB_DESCRIPTOR_ENDPOINT_ADDRESS_DIRECTION_SHIFT), /* Synchronization Endpoint (if used) is endpoint 0x81  */

    /* Audio Class Specific ENDPOINT Descriptor  */
    USB_AUDIO_STREAMING_ENDP_DESC_SIZE,      /*  Size of the descriptor, in bytes  */
    USB_AUDIO_STREAM_ENDPOINT_DESCRIPTOR,    /* CS_ENDPOINT Descriptor Type  */
    USB_AUDIO_EP_GENERAL_DESCRIPTOR_SUBTYPE, /* AUDIO_EP_GENERAL descriptor subtype  */
    0x01U,                                   /* Bit 0: Sampling Frequency 0
                                                Bit 1: Pitch 0
                                                Bit 7: MaxPacketsOnly 0   */
    0x00U,                                   /* Indicates the units used for the wLockDelay field: 0: Undefined  */
    0x00U,
    0x00U, /* Indicates the time it takes this endpoint to reliably lock its internal clock recovery circuitry */

    /* Endpoint 1 Feedback ENDPOINT */
    USB_ENDPOINT_AUDIO_DESCRIPTOR_LENGTH, /* bLength */
    USB_DESCRIPTOR_TYPE_ENDPOINT,         /* bDescriptorType */
    USB_AUDIO_SPEAKER_FEEDBACK_ENDPOINT |
        (USB_IN << USB_DESCRIPTOR_ENDPOINT_ADDRESS_DIRECTION_SHIFT), /* This is an IN endpoint with endpoint number 3 */
    USB_ENDPOINT_ISOCHRONOUS,                          				 /*  Types -
                                                                         Transfer: ISOCHRONOUS
                                                                         Sync: Async
                                                                         Usage: Feedback EP   */
    FS_ISO_FEEDBACK_ENDP_PACKET_SIZE,
    0x00, /* wMaxPacketSize */
    0x01, /* interval polling(2^x ms) */
	FEEDBACK_REFRESH, /* bRefresh(8ms)  */
    0x00, /* unused */

	/* Audio Class Specific INTERFACE Descriptor, alternative interface 0  */
	USB_DESCRIPTOR_LENGTH_INTERFACE,  /* Descriptor size is 9 bytes  */
	USB_DESCRIPTOR_TYPE_INTERFACE,    /* INTERFACE Descriptor Type   */
	USB_AUDIO_IN_STREAM_INTERFACE_INDEX, /* The number of this interface is 2.  */
	0x00U,                            /* The value used to select the alternate setting for this interface is 0   */
	0x00U,                    /* The number of endpoints used by this interface is 0 (excluding endpoint zero)   */
	USB_AUDIO_CLASS,          /* The interface implements the Audio Interface class   */
	USB_SUBCLASS_AUDIOSTREAM, /* The interface implements the AUDIOSTREAMING Subclass   */
	0x00U,                    /* The interface doesn't use any class-specific protocols   */
	USBD_IDX_AUDIO_IN_STR,	  /* The device doesn't have a string descriptor describing this iInterface  */

	/* Audio Class Specific INTERFACE Descriptor, alternative interface 1 */
	USB_DESCRIPTOR_LENGTH_INTERFACE,  /* Descriptor size is 9 bytes  */
	USB_DESCRIPTOR_TYPE_INTERFACE,    /* INTERFACE Descriptor Type  */
	USB_AUDIO_IN_STREAM_INTERFACE_INDEX, /*The number of this interface is 2.  */
	0x01U,                            /* The value used to select the alternate setting for this interface is 1  */
	0x01U,                    /* The number of endpoints used by this interface is 1 (excluding endpoint zero)    */
	USB_AUDIO_CLASS,          /* The interface implements the Audio Interface class   */
	USB_SUBCLASS_AUDIOSTREAM, /* The interface implements the AUDIOSTREAMING Subclass   */
	0x00U,                    /* The interface doesn't use any class-specific protocols  */
	USBD_IDX_AUDIO_IN_STR,    /* The device doesn't have a string descriptor describing this iInterface  */

	/* Audio Class Specific CS INTERFACE Descriptor*/
	USB_AUDIO_STREAMING_IFACE_DESC_SIZE,            /* Size of the descriptor, in bytes  */
	USB_DESCRIPTOR_TYPE_AUDIO_CS_INTERFACE,         /* CS_INTERFACE Descriptor Type  */
	USB_DESCRIPTOR_SUBTYPE_AUDIO_STREAMING_GENERAL, /* AS_GENERAL descriptor subtype  */
	USB_AUDIO_RECORDER_CONTROL_OUTPUT_TERMINAL_ID,    /* The Terminal ID of the Terminal to which the endpoint of this
                                                      interface is connected. */
	0x00U, /* Delay introduced by the data path. Expressed in number of frames.  */
	0x01U,
	0x00U, /* PCM  */

	/* Audio Class Specific type I format INTERFACE Descriptor */
	USB_AUDIO_STREAMING_TYPE_I_DESC_SIZE,               /* bLength (11) */
	USB_DESCRIPTOR_TYPE_AUDIO_CS_INTERFACE,             /* bDescriptorType (CS_INTERFACE) */
	USB_DESCRIPTOR_SUBTYPE_AUDIO_STREAMING_FORMAT_TYPE, /* DescriptorSubtype: AUDIO STREAMING FORMAT TYPE */
	USB_AUDIO_FORMAT_TYPE_I,                            /* Format Type: Type I */
	AUDIO_FORMAT_CHANNELS,                              /* Number of Channels: one channel */
	AUDIO_FORMAT_SIZE,                                  /* SubFrame Size: one byte per audio subframe */
	AUDIO_FORMAT_BITS,                                  /* Bit Resolution: 8 bits per sample */
	0x01U,                                              /* One frequency supported */
	TSAMFREQ2BYTES(44100),

	/* ENDPOINT Descriptor */
	USB_ENDPOINT_AUDIO_DESCRIPTOR_LENGTH, /* Descriptor size is 9 bytes  */
	USB_DESCRIPTOR_TYPE_ENDPOINT,         /* Descriptor type (endpoint descriptor) */
	USB_AUDIO_RECORDER_STREAM_ENDPOINT |
		(USB_IN << USB_DESCRIPTOR_ENDPOINT_ADDRESS_DIRECTION_SHIFT), /* OUT endpoint address 1 */
	USB_ENDPOINT_ISOCHRONOUS | 0x04,                                  /* Isochronous endpoint */
	USB_SHORT_GET_LOW(FS_ISO_IN_ENDP_PACKET_SIZE + AUDIO_IN_FORMAT_CHANNELS * AUDIO_FORMAT_SIZE),
	USB_SHORT_GET_HIGH(FS_ISO_IN_ENDP_PACKET_SIZE + AUDIO_IN_FORMAT_CHANNELS * AUDIO_FORMAT_SIZE), /* 16 bytes  */
	FS_ISO_IN_ENDP_INTERVAL, /* bInterval(0x01U): x ms */
	0x00U,                    /* Unused */
	0x00U,					/* bSynchAddress */

	/* Audio Class Specific ENDPOINT Descriptor  */
	USB_AUDIO_STREAMING_ENDP_DESC_SIZE,      /*  Size of the descriptor, in bytes  */
	USB_AUDIO_STREAM_ENDPOINT_DESCRIPTOR,    /* CS_ENDPOINT Descriptor Type  */
	USB_AUDIO_EP_GENERAL_DESCRIPTOR_SUBTYPE, /* AUDIO_EP_GENERAL descriptor subtype  */
	0x01U,                                   /* Bit 0: Sampling Frequency 0
                                               Bit 1: Pitch 0
                                               Bit 7: MaxPacketsOnly 0   */
	0x00U,                                   /* Indicates the units used for the wLockDelay field: 0: Undefined  */
	0x00U,
	0x00U, /* Indicates the time it takes this endpoint to reliably lock its internal clock recovery circuitry */

	/// @note Add MIDI Streaming Class
	/* MIDI Class Specific INTERFACE Descriptor, alternative interface 0  */
	USB_DESCRIPTOR_LENGTH_INTERFACE,  /* Descriptor size is 9 bytes  */
	USB_DESCRIPTOR_TYPE_INTERFACE,    /* INTERFACE Descriptor Type   */
	USB_MIDI_CONTROL_INTERFACE_INDEX, /* The number of this interface is 1.  */
	0x00U,                            /* The value used to select the alternate setting for this interface is 0   */
	0x00U,                    /* The number of endpoints used by this interface is 0 (excluding endpoint zero)   */
	USB_AUDIO_CLASS,          /* The interface implements the Audio Interface class   */
	USB_SUBCLASS_AUDIOCONTROL,  /*The interface implements the AUDIOCONTROL Subclass  */
	0x00U,                    /* The interface doesn't use any class-specific protocols   */
	0x00U,                    /* The device doesn't have a string descriptor describing this iInterface  */

	/* MIDI Class Specific type of INTERFACE Descriptor */
	USB_MIDI_CONTROL_INTERFACE_HEADER_LENGTH,   /* Size of the descriptor, in bytes  */
	USB_DESCRIPTOR_TYPE_AUDIO_CS_INTERFACE,      /* CS_INTERFACE Descriptor Type   */
	USB_DESCRIPTOR_SUBTYPE_AUDIO_CONTROL_HEADER, /* HEADER descriptor subtype   */
	0x00U, 0x01U, /* Audio Device compliant to the USB MIDI specification version 1.00  */
	USB_MIDI_CONTROL_INTERFACE_HEADER_LENGTH, 0x00U,
	0x01U,									/* bInCollection */
	USB_MIDI_STREAM_INTERFACE_INDEX,		/* baInterfaceNr(1) */

	/* Audio Class Specific INTERFACE Descriptor, alternative interface 0  */
	USB_DESCRIPTOR_LENGTH_INTERFACE,  /* Descriptor size is 9 bytes  */
	USB_DESCRIPTOR_TYPE_INTERFACE,    /* INTERFACE Descriptor Type   */
	USB_MIDI_STREAM_INTERFACE_INDEX, /* The number of this interface is 1.  */
	0x00U,                            /* The value used to select the alternate setting for this interface is 0   */
	0x02U,                    /* The number of endpoints used by this interface is 0 (excluding endpoint zero)   */
	USB_AUDIO_CLASS,          /* The interface implements the Audio Interface class   */
	USB_SUBCLASS_MIDISTREAM,  /* The interface implements the AUDIOSTREAMING Subclass   */
	0x00U,                    /* The interface doesn't use any class-specific protocols   */
	0x00U,                    /* The device doesn't have a string descriptor describing this iInterface  */

	/* ---------- Class-Specific MS Interface Header Descriptor (midi 1.0 section 6.1.2) ---------- */
	SIZE_OF_CS_MS_IF_HDR_DESC,					/* bLength							*/
	USB_DESCRIPTOR_TYPE_AUDIO_CS_INTERFACE,		/* bDescriptorType (CS_INTERFACE)	*/
	TYPE_MS_HEADER,								/* bDescriptorSubtype (MS_HEADER)	*/
	0x00U, 0x01U, /* Audio Device compliant to the USB MIDI specification version 1.00  */
	SIZE_OF_MS_HEADER_CONTENT, 0x00U,

	/* ---------- MIDI IN (host to device, EMBEDDED) Jack Descriptor (midi 1.0 section 6.1.2.2) ---------- */
	SIZE_OF_MIDI_IN_JACK_DESC,				/* bLength 							*/
	USB_DESCRIPTOR_TYPE_AUDIO_CS_INTERFACE,		/* bDescriptorType */
	TYPE_MIDI_IN_JACK,						/* bDescriptorSubtype */
	TYPE_MS_EMBEDDED,						/* bJackType	*/
	0x10,									/* bJackID */
	USBD_IDX_MIDI_STR,                      /* iJack	*/

	/* ---------- MIDI OUT (device to host, External) Jack Descriptor (midi 1.0 section 6.1.2.3) ---------- */
	SIZE_OF_MIDI_OUT_JACK_DESC,				/* bLength */
	USB_DESCRIPTOR_TYPE_AUDIO_CS_INTERFACE,		/* bDescriptorType */
	TYPE_MIDI_OUT_JACK,						/* bDescriptorSubType */
	TYPE_MS_EXTERNAL,						/* bJackType */
	0x40,									/* bJackID */
	0x01,									/* bNrInputPins */
	0x10,									/* baSourceID(1): ID of Entity to which the first Input Pin of this MIDI OUT Jack is connected */
	0x01,									/* baSourcePin(1): Output pin number of the Entity to which the first Input Pin of this MIDI OUT Jack is connected */
	0x00,									/* iJack */

	/* ---------- MIDI OUT (device to host, EMBEDDED) Jack Descriptor (midi 1.0 section 6.1.2.3) ---------- */
	SIZE_OF_MIDI_OUT_JACK_DESC,				/* bLength */
	USB_DESCRIPTOR_TYPE_AUDIO_CS_INTERFACE,		/* bDescriptorType */
	TYPE_MIDI_OUT_JACK,						/* bDescriptorSubType */
	TYPE_MS_EMBEDDED,						/* bJackType */
	0x30,									/* bJackID */
	0x01,									/* bNrInputPins */
	0x20,									/* baSourceID(1): ID of Entity to which the first Input Pin of this MIDI OUT Jack is connected */
	0x01,									/* baSourcePin(1): Output pin number of the Entity to which the first Input Pin of this MIDI OUT Jack is connected */
	USBD_IDX_MIDI_STR,						/* iJack */

	/* ---------- MIDI IN (host to device, External) Jack Descriptor (midi 1.0 section 6.1.2.2) ---------- */
	SIZE_OF_MIDI_IN_JACK_DESC,				/* bLength 							*/
	USB_DESCRIPTOR_TYPE_AUDIO_CS_INTERFACE,		/* bDescriptorType */
	TYPE_MIDI_IN_JACK,						/* bDescriptorSubtype */
	TYPE_MS_EXTERNAL,						/* bJackType	*/
	0x20,									/* bJackID */
	0x00,									/* iJack	*/

	/* ---------- Standard MS Bulk Data Endpoint Descriptor (midi 1.0 section 6.2.1) ---------- */
	SIZE_OF_STD_MS_BD_EP_DESC,				/* bLength */
	USB_DESCRIPTOR_TYPE_ENDPOINT,			/* Descriptor type: Endpopint descriptor */
	USB_MIDI_STREAM_RX_ENDPOINT,							/* bEndpointAddress (bulk Out endpoint 4) */
	0x02,									/* bmAttribute: Bulk Transfer */
	USB_MIDI_ENDP_MAX_PACKET_SIZE,	0x00U,		/* wMaxPacketSize */
	0x00,									/* bInterval: must be 0 */
	0x00,									/* bRefresh: must be 0 */
	0x00,									/* bSyncAddress: must be 0 */

	/* ---------- Class-Specific MS Bulk Data Endpoint Descriptor (midi 1.0 section 6.2.2) ---------- */
	SIZE_OF_CS_MS_BD_EP_DESC,				/* bLength */
	USB_AUDIO_STREAM_ENDPOINT_DESCRIPTOR,			/* bDescriptorType (CS_ENDPOINT) */
	TYPE_MS_GENERAL,						/* bDescriptorSubType (MS_GENERAL) */
	0x01,									/* bNumEmbMIDIJack	*/
	0x10,									/* baAssocJackID(1): to Embedded MIDI In Jack	*/

	/* ---------- Standard MS Bulk Data Endpoint Descriptor (midi 1.0 section 6.2.1) ---------- */
	SIZE_OF_STD_MS_BD_EP_DESC,				/* Descriptor size */
	USB_DESCRIPTOR_TYPE_ENDPOINT,					/* Descriptor type: Endpopint descriptor */
	USB_MIDI_STREAM_TX_ENDPOINT |
		(USB_IN << USB_DESCRIPTOR_ENDPOINT_ADDRESS_DIRECTION_SHIFT),	/* bEndpointAddress (Bulk In endpoint 5) */
	0x02,									/* bmAttribute: Bulk Transfer */
	USB_MIDI_ENDP_MAX_PACKET_SIZE,	0x00U,		/* wMaxPacketSize */
	0x00,									/* bInterval: must be 0	*/
	0x00,									/* bRefresh: must be 0	*/
	0x00,									/* bSyncAddress: must be 0 */

	/* ---------- Class-Specific MS Bulk Data Endpoint Descriptor (midi 1.0 section 6.2.2) ---------- */
	SIZE_OF_CS_MS_BD_EP_DESC,				/* bLength */
	USB_AUDIO_STREAM_ENDPOINT_DESCRIPTOR,			/* bDescriptorType (CS_ENDPOINT) */
	TYPE_MS_GENERAL,						/* bDescriptorSubType (MS_GENERAL) */
	0x01,									/* bNumEmbMIDIJack	*/
	0x30									/* baAssocJackID(1): to Embedded MIDI OUT Jack	*/
};
 #endif	// #if SUPPORT_USB_HIGH_SPEED
#endif	// DESCRIPTOR_X19730
#if DESCRIPTOR_X19850
uint8_t g_UsbDeviceConfigurationDescriptor[] = {
	/* Configuration Descriptor Size - always 9 bytes*/
	USB_DESCRIPTOR_LENGTH_CONFIGURE, /* Size of this descriptor in bytes */
	USB_DESCRIPTOR_TYPE_CONFIGURE,   /* CONFIGURATION Descriptor Type */
	USB_SHORT_GET_LOW(
		SIZE_OF_TOTAL_CONFIG_DESC
	),
	USB_SHORT_GET_HIGH(
		SIZE_OF_TOTAL_CONFIG_DESC
	),	/* Total length of data returned for this configuration. */
	USB_AUDIO_PLAYER_INTERFACE_COUNT,         /* Number of interfaces supported by this configuration */
	USB_AUDIO_MIDI_CONFIGURE_INDEX,           /* Value to use as an argument to the
                                                    SetConfiguration() request to select this configuration */
    0x00U,                                     /* Index of string descriptor describing this configuration */
    (USB_DESCRIPTOR_CONFIGURE_ATTRIBUTE_D7_MASK) |
        (USB_DEVICE_CONFIG_SELF_POWER << USB_DESCRIPTOR_CONFIGURE_ATTRIBUTE_SELF_POWERED_SHIFT) |
        (USB_DEVICE_CONFIG_REMOTE_WAKEUP << USB_DESCRIPTOR_CONFIGURE_ATTRIBUTE_REMOTE_WAKEUP_SHIFT),
    /* Configuration characteristics
       D7: Reserved (set to one)
       D6: Self-powered
       D5: Remote Wakeup
       D4...0: Reserved (reset to zero)
    */
    USB_DEVICE_MAX_POWER, /* Maximum power consumption of the USB
                           * device from the bus in this specific
                           * configuration when the device is fully
                           * operational. Expressed in 2 mA units
                           *  (i.e., 50 = 100 mA).
                           */

	USB_DESCRIPTOR_LENGTH_INTERFACE_ASSOSIATION,
	USB_DESCRIPTOR_TYPE_INTERFACE_ASSOCIATION,
	USB_AUDIO_CONTROL_INTERFACE_INDEX,		/* bFirstInterface */
	0x03,									/* bInterfaceCount */	// Audio Control, Audio Streaming, Audio Streaming
	0x01,									/* bFunctionClass */
	0x00,									/* bFuctionSubClass */
	0x00,									/* bFunctionProtocol */
	0x00,									/* iFunction */

    USB_DESCRIPTOR_LENGTH_INTERFACE,   /* Size of this descriptor in bytes */
    USB_DESCRIPTOR_TYPE_INTERFACE,     /* INTERFACE Descriptor Type */
    USB_AUDIO_CONTROL_INTERFACE_INDEX, /* Number of this interface. */
    0x00U,                             /* Value used to select this alternate setting
                                          for the interface identified in the prior field */
	0x00U,
    USB_AUDIO_CLASS,                   /*The interface implements the Audio Interface class  */
    USB_SUBCLASS_AUDIOCONTROL,         /*The interface implements the AUDIOCONTROL Subclass  */
    0x00U,                             /*The interface doesn't use any class-specific protocols  */
    0x00U,                             /* The device doesn't have a string descriptor describing this iInterface  */

    /* Audio Class Specific type of INTERFACE Descriptor */
    USB_AUDIO_CONTROL_INTERFACE_HEADER_LENGTH,   /* Size of the descriptor, in bytes  */
    USB_DESCRIPTOR_TYPE_AUDIO_CS_INTERFACE,      /* CS_INTERFACE Descriptor Type   */
    USB_DESCRIPTOR_SUBTYPE_AUDIO_CONTROL_HEADER, /* HEADER descriptor subtype   */
    0x00U, 0x01U, /* Audio Device compliant to the USB Audio specification version 1.00  */
	0x34U, 0x00U,	// 10 + 12 + 9 + 12 + 9
	0x02U,									/* bInCollection */
	USB_AUDIO_STREAM_INTERFACE_INDEX,		/* baInterfaceNr(1) */
	USB_AUDIO_IN_STREAM_INTERFACE_INDEX,	/* baInterfaceNr(2) */

    /* Audio Class Specific type of Input Terminal*/
    USB_AUDIO_INPUT_TERMINAL_ONLY_DESC_SIZE, /* Size of the descriptor, in bytes  */
    USB_DESCRIPTOR_TYPE_AUDIO_CS_INTERFACE,  /* CS_INTERFACE Descriptor Type   */
    USB_DESCRIPTOR_SUBTYPE_AUDIO_CONTROL_INPUT_TERMINAL,
    /* INPUT_TERMINAL descriptor subtype  */
    USB_AUDIO_SPEAKER_CONTROL_INPUT_TERMINAL_ID, /* Constant uniquely identifying the Terminal within the audio
              function. This value is used in all requests
              to address this Terminal.  */
    0x01U,
    0x01,         /* A generic microphone that does not fit under any of the other classifications.  */
    0x00U,        /* This Input Terminal has no association  */
    0x02U,        /* This Terminal's output audio channel cluster has 1 logical output channels  */
    0x03U, 0x00U, /* Spatial locations present in the cluster */
    0x00U,        /* Index of a string descriptor, describing the name of the first logical channel.   */
    0x00U,        /* Index of a string descriptor, describing the Input Terminal.   */

    /* Audio Class Specific type of  Output Terminal */
    USB_AUDIO_OUTPUT_TERMINAL_ONLY_DESC_SIZE, /* Size of the descriptor, in bytes  */
    USB_DESCRIPTOR_TYPE_AUDIO_CS_INTERFACE,   /* CS_INTERFACE Descriptor Type   */
    USB_DESCRIPTOR_SUBTYPE_AUDIO_CONTROL_OUTPUT_TERMINAL,
    /* OUTPUT_TERMINAL descriptor subtype  */
    USB_AUDIO_SPEAKER_CONTROL_OUTPUT_TERMINAL_ID, /* Constant uniquely identifying the Terminal within the audio
                                                     function*/
    0x03U,
    0x06U, /* A Terminal dealing with a signal carried over an endpoint in an AudioStreaming interface */
    0x00U, /*  This Output Terminal has no association   */
	USB_AUDIO_SPEAKER_CONTROL_INPUT_TERMINAL_ID, /* ID of the Unit or Terminal to which this Terminal is connected.   */
    0x00U,                                     /* Index of a string descriptor, describing the Output Terminal.  */

	/* Audio Class Specific type of Input Terminal [ID3] */
	USB_AUDIO_INPUT_TERMINAL_ONLY_DESC_SIZE, /* Size of the descriptor, in bytes  */
	USB_DESCRIPTOR_TYPE_AUDIO_CS_INTERFACE,  /* CS_INTERFACE Descriptor Type   */
	USB_DESCRIPTOR_SUBTYPE_AUDIO_CONTROL_INPUT_TERMINAL,
	/* INPUT_TERMINAL descriptor subtype  */
	USB_AUDIO_RECORDER_CONTROL_INPUT_TERMINAL_ID, /* Constant uniquely identifying the Terminal within the audio
             function. This value is used in all requests
             to address this Terminal.  */
	0x03U,
	0x06U,        /* A generic microphone that does not fit under any of the other classifications.  */
	0x00U,        /* This Input Terminal has no association  */
	0x04U,        /* This Terminal's output audio channel cluster has 1 logical output channels  */		/// @note 4ch
	0x03U, 0x00U, /* Spatial locations present in the cluster */
	0x00U,        /* Index of a string descriptor, describing the name of the first logical channel.   */
	0x00U,        /* Index of a string descriptor, describing the Input Terminal.   */

	/* Audio Class Specific type of  Output Terminal [ID4] */
	USB_AUDIO_OUTPUT_TERMINAL_ONLY_DESC_SIZE, /* Size of the descriptor, in bytes  */
	USB_DESCRIPTOR_TYPE_AUDIO_CS_INTERFACE,   /* CS_INTERFACE Descriptor Type   */
	USB_DESCRIPTOR_SUBTYPE_AUDIO_CONTROL_OUTPUT_TERMINAL,
	/* OUTPUT_TERMINAL descriptor subtype  */
	USB_AUDIO_RECORDER_CONTROL_OUTPUT_TERMINAL_ID, /* Constant uniquely identifying the Terminal within the audio
                                                    function*/
	0x01U,
	0x01U, /* A Terminal dealing with a signal carried over an endpoint in an AudioStreaming interface */
	0x00U, /*  This Output Terminal has no association   */
	USB_AUDIO_RECORDER_CONTROL_INPUT_TERMINAL_ID, /* ID of the Unit or Terminal to which this Terminal is connected.   */
	0x00U,                                     /* Index of a string descriptor, describing the Output Terminal.  */

    /* Audio Class Specific INTERFACE Descriptor, alternative interface 0  */
    USB_DESCRIPTOR_LENGTH_INTERFACE,  /* Descriptor size is 9 bytes  */
    USB_DESCRIPTOR_TYPE_INTERFACE,    /* INTERFACE Descriptor Type   */
    USB_AUDIO_STREAM_INTERFACE_INDEX, /* The number of this interface is 1.  */
    0x00U,                            /* The value used to select the alternate setting for this interface is 0   */
    0x00U,                    /* The number of endpoints used by this interface is 0 (excluding endpoint zero)   */
    USB_AUDIO_CLASS,          /* The interface implements the Audio Interface class   */
    USB_SUBCLASS_AUDIOSTREAM, /* The interface implements the AUDIOSTREAMING Subclass   */
    0x00U,                    /* The interface doesn't use any class-specific protocols   */
	USBD_IDX_AUDIO_STR,   /* The device doesn't have a string descriptor describing this iInterface  */

    /* Audio Class Specific INTERFACE Descriptor, alternative interface 1 */
    USB_DESCRIPTOR_LENGTH_INTERFACE,  /* Descriptor size is 9 bytes  */
    USB_DESCRIPTOR_TYPE_INTERFACE,    /* INTERFACE Descriptor Type  */
    USB_AUDIO_STREAM_INTERFACE_INDEX, /*The number of this interface is 1.  */
    0x01U,                            /* The value used to select the alternate setting for this interface is 1  */
    0x02U,                    /* The number of endpoints used by this interface is 2 (excluding endpoint zero)    */
    USB_AUDIO_CLASS,          /* The interface implements the Audio Interface class   */
    USB_SUBCLASS_AUDIOSTREAM, /* The interface implements the AUDIOSTREAMING Subclass   */
    0x00U,                    /* The interface doesn't use any class-specific protocols  */
	USBD_IDX_AUDIO_STR,   /* The device doesn't have a string descriptor describing this iInterface  */

    /* Audio Class Specific CS INTERFACE Descriptor*/
    USB_AUDIO_STREAMING_IFACE_DESC_SIZE,            /* Size of the descriptor, in bytes  */
    USB_DESCRIPTOR_TYPE_AUDIO_CS_INTERFACE,         /* CS_INTERFACE Descriptor Type  */
    USB_DESCRIPTOR_SUBTYPE_AUDIO_STREAMING_GENERAL, /* AS_GENERAL descriptor subtype  */
    USB_AUDIO_SPEAKER_CONTROL_INPUT_TERMINAL_ID,    /* The Terminal ID of the Terminal to which the endpoint of this
                                                       interface is connected. */
	0x00U,
    0x01U,
    0x00U, /* PCM  */

    /* Audio Class Specific type I format INTERFACE Descriptor */
    USB_AUDIO_STREAMING_TYPE_I_DESC_SIZE,               /* bLength (11) */
    USB_DESCRIPTOR_TYPE_AUDIO_CS_INTERFACE,             /* bDescriptorType (CS_INTERFACE) */
    USB_DESCRIPTOR_SUBTYPE_AUDIO_STREAMING_FORMAT_TYPE, /* DescriptorSubtype: AUDIO STREAMING FORMAT TYPE */
    USB_AUDIO_FORMAT_TYPE_I,                            /* Format Type: Type I */
    AUDIO_FORMAT_CHANNELS,                              /* Number of Channels: one channel */
    AUDIO_FORMAT_SIZE,                                  /* SubFrame Size: one byte per audio subframe */
    AUDIO_FORMAT_BITS,                                  /* Bit Resolution: 8 bits per sample */
    0x01U,                                              /* One frequency supported */
	TSAMFREQ2BYTES(44100),

    /* ENDPOINT Descriptor */
    USB_ENDPOINT_AUDIO_DESCRIPTOR_LENGTH, /* Descriptor size is 9 bytes  */
    USB_DESCRIPTOR_TYPE_ENDPOINT,         /* Descriptor type (endpoint descriptor) */
    USB_AUDIO_SPEAKER_STREAM_ENDPOINT |
        (USB_OUT << USB_DESCRIPTOR_ENDPOINT_ADDRESS_DIRECTION_SHIFT), /* OUT endpoint address 1 */
    USB_ENDPOINT_ISOCHRONOUS | 0x04,                                  /* Isochronous endpoint */
    USB_SHORT_GET_LOW(FS_ISO_OUT_ENDP_PACKET_SIZE + AUDIO_FORMAT_CHANNELS * AUDIO_FORMAT_SIZE),
    USB_SHORT_GET_HIGH(FS_ISO_OUT_ENDP_PACKET_SIZE + AUDIO_FORMAT_CHANNELS * AUDIO_FORMAT_SIZE), /* 16 bytes  */
    FS_ISO_OUT_ENDP_INTERVAL, /* bInterval(0x01U): x ms */
    0x00U,                    /* Unused */
    USB_AUDIO_SPEAKER_FEEDBACK_ENDPOINT |
        (USB_IN
         << USB_DESCRIPTOR_ENDPOINT_ADDRESS_DIRECTION_SHIFT), /* Synchronization Endpoint (if used) is endpoint 0x81  */

    /* Audio Class Specific ENDPOINT Descriptor  */
    USB_AUDIO_STREAMING_ENDP_DESC_SIZE,      /*  Size of the descriptor, in bytes  */
    USB_AUDIO_STREAM_ENDPOINT_DESCRIPTOR,    /* CS_ENDPOINT Descriptor Type  */
    USB_AUDIO_EP_GENERAL_DESCRIPTOR_SUBTYPE, /* AUDIO_EP_GENERAL descriptor subtype  */
    0x01U,                                   /* Bit 0: Sampling Frequency 0
                                                Bit 1: Pitch 0
                                                Bit 7: MaxPacketsOnly 0   */
    0x00U,                                   /* Indicates the units used for the wLockDelay field: 0: Undefined  */
    0x00U,
    0x00U, /* Indicates the time it takes this endpoint to reliably lock its internal clock recovery circuitry */

    /* Endpoint 1 Feedback ENDPOINT */
    USB_ENDPOINT_AUDIO_DESCRIPTOR_LENGTH, /* bLength */
    USB_DESCRIPTOR_TYPE_ENDPOINT,         /* bDescriptorType */
    USB_AUDIO_SPEAKER_FEEDBACK_ENDPOINT |
        (USB_IN << USB_DESCRIPTOR_ENDPOINT_ADDRESS_DIRECTION_SHIFT), /* This is an IN endpoint with endpoint number 3 */
    USB_ENDPOINT_ISOCHRONOUS,                          				 /*  Types -
                                                                         Transfer: ISOCHRONOUS
                                                                         Sync: Async
                                                                         Usage: Feedback EP   */
    FS_ISO_FEEDBACK_ENDP_PACKET_SIZE,
    0x00, /* wMaxPacketSize */
    0x01, /* interval polling(2^x ms) */
	FEEDBACK_REFRESH, /* bRefresh(8ms)  */
    0x00, /* unused */

	/* Audio Class Specific INTERFACE Descriptor, alternative interface 0  */
	USB_DESCRIPTOR_LENGTH_INTERFACE,  /* Descriptor size is 9 bytes  */
	USB_DESCRIPTOR_TYPE_INTERFACE,    /* INTERFACE Descriptor Type   */
	USB_AUDIO_IN_STREAM_INTERFACE_INDEX, /* The number of this interface is 2.  */
	0x00U,                            /* The value used to select the alternate setting for this interface is 0   */
	0x00U,                    /* The number of endpoints used by this interface is 0 (excluding endpoint zero)   */
	USB_AUDIO_CLASS,          /* The interface implements the Audio Interface class   */
	USB_SUBCLASS_AUDIOSTREAM, /* The interface implements the AUDIOSTREAMING Subclass   */
	0x00U,                    /* The interface doesn't use any class-specific protocols   */
	USBD_IDX_AUDIO_STR,	  /* The device doesn't have a string descriptor describing this iInterface  */

	/* Audio Class Specific INTERFACE Descriptor, alternative interface 1 */
	USB_DESCRIPTOR_LENGTH_INTERFACE,  /* Descriptor size is 9 bytes  */
	USB_DESCRIPTOR_TYPE_INTERFACE,    /* INTERFACE Descriptor Type  */
	USB_AUDIO_IN_STREAM_INTERFACE_INDEX, /*The number of this interface is 2.  */
	0x01U,                            /* The value used to select the alternate setting for this interface is 1  */
	0x01U,                    /* The number of endpoints used by this interface is 1 (excluding endpoint zero)    */
	USB_AUDIO_CLASS,          /* The interface implements the Audio Interface class   */
	USB_SUBCLASS_AUDIOSTREAM, /* The interface implements the AUDIOSTREAMING Subclass   */
	0x00U,                    /* The interface doesn't use any class-specific protocols  */
	USBD_IDX_AUDIO_STR,    /* The device doesn't have a string descriptor describing this iInterface  */

	/* Audio Class Specific CS INTERFACE Descriptor*/
	USB_AUDIO_STREAMING_IFACE_DESC_SIZE,            /* Size of the descriptor, in bytes  */
	USB_DESCRIPTOR_TYPE_AUDIO_CS_INTERFACE,         /* CS_INTERFACE Descriptor Type  */
	USB_DESCRIPTOR_SUBTYPE_AUDIO_STREAMING_GENERAL, /* AS_GENERAL descriptor subtype  */
	USB_AUDIO_RECORDER_CONTROL_OUTPUT_TERMINAL_ID,    /* The Terminal ID of the Terminal to which the endpoint of this
                                                      interface is connected. */
	0x00U, /* Delay introduced by the data path. Expressed in number of frames.  */
	0x01U,
	0x00U, /* PCM  */

	/* Audio Class Specific type I format INTERFACE Descriptor */
	USB_AUDIO_STREAMING_TYPE_I_DESC_SIZE,               /* bLength (11) */
	USB_DESCRIPTOR_TYPE_AUDIO_CS_INTERFACE,             /* bDescriptorType (CS_INTERFACE) */
	USB_DESCRIPTOR_SUBTYPE_AUDIO_STREAMING_FORMAT_TYPE, /* DescriptorSubtype: AUDIO STREAMING FORMAT TYPE */
	USB_AUDIO_FORMAT_TYPE_I,                            /* Format Type: Type I */
	AUDIO_IN_FORMAT_CHANNELS,                           /* Number of Channels: one channel */
	AUDIO_FORMAT_SIZE,                                  /* SubFrame Size: one byte per audio subframe */
	AUDIO_FORMAT_BITS,                                  /* Bit Resolution: 8 bits per sample */
	0x01U,                                              /* One frequency supported */
	TSAMFREQ2BYTES(44100),

	/* ENDPOINT Descriptor */
	USB_ENDPOINT_AUDIO_DESCRIPTOR_LENGTH, /* Descriptor size is 9 bytes  */
	USB_DESCRIPTOR_TYPE_ENDPOINT,         /* Descriptor type (endpoint descriptor) */
	USB_AUDIO_RECORDER_STREAM_ENDPOINT |
		(USB_IN << USB_DESCRIPTOR_ENDPOINT_ADDRESS_DIRECTION_SHIFT), /* OUT endpoint address 1 */
	USB_ENDPOINT_ISOCHRONOUS | 0x04,                                  /* Isochronous endpoint */
	USB_SHORT_GET_LOW(FS_ISO_IN_ENDP_PACKET_SIZE + AUDIO_IN_FORMAT_CHANNELS * AUDIO_FORMAT_SIZE),
	USB_SHORT_GET_HIGH(FS_ISO_IN_ENDP_PACKET_SIZE + AUDIO_IN_FORMAT_CHANNELS * AUDIO_FORMAT_SIZE), /* 16 bytes  */
	FS_ISO_IN_ENDP_INTERVAL, /* bInterval(0x01U): x ms */
	0x00U,                    /* Unused */
	0x00U,					/* bSynchAddress */

	/* Audio Class Specific ENDPOINT Descriptor  */
	USB_AUDIO_STREAMING_ENDP_DESC_SIZE,      /*  Size of the descriptor, in bytes  */
	USB_AUDIO_STREAM_ENDPOINT_DESCRIPTOR,    /* CS_ENDPOINT Descriptor Type  */
	USB_AUDIO_EP_GENERAL_DESCRIPTOR_SUBTYPE, /* AUDIO_EP_GENERAL descriptor subtype  */
	0x01U,                                   /* Bit 0: Sampling Frequency 0
                                               Bit 1: Pitch 0
                                               Bit 7: MaxPacketsOnly 0   */
	0x00U,                                   /* Indicates the units used for the wLockDelay field: 0: Undefined  */
	0x00U,
	0x00U, /* Indicates the time it takes this endpoint to reliably lock its internal clock recovery circuitry */

	USB_DESCRIPTOR_LENGTH_INTERFACE_ASSOSIATION,
	USB_DESCRIPTOR_TYPE_INTERFACE_ASSOCIATION,
	USB_MIDI_CONTROL_INTERFACE_INDEX,		/* bFirstInterface */
	0x02,									/* bInterfaceCount */	// Audio Control, MIDI Streaming
	0x01,									/* bFunctionClass */
	0x00,									/* bFuctionSubClass */
	0x00,									/* bFunctionProtocol */
	0x00,									/* iFunction */

	/// @note MIDI Streaming Class
	/* MIDI Class Specific INTERFACE Descriptor, alternative interface 0  */
	USB_DESCRIPTOR_LENGTH_INTERFACE,  /* Descriptor size is 9 bytes  */
	USB_DESCRIPTOR_TYPE_INTERFACE,    /* INTERFACE Descriptor Type   */
	USB_MIDI_CONTROL_INTERFACE_INDEX, /* The number of this interface is 1.  */
	0x00U,                            /* The value used to select the alternate setting for this interface is 0   */
	0x00U,                    /* The number of endpoints used by this interface is 0 (excluding endpoint zero)   */
	USB_AUDIO_CLASS,          /* The interface implements the Audio Interface class   */
	USB_SUBCLASS_AUDIOCONTROL,  /*The interface implements the AUDIOCONTROL Subclass  */
	0x00U,                    /* The interface doesn't use any class-specific protocols   */
	0x00U,                    /* The device doesn't have a string descriptor describing this iInterface  */

	/* MIDI Class Specific type of INTERFACE Descriptor */
	USB_MIDI_CONTROL_INTERFACE_HEADER_LENGTH,   /* Size of the descriptor, in bytes  */
	USB_DESCRIPTOR_TYPE_AUDIO_CS_INTERFACE,      /* CS_INTERFACE Descriptor Type   */
	USB_DESCRIPTOR_SUBTYPE_AUDIO_CONTROL_HEADER, /* HEADER descriptor subtype   */
	0x00U, 0x01U, /* Audio Device compliant to the USB MIDI specification version 1.00  */
	USB_MIDI_CONTROL_INTERFACE_HEADER_LENGTH, 0x00U,
	0x01U,									/* bInCollection */
	USB_MIDI_STREAM_INTERFACE_INDEX,		/* baInterfaceNr(1) */

	/* Audio Class Specific INTERFACE Descriptor, alternative interface 0  */
	USB_DESCRIPTOR_LENGTH_INTERFACE,  /* Descriptor size is 9 bytes  */
	USB_DESCRIPTOR_TYPE_INTERFACE,    /* INTERFACE Descriptor Type   */
	USB_MIDI_STREAM_INTERFACE_INDEX, /* The number of this interface is 1.  */
	0x00U,                            /* The value used to select the alternate setting for this interface is 0   */
	0x02U,                    /* The number of endpoints used by this interface is 0 (excluding endpoint zero)   */
	USB_AUDIO_CLASS,          /* The interface implements the Audio Interface class   */
	USB_SUBCLASS_MIDISTREAM,  /* The interface implements the AUDIOSTREAMING Subclass   */
	0x00U,                    /* The interface doesn't use any class-specific protocols   */
	0x00U,                    /* The device doesn't have a string descriptor describing this iInterface  */

	/* ---------- Class-Specific MS Interface Header Descriptor (midi 1.0 section 6.1.2) ---------- */
	SIZE_OF_CS_MS_IF_HDR_DESC,					/* bLength							*/
	USB_DESCRIPTOR_TYPE_AUDIO_CS_INTERFACE,		/* bDescriptorType (CS_INTERFACE)	*/
	TYPE_MS_HEADER,								/* bDescriptorSubtype (MS_HEADER)	*/
	0x00U, 0x01U, /* Audio Device compliant to the USB MIDI specification version 1.00  */
	SIZE_OF_MS_HEADER_CONTENT, 0x00U,

	/* ---------- MIDI IN (host to device, EMBEDDED) Jack Descriptor (midi 1.0 section 6.1.2.2) ---------- */
	SIZE_OF_MIDI_IN_JACK_DESC,				/* bLength 							*/
	USB_DESCRIPTOR_TYPE_AUDIO_CS_INTERFACE,		/* bDescriptorType */
	TYPE_MIDI_IN_JACK,						/* bDescriptorSubtype */
	TYPE_MS_EMBEDDED,						/* bJackType	*/
	0x10,									/* bJackID */
	USBD_IDX_MIDI_IN_STR,                   /* iJack	*/

	/* ---------- MIDI OUT (device to host, External) Jack Descriptor (midi 1.0 section 6.1.2.3) ---------- */
	SIZE_OF_MIDI_OUT_JACK_DESC,				/* bLength */
	USB_DESCRIPTOR_TYPE_AUDIO_CS_INTERFACE,		/* bDescriptorType */
	TYPE_MIDI_OUT_JACK,						/* bDescriptorSubType */
	TYPE_MS_EXTERNAL,						/* bJackType */
	0x40,									/* bJackID */
	0x01,									/* bNrInputPins */
	0x10,									/* baSourceID(1): ID of Entity to which the first Input Pin of this MIDI OUT Jack is connected */
	0x01,									/* baSourcePin(1): Output pin number of the Entity to which the first Input Pin of this MIDI OUT Jack is connected */
	0x00,									/* iJack */

	/* ---------- MIDI OUT (device to host, EMBEDDED) Jack Descriptor (midi 1.0 section 6.1.2.3) ---------- */
	SIZE_OF_MIDI_OUT_JACK_DESC,				/* bLength */
	USB_DESCRIPTOR_TYPE_AUDIO_CS_INTERFACE,		/* bDescriptorType */
	TYPE_MIDI_OUT_JACK,						/* bDescriptorSubType */
	TYPE_MS_EMBEDDED,						/* bJackType */
	0x30,									/* bJackID */
	0x01,									/* bNrInputPins */
	0x20,									/* baSourceID(1): ID of Entity to which the first Input Pin of this MIDI OUT Jack is connected */
	0x01,									/* baSourcePin(1): Output pin number of the Entity to which the first Input Pin of this MIDI OUT Jack is connected */
	USBD_IDX_MIDI_OUT_STR,					/* iJack */

	/* ---------- MIDI IN (host to device, External) Jack Descriptor (midi 1.0 section 6.1.2.2) ---------- */
	SIZE_OF_MIDI_IN_JACK_DESC,				/* bLength 							*/
	USB_DESCRIPTOR_TYPE_AUDIO_CS_INTERFACE,		/* bDescriptorType */
	TYPE_MIDI_IN_JACK,						/* bDescriptorSubtype */
	TYPE_MS_EXTERNAL,						/* bJackType	*/
	0x20,									/* bJackID */
	0x00,									/* iJack	*/

	/* ---------- Standard MS Bulk Data Endpoint Descriptor (midi 1.0 section 6.2.1) ---------- */
	SIZE_OF_STD_MS_BD_EP_DESC,				/* bLength */
	USB_DESCRIPTOR_TYPE_ENDPOINT,			/* Descriptor type: Endpopint descriptor */
	USB_MIDI_STREAM_RX_ENDPOINT,							/* bEndpointAddress (bulk Out endpoint 4) */
	0x02,									/* bmAttribute: Bulk Transfer */
	(USB_MIDI_ENDP_MAX_PACKET_SIZE & 0xFF),
	(USB_MIDI_ENDP_MAX_PACKET_SIZE >> 8),		/* wMaxPacketSize */
	0x00,									/* bInterval: must be 0 */
	0x00,									/* bRefresh: must be 0 */
	0x00,									/* bSyncAddress: must be 0 */

	/* ---------- Class-Specific MS Bulk Data Endpoint Descriptor (midi 1.0 section 6.2.2) ---------- */
	SIZE_OF_CS_MS_BD_EP_DESC,				/* bLength */
	USB_AUDIO_STREAM_ENDPOINT_DESCRIPTOR,			/* bDescriptorType (CS_ENDPOINT) */
	TYPE_MS_GENERAL,						/* bDescriptorSubType (MS_GENERAL) */
	0x01,									/* bNumEmbMIDIJack	*/
	0x10,									/* baAssocJackID(1): to Embedded MIDI In Jack	*/

	/* ---------- Standard MS Bulk Data Endpoint Descriptor (midi 1.0 section 6.2.1) ---------- */
	SIZE_OF_STD_MS_BD_EP_DESC,				/* Descriptor size */
	USB_DESCRIPTOR_TYPE_ENDPOINT,					/* Descriptor type: Endpopint descriptor */
	USB_MIDI_STREAM_TX_ENDPOINT |
		(USB_IN << USB_DESCRIPTOR_ENDPOINT_ADDRESS_DIRECTION_SHIFT),	/* bEndpointAddress (Bulk In endpoint 5) */
	0x02,									/* bmAttribute: Bulk Transfer */
	(USB_MIDI_ENDP_MAX_PACKET_SIZE & 0xFF),
	(USB_MIDI_ENDP_MAX_PACKET_SIZE >> 8),		/* wMaxPacketSize */
	0x00,									/* bInterval: must be 0	*/
	0x00,									/* bRefresh: must be 0	*/
	0x00,									/* bSyncAddress: must be 0 */

	/* ---------- Class-Specific MS Bulk Data Endpoint Descriptor (midi 1.0 section 6.2.2) ---------- */
	SIZE_OF_CS_MS_BD_EP_DESC,				/* bLength */
	USB_AUDIO_STREAM_ENDPOINT_DESCRIPTOR,			/* bDescriptorType (CS_ENDPOINT) */
	TYPE_MS_GENERAL,						/* bDescriptorSubType (MS_GENERAL) */
	0x01,									/* bNumEmbMIDIJack	*/
	0x30									/* baAssocJackID(1): to Embedded MIDI OUT Jack	*/
};
#endif	// DESCRIPTOR_X19850

USB_DMA_INIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE)
/*--------- USB Device Qualifier Descriptor -----------*/
uint8_t g_UsbDeviceQualifierDescriptor[] = {
	USB_DESCRIPTOR_LENGTH_DEVICE_QUALITIER,
	USB_DESCRIPTOR_TYPE_DEVICE_QUALITIER,
    USB_SHORT_GET_LOW(USB_DEVICE_SPECIFIC_BCD_VERSION),
    USB_SHORT_GET_HIGH(USB_DEVICE_SPECIFIC_BCD_VERSION), /* USB Specification Release Number in
                                                            Binary-Coded Decimal (i.e., 2.10 is 210H). */
	0x00,
	0x00,
	0x00,
	USB_CONTROL_MAX_PACKET_SIZE,	// max packet size(endpoint 0) for other speed configuration (high speed)
	0x01,	// number of other speed configurations
	0x00	// reserved, must be 0
};

USB_DMA_INIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE)
/*--------- USB Other Speed Configuration  Descriptor -----------*/
uint8_t g_UsbDeviceOtherSpeedConfigurationDescriptor[] = {
	USB_DESCRIPTOR_LENGTH_OTHER_SPEED_CONFIGURATION,
	USB_DESCRIPTOR_TYPE_OTHER_SPEED_CONFIGURATION,
	USB_SHORT_GET_LOW(
		SIZE_OF_TOTAL_CONFIG_DESC
	),
	USB_SHORT_GET_HIGH(
		SIZE_OF_TOTAL_CONFIG_DESC
	),	/* Total length of data returned for this configuration. */
	USB_AUDIO_PLAYER_INTERFACE_COUNT,
	USB_AUDIO_MIDI_CONFIGURE_INDEX,           /* Value to use as an argument to the
                                                    SetConfiguration() request to select this configuration */
    0x00U,                                     /* Index of string descriptor describing this configuration */
    (USB_DESCRIPTOR_CONFIGURE_ATTRIBUTE_D7_MASK) |
        (USB_DEVICE_CONFIG_SELF_POWER << USB_DESCRIPTOR_CONFIGURE_ATTRIBUTE_SELF_POWERED_SHIFT) |
        (USB_DEVICE_CONFIG_REMOTE_WAKEUP << USB_DESCRIPTOR_CONFIGURE_ATTRIBUTE_REMOTE_WAKEUP_SHIFT),
    /* Configuration characteristics
       D7: Reserved (set to one)
       D6: Self-powered
       D5: Remote Wakeup
       D4...0: Reserved (reset to zero)
    */
    USB_DEVICE_MAX_POWER, /* Maximum power consumption of the USB
                           * device from the bus in this specific
                           * configuration when the device is fully
                           * operational. Expressed in 2 mA units
                           *  (i.e., 50 = 100 mA).
                           */
};


/* Define string descriptor */
USB_DMA_INIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE)
uint8_t g_UsbDeviceString0[] = {
    2U + 2U, USB_DESCRIPTOR_TYPE_STRING, 0x09U, 0x04U,
};
#if DESCRIPTOR_X19850
/* KORG INC. */
USB_DMA_INIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE)
uint8_t g_UsbDeviceString1[] = {
    2U + 2U * 9U, USB_DESCRIPTOR_TYPE_STRING,
    'K',           0x00U,
    'O',           0x00U,
    'R',           0x00U,
    'G',           0x00U,
    ' ',           0x00U,
    'I',           0x00U,
    'N',           0x00U,
    'C',           0x00U,
    '.',           0x00U,
};

/* KORG Replikator */
USB_DMA_INIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE)
uint8_t g_UsbDeviceString2[] = {
    2U + 2U * 15U, USB_DESCRIPTOR_TYPE_STRING,
    'K',           0x00U,
    'O',           0x00U,
    'R',           0x00U,
    'G',           0x00U,
    ' ',           0x00U,
    'R',           0x00U,
    'e',           0x00U,
    'p',           0x00U,
    'l',           0x00U,
    'i',           0x00U,
    'k',           0x00U,
    'a',           0x00U,
    't',           0x00U,
    'o',           0x00U,
    'r',           0x00U,
};

/* KORG Replikator Audio Device */
USB_DMA_INIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE)
uint8_t g_UsbDeviceString3[] = {
    2U + 2U * 28U, USB_DESCRIPTOR_TYPE_STRING,
    'K',           0x00U,
    'O',           0x00U,
    'R',           0x00U,
    'G',           0x00U,
    ' ',           0x00U,
    'R',           0x00U,
    'e',           0x00U,
    'p',           0x00U,
    'l',           0x00U,
    'i',           0x00U,
    'k',           0x00U,
    'a',           0x00U,
    't',           0x00U,
    'o',           0x00U,
    'r',           0x00U,
    ' ',           0x00U,
    'A',           0x00U,
    'u',           0x00U,
    'd',           0x00U,
    'i',           0x00U,
    'o',           0x00U,
    ' ',           0x00U,
    'D',           0x00U,
    'e',           0x00U,
    'v',           0x00U,
    'i',           0x00U,
    'c',           0x00U,
	'e',           0x00U,
};

/* KORG_REPLICATOR _ CTRL OUT */
USB_DMA_INIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE)
uint8_t g_UsbDeviceString4[] = {
    2U + 2U * 26U, USB_DESCRIPTOR_TYPE_STRING,
    'K',           0x00U,
    'O',           0x00U,
    'R',           0x00U,
    'G',           0x00U,
    '_',           0x00U,
    'R',           0x00U,
    'E',           0x00U,
    'P',           0x00U,
    'L',           0x00U,
    'I',           0x00U,
    'K',           0x00U,
    'A',           0x00U,
    'T',           0x00U,
    'O',           0x00U,
    'R',           0x00U,
    ' ',           0x00U,
    '_',           0x00U,
    ' ',           0x00U,
    'C',           0x00U,
    'T',           0x00U,
    'R',           0x00U,
    'L',           0x00U,
    ' ',           0x00U,
    'O',           0x00U,
    'U',           0x00U,
    'T',           0x00U,
};

/* KORG_REPLICATOR _ CTRL IN */
USB_DMA_INIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE)
uint8_t g_UsbDeviceString5[] = {
    2U + 2U * 25U, USB_DESCRIPTOR_TYPE_STRING,
    'K',           0x00U,
    'O',           0x00U,
    'R',           0x00U,
    'G',           0x00U,
    '_',           0x00U,
    'R',           0x00U,
    'E',           0x00U,
    'P',           0x00U,
    'L',           0x00U,
    'I',           0x00U,
    'K',           0x00U,
    'A',           0x00U,
    'T',           0x00U,
    'O',           0x00U,
    'R',           0x00U,
    ' ',           0x00U,
    '_',           0x00U,
    ' ',           0x00U,
    'C',           0x00U,
    'T',           0x00U,
    'R',           0x00U,
    'L',           0x00U,
    ' ',           0x00U,
    'I',           0x00U,
    'N',           0x00U,
};
#endif	// DESCRIPTOR_X19850
#if DESCRIPTOR_X19730
/* KORG INC. */
USB_DMA_INIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE)
uint8_t g_UsbDeviceString1[] = {
    2U + 2U * 9U, USB_DESCRIPTOR_TYPE_STRING,
    'K',           0x00U,
    'O',           0x00U,
    'R',           0x00U,
    'G',           0x00U,
    ' ',           0x00U,
    'I',           0x00U,
    'N',           0x00U,
    'C',           0x00U,
    '.',           0x00U,
};

/* KORG Replikator */
USB_DMA_INIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE)
uint8_t g_UsbDeviceString2[] = {
    2U + 2U * 15U, USB_DESCRIPTOR_TYPE_STRING,
    'K',           0x00U,
    'O',           0x00U,
    'R',           0x00U,
    'G',           0x00U,
    ' ',           0x00U,
    'R',           0x00U,
    'e',           0x00U,
    'p',           0x00U,
    'l',           0x00U,
    'i',           0x00U,
    'k',           0x00U,
    'a',           0x00U,
    't',           0x00U,
    'o',           0x00U,
    'r',           0x00U,
};

/* KORG Replikator Audio Device */
USB_DMA_INIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE)
uint8_t g_UsbDeviceString3[] = {
    2U + 2U * 28U, USB_DESCRIPTOR_TYPE_STRING,
    'K',           0x00U,
    'O',           0x00U,
    'R',           0x00U,
    'G',           0x00U,
    ' ',           0x00U,
    'R',           0x00U,
    'e',           0x00U,
    'p',           0x00U,
    'l',           0x00U,
    'i',           0x00U,
    'k',           0x00U,
    'a',           0x00U,
    't',           0x00U,
    'o',           0x00U,
    'r',           0x00U,
    ' ',           0x00U,
    'A',           0x00U,
    'u',           0x00U,
    'd',           0x00U,
    'i',           0x00U,
    'o',           0x00U,
    ' ',           0x00U,
    'D',           0x00U,
    'e',           0x00U,
    'v',           0x00U,
    'i',           0x00U,
    'c',           0x00U,
	'e',           0x00U,
};

/* KORG_REPLICATOR _ CTRL OUT */
USB_DMA_INIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE)
uint8_t g_UsbDeviceString4[] = {
    2U + 2U * 26U, USB_DESCRIPTOR_TYPE_STRING,
    'K',           0x00U,
    'O',           0x00U,
    'R',           0x00U,
    'G',           0x00U,
    '_',           0x00U,
    'R',           0x00U,
    'E',           0x00U,
    'P',           0x00U,
    'L',           0x00U,
    'I',           0x00U,
    'K',           0x00U,
    'A',           0x00U,
    'T',           0x00U,
    'O',           0x00U,
    'R',           0x00U,
    ' ',           0x00U,
    '_',           0x00U,
    ' ',           0x00U,
    'C',           0x00U,
    'T',           0x00U,
    'R',           0x00U,
    'L',           0x00U,
    ' ',           0x00U,
    'O',           0x00U,
    'U',           0x00U,
    'T',           0x00U,
};

/* KORG_REPLICATOR _ CTRL IN */
USB_DMA_INIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE)
uint8_t g_UsbDeviceString5[] = {
    2U + 2U * 25U, USB_DESCRIPTOR_TYPE_STRING,
    'K',           0x00U,
    'O',           0x00U,
    'R',           0x00U,
    'G',           0x00U,
    '_',           0x00U,
    'R',           0x00U,
    'E',           0x00U,
    'P',           0x00U,
    'L',           0x00U,
    'I',           0x00U,
    'K',           0x00U,
    'A',           0x00U,
    'T',           0x00U,
    'O',           0x00U,
    'R',           0x00U,
    ' ',           0x00U,
    '_',           0x00U,
    ' ',           0x00U,
    'C',           0x00U,
    'T',           0x00U,
    'R',           0x00U,
    'L',           0x00U,
    ' ',           0x00U,
    'I',           0x00U,
    'N',           0x00U,
};
#endif	// DESCRIPTOR_X19850

uint32_t g_UsbDeviceStringDescriptorLength[USB_DEVICE_STRING_COUNT] = {
    sizeof(g_UsbDeviceString0), sizeof(g_UsbDeviceString1), sizeof(g_UsbDeviceString2),
	sizeof(g_UsbDeviceString3), sizeof(g_UsbDeviceString4), sizeof(g_UsbDeviceString5)
};

uint8_t *g_UsbDeviceStringDescriptorArray[USB_DEVICE_STRING_COUNT] = {
    g_UsbDeviceString0, g_UsbDeviceString1, g_UsbDeviceString2,
    g_UsbDeviceString3, g_UsbDeviceString4, g_UsbDeviceString5,
};

usb_language_t g_UsbDeviceLanguage[USB_DEVICE_LANGUAGE_COUNT] = {{
    g_UsbDeviceStringDescriptorArray, g_UsbDeviceStringDescriptorLength, (uint16_t)0x0409U,
}};

usb_language_list_t g_UsbDeviceLanguageList = {
    g_UsbDeviceString0, sizeof(g_UsbDeviceString0), g_UsbDeviceLanguage, USB_DEVICE_LANGUAGE_COUNT,
};

/*******************************************************************************
* Code
******************************************************************************/
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
                                           usb_device_get_device_descriptor_struct_t *deviceDescriptor)
{
    deviceDescriptor->buffer = g_UsbDeviceDescriptor;
    deviceDescriptor->length = USB_DESCRIPTOR_LENGTH_DEVICE;
    return kStatus_USB_Success;
}

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
    usb_device_handle handle, usb_device_get_configuration_descriptor_struct_t *configurationDescriptor)
{
    if (USB_AUDIO_MIDI_CONFIGURE_INDEX > configurationDescriptor->configuration)
    {
        configurationDescriptor->buffer = g_UsbDeviceConfigurationDescriptor;
        configurationDescriptor->length = USB_DESCRIPTOR_LENGTH_CONFIGURATION_ALL;
        return kStatus_USB_Success;
    }
    return kStatus_USB_InvalidRequest;
}

/*!
 * @brief USB device get string descriptor function.
 *
 * This function gets the string descriptor of the USB device.
 *
 * @param handle The USB device handle.
 * @param stringDescriptor The pointer to the string descriptor structure.
 *
 * @return A USB error code or kStatus_USB_Success.
 */
usb_status_t USB_DeviceGetStringDescriptor(usb_device_handle handle,
                                           usb_device_get_string_descriptor_struct_t *stringDescriptor)
{
    if (stringDescriptor->stringIndex == 0U)
    {
        stringDescriptor->buffer = (uint8_t *)g_UsbDeviceLanguageList.languageString;
        stringDescriptor->length = g_UsbDeviceLanguageList.stringLength;
    }
    else
    {
        uint8_t languageId = 0U;
        uint8_t languageIndex = USB_DEVICE_STRING_COUNT;

        for (; languageId < USB_DEVICE_LANGUAGE_COUNT; languageId++)
        {
            if (stringDescriptor->languageId == g_UsbDeviceLanguageList.languageList[languageId].languageId)
            {
                if (stringDescriptor->stringIndex < USB_DEVICE_STRING_COUNT)
                {
                    languageIndex = stringDescriptor->stringIndex;
                }
                break;
            }
        }

        if (USB_DEVICE_STRING_COUNT == languageIndex)
        {
            return kStatus_USB_InvalidRequest;
        }
        stringDescriptor->buffer = (uint8_t *)g_UsbDeviceLanguageList.languageList[languageId].string[languageIndex];
        stringDescriptor->length = g_UsbDeviceLanguageList.languageList[languageId].length[languageIndex];
    }
    return kStatus_USB_Success;
}

/* Due to the difference of HS and FS descriptors, the device descriptors and configurations need to be updated to match
 * current speed.
 * As the default, the device descriptors and configurations are configured by using FS parameters for both EHCI and
 * KHCI.
 * When the EHCI is enabled, the application needs to call this fucntion to update device by using current speed.
 * The updated information includes endpoint max packet size, endpoint interval, etc. */
usb_status_t USB_DeviceSetSpeed(usb_device_handle handle, uint8_t speed)
{
#if SUPPORT_USB_HIGH_SPEED
	/* bInterval */
	g_UsbDeviceConfigurationDescriptor[OFFSET_EP1_OUT_BINTERVAL] = (USB_SPEED_HIGH == speed) ? HS_ISO_OUT_ENDP_INTERVAL : FS_ISO_OUT_ENDP_INTERVAL;
	g_UsbDeviceConfigurationDescriptor[OFFSET_EP1_IN_BINTERVAL] = (USB_SPEED_HIGH == speed) ? HS_ISO_IN_ENDP_INTERVAL : FS_ISO_IN_ENDP_INTERVAL;
	g_UsbDeviceConfigurationDescriptor[OFFSET_EP3_IN_BINTERVAL] = (USB_SPEED_HIGH == speed) ? HS_ISO_IN_ENDP_INTERVAL : FS_ISO_IN_ENDP_INTERVAL;

	/* MAX Packet Size */
	if (USB_SPEED_HIGH == speed) {
		g_UsbDeviceConfigurationDescriptor[OFFSET_EP2_OUT_MAX_PACKET_SIZE] = (HS_USB_MIDI_ENDP_MAX_PACKET_SIZE & 0xFF);
		g_UsbDeviceConfigurationDescriptor[OFFSET_EP2_OUT_MAX_PACKET_SIZE+1] = (HS_USB_MIDI_ENDP_MAX_PACKET_SIZE >> 8);
		g_UsbDeviceConfigurationDescriptor[OFFSET_EP2_IN_MAX_PACKET_SIZE] = (HS_USB_MIDI_ENDP_MAX_PACKET_SIZE & 0xFF);
		g_UsbDeviceConfigurationDescriptor[OFFSET_EP2_IN_MAX_PACKET_SIZE+1] = (HS_USB_MIDI_ENDP_MAX_PACKET_SIZE >> 8);
	}
	else {
		g_UsbDeviceConfigurationDescriptor[OFFSET_EP2_OUT_MAX_PACKET_SIZE] = (USB_MIDI_ENDP_MAX_PACKET_SIZE & 0xFF);
		g_UsbDeviceConfigurationDescriptor[OFFSET_EP2_OUT_MAX_PACKET_SIZE+1] = (USB_MIDI_ENDP_MAX_PACKET_SIZE >> 8);
		g_UsbDeviceConfigurationDescriptor[OFFSET_EP2_IN_MAX_PACKET_SIZE] = (USB_MIDI_ENDP_MAX_PACKET_SIZE & 0xFF);
		g_UsbDeviceConfigurationDescriptor[OFFSET_EP2_IN_MAX_PACKET_SIZE+1] = (USB_MIDI_ENDP_MAX_PACKET_SIZE >> 8);
	}
#else
	/// @note not support full speed
#endif
#if 0	/// @note sample source
    usb_descriptor_union_t *descriptorHead;
    usb_descriptor_union_t *descriptorTail;

    descriptorHead = (usb_descriptor_union_t *)&g_UsbDeviceConfigurationDescriptor[0];
    descriptorTail =
        (usb_descriptor_union_t *)(&g_UsbDeviceConfigurationDescriptor[USB_DESCRIPTOR_LENGTH_CONFIGURATION_ALL - 1U]);

    while (descriptorHead < descriptorTail)
    {
        if (descriptorHead->common.bDescriptorType == USB_DESCRIPTOR_TYPE_ENDPOINT)
        {
            if (USB_SPEED_HIGH == speed)
            {
                if ((USB_AUDIO_SPEAKER_STREAM_ENDPOINT ==
                     (descriptorHead->endpoint.bEndpointAddress & USB_ENDPOINT_NUMBER_MASK)) &&
                    ((descriptorHead->endpoint.bEndpointAddress >> USB_DESCRIPTOR_ENDPOINT_ADDRESS_DIRECTION_SHIFT) ==
                     USB_OUT))
                {
                    descriptorHead->endpoint.bInterval = HS_ISO_OUT_ENDP_INTERVAL;
                    USB_SHORT_TO_LITTLE_ENDIAN_ADDRESS(
                        (HS_ISO_OUT_ENDP_PACKET_SIZE + AUDIO_FORMAT_CHANNELS * AUDIO_FORMAT_SIZE),
                        descriptorHead->endpoint.wMaxPacketSize);
                }
                else if ((USB_AUDIO_SPEAKER_STREAM_ENDPOINT ==
                          (descriptorHead->endpoint.bEndpointAddress & USB_ENDPOINT_NUMBER_MASK)) &&
                         ((descriptorHead->endpoint.bEndpointAddress >>
                           USB_DESCRIPTOR_ENDPOINT_ADDRESS_DIRECTION_SHIFT) == USB_IN))
                {
                    USB_SHORT_TO_LITTLE_ENDIAN_ADDRESS(HS_ISO_FEEDBACK_ENDP_PACKET_SIZE,
                                                       descriptorHead->endpoint.wMaxPacketSize);
                }
                else
                {
                }
            }
            else
            {
                if ((USB_AUDIO_SPEAKER_STREAM_ENDPOINT ==
                     (descriptorHead->endpoint.bEndpointAddress & USB_ENDPOINT_NUMBER_MASK)) &&
                    ((descriptorHead->endpoint.bEndpointAddress >> USB_DESCRIPTOR_ENDPOINT_ADDRESS_DIRECTION_SHIFT) ==
                     USB_OUT))
                {
                    descriptorHead->endpoint.bInterval = FS_ISO_OUT_ENDP_INTERVAL;
                    USB_SHORT_TO_LITTLE_ENDIAN_ADDRESS(
                        (FS_ISO_OUT_ENDP_PACKET_SIZE + AUDIO_FORMAT_CHANNELS * AUDIO_FORMAT_SIZE),
                        descriptorHead->endpoint.wMaxPacketSize);
                }
                else if ((USB_AUDIO_SPEAKER_STREAM_ENDPOINT ==
                          (descriptorHead->endpoint.bEndpointAddress & USB_ENDPOINT_NUMBER_MASK)) &&
                         ((descriptorHead->endpoint.bEndpointAddress >>
                           USB_DESCRIPTOR_ENDPOINT_ADDRESS_DIRECTION_SHIFT) == USB_IN))
                {
                    USB_SHORT_TO_LITTLE_ENDIAN_ADDRESS(FS_ISO_FEEDBACK_ENDP_PACKET_SIZE,
                                                       descriptorHead->endpoint.wMaxPacketSize);
                }
                else
                {
                }
            }
        }
        descriptorHead = (usb_descriptor_union_t *)((uint8_t *)descriptorHead + descriptorHead->common.bLength);
    }

    if (USB_SPEED_HIGH == speed)
    {
        g_UsbDeviceAudioSpeakerEndpoints[0].maxPacketSize =
            (HS_ISO_OUT_ENDP_PACKET_SIZE + AUDIO_FORMAT_CHANNELS * AUDIO_FORMAT_SIZE);
        g_UsbDeviceAudioSpeakerEndpoints[1].maxPacketSize = HS_ISO_FEEDBACK_ENDP_PACKET_SIZE;
    }
    else
    {
        g_UsbDeviceAudioSpeakerEndpoints[0].maxPacketSize =
            (FS_ISO_OUT_ENDP_PACKET_SIZE + AUDIO_FORMAT_CHANNELS * AUDIO_FORMAT_SIZE);
        g_UsbDeviceAudioSpeakerEndpoints[1].maxPacketSize = FS_ISO_FEEDBACK_ENDP_PACKET_SIZE;
    }
#endif

    return kStatus_USB_Success;
}

/*--------- USB Device Qualifier Descriptor -----------*/
usb_status_t USB_DeviceGetDeviceQualifierDescriptor(
    usb_device_handle handle, usb_device_get_device_qualifier_descriptor_struct_t *deviceQualifierDescriptor)
{
	deviceQualifierDescriptor->buffer = g_UsbDeviceQualifierDescriptor;
	deviceQualifierDescriptor->length = USB_DESCRIPTOR_LENGTH_DEVICE_QUALITIER;
	return kStatus_USB_Success;
}

/*--------- USB Other Speed Configuration  Descriptor -----------*/
usb_status_t USB_DeviceGetOtherSpeedConfigurationDescriptor(
    usb_device_handle handle, usb_device_get_device_other_speed_configuration_descriptor_struct_t *deviceOtherSpeedConfigDescriptor)
{
	deviceOtherSpeedConfigDescriptor->buffer = g_UsbDeviceOtherSpeedConfigurationDescriptor;
	deviceOtherSpeedConfigDescriptor->length = USB_DESCRIPTOR_LENGTH_OTHER_SPEED_CONFIGURATION;
	return kStatus_USB_Success;
}

