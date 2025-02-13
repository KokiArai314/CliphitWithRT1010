/*
 * usb_ehci.h
 *
 *  Created on: 2020/02/06
 *      Author: higuchi
 */

#ifndef DEVICE_SOURCE_EHCI_USB_EHCI_H_
#define DEVICE_SOURCE_EHCI_USB_EHCI_H_

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/* Device QH */
#define USB_DEVICE_EHCI_QH_POINTER_MASK (0xFFFFFFC0U)
#define USB_DEVICE_EHCI_QH_MULT_MASK (0xC0000000U)
#define USB_DEVICE_EHCI_QH_ZLT_MASK (0x20000000U)
#define USB_DEVICE_EHCI_QH_MAX_PACKET_SIZE_MASK (0x07FF0000U)
#define USB_DEVICE_EHCI_QH_MAX_PACKET_SIZE (0x00000800U)
#define USB_DEVICE_EHCI_QH_IOS_MASK (0x00008000U)

/* Device DTD */
#define USB_DEVICE_ECHI_DTD_POINTER_MASK (0xFFFFFFE0U)
#define USB_DEVICE_ECHI_DTD_TERMINATE_MASK (0x00000001U)
#define USB_DEVICE_ECHI_DTD_PAGE_MASK (0xFFFFF000U)
#define USB_DEVICE_ECHI_DTD_PAGE_OFFSET_MASK (0x00000FFFU)
#define USB_DEVICE_ECHI_DTD_PAGE_BLOCK (0x00001000U)
#define USB_DEVICE_ECHI_DTD_TOTAL_BYTES_MASK (0x7FFF0000U)
#define USB_DEVICE_ECHI_DTD_TOTAL_BYTES (0x00004000U)
#define USB_DEVICE_ECHI_DTD_IOC_MASK (0x00008000U)
#define USB_DEVICE_ECHI_DTD_MULTIO_MASK (0x00000C00U)
#define USB_DEVICE_ECHI_DTD_STATUS_MASK (0x000000FFU)
#define USB_DEVICE_EHCI_DTD_STATUS_ERROR_MASK (0x00000068U)
#define USB_DEVICE_ECHI_DTD_STATUS_ACTIVE (0x00000080U)
#define USB_DEVICE_ECHI_DTD_STATUS_HALTED (0x00000040U)
#define USB_DEVICE_ECHI_DTD_STATUS_DATA_BUFFER_ERROR (0x00000020U)
#define USB_DEVICE_ECHI_DTD_STATUS_TRANSACTION_ERROR (0x00000008U)

typedef struct _usb_device_ehci_qh_struct
{
    union
    {
        volatile uint32_t capabilttiesCharacteristics;
        struct
        {
            volatile uint32_t reserved1 : 15;
            volatile uint32_t ios : 1;
            volatile uint32_t maxPacketSize : 11;
            volatile uint32_t reserved2 : 2;
            volatile uint32_t zlt : 1;
            volatile uint32_t mult : 2;
        } capabilttiesCharacteristicsBitmap;
    } capabilttiesCharacteristicsUnion;
    volatile uint32_t currentDtdPointer;
    volatile uint32_t nextDtdPointer;
    union
    {
        volatile uint32_t dtdToken;
        struct
        {
            volatile uint32_t status : 8;
            volatile uint32_t reserved1 : 2;
            volatile uint32_t multiplierOverride : 2;
            volatile uint32_t reserved2 : 3;
            volatile uint32_t ioc : 1;
            volatile uint32_t totalBytes : 15;
            volatile uint32_t reserved3 : 1;
        } dtdTokenBitmap;
    } dtdTokenUnion;
    volatile uint32_t bufferPointerPage[5];
    volatile uint32_t reserved1;
    uint32_t setupBuffer[2];
    uint32_t setupBufferBack[2];
    union
    {
        uint32_t endpointStatus;
        struct
        {
            uint32_t isOpened : 1;
            uint32_t : 31;
        } endpointStatusBitmap;
    } endpointStatusUnion;
    uint32_t reserved2;
} usb_device_ehci_qh_struct_t;

typedef struct _usb_device_ehci_dtd_struct
{
    volatile uint32_t nextDtdPointer;
    union
    {
        volatile uint32_t dtdToken;
        struct
        {
            volatile uint32_t status : 8;
            volatile uint32_t reserved1 : 2;
            volatile uint32_t multiplierOverride : 2;
            volatile uint32_t reserved2 : 3;
            volatile uint32_t ioc : 1;
            volatile uint32_t totalBytes : 15;
            volatile uint32_t reserved3 : 1;
        } dtdTokenBitmap;
    } dtdTokenUnion;
    volatile uint32_t bufferPointerPage[5];
    union
    {
        volatile uint32_t reserved;
        struct
        {
            uint32_t originalBufferOffest : 12;
            uint32_t originalBufferLength : 19;
            uint32_t dtdInvalid : 1;
        } originalBufferInfo;
    } reservedUnion;
} usb_device_ehci_dtd_struct_t;

#endif /* DEVICE_SOURCE_EHCI_USB_EHCI_H_ */
