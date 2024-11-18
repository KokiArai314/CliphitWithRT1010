/*
 * Copyright (c) 2015 - 2016, Freescale Semiconductor, Inc.
 * Copyright 2016 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "usb_device_config.h"
#include "usb.h"
#include "usb_device.h"

#include "usb_device_class.h"

#include "usb_device_descriptor.h"

#if ((defined(USB_DEVICE_CONFIG_AUDIO)) && (USB_DEVICE_CONFIG_AUDIO > 0U))
#include "usb_device_audio.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
/// @note memo: とりあえず今必要のない関数は宣言せず,必要な関数を追加で用意するスタンスで
static usb_status_t USB_DeviceAudioAllocateHandle(usb_device_audio_struct_t **handle);
static usb_status_t USB_DeviceAudioFreeHandle(usb_device_audio_struct_t *handle);

/// @note add AudioRec (add streamOut/InInterfaceHandle)
usb_status_t USB_DeviceAudioOutIsochronousIn(usb_device_handle handle,
                                             usb_device_endpoint_callback_message_struct_t *message,
                                             void *callbackParam);
usb_status_t USB_DeviceAudioOutIsochronousOut(usb_device_handle handle,
                                              usb_device_endpoint_callback_message_struct_t *message,
                                              void *callbackParam);
usb_status_t USB_DeviceAudioInIsochronousIn(usb_device_handle handle,
                                            usb_device_endpoint_callback_message_struct_t *message,
                                            void *callbackParam);
usb_status_t USB_DeviceAudioOutStreamEndpointsInit(usb_device_audio_struct_t *audioHandle);
usb_status_t USB_DeviceAudioOutStreamEndpointsDeinit(usb_device_audio_struct_t *audioHandle);
usb_status_t USB_DeviceAudioInStreamEndpointsInit(usb_device_audio_struct_t *audioHandle);
usb_status_t USB_DeviceAudioInStreamEndpointsDeinit(usb_device_audio_struct_t *audioHandle);

usb_status_t USB_DeviceAudioSetRequestEndpoint(usb_device_audio_struct_t *audioHandle,
                                               usb_device_control_request_struct_t *controlRequest);
usb_status_t USB_DeviceAudioGetRequestEndpoint(usb_device_audio_struct_t *audioHandle,
                                               usb_device_control_request_struct_t *controlRequest);
usb_status_t USB_DeviceAudioSetRequestInterface(usb_device_audio_struct_t *audioHandle,
                                                usb_device_control_request_struct_t *controlRequest);
usb_status_t USB_DeviceAudioGetRequestInterface(usb_device_audio_struct_t *audioHandle,
                                                usb_device_control_request_struct_t *controlRequest);

/*******************************************************************************
 * Variables
 ******************************************************************************/

USB_GLOBAL USB_RAM_ADDRESS_ALIGNMENT(USB_DATA_ALIGN_SIZE) static usb_device_audio_struct_t
    s_UsbDeviceAudioHandle[USB_DEVICE_CONFIG_AUDIO];

/*******************************************************************************
 * Code
 ******************************************************************************/

/*!
 * @brief Allocate a device audio class handle.
 *
 * This function allocates a device audio class handle.
 *
 * @param handle          It is out parameter, is used to return pointer of the device audio class handle to the caller.
 *
 * @retval kStatus_USB_Success              Get a device audio class handle successfully.
 * @retval kStatus_USB_Busy                 Cannot allocate a device audio class handle.
 */
static usb_status_t USB_DeviceAudioAllocateHandle(usb_device_audio_struct_t **handle) {
  int32_t count;
  USB_OSA_SR_ALLOC();

  USB_OSA_ENTER_CRITICAL();
  for (count = 0; count < USB_DEVICE_CONFIG_AUDIO; count++) {
    if (NULL == s_UsbDeviceAudioHandle[count].handle) {
      *handle = &s_UsbDeviceAudioHandle[count];
      USB_OSA_EXIT_CRITICAL();
      return kStatus_USB_Success;
    }
  }
  USB_OSA_EXIT_CRITICAL();
  return kStatus_USB_Busy;
}

/*!
 * @brief Free a device audio class hanlde.
 *
 * This function frees a device audio class hanlde.
 *
 * @param handle          The device audio class handle.
 *
 * @retval kStatus_USB_Success              Free device audio class hanlde successfully.
 */
static usb_status_t USB_DeviceAudioFreeHandle(usb_device_audio_struct_t *handle) {
  USB_OSA_SR_ALLOC();

  USB_OSA_ENTER_CRITICAL();
  handle->handle = NULL;
  handle->configStruct = (usb_device_class_config_struct_t *)NULL;
  handle->configuration = 0U;

  /// @note add AudioRec (add streamOut/InInterfaceHandle)
  handle->streamOutAlternate = 0;
  handle->streamInAlternate = 0;

  USB_OSA_EXIT_CRITICAL();
  return kStatus_USB_Success;
}

/// @note add AudioRec (add streamOut/InInterfaceHandle)
/*!
 * @brief ISO IN endpoint callback function.
 *
 * This callback function is used to notify uplayer the tranfser result of a transfer.
 * This callback pointer is passed when the ISO IN pipe initialized.
 *
 * @param handle          The device handle. It equals the value returned from USB_DeviceInit.
 * @param message         The result of the ISO IN pipe transfer.
 * @param callbackParam  The paramter for this callback. It is same with
 * usb_device_endpoint_callback_struct_t::callbackParam. In the class, the value is the audio class handle.
 *
 * @return A USB error code or kStatus_USB_Success.
 */
usb_status_t USB_DeviceAudioOutIsochronousIn(usb_device_handle handle,
                                             usb_device_endpoint_callback_message_struct_t *message,
                                             void *callbackParam) {
  usb_device_audio_struct_t *audioHandle;
  usb_status_t error = kStatus_USB_Error;

  /* Get the audio class handle */
  audioHandle = (usb_device_audio_struct_t *)callbackParam;
  if (!audioHandle) {
    return kStatus_USB_InvalidHandle;
  }
  audioHandle->streamOutEpInPipeBusy = 0U;
  if ((NULL != audioHandle->configStruct) && (audioHandle->configStruct->classCallback)) {
    /* Notify the application stream data sent by calling the audio class callback.
        classCallback is initialized in classInit of s_UsbDeviceClassInterfaceMap,
        it is from the second parameter of classInit */
    error = audioHandle->configStruct->classCallback((class_handle_t)audioHandle,
                                                     kUSB_DeviceAudioOutEventStreamSendResponse, message);
  }

  return error;
}

/*!
 * @brief ISO OUT endpoint callback function.
 *
 * This callback function is used to notify uplayer the tranfser result of a transfer.
 * This callback pointer is passed when the ISO OUT pipe initialized.
 *
 * @param handle          The device handle. It equals the value returned from USB_DeviceInit.
 * @param message         The result of the ISO OUT pipe transfer.
 * @param callbackParam  The paramter for this callback. It is same with
 * usb_device_endpoint_callback_struct_t::callbackParam. In the class, the value is the audio class handle.
 *
 * @return A USB error code or kStatus_USB_Success.
 */
usb_status_t USB_DeviceAudioOutIsochronousOut(usb_device_handle handle,
                                              usb_device_endpoint_callback_message_struct_t *message,
                                              void *callbackParam) {
  usb_device_audio_struct_t *audioHandle;
  usb_status_t error = kStatus_USB_Error;

  /* Get the audio class handle */
  audioHandle = (usb_device_audio_struct_t *)callbackParam;
  if (!audioHandle) {
    return kStatus_USB_InvalidHandle;
  }
  audioHandle->streamOutEpOutPipeBusy = 0U;

  if ((NULL != audioHandle->configStruct) && (audioHandle->configStruct->classCallback)) {
    /* classCallback is initialized in classInit of s_UsbDeviceClassInterfaceMap,
        it is from the second parameter of classInit */
    error = audioHandle->configStruct->classCallback((class_handle_t)audioHandle,
                                                     kUSB_DeviceAudioOutEventStreamRecvResponse, message);
  }
  return error;
}

usb_status_t USB_DeviceAudioInIsochronousIn(usb_device_handle handle,
                                            usb_device_endpoint_callback_message_struct_t *message,
                                            void *callbackParam) {
  usb_device_audio_struct_t *audioHandle;
  usb_status_t error = kStatus_USB_Error;

  /* Get the audio class handle */
  audioHandle = (usb_device_audio_struct_t *)callbackParam;
  if (!audioHandle) {
    return kStatus_USB_InvalidHandle;
  }
  audioHandle->streamInEpInPipeBusy = 0U;
  if ((NULL != audioHandle->configStruct) && (audioHandle->configStruct->classCallback)) {
    /* Notify the application stream data sent by calling the audio class callback.
        classCallback is initialized in classInit of s_UsbDeviceClassInterfaceMap,
        it is from the second parameter of classInit */
    error = audioHandle->configStruct->classCallback((class_handle_t)audioHandle,
                                                     kUSB_DeviceAudioInEventStreamSendResponse, message);
  }

  return error;
}

/*!
 * @brief Initialize the stream endpoints of the audio class.
 *
 * This callback function is used to initialize the stream endpoints of the audio class.
 *
 * @param audioHandle          The device audio class handle. It equals the value returned from
 * usb_device_class_config_struct_t::classHandle.
 *
 * @return A USB error code or kStatus_USB_Success.
 */
usb_status_t USB_DeviceAudioOutStreamEndpointsInit(usb_device_audio_struct_t *audioHandle) {
  usb_device_interface_list_t *interfaceList;
  usb_device_interface_struct_t *interface = (usb_device_interface_struct_t *)NULL;
  usb_status_t error = kStatus_USB_Error;

  /* Check the configuration is valid or not. */
  if (!audioHandle->configuration) {
    return error;
  }

  /* Check the configuration is valid or not. */
  if (audioHandle->configuration > audioHandle->configStruct->classInfomation->configurations) {
    return error;
  }

  if (NULL == audioHandle->configStruct->classInfomation->interfaceList) {
    return error;
  }

  /* Get the interface list of the new configuration. */
  interfaceList = &audioHandle->configStruct->classInfomation->interfaceList[audioHandle->configuration - 1];

  /* Find stream interface by using the alternate setting of the interface. */
  /// @note memo: interfaceListは1つ. interfaceList->countはインターフェース数(0,1,2 = 3つ)で、各interfaceにもcountがあるが, Audio InterfaceはaltSetting分(0,1 = 2つ)
  {
    const int count = USB_AUDIO_OFFSET_ENDPOINT_INTERFACE_INDEX; /// @note memo: Audio OUT Interface

    if ((USB_DEVICE_CONFIG_AUDIO_CLASS_CODE == interfaceList->interfaces[count].classCode) &&
        (USB_DEVICE_AUDIO_STREAM_SUBCLASS == interfaceList->interfaces[count].subclassCode)) {
      for (int index = 0; index < interfaceList->interfaces[count].count; index++) {
        if (interfaceList->interfaces[count].interface[index].alternateSetting == audioHandle->streamOutAlternate) {
          interface = &interfaceList->interfaces[count].interface[index];
          break;
        }
      }
      audioHandle->streamOutInterfaceNumber = interfaceList->interfaces[count].interfaceNumber;
    }
  }

  if (!interface) {
    return error;
  }
  /* Keep new stream interface handle. */
  audioHandle->streamOutInterfaceHandle = interface;

  /* Initialize the endpoints of the new interface. */
  /// @note memo: endpointList.count -> Audio Outで必要なendpointの数と考えていい.
  for (int count = 0U; count < interface->endpointList.count; count++) {
    usb_device_endpoint_init_struct_t epInitStruct;
    usb_device_endpoint_callback_struct_t epCallback;
    epInitStruct.zlt = 0U;
    epInitStruct.endpointAddress = interface->endpointList.endpoint[count].endpointAddress;
    epInitStruct.maxPacketSize = interface->endpointList.endpoint[count].maxPacketSize;
    epInitStruct.transferType = interface->endpointList.endpoint[count].transferType;

    if ((USB_ENDPOINT_ISOCHRONOUS == (epInitStruct.transferType & USB_DESCRIPTOR_ENDPOINT_ATTRIBUTE_TYPE_MASK)) &&
        (USB_IN == ((epInitStruct.endpointAddress & USB_DESCRIPTOR_ENDPOINT_ADDRESS_DIRECTION_MASK) >>
                    USB_DESCRIPTOR_ENDPOINT_ADDRESS_DIRECTION_SHIFT))) {
      epCallback.callbackFn = USB_DeviceAudioOutIsochronousIn;
    } else {
      epCallback.callbackFn = USB_DeviceAudioOutIsochronousOut;
    }
    epCallback.callbackParam = audioHandle;

    error = USB_DeviceInitEndpoint(audioHandle->handle, &epInitStruct, &epCallback);
  }
  return error;
}

/*!
 * @brief De-initialize the stream endpoints of the audio class.
 *
 * This callback function is used to de-initialize the stream endpoints of the audio class.
 *
 * @param audioHandle          The device audio class handle. It equals the value returned from
 * usb_device_class_config_struct_t::classHandle.
 *
 * @return A USB error code or kStatus_USB_Success.
 */
usb_status_t USB_DeviceAudioOutStreamEndpointsDeinit(usb_device_audio_struct_t *audioHandle) {
  usb_status_t error = kStatus_USB_Error;
  usb_device_endpoint_callback_message_struct_t message;

  if (!audioHandle->streamOutInterfaceHandle) {
    return error;
  }
  /* De-initialize all stream endpoints of the interface */
  for (int count = 0U; count < audioHandle->streamOutInterfaceHandle->endpointList.count; count++) {
    error = USB_DeviceDeinitEndpoint(
        audioHandle->handle, audioHandle->streamOutInterfaceHandle->endpointList.endpoint[count].endpointAddress);
  }

  for (int count = 0U; count < audioHandle->streamOutInterfaceHandle->endpointList.count; count++) {
    if ((audioHandle->streamOutInterfaceHandle->endpointList.endpoint[count].endpointAddress &
         USB_DESCRIPTOR_ENDPOINT_ADDRESS_DIRECTION_MASK) >>
            USB_DESCRIPTOR_ENDPOINT_ADDRESS_DIRECTION_SHIFT ==
        USB_IN) {
      if (audioHandle->streamOutEpInPipeBusy) {
        message.length = USB_UNINITIALIZED_VAL_32;
        USB_DeviceAudioOutIsochronousIn(audioHandle->handle, &message, audioHandle);
      }
    } else {
      if (audioHandle->streamOutEpOutPipeBusy) {
        message.length = USB_UNINITIALIZED_VAL_32;
        USB_DeviceAudioOutIsochronousOut(audioHandle->handle, &message, audioHandle);
      }
    }
  }

  audioHandle->streamOutInterfaceHandle = NULL;
  return error;
}

/*!
 * @brief Initialize the stream endpoints of the audio class.
 *
 * This callback function is used to initialize the stream endpoints of the audio class.
 *
 * @param audioHandle          The device audio class handle. It equals the value returned from
 * usb_device_class_config_struct_t::classHandle.
 *
 * @return A USB error code or kStatus_USB_Success.
 */
usb_status_t USB_DeviceAudioInStreamEndpointsInit(usb_device_audio_struct_t *audioHandle) {
  usb_device_interface_list_t *interfaceList;
  usb_device_interface_struct_t *interface = (usb_device_interface_struct_t *)NULL;
  usb_status_t error = kStatus_USB_Error;

  /* Check the configuration is valid or not. */
  if (!audioHandle->configuration) {
    return error;
  }

  /* Check the configuration is valid or not. */
  if (audioHandle->configuration > audioHandle->configStruct->classInfomation->configurations) {
    return error;
  }

  if (NULL == audioHandle->configStruct->classInfomation->interfaceList) {
    return error;
  }

  /* Get the interface list of the new configuration. */
  interfaceList = &audioHandle->configStruct->classInfomation->interfaceList[audioHandle->configuration - 1];

  /* Find stream interface by using the alternate setting of the interface. */
  /// @note memo: interfaceListは1つ. interfaceList->countはインターフェース数(0,1,2 = 3つ)で、各interfaceにもcountがあるが, Audio InterfaceはaltSetting分(0,1 = 2つ)
  {
    const int count = 2; /// @note memo: Audio IN Interface

    if ((USB_DEVICE_CONFIG_AUDIO_CLASS_CODE == interfaceList->interfaces[count].classCode) &&
        (USB_DEVICE_AUDIO_STREAM_SUBCLASS == interfaceList->interfaces[count].subclassCode)) {
      for (int index = 0; index < interfaceList->interfaces[count].count; index++) {
        if (interfaceList->interfaces[count].interface[index].alternateSetting == audioHandle->streamInAlternate) {
          interface = &interfaceList->interfaces[count].interface[index];
          break;
        }
      }
      audioHandle->streamInInterfaceNumber = interfaceList->interfaces[count].interfaceNumber;
    }
  }

  if (!interface) {
    return error;
  }
  /* Keep new stream interface handle. */
  audioHandle->streamInInterfaceHandle = interface;

  /* Initialize the endpoints of the new interface. */
  /// @note memo: endpointList.count -> Audio Outで必要なendpointの数と考えていい.
  for (int count = 0U; count < interface->endpointList.count; count++) {
    usb_device_endpoint_init_struct_t epInitStruct;
    usb_device_endpoint_callback_struct_t epCallback;
    epInitStruct.zlt = 0U;
    epInitStruct.endpointAddress = interface->endpointList.endpoint[count].endpointAddress;
    epInitStruct.maxPacketSize = interface->endpointList.endpoint[count].maxPacketSize;
    epInitStruct.transferType = interface->endpointList.endpoint[count].transferType;

    if ((USB_ENDPOINT_ISOCHRONOUS == (epInitStruct.transferType & USB_DESCRIPTOR_ENDPOINT_ATTRIBUTE_TYPE_MASK)) &&
        (USB_IN == ((epInitStruct.endpointAddress & USB_DESCRIPTOR_ENDPOINT_ADDRESS_DIRECTION_MASK) >>
                    USB_DESCRIPTOR_ENDPOINT_ADDRESS_DIRECTION_SHIFT))) {
      epCallback.callbackFn = USB_DeviceAudioInIsochronousIn;
    }
    epCallback.callbackParam = audioHandle;

    error = USB_DeviceInitEndpoint(audioHandle->handle, &epInitStruct, &epCallback);
  }
  return error;
}

/*!
 * @brief De-initialize the stream endpoints of the audio class.
 *
 * This callback function is used to de-initialize the stream endpoints of the audio class.
 *
 * @param audioHandle          The device audio class handle. It equals the value returned from
 * usb_device_class_config_struct_t::classHandle.
 *
 * @return A USB error code or kStatus_USB_Success.
 */
usb_status_t USB_DeviceAudioInStreamEndpointsDeinit(usb_device_audio_struct_t *audioHandle) {
  usb_status_t error = kStatus_USB_Error;
  usb_device_endpoint_callback_message_struct_t message;

  if (audioHandle->streamInInterfaceHandle == NULL) {
    return error;
  }
  /* De-initialize all stream endpoints of the interface */
  for (int count = 0U; count < audioHandle->streamInInterfaceHandle->endpointList.count; count++) {
    error = USB_DeviceDeinitEndpoint(
        audioHandle->handle, audioHandle->streamInInterfaceHandle->endpointList.endpoint[count].endpointAddress);
  }

  for (int count = 0U; count < audioHandle->streamInInterfaceHandle->endpointList.count; count++) {
    if ((audioHandle->streamInInterfaceHandle->endpointList.endpoint[count].endpointAddress &
         USB_DESCRIPTOR_ENDPOINT_ADDRESS_DIRECTION_MASK) >>
            USB_DESCRIPTOR_ENDPOINT_ADDRESS_DIRECTION_SHIFT ==
        USB_IN) {
      if (audioHandle->streamInEpInPipeBusy) {
        message.length = USB_UNINITIALIZED_VAL_32;
        USB_DeviceAudioInIsochronousIn(audioHandle->handle, &message, audioHandle);
      }
    }
  }

  audioHandle->streamInInterfaceHandle = NULL;
  return error;
}

/*!
 * @brief Handle the audio device set request endpoint.
 *
 * This callback function provides flexibility to add class and vendor specific requests
 *
 * @param audioHandle          The device audio class handle. It equals the value returned from
 * usb_device_class_config_struct_t::classHandle.
 * @param controlRequest       The pointer of the control request structure.
 *
 * @return A USB error code or kStatus_USB_Success.
 */
usb_status_t USB_DeviceAudioSetRequestEndpoint(usb_device_audio_struct_t *audioHandle,
                                               usb_device_control_request_struct_t *controlRequest) {
  usb_status_t error = kStatus_USB_Error;
  uint8_t controlSelector = (controlRequest->setup->wValue >> 0x08) & 0xFFU;
  uint32_t audioCommand = 0;

  /* Select SET request Control Feature Unit Module */
  switch (controlRequest->setup->bRequest) {
    case USB_DEVICE_AUDIO_SET_CUR_VOLUME_REQUEST:
      switch (controlSelector) {
        case USB_DEVICE_AUDIO_SAMPLING_FREQ_CONTROL_SELECTOR:
          audioCommand = USB_DEVICE_AUDIO_SET_CUR_SAMPLING_FREQ_CONTROL;
          break;
        case USB_DEVICE_AUDIO_PITCH_CONTROL_SELECTOR:
          audioCommand = USB_DEVICE_AUDIO_SET_CUR_PITCH_CONTROL;
          break;
        default:
          break;
      }
      break;
    case USB_DEVICE_AUDIO_SET_MIN_VOLUME_REQUEST:
      switch (controlSelector) {
        case USB_DEVICE_AUDIO_SAMPLING_FREQ_CONTROL_SELECTOR:
          audioCommand = USB_DEVICE_AUDIO_SET_MIN_SAMPLING_FREQ_CONTROL;
          break;
        default:
          break;
      }
      break;
    case USB_DEVICE_AUDIO_SET_MAX_VOLUME_REQUEST:
      switch (controlSelector) {
        case USB_DEVICE_AUDIO_SAMPLING_FREQ_CONTROL_SELECTOR:
          audioCommand = USB_DEVICE_AUDIO_SET_MAX_SAMPLING_FREQ_CONTROL;
          break;
        default:
          break;
      }
      break;
    case USB_DEVICE_AUDIO_SET_RES_VOLUME_REQUEST:
      switch (controlSelector) {
        case USB_DEVICE_AUDIO_SAMPLING_FREQ_CONTROL_SELECTOR:
          audioCommand = USB_DEVICE_AUDIO_SET_RES_SAMPLING_FREQ_CONTROL;
          break;
        default:
          break;
      }
      break;

    default:
      break;
  }
  if ((audioCommand) && (NULL != audioHandle->configStruct) && (audioHandle->configStruct->classCallback)) {
    /* classCallback is initialized in classInit of s_UsbDeviceClassInterfaceMap,
        it is from the second parameter of classInit */
    error = audioHandle->configStruct->classCallback((class_handle_t)audioHandle, audioCommand, controlRequest);
  }
  return error;
}

/*!
 * @brief Handle the audio device get request endpoint.
 *
 * This callback function provides flexibility to add class and vendor specific requests
 *
 * @param audioHandle          The device audio class handle. It equals the value returned from
 * usb_device_class_config_struct_t::classHandle.
 * @param controlRequest       The pointer of the control request structure.
 *
 * @return A USB error code or kStatus_USB_Success.
 */
usb_status_t USB_DeviceAudioGetRequestEndpoint(usb_device_audio_struct_t *audioHandle,
                                               usb_device_control_request_struct_t *controlRequest) {
  usb_status_t error = kStatus_USB_Error;
  uint8_t controlSelector = (controlRequest->setup->wValue >> 0x08) & 0xFFU;
  uint32_t audioCommand = 0;
  /* Select SET request Control Feature Unit Module */
  switch (controlRequest->setup->bRequest) {
    case USB_DEVICE_AUDIO_GET_CUR_VOLUME_REQUEST:
      switch (controlSelector) {
        case USB_DEVICE_AUDIO_SAMPLING_FREQ_CONTROL_SELECTOR:

          audioCommand = USB_DEVICE_AUDIO_GET_CUR_SAMPLING_FREQ_CONTROL;
          break;
        default:
          break;
      }
      break;
    case USB_DEVICE_AUDIO_GET_MIN_VOLUME_REQUEST:
      switch (controlSelector) {
        case USB_DEVICE_AUDIO_SAMPLING_FREQ_CONTROL_SELECTOR:

          audioCommand = USB_DEVICE_AUDIO_GET_MIN_SAMPLING_FREQ_CONTROL;
          break;
        default:
          break;
      }
      break;
    case USB_DEVICE_AUDIO_GET_MAX_VOLUME_REQUEST:
      switch (controlSelector) {
        case USB_DEVICE_AUDIO_SAMPLING_FREQ_CONTROL_SELECTOR:

          audioCommand = USB_DEVICE_AUDIO_GET_MAX_SAMPLING_FREQ_CONTROL;
          break;
        default:
          break;
      }
      break;
    case USB_DEVICE_AUDIO_GET_RES_VOLUME_REQUEST:
      switch (controlSelector) {
        case USB_DEVICE_AUDIO_SAMPLING_FREQ_CONTROL_SELECTOR:
          audioCommand = USB_DEVICE_AUDIO_GET_RES_SAMPLING_FREQ_CONTROL;

          break;
        default:
          break;
      }
      break;

    default:
      break;
  }
  if ((audioCommand) && (NULL != audioHandle->configStruct) && (audioHandle->configStruct->classCallback)) {
    /* classCallback is initialized in classInit of s_UsbDeviceClassInterfaceMap,
        it is from the second parameter of classInit */
    error = audioHandle->configStruct->classCallback((class_handle_t)audioHandle, audioCommand, controlRequest);
  }
  return error;
}

/*!
 * @brief Handle the event passed to the audio class.
 *
 * This function handles the event passed to the audio class.
 *
 * @param handle          The audio class handle, got from the usb_device_class_config_struct_t::classHandle.
 * @param event           The event codes. Please refer to the enumeration usb_device_class_event_t.
 * @param param           The param type is determined by the event code.
 *
 * @return A USB error code or kStatus_USB_Success.
 * @retval kStatus_USB_Success              Free device handle successfully.
 * @retval kStatus_USB_InvalidParameter     The device handle not be found.
 * @retval kStatus_USB_InvalidRequest       The request is invalid, and the control pipe will be stalled by the caller.
 */
usb_status_t USB_DeviceAudioEvent(void *handle, uint32_t event, void *param) {
  usb_device_audio_struct_t *audioHandle;
  usb_status_t error = kStatus_USB_Error;
  uint16_t interfaceAlternate;
  uint8_t *temp8;
  uint8_t alternate;

  if ((!param) || (!handle)) {
    return kStatus_USB_InvalidHandle;
  }

  /* Get the audio class handle. */
  audioHandle = (usb_device_audio_struct_t *)handle;

  switch (event) {
    case kUSB_DeviceClassEventDeviceReset:
      /* Bus reset, clear the configuration. */
      audioHandle->configuration = 0;
      /// @note add AudioRec (add streamOut/InInterfaceHandle)
      audioHandle->streamOutEpOutPipeBusy = 0;
      audioHandle->streamOutEpInPipeBusy = 0;
      audioHandle->streamInEpInPipeBusy = 0;
      break;
    case kUSB_DeviceClassEventSetConfiguration:
      /* Get the new configuration. */
      temp8 = ((uint8_t *)param);
      if (!audioHandle->configStruct) {
        break;
      }
      if (*temp8 == audioHandle->configuration) {
        break;
      }
      /* De-initialize the endpoints when current configuration is none zero. */

      /// @note add AudioRec (add streamOut/InInterfaceHandle)
      if (audioHandle->configuration) {
        error = USB_DeviceAudioOutStreamEndpointsDeinit(audioHandle);
        error = USB_DeviceAudioInStreamEndpointsDeinit(audioHandle);
      }
      /* Save new configuration. */
      audioHandle->configuration = *temp8;
      /* Clear the alternate setting value. */
      audioHandle->streamOutAlternate = 0;
      audioHandle->streamOutInterfaceHandle = NULL;
      audioHandle->streamInAlternate = 0;
      audioHandle->streamInInterfaceHandle = NULL;
      /* Initialize the stream endpoints of the new current configuration by using the alternate setting 0. */
      error = USB_DeviceAudioOutStreamEndpointsInit(audioHandle);
      error = USB_DeviceAudioInStreamEndpointsInit(audioHandle);
      break;
    case kUSB_DeviceClassEventSetInterface:
      if (!audioHandle->configStruct) {
        break;
      }
      /* Get the new alternate setting of the interface */
      interfaceAlternate = *((uint16_t *)param);
      /* Get the alternate setting value */
      alternate = (uint8_t)(interfaceAlternate & 0xFFU);

      /// @note add AudioRec (add streamOut/InInterfaceHandle)
      /* Whether the interface belongs to the class. */
      if (audioHandle->streamOutInterfaceNumber == ((uint8_t)(interfaceAlternate >> 8U))) {
        /* When the interface is control interface. */
        /* Only handle new alternate setting. */
        if (alternate == audioHandle->streamOutAlternate) {
          break;
        }
        /* De-initialize old endpoints */
        error = USB_DeviceAudioOutStreamEndpointsDeinit(audioHandle);
        audioHandle->streamOutAlternate = alternate;
        /* Initialize new endpoints */
        error = USB_DeviceAudioOutStreamEndpointsInit(audioHandle);
      } else if (audioHandle->streamInInterfaceNumber == ((uint8_t)(interfaceAlternate >> 8U))) {
        /* When the interface is stream interface. */
        /* Only handle new alternate setting. */
        if (alternate == audioHandle->streamInAlternate) {
          break;
        }
        /* De-initialize old endpoints */
        error = USB_DeviceAudioInStreamEndpointsDeinit(audioHandle);
        audioHandle->streamInAlternate = alternate;
        /* Initialize new endpoints */
        error = USB_DeviceAudioInStreamEndpointsInit(audioHandle);
      }
      break;
    case kUSB_DeviceClassEventSetEndpointHalt:
      if (!audioHandle->configStruct) {
        break;
      }
      /* Get the endpoint address */
      temp8 = ((uint8_t *)param);
      /// @note add AudioRec (add streamOut/InInterfaceHandle)
      if (audioHandle->streamOutInterfaceHandle) {
        for (int count = 0U; count < audioHandle->streamOutInterfaceHandle->endpointList.count; count++) {
          if (*temp8 == audioHandle->streamOutInterfaceHandle->endpointList.endpoint[count].endpointAddress) {
            /* Only stall the endpoint belongs to control interface of the class */
            error = USB_DeviceStallEndpoint(audioHandle->handle, *temp8);
          }
        }
      }
      if (audioHandle->streamInInterfaceHandle) {
        for (int count = 0U; count < audioHandle->streamInInterfaceHandle->endpointList.count; count++) {
          if (*temp8 == audioHandle->streamInInterfaceHandle->endpointList.endpoint[count].endpointAddress) {
            /* Only stall the endpoint belongs to stream interface of the class */
            error = USB_DeviceStallEndpoint(audioHandle->handle, *temp8);
          }
        }
      }
      break;
    case kUSB_DeviceClassEventClearEndpointHalt:
      if (!audioHandle->configStruct) {
        break;
      }
      /* Get the endpoint address */
      temp8 = ((uint8_t *)param);
      /// @note add AudioRec (add streamOut/InInterfaceHandle)
      if (audioHandle->streamOutInterfaceHandle) {
        for (int count = 0U; count < audioHandle->streamOutInterfaceHandle->endpointList.count; count++) {
          if (*temp8 == audioHandle->streamOutInterfaceHandle->endpointList.endpoint[count].endpointAddress) {
            /* Only un-stall the endpoint belongs to control interface of the class */
            error = USB_DeviceUnstallEndpoint(audioHandle->handle, *temp8);
          }
        }
      }
      if (audioHandle->streamInInterfaceHandle) {
        for (int count = 0U; count < audioHandle->streamInInterfaceHandle->endpointList.count; count++) {
          if (*temp8 == audioHandle->streamInInterfaceHandle->endpointList.endpoint[count].endpointAddress) {
            /* Only un-stall the endpoint belongs to stream interface of the class */
            error = USB_DeviceUnstallEndpoint(audioHandle->handle, *temp8);
          }
        }
      }
      break;
    case kUSB_DeviceClassEventClassRequest:
      if (param) {
        /* Handle the audio class specific request. */
        usb_device_control_request_struct_t *controlRequest = (usb_device_control_request_struct_t *)param;
        if ((controlRequest->setup->bmRequestType & USB_REQUEST_TYPE_RECIPIENT_MASK) !=
            USB_REQUEST_TYPE_RECIPIENT_INTERFACE) {
          if (USB_REQUEST_TYPE_TYPE_CLASS == (controlRequest->setup->bmRequestType & USB_REQUEST_TYPE_TYPE_MASK)) {
            switch (controlRequest->setup->bmRequestType) {
              case USB_DEVICE_AUDIO_SET_REQUEST_ENDPOINT:
                error = USB_DeviceAudioSetRequestEndpoint(audioHandle, controlRequest);
                break;
              case USB_DEVICE_AUDIO_GET_REQUEST_ENDPOINT:
                error = USB_DeviceAudioGetRequestEndpoint(audioHandle, controlRequest);
                break;
              default:
                break;
            }
          }
        }
      }
      break;
    default:
      break;
  }
  return error;
}

/*!
 * @brief Initialize the audio class.
 *
 * This function is used to initialize the audio class.
 *
 * @param controllerId   The controller id of the USB IP. Please refer to the enumeration usb_controller_index_t.
 * @param config          The class configuration information.
 * @param handle          It is out parameter, is used to return pointer of the audio class handle to the caller.
 *
 * @return A USB error code or kStatus_USB_Success.
 */
usb_status_t USB_DeviceAudioInit(uint8_t controllerId, usb_device_class_config_struct_t *config,
                                 class_handle_t *handle) {
  usb_device_audio_struct_t *audioHandle;
  usb_status_t error = kStatus_USB_Error;

  /* Allocate a audio class handle. */
  error = USB_DeviceAudioAllocateHandle(&audioHandle);

  if (kStatus_USB_Success != error) {
    return error;
  }

  /* Get the device handle according to the controller id. */
  error = USB_DeviceClassGetDeviceHandle(controllerId, &audioHandle->handle);

  if (kStatus_USB_Success != error) {
    return error;
  }

  if (!audioHandle->handle) {
    return kStatus_USB_InvalidHandle;
  }
  /* Save the configuration of the class. */
  audioHandle->configStruct = config;
  /* Clear the configuration value. */
  audioHandle->configuration = 0U;

  /// @note add AudioRec (add streamOut/InInterfaceHandle)
  audioHandle->streamOutAlternate = 0xffU;
  audioHandle->streamInAlternate = 0xffU;

  *handle = (class_handle_t)audioHandle;
  return error;
}

/*!
 * @brief De-initialize the device audio class.
 *
 * The function de-initializes the device audio class.
 *
 * @param handle The ccid class handle got from usb_device_class_config_struct_t::classHandle.
 *
 * @return A USB error code or kStatus_USB_Success.
 */
usb_status_t USB_DeviceAudioDeinit(class_handle_t handle) {
  usb_device_audio_struct_t *audioHandle;
  usb_status_t error = kStatus_USB_Error;

  audioHandle = (usb_device_audio_struct_t *)handle;

  if (!audioHandle) {
    return kStatus_USB_InvalidHandle;
  }

  /// @note add AudioRec (add streamOut/InInterfaceHandle)
  error = USB_DeviceAudioOutStreamEndpointsDeinit(audioHandle);
  error = USB_DeviceAudioInStreamEndpointsDeinit(audioHandle);

  USB_DeviceAudioFreeHandle(audioHandle);
  return error;
}

/*!
 * @brief Send data through a specified endpoint.
 *
 * The function is used to send data through a specified endpoint.
 * The function calls USB_DeviceSendRequest internally.
 *
 * @param handle The audio class handle got from usb_device_class_config_struct_t::classHandle.
 * @param endpointAddress Endpoint index.
 * @param buffer The memory address to hold the data need to be sent.
 * @param length The data length need to be sent.
 *
 * @return A USB error code or kStatus_USB_Success.
 *
 * @note The return value just means if the sending request is successful or not; the transfer done is notified by
 * usb_device_audio_stream_in or usb_device_audio_control_in.
 * Currently, only one transfer request can be supported for one specific endpoint.
 * If there is a specific requirement to support multiple transfer requests for one specific endpoint, the application
 * should implement a queue in the application level.
 * The subsequent transfer could begin only when the previous transfer is done (get notification through the endpoint
 * callback).
 */
usb_status_t USB_DeviceAudioSend(class_handle_t handle, uint8_t ep, uint8_t *buffer, uint32_t length) {
  usb_device_audio_struct_t *audioHandle;
  usb_status_t error = kStatus_USB_Error;

  if (!handle) {
    return kStatus_USB_InvalidHandle;
  }
  audioHandle = (usb_device_audio_struct_t *)handle;

  /// @note add AudioRec (add streamOut/InInInterfaceHandle)
  if (audioHandle->streamInEpInPipeBusy) {
    return kStatus_USB_Busy;
  }

  error = USB_DeviceSendRequest(audioHandle->handle, ep, buffer, length);
  if (kStatus_USB_Success == error) {
    audioHandle->streamInEpInPipeBusy = 1U;
  }

  return error;
}

/*!
 * @brief Receive data through a specified endpoint.
 *
 * The function is used to receive data through a specified endpoint.
 * The function calls USB_DeviceRecvRequest internally.
 *
 * @param handle The audio class handle got from usb_device_class_config_struct_t::classHandle.
 * @param endpointAddress Endpoint index.
 * @param buffer The memory address to save the received data.
 * @param length The data length want to be received.
 *
 * @return A USB error code or kStatus_USB_Success.
 *
 * @note The return value just means if the receiving request is successful or not; the transfer done is notified by
 * usb_device_audio_stream_out.
 * Currently, only one transfer request can be supported for one specific endpoint.
 * If there is a specific requirement to support multiple transfer requests for one specific endpoint, the application
 * should implement a queue in the application level.
 * The subsequent transfer could begin only when the previous transfer is done (get notification through the endpoint
 * callback).
 */
usb_status_t USB_DeviceAudioRecv(class_handle_t handle, uint8_t ep, uint8_t *buffer, uint32_t length) {
  usb_device_audio_struct_t *audioHandle;
  usb_status_t error = kStatus_USB_Error;

  if (!handle) {
    return kStatus_USB_InvalidHandle;
  }
  audioHandle = (usb_device_audio_struct_t *)handle;

  /// @note add AudioRec (add streamOut/InInInterfaceHandle)
  if (audioHandle->streamOutEpOutPipeBusy) {
    return kStatus_USB_Busy;
  }

  error = USB_DeviceRecvRequest(audioHandle->handle, ep, buffer, length);
  if (kStatus_USB_Success == error) {
    audioHandle->streamOutEpOutPipeBusy = 1U;
  }

  return error;
}

/// @note add AudioRec (add streamOut/InInInterfaceHandle)
usb_status_t USB_DeviceAudioFeedbackInfoSend(class_handle_t handle, uint8_t ep, uint8_t *buffer, uint32_t length) {
  usb_device_audio_struct_t *audioHandle;
  usb_status_t error = kStatus_USB_Error;

  if (!handle) {
    return kStatus_USB_InvalidHandle;
  }
  audioHandle = (usb_device_audio_struct_t *)handle;
  if (audioHandle->streamOutEpInPipeBusy) {
    return kStatus_USB_Busy;
  }

  error = USB_DeviceSendRequest(audioHandle->handle, ep, buffer, length);
  if (kStatus_USB_Success == error) {
    audioHandle->streamOutEpInPipeBusy = 1U;
  }
  return error;
}

#endif
