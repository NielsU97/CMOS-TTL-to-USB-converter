/* usbd_desc.h - USB Device Descriptors Header */

#ifndef __USBD_DESC_H
#define __USBD_DESC_H

#ifdef __cplusplus
extern "C" {
#endif

#include "usbd_def.h"

/* USB Device Descriptor */
#define DEVICE_ID1                      (UID_BASE)
#define DEVICE_ID2                      (UID_BASE + 0x4)
#define DEVICE_ID3                      (UID_BASE + 0x8)

#define USB_SIZ_STRING_SERIAL           0x1A

/* Exported variables */
extern USBD_DescriptorsTypeDef VIDEO_Desc;

/* Helper function */
uint8_t USBD_GetLen(uint8_t *buf);

#ifdef __cplusplus
}
#endif