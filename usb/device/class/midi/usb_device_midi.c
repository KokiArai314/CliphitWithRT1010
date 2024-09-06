/*
 * usb_device_midi.c
 *
 *  Created on: 2020/02/06
 *      Author: higuchi
 */

#include "usb_device/usb_device_config.h"
#include "usb.h"
#include "usb_device.h"

#include "usb_device_class.h"

#include "usb_device/usb_device_descriptor.h"

#if ((defined(USB_DEVICE_CONFIG_MIDI)) && (USB_DEVICE_CONFIG_MIDI > 0U))
#include "usb_device_midi.h"

#include "board.h"	/// @note SUPPORT_USB_HIGH_SPEED
#if SUPPORT_USB_HIGH_SPEED
/* Switching of Max Packet Size */
#include "composite.h"
#include "usb_device/usb_device_descriptor.h"
extern usb_device_composite_struct_t g_composite;

#endif

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static usb_status_t USB_DeviceMidiAllocateHandle(usb_device_midi_struct_t **handle);
static usb_status_t USB_DeviceMidiFreeHandle(usb_device_midi_struct_t *handle);
usb_status_t USB_DeviceMidiBulkOut(usb_device_handle handle,
                                   usb_device_endpoint_callback_message_struct_t *message,
                                   void *callbackParam);
usb_status_t USB_DeviceMidiBulkIn(usb_device_handle handle,
                                  usb_device_endpoint_callback_message_struct_t *message,
                                  void *callbackParam);
usb_status_t USB_DeviceMidiStreamEndpointsInit(usb_device_midi_struct_t *midiHandle);
usb_status_t USB_DeviceMidiStreamEndpointsDeinit(usb_device_midi_struct_t *midiHandle);

/*******************************************************************************
 * Variables
 ******************************************************************************/

USB_GLOBAL USB_RAM_ADDRESS_ALIGNMENT(USB_DATA_ALIGN_SIZE) static usb_device_midi_struct_t
    s_UsbDeviceMidiHandle[USB_DEVICE_CONFIG_MIDI];

/*******************************************************************************
 * Code
 ******************************************************************************/
static usb_status_t USB_DeviceMidiAllocateHandle(usb_device_midi_struct_t **handle)
{
    int32_t count;
    USB_OSA_SR_ALLOC();

    USB_OSA_ENTER_CRITICAL();
    for (count = 0; count < USB_DEVICE_CONFIG_MIDI; count++)
    {
        if (NULL == s_UsbDeviceMidiHandle[count].handle)
        {
            *handle = &s_UsbDeviceMidiHandle[count];
            USB_OSA_EXIT_CRITICAL();
            return kStatus_USB_Success;
        }
    }
    USB_OSA_EXIT_CRITICAL();

    return kStatus_USB_Busy;
}

static usb_status_t USB_DeviceMidiFreeHandle(usb_device_midi_struct_t *handle)
{
    USB_OSA_SR_ALLOC();

    USB_OSA_ENTER_CRITICAL();
    handle->handle = NULL;
    handle->configStruct = (usb_device_class_config_struct_t *)NULL;
    handle->configuration = 0U;
    handle->alternate = 0;

    USB_OSA_EXIT_CRITICAL();

    return kStatus_USB_Success;
}

usb_status_t USB_DeviceMidiBulkOut(usb_device_handle handle,
                                           usb_device_endpoint_callback_message_struct_t *message,
                                           void *callbackParam)
{
    usb_device_midi_struct_t *midiHandle;
    usb_status_t error = kStatus_USB_Error;

    /* Get the midi class handle */
    midiHandle = (usb_device_midi_struct_t *)callbackParam;
    if (!midiHandle)
    {
        return kStatus_USB_InvalidHandle;
    }

    if ((NULL != midiHandle->configStruct) && (midiHandle->configStruct->classCallback))
    {
        /* classCallback is initialized in classInit of s_UsbDeviceClassInterfaceMap,
        it is from the second parameter of classInit */
        error = midiHandle->configStruct->classCallback((class_handle_t)midiHandle,
        		kUSB_DeviceMidiEventStreamRecvResponse, message);
    }

    return error;
}

usb_status_t USB_DeviceMidiBulkIn(usb_device_handle handle,
                                          usb_device_endpoint_callback_message_struct_t *message,
                                          void *callbackParam)
{
    usb_device_midi_struct_t *midiHandle;
    usb_status_t error = kStatus_USB_Error;

    /* Get the midi class handle */
    midiHandle = (usb_device_midi_struct_t *)callbackParam;
    if (!midiHandle)
    {
        return kStatus_USB_InvalidHandle;
    }
    if ((NULL != midiHandle->configStruct) && (midiHandle->configStruct->classCallback))
    {
        /* Notify the application stream data sent by calling the midi class callback.
        classCallback is initialized in classInit of s_UsbDeviceClassInterfaceMap,
        it is from the second parameter of classInit */
        error = midiHandle->configStruct->classCallback((class_handle_t)midiHandle,
        		kUSB_DeviceMidiEventStreamSendResponse, message);
    }

    return error;
}

usb_status_t USB_DeviceMidiStreamEndpointsInit(usb_device_midi_struct_t *midiHandle)
{
    usb_device_interface_list_t *interfaceList;
    usb_device_interface_struct_t *interface = (usb_device_interface_struct_t *)NULL;
    usb_status_t error = kStatus_USB_Error;

    /* Check the configuration is valid or not. */
    if (!midiHandle->configuration)
    {
        return error;
    }

    /* Check the configuration is valid or not. */
    if (midiHandle->configuration > midiHandle->configStruct->classInfomation->configurations)
    {
        return error;
    }

    if (NULL == midiHandle->configStruct->classInfomation->interfaceList)
    {
        return error;
    }

    /* Get the interface list of the new configuration. */
    interfaceList = &midiHandle->configStruct->classInfomation->interfaceList[midiHandle->configuration - 1];

    /* Find stream interface by using the alternate setting of the interface. */
    /// @note memo: interfaceListは1つ. interfaceList->countは2(=0,1)で、各interfaceにもcountがあるが, MIDI InterfaceはaltSetting分(0 = 1つ)
    {
    	const int count = USB_MIDI_OFFSET_ENDPOINT_INTERFACE_INDEX;	/// @note memo: MIDI Interface

        if ((USB_MIDI_DEVICE_CONFIG_AUDIO_CLASS_CODE == interfaceList->interfaces[count].classCode) &&
            (USB_DEVICE_MIDI_STREAM_SUBCLASS == interfaceList->interfaces[count].subclassCode))
        {
        	interface = &interfaceList->interfaces[count].interface[0];
        }
        midiHandle->interfaceNumber = interfaceList->interfaces[count].interfaceNumber;
    }
    if (!interface)
    {
        return error;
    }
    /* Keep new stream interface handle. */
    midiHandle->interfaceHandle = interface;

    /* Initialize the endpoints of the new interface. */
    /// @note memo: endpointList.count -> MIDIで必要なendpointの数(MIDI In/Out = 2つ)と考えていい.
    for (int count = 0U; count < interface->endpointList.count; count++)
    {
        usb_device_endpoint_init_struct_t epInitStruct;
        usb_device_endpoint_callback_struct_t epCallback;
        epInitStruct.zlt = 0U;
        epInitStruct.endpointAddress = interface->endpointList.endpoint[count].endpointAddress;
#if SUPPORT_USB_HIGH_SPEED
        /* Switching of Max Packet Size */
        if (g_composite.midiPlayer.speed == USB_SPEED_HIGH) {
        	/* SetConfiguration => reset Max Packet Size */
        	epInitStruct.maxPacketSize = HS_USB_MIDI_ENDP_MAX_PACKET_SIZE;
        }
        else {
        	epInitStruct.maxPacketSize = interface->endpointList.endpoint[count].maxPacketSize;
        }
#else
        epInitStruct.maxPacketSize = interface->endpointList.endpoint[count].maxPacketSize;
#endif
        epInitStruct.transferType = interface->endpointList.endpoint[count].transferType;

        if ((USB_ENDPOINT_BULK == (epInitStruct.transferType & USB_DESCRIPTOR_ENDPOINT_ATTRIBUTE_TYPE_MASK)) &&
            (USB_IN == ((epInitStruct.endpointAddress & USB_DESCRIPTOR_ENDPOINT_ADDRESS_DIRECTION_MASK) >>
                        USB_DESCRIPTOR_ENDPOINT_ADDRESS_DIRECTION_SHIFT)))
        {
        	midiHandle->bulkInEndpoint = epInitStruct.endpointAddress;
            epCallback.callbackFn = USB_DeviceMidiBulkIn;
        }
        else
        {
        	midiHandle->bulkOutEndpoint = epInitStruct.endpointAddress;
            epCallback.callbackFn = USB_DeviceMidiBulkOut;
        }
        epCallback.callbackParam = midiHandle;

        error = USB_DeviceInitEndpoint(midiHandle->handle, &epInitStruct, &epCallback);
    }

    return error;
}

usb_status_t USB_DeviceMidiStreamEndpointsDeinit(usb_device_midi_struct_t *midiHandle)
{
    usb_status_t error = kStatus_USB_Error;

    if (!midiHandle->interfaceHandle)
    {
        return error;
    }
    /* De-initialize all stream endpoints of the interface */
    for (int count = 0U; count < midiHandle->interfaceHandle->endpointList.count; count++)
    {
        error = USB_DeviceDeinitEndpoint(
            midiHandle->handle, midiHandle->interfaceHandle->endpointList.endpoint[count].endpointAddress);
    }
    midiHandle->interfaceHandle = NULL;

    return error;
}


usb_status_t USB_DeviceMidiEvent(void *handle, uint32_t event, void *param)
{
    usb_device_midi_struct_t *midiHandle;
    usb_status_t error = kStatus_USB_Error;
    uint8_t *temp8;

    if ((!param) || (!handle))
    {
        return kStatus_USB_InvalidHandle;
    }

    /* Get the midi class handle. */
    midiHandle = (usb_device_midi_struct_t *)handle;

    switch (event) {
    case kUSB_DeviceClassEventDeviceReset:
    	{
    		/* Bus reset, clear the configuration. */
    		midiHandle->configuration = 0;
    	}
    	break;
    case kUSB_DeviceClassEventSetConfiguration:
    	{
    		/* Get the new configuration. */
    		temp8 = ((uint8_t *)param);
    		if (!midiHandle->configStruct)
    		{
    			break;
    		}
    		if (*temp8 == midiHandle->configuration)
    		{
    			break;
    		}
    		/* De-initialize the endpoints when current configuration is none zero. */
    		if (midiHandle->configuration)
    		{
    			error = USB_DeviceMidiStreamEndpointsDeinit(midiHandle);
    		}
            /* Save new configuration. */
            midiHandle->configuration = *temp8;
            /* Clear the alternate setting value. */
            midiHandle->interfaceHandle = NULL;
            /* Initialize the stream endpoints of the new current configuration by using the alternate setting 0. */
            error = USB_DeviceMidiStreamEndpointsInit(midiHandle);
    	}
    	break;
    case kUSB_DeviceClassEventSetInterface:
    	break;
    case kUSB_DeviceClassEventSetEndpointHalt:
    	{
    		if (!midiHandle->configStruct)
    		{
    			break;
    		}
            /* Get the endpoint address */
            temp8 = ((uint8_t *)param);
            if (midiHandle->interfaceHandle)
            {
                for (int count = 0U; count < midiHandle->interfaceHandle->endpointList.count; count++)
                {
                    if (*temp8 == midiHandle->interfaceHandle->endpointList.endpoint[count].endpointAddress)
                    {
                        /* Only stall the endpoint belongs to control interface of the class */
                        error = USB_DeviceStallEndpoint(midiHandle->handle, *temp8);
                    }
                }
            }
    	}
    	break;
    case kUSB_DeviceClassEventClearEndpointHalt:
    	{
            if (!midiHandle->configStruct)
            {
                break;
            }
            /* Get the endpoint address */
            temp8 = ((uint8_t *)param);
            if (midiHandle->interfaceHandle)
            {
                for (int count = 0U; count < midiHandle->interfaceHandle->endpointList.count; count++)
                {
                    if (*temp8 == midiHandle->interfaceHandle->endpointList.endpoint[count].endpointAddress)
                    {
                        /* Only un-stall the endpoint belongs to control interface of the class */
                        error = USB_DeviceUnstallEndpoint(midiHandle->handle, *temp8);
                    }
                }
            }
    	}
    	break;
    case kUSB_DeviceClassEventClassRequest:
    	break;
    default:
    	break;
    }

    return error;
}

usb_status_t USB_DeviceMidiInit(uint8_t controllerId, usb_device_class_config_struct_t *config, class_handle_t *handle)
{
    usb_device_midi_struct_t *midiHandle;
    usb_status_t error = kStatus_USB_Error;

    /* Allocate a audio class handle. */
    error = USB_DeviceMidiAllocateHandle(&midiHandle);

    if (kStatus_USB_Success != error)
    {
         return error;
    }

    /* Get the device handle according to the controller id. */
    error = USB_DeviceClassGetDeviceHandle(controllerId, &midiHandle->handle);

    if (kStatus_USB_Success != error)
    {
    	USB_DeviceMidiFreeHandle(midiHandle);
        return error;
    }

    if (!midiHandle->handle)
    {
    	USB_DeviceMidiFreeHandle(midiHandle);
        return kStatus_USB_InvalidHandle;
    }
    /* Save the configuration of the class. */
    midiHandle->configStruct = config;
    /* Clear the configuration value. */
    midiHandle->configuration = 0U;
    midiHandle->alternate = 0xff;

    midiHandle->bulkOutEndpoint = USB_MIDI_STREAM_RX_ENDPOINT;
    midiHandle->bulkInEndpoint = USB_MIDI_STREAM_TX_ENDPOINT;

    *handle = (class_handle_t)midiHandle;

    return error;
}

usb_status_t USB_DeviceMidiDeinit(class_handle_t handle)
{
    usb_device_midi_struct_t *midiHandle;
    usb_status_t error = kStatus_USB_Error;

    midiHandle = (usb_device_midi_struct_t *)handle;

    if (!midiHandle)
    {
        return kStatus_USB_InvalidHandle;
    }
    error = USB_DeviceMidiStreamEndpointsDeinit(midiHandle);
    USB_DeviceMidiFreeHandle(midiHandle);

    return error;
}

usb_status_t USB_DeviceMidiSend(class_handle_t handle, uint8_t *buffer, uint32_t length)
{
    usb_device_midi_struct_t *midiHandle;
    usb_status_t error = kStatus_USB_Error;

    if (!handle)
    {
        return kStatus_USB_InvalidHandle;
    }
    midiHandle = (usb_device_midi_struct_t *)handle;

   	error = USB_DeviceSendRequest(midiHandle->handle, midiHandle->bulkInEndpoint, buffer, length);

    return error;
}

usb_status_t USB_DeviceMidiRecv(class_handle_t handle, uint8_t *buffer, uint32_t length)
{
    usb_device_midi_struct_t *midiHandle;
    usb_status_t error = kStatus_USB_Error;

    if (!handle)
    {
        return kStatus_USB_InvalidHandle;
    }
    midiHandle = (usb_device_midi_struct_t *)handle;

    error = USB_DeviceRecvRequest(midiHandle->handle, midiHandle->bulkOutEndpoint, buffer, length);

    return error;
}

#endif	// #if defined(USB_DEVICE_CONFIG_MIDI)
