#include "usbd_core.h"
#include "usbd_desc.h"
#include "usbd_conf.h"

/* USB Standard Device Descriptor */
__ALIGN_BEGIN uint8_t USBD_HS_DeviceDesc[USB_LEN_DEV_DESC] __ALIGN_END = {
    0x12,                       /* bLength */
    USB_DESC_TYPE_DEVICE,       /* bDescriptorType */
    0x00, 0x02,                /* bcdUSB */
    0xEF,                       /* bDeviceClass (Miscellaneous) */
    0x02,                       /* bDeviceSubClass */
    0x01,                       /* bDeviceProtocol (Interface Association) */
    USB_MAX_EP0_SIZE,          /* bMaxPacketSize */
    LOBYTE(USBD_VID),          /* idVendor */
    HIBYTE(USBD_VID),
    LOBYTE(USBD_PID),          /* idProduct */
    HIBYTE(USBD_PID),
    0x00, 0x02,                /* bcdDevice rel. 2.00 */
    USBD_IDX_MFC_STR,          /* Index of manufacturer string */
    USBD_IDX_PRODUCT_STR,      /* Index of product string */
    USBD_IDX_SERIAL_STR,       /* Index of serial number string */
    USBD_MAX_NUM_CONFIGURATION /* bNumConfigurations */
};

/* USB Standard String Descriptor */
#define USBD_LANGID_STRING              0x409  /* US English */

/* Manufacturer String */
__ALIGN_BEGIN uint8_t USBD_StrDesc[USBD_MAX_STR_DESC_SIZ] __ALIGN_END;

static uint8_t *USBD_VIDEO_DeviceDescriptor(USBD_SpeedTypeDef speed, uint16_t *length);
static uint8_t *USBD_VIDEO_LangIDStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *length);
static uint8_t *USBD_VIDEO_ManufacturerStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *length);
static uint8_t *USBD_VIDEO_ProductStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *length);
static uint8_t *USBD_VIDEO_SerialStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *length);

USBD_DescriptorsTypeDef VIDEO_Desc = {
    USBD_VIDEO_DeviceDescriptor,
    USBD_VIDEO_LangIDStrDescriptor,
    USBD_VIDEO_ManufacturerStrDescriptor,
    USBD_VIDEO_ProductStrDescriptor,
    USBD_VIDEO_SerialStrDescriptor,
    NULL,
    NULL,
};

/* USB Standard Device Descriptor */
static uint8_t *USBD_VIDEO_DeviceDescriptor(USBD_SpeedTypeDef speed, uint16_t *length)
{
    *length = sizeof(USBD_HS_DeviceDesc);
    return USBD_HS_DeviceDesc;
}

/* USB Standard Language ID Descriptor */
__ALIGN_BEGIN static uint8_t USBD_LangIDDesc[USB_LEN_LANGID_STR_DESC] __ALIGN_END = {
    USB_LEN_LANGID_STR_DESC,
    USB_DESC_TYPE_STRING,
    LOBYTE(USBD_LANGID_STRING),
    HIBYTE(USBD_LANGID_STRING)
};

static uint8_t *USBD_VIDEO_LangIDStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *length)
{
    *length = sizeof(USBD_LangIDDesc);
    return USBD_LangIDDesc;
}

/* Internal string conversion */
void USBD_GetString(uint8_t *desc, uint8_t *unicode, uint16_t *len)
{
    uint8_t idx = 0;
    
    if (desc != NULL)
    {
        *len = USBD_GetLen(desc) * 2 + 2;
        unicode[idx++] = *len;
        unicode[idx++] = USB_DESC_TYPE_STRING;
        
        while (*desc != '\0')
        {
            unicode[idx++] = *desc++;
            unicode[idx++] = 0x00;
        }
    }
}

static uint8_t *USBD_VIDEO_ManufacturerStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *length)
{
    USBD_GetString((uint8_t *)"STMicroelectronics", USBD_StrDesc, length);
    return USBD_StrDesc;
}

static uint8_t *USBD_VIDEO_ProductStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *length)
{
    USBD_GetString((uint8_t *)"STM32 UVC Camera", USBD_StrDesc, length);
    return USBD_StrDesc;
}

static uint8_t *USBD_VIDEO_SerialStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *length)
{
    USBD_GetString((uint8_t *)"00000000001A", USBD_StrDesc, length);
    return USBD_StrDesc;
}
