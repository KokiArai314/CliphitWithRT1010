/*
 * usb_device_midi.h
 *
 *  Created on: 2020/02/06
 *      Author: higuchi
 */

#ifndef DEVICE_CLASS_MIDI_USB_DEVICE_MIDI_H_
#define DEVICE_CLASS_MIDI_USB_DEVICE_MIDI_H_

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/* usb_device_audio.hを参照 */

/*! @brief Audio device class code */
#define USB_MIDI_DEVICE_CONFIG_AUDIO_CLASS_CODE (0x01)

/*! @brief Audio device subclass code */
#define USB_DEVICE_MIDI_STREAM_SUBCLASS (0x03)

/*! @brief Available common EVENT types in USB MIDI Streaming class callback */
typedef enum
{
    kUSB_DeviceMidiEventStreamSendResponse = 0x01U, /*!< Send data completed in stream pipe */
    kUSB_DeviceMidiEventStreamRecvResponse         /*!< Data received in stream pipe */
} usb_device_midi_event_t;

/*! @brief The USB MIDI device structure */
typedef struct _usb_device_midi_struct
{
	usb_device_handle handle;						/*!< The device handle */
	usb_device_class_config_struct_t *configStruct;	/*!< The configuration of the class*/
	usb_device_interface_struct_t *interfaceHandle;	/*!< Current interface handle */

    uint8_t bulkInEndpoint;							/*!< Bulk in endpoint number*/
    uint8_t bulkOutEndpoint;						/*!< Bulk out endpoint number*/
	uint8_t alternate;								/*!< Current alternate setting of the interface */
	uint8_t configuration;							/*!< Current configuration */
	uint8_t interfaceNumber;						/*!< The interface number of the class */
} usb_device_midi_struct_t;

/*******************************************************************************
 * API
 ******************************************************************************/
/* usb_device_audio.hを参照 */
usb_status_t USB_DeviceMidiInit(uint8_t controllerId,
                                        usb_device_class_config_struct_t *config,
                                        class_handle_t *handle);

usb_status_t USB_DeviceMidiDeinit(class_handle_t handle);

usb_status_t USB_DeviceMidiEvent(void *handle, uint32_t event, void *param);

usb_status_t USB_DeviceMidiSend(class_handle_t handle, uint8_t *buffer, uint32_t length);

usb_status_t USB_DeviceMidiRecv(class_handle_t handle, uint8_t *buffer, uint32_t length);

#endif /* DEVICE_CLASS_MIDI_USB_DEVICE_MIDI_H_ */
