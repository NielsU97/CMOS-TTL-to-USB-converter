* usbd_video.h - USB UVC Video Class Header */

#ifndef __USBD_VIDEO_H
#define __USBD_VIDEO_H

#ifdef __cplusplus
extern "C" {
#endif

#include "usbd_ioreq.h"

/* USB Video Device Configuration */
#define USBD_VID                        0x0483  /* STMicroelectronics */
#define USBD_PID                        0x5740  /* UVC Camera */
#define USBD_MAX_NUM_CONFIGURATION      1
#define USBD_MAX_NUM_INTERFACES         2
#define USB_MAX_EP0_SIZE                64

/* String Descriptors */
#define USBD_IDX_MFC_STR                1
#define USBD_IDX_PRODUCT_STR            2
#define USBD_IDX_SERIAL_STR             3

/* Configuration Descriptor Size */
#define USB_VIDEO_CONFIG_DESC_SIZ       155

/* Video Class Interface Descriptor */
extern USBD_ClassTypeDef USBD_VIDEO;
extern USBD_DescriptorsTypeDef VIDEO_Desc;

/* Video Class Functions */
void USBD_VIDEO_StartStreaming(USBD_HandleTypeDef *pdev);

#ifdef __cplusplus
}
#endif

#endif /* __USBD_VIDEO_H */

