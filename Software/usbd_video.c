/* usbd_video.c - USB UVC Video Class Implementation */

#include "usbd_video.h"
#include "usbd_ctlreq.h"

#define CAM_WIDTH           400
#define CAM_HEIGHT          400
#define CAM_FPS             30
#define CAM_FRAME_SIZE      (CAM_WIDTH * CAM_HEIGHT * 2)  // YUY2

/* USB Video Device Class Codes */
#define USB_DEVICE_CLASS_VIDEO              0x0E
#define USB_VIDEO_CLASS_INTERFACE           0x0E
#define USB_VIDEO_SUBCLASS_CONTROL          0x01
#define USB_VIDEO_SUBCLASS_STREAMING        0x02
#define USB_VIDEO_PROTOCOL_UNDEFINED        0x00

/* Video Interface Subclass Codes */
#define VC_DESCRIPTOR_UNDEFINED             0x00
#define VC_HEADER                           0x01
#define VC_INPUT_TERMINAL                   0x02
#define VC_OUTPUT_TERMINAL                  0x03
#define VC_SELECTOR_UNIT                    0x04
#define VC_PROCESSING_UNIT                  0x05

/* Video Class-Specific Descriptor Types */
#define CS_UNDEFINED                        0x20
#define CS_DEVICE                           0x21
#define CS_CONFIGURATION                    0x22
#define CS_STRING                           0x23
#define CS_INTERFACE                        0x24
#define CS_ENDPOINT                         0x25

/* Video Streaming Interface Descriptor Subtypes */
#define VS_UNDEFINED                        0x00
#define VS_INPUT_HEADER                     0x01
#define VS_OUTPUT_HEADER                    0x02
#define VS_STILL_IMAGE_FRAME                0x03
#define VS_FORMAT_UNCOMPRESSED              0x04
#define VS_FRAME_UNCOMPRESSED               0x05
#define VS_FORMAT_MJPEG                     0x06
#define VS_FRAME_MJPEG                      0x07
#define VS_COLORFORMAT                      0x0D

/* Terminal Types */
#define ITT_CAMERA                          0x0201
#define OTT_STREAMING                       0x0101

/* UVC Request Codes */
#define UVC_SET_CUR                         0x01
#define UVC_GET_CUR                         0x81
#define UVC_GET_MIN                         0x82
#define UVC_GET_MAX                         0x83
#define UVC_GET_RES                         0x84
#define UVC_GET_LEN                         0x85
#define UVC_GET_INFO                        0x86
#define UVC_GET_DEF                         0x87

/* VideoControl Interface Control Selectors */
#define VC_CONTROL_UNDEFINED                0x00
#define VC_VIDEO_POWER_MODE_CONTROL         0x01

/* VideoStreaming Interface Control Selectors */
#define VS_CONTROL_UNDEFINED                0x00
#define VS_PROBE_CONTROL                    0x01
#define VS_COMMIT_CONTROL                   0x02

/* Video Probe and Commit Controls */
typedef struct {
    uint16_t bmHint;
    uint8_t  bFormatIndex;
    uint8_t  bFrameIndex;
    uint32_t dwFrameInterval;
    uint16_t wKeyFrameRate;
    uint16_t wPFrameRate;
    uint16_t wCompQuality;
    uint16_t wCompWindowSize;
    uint16_t wDelay;
    uint32_t dwMaxVideoFrameSize;
    uint32_t dwMaxPayloadTransferSize;
} __attribute__((packed)) VideoControl_t;

static VideoControl_t videoCommitControl = {
    .bmHint = 0x0001,
    .bFormatIndex = 1,
    .bFrameIndex = 1,
    .dwFrameInterval = 333333,  // 30 fps (100ns units)
    .wKeyFrameRate = 0,
    .wPFrameRate = 0,
    .wCompQuality = 0,
    .wCompWindowSize = 0,
    .wDelay = 0,
    .dwMaxVideoFrameSize = CAM_FRAME_SIZE,
    .dwMaxPayloadTransferSize = 512
};

static VideoControl_t videoProbeControl = {
    .bmHint = 0x0001,
    .bFormatIndex = 1,
    .bFrameIndex = 1,
    .dwFrameInterval = 333333,
    .wKeyFrameRate = 0,
    .wPFrameRate = 0,
    .wCompQuality = 0,
    .wCompWindowSize = 0,
    .wDelay = 0,
    .dwMaxVideoFrameSize = CAM_FRAME_SIZE,
    .dwMaxPayloadTransferSize = 512
};

/* UVC Configuration Descriptor */
__ALIGN_BEGIN static uint8_t USBD_VIDEO_Init(USBD_HandleTypeDef *pdev, uint8_t cfgidx)
{
    /* Open EP IN */
    USBD_LL_OpenEP(pdev, 0x81, USBD_EP_TYPE_BULK, 512);
    pdev->ep_in[0x81 & 0xFU].is_used = 1U;
    
    return USBD_OK;
}

static uint8_t USBD_VIDEO_DeInit(USBD_HandleTypeDef *pdev, uint8_t cfgidx)
{
    /* Close EP IN */
    USBD_LL_CloseEP(pdev, 0x81);
    pdev->ep_in[0x81 & 0xFU].is_used = 0U;
    
    return USBD_OK;
}

static uint8_t USBD_VIDEO_Setup(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{
    uint16_t len = 0;
    uint8_t *pbuf = NULL;
    uint16_t status_info = 0;
    
    switch (req->bmRequest & USB_REQ_TYPE_MASK)
    {
        case USB_REQ_TYPE_CLASS:
            switch (req->bRequest)
            {
                case UVC_GET_CUR:
                case UVC_GET_DEF:
                case UVC_GET_MIN:
                case UVC_GET_MAX:
                    if (req->wValue == (VS_PROBE_CONTROL << 8))
                    {
                        pbuf = (uint8_t *)&videoProbeControl;
                        len = sizeof(videoProbeControl);
                    }
                    else if (req->wValue == (VS_COMMIT_CONTROL << 8))
                    {
                        pbuf = (uint8_t *)&videoCommitControl;
                        len = sizeof(videoCommitControl);
                    }
                    
                    if (len > 0)
                    {
                        len = MIN(len, req->wLength);
                        USBD_CtlSendData(pdev, pbuf, len);
                    }
                    break;
                    
                case UVC_SET_CUR:
                    if (req->wValue == (VS_PROBE_CONTROL << 8))
                    {
                        pbuf = (uint8_t *)&videoProbeControl;
                        len = sizeof(videoProbeControl);
                    }
                    else if (req->wValue == (VS_COMMIT_CONTROL << 8))
                    {
                        pbuf = (uint8_t *)&videoCommitControl;
                        len = sizeof(videoCommitControl);
                    }
                    
                    if (len > 0)
                    {
                        len = MIN(len, req->wLength);
                        USBD_CtlPrepareRx(pdev, pbuf, len);
                    }
                    break;
                    
                case UVC_GET_LEN:
                    pbuf = (uint8_t *)&len;
                    len = sizeof(videoProbeControl);
                    USBD_CtlSendData(pdev, pbuf, 2);
                    break;
                    
                case UVC_GET_INFO:
                    status_info = 0x03;  /* GET/SET supported */
                    USBD_CtlSendData(pdev, (uint8_t *)&status_info, 2);
                    break;
                    
                default:
                    USBD_CtlError(pdev, req);
                    return USBD_FAIL;
            }
            break;
            
        case USB_REQ_TYPE_STANDARD:
            switch (req->bRequest)
            {
                case USB_REQ_GET_DESCRIPTOR:
                    if ((req->wValue >> 8) == CS_DEVICE)
                    {
                        pbuf = USBD_VIDEO_DeviceDesc;
                        len = MIN(USB_LEN_DEV_DESC, req->wLength);
                        USBD_CtlSendData(pdev, pbuf, len);
                    }
                    break;
                    
                case USB_REQ_GET_INTERFACE:
                    USBD_CtlSendData(pdev, (uint8_t *)&status_info, 1);
                    break;
                    
                case USB_REQ_SET_INTERFACE:
                    /* Handle alternate setting if needed */
                    break;
                    
                default:
                    USBD_CtlError(pdev, req);
                    return USBD_FAIL;
            }
            break;
            
        default:
            USBD_CtlError(pdev, req);
            return USBD_FAIL;
    }
    
    return USBD_OK;
}

/* Video streaming state */
static uint8_t streaming_active = 0;
static uint32_t frame_offset = 0;
static uint8_t payload_header[2] = {0x02, 0x00};  // Header length, no flags

static uint8_t USBD_VIDEO_DataIn(USBD_HandleTypeDef *pdev, uint8_t epnum)
{
    if (epnum == 0x81)
    {
        uint8_t *frame_buffer = USB_GetFrameBuffer();
        
        if (frame_buffer != NULL && streaming_active)
        {
            uint32_t remaining = CAM_FRAME_SIZE - frame_offset;
            uint32_t chunk_size = (remaining > 510) ? 510 : remaining;  // 512 - 2 byte header
            
            uint8_t packet[512];
            
            /* Add payload header */
            packet[0] = payload_header[0];
            packet[1] = payload_header[1];
            
            /* Copy frame data */
            memcpy(&packet[2], &frame_buffer[frame_offset], chunk_size);
            
            frame_offset += chunk_size;
            
            /* Check if frame is complete */
            if (frame_offset >= CAM_FRAME_SIZE)
            {
                payload_header[1] |= 0x02;  // Set End of Frame flag
                frame_offset = 0;
                USB_FrameSent();
                streaming_active = 0;
            }
            else
            {
                payload_header[1] &= ~0x02;  // Clear End of Frame flag
            }
            
            /* Toggle Frame ID bit */
            payload_header[1] ^= 0x01;
            
            /* Send next packet */
            USBD_LL_Transmit(pdev, 0x81, packet, chunk_size + 2);
        }
        else
        {
            /* Check for new frame */
            frame_buffer = USB_GetFrameBuffer();
            if (frame_buffer != NULL)
            {
                streaming_active = 1;
                frame_offset = 0;
                
                /* Start transmission */
                uint8_t packet[512];
                packet[0] = payload_header[0];
                packet[1] = payload_header[1] & ~0x02;  // Clear EOF
                
                uint32_t chunk_size = (CAM_FRAME_SIZE > 510) ? 510 : CAM_FRAME_SIZE;
                memcpy(&packet[2], frame_buffer, chunk_size);
                
                frame_offset = chunk_size;
                payload_header[1] ^= 0x01;  // Toggle Frame ID
                
                USBD_LL_Transmit(pdev, 0x81, packet, chunk_size + 2);
            }
        }
    }
    
    return USBD_OK;
}

static uint8_t *USBD_VIDEO_GetCfgDesc(uint16_t *length)
{
    *length = sizeof(USBD_VIDEO_CfgDesc);
    return USBD_VIDEO_CfgDesc;
}

/* Start video streaming */
void USBD_VIDEO_StartStreaming(USBD_HandleTypeDef *pdev)
{
    streaming_active = 0;
    frame_offset = 0;
    
    /* Trigger first transmission */
    USBD_VIDEO_DataIn(pdev, 0x81);
}BD_VIDEO_CfgDesc[USB_VIDEO_CONFIG_DESC_SIZ] __ALIGN_END = {
    /* Configuration Descriptor */
    0x09,                               /* bLength */
    USB_DESC_TYPE_CONFIGURATION,        /* bDescriptorType */
    LOBYTE(USB_VIDEO_CONFIG_DESC_SIZ),  /* wTotalLength */
    HIBYTE(USB_VIDEO_CONFIG_DESC_SIZ),
    0x02,                               /* bNumInterfaces */
    0x01,                               /* bConfigurationValue */
    0x00,                               /* iConfiguration */
    0xC0,                               /* bmAttributes (Self-powered) */
    0xFA,                               /* bMaxPower (500mA) */

    /* Interface Association Descriptor */
    0x08,                               /* bLength */
    0x0B,                               /* bDescriptorType (IAD) */
    0x00,                               /* bFirstInterface */
    0x02,                               /* bInterfaceCount */
    USB_DEVICE_CLASS_VIDEO,             /* bFunctionClass */
    USB_VIDEO_SUBCLASS_CONTROL,         /* bFunctionSubClass */
    0x00,                               /* bFunctionProtocol */
    0x00,                               /* iFunction */

    /* VideoControl Interface Descriptor */
    0x09,                               /* bLength */
    USB_DESC_TYPE_INTERFACE,            /* bDescriptorType */
    0x00,                               /* bInterfaceNumber */
    0x00,                               /* bAlternateSetting */
    0x00,                               /* bNumEndpoints */
    USB_VIDEO_CLASS_INTERFACE,          /* bInterfaceClass */
    USB_VIDEO_SUBCLASS_CONTROL,         /* bInterfaceSubClass */
    USB_VIDEO_PROTOCOL_UNDEFINED,       /* bInterfaceProtocol */
    0x00,                               /* iInterface */

    /* Class-specific VC Interface Header Descriptor */
    0x0D,                               /* bLength */
    CS_INTERFACE,                       /* bDescriptorType */
    VC_HEADER,                          /* bDescriptorSubtype */
    0x00, 0x01,                         /* bcdUVC (1.0) */
    0x28, 0x00,                         /* wTotalLength */
    0x00, 0x6C, 0xDC, 0x02,            /* dwClockFrequency (48MHz) */
    0x01,                               /* bInCollection */
    0x01,                               /* baInterfaceNr(1) */

    /* Input Terminal Descriptor (Camera) */
    0x11,                               /* bLength */
    CS_INTERFACE,                       /* bDescriptorType */
    VC_INPUT_TERMINAL,                  /* bDescriptorSubtype */
    0x01,                               /* bTerminalID */
    LOBYTE(ITT_CAMERA),                 /* wTerminalType */
    HIBYTE(ITT_CAMERA),
    0x00,                               /* bAssocTerminal */
    0x00,                               /* iTerminal */
    0x00, 0x00,                         /* wObjectiveFocalLengthMin */
    0x00, 0x00,                         /* wObjectiveFocalLengthMax */
    0x00, 0x00,                         /* wOcularFocalLength */
    0x02,                               /* bControlSize */
    0x00, 0x00,                         /* bmControls */

    /* Output Terminal Descriptor */
    0x09,                               /* bLength */
    CS_INTERFACE,                       /* bDescriptorType */
    VC_OUTPUT_TERMINAL,                 /* bDescriptorSubtype */
    0x02,                               /* bTerminalID */
    LOBYTE(OTT_STREAMING),              /* wTerminalType */
    HIBYTE(OTT_STREAMING),
    0x00,                               /* bAssocTerminal */
    0x01,                               /* bSourceID */
    0x00,                               /* iTerminal */

    /* VideoStreaming Interface Descriptor (Alternate Setting 0) */
    0x09,                               /* bLength */
    USB_DESC_TYPE_INTERFACE,            /* bDescriptorType */
    0x01,                               /* bInterfaceNumber */
    0x00,                               /* bAlternateSetting */
    0x01,                               /* bNumEndpoints */
    USB_VIDEO_CLASS_INTERFACE,          /* bInterfaceClass */
    USB_VIDEO_SUBCLASS_STREAMING,       /* bInterfaceSubClass */
    0x00,                               /* bInterfaceProtocol */
    0x00,                               /* iInterface */

    /* Class-specific VS Interface Input Header Descriptor */
    0x0E,                               /* bLength */
    CS_INTERFACE,                       /* bDescriptorType */
    VS_INPUT_HEADER,                    /* bDescriptorSubtype */
    0x01,                               /* bNumFormats */
    0x47, 0x00,                         /* wTotalLength */
    0x81,                               /* bEndpointAddress */
    0x00,                               /* bmInfo */
    0x02,                               /* bTerminalLink */
    0x00,                               /* bStillCaptureMethod */
    0x00,                               /* bTriggerSupport */
    0x00,                               /* bTriggerUsage */
    0x01,                               /* bControlSize */
    0x00,                               /* bmaControls(0) */

    /* VS Format Descriptor (YUY2) */
    0x1B,                               /* bLength */
    CS_INTERFACE,                       /* bDescriptorType */
    VS_FORMAT_UNCOMPRESSED,             /* bDescriptorSubtype */
    0x01,                               /* bFormatIndex */
    0x01,                               /* bNumFrameDescriptors */
    'Y', 'U', 'Y', '2',                /* guidFormat (YUY2) */
    0x00, 0x00, 0x10, 0x00,
    0x80, 0x00, 0x00, 0xAA,
    0x00, 0x38, 0x9B, 0x71,
    0x10,                               /* bBitsPerPixel */
    0x01,                               /* bDefaultFrameIndex */
    0x00,                               /* bAspectRatioX */
    0x00,                               /* bAspectRatioY */
    0x00,                               /* bmInterlaceFlags */
    0x00,                               /* bCopyProtect */

    /* VS Frame Descriptor (400x400 @ 30fps) */
    0x1E,                               /* bLength */
    CS_INTERFACE,                       /* bDescriptorType */
    VS_FRAME_UNCOMPRESSED,              /* bDescriptorSubtype */
    0x01,                               /* bFrameIndex */
    0x00,                               /* bmCapabilities */
    LOBYTE(CAM_WIDTH),                  /* wWidth */
    HIBYTE(CAM_WIDTH),
    LOBYTE(CAM_HEIGHT),                 /* wHeight */
    HIBYTE(CAM_HEIGHT),
    0x00, 0x00, 0x65, 0x04,            /* dwMinBitRate */
    0x00, 0x00, 0x65, 0x04,            /* dwMaxBitRate */
    LOBYTE(CAM_FRAME_SIZE),             /* dwMaxVideoFrameBufferSize */
    HIBYTE(CAM_FRAME_SIZE),
    (CAM_FRAME_SIZE >> 16) & 0xFF,
    (CAM_FRAME_SIZE >> 24) & 0xFF,
    0x15, 0x16, 0x05, 0x00,            /* dwDefaultFrameInterval (30fps) */
    0x01,                               /* bFrameIntervalType */
    0x15, 0x16, 0x05, 0x00,            /* dwFrameInterval(1) */

    /* Endpoint Descriptor (Bulk IN) */
    0x07,                               /* bLength */
    USB_DESC_TYPE_ENDPOINT,             /* bDescriptorType */
    0x81,                               /* bEndpointAddress (IN) */
    0x02,                               /* bmAttributes (Bulk) */
    0x00, 0x02,                         /* wMaxPacketSize (512) */
    0x01,                               /* bInterval */
};

/* USB Device Descriptor */
__ALIGN_BEGIN static uint8_t USBD_VIDEO_DeviceDesc[USB_LEN_DEV_DESC] __ALIGN_END = {
    0x12,                       /* bLength */
    USB_DESC_TYPE_DEVICE,       /* bDescriptorType */
    0x00, 0x02,                /* bcdUSB (2.0) */
    0xEF,                       /* bDeviceClass (Miscellaneous) */
    0x02,                       /* bDeviceSubClass */
    0x01,                       /* bDeviceProtocol */
    USB_MAX_EP0_SIZE,          /* bMaxPacketSize */
    LOBYTE(USBD_VID),          /* idVendor */
    HIBYTE(USBD_VID),
    LOBYTE(USBD_PID),          /* idProduct */
    HIBYTE(USBD_PID),
    0x00, 0x02,                /* bcdDevice */
    USBD_IDX_MFC_STR,          /* iManufacturer */
    USBD_IDX_PRODUCT_STR,      /* iProduct */
    USBD_IDX_SERIAL_STR,       /* iSerialNumber */
    USBD_MAX_NUM_CONFIGURATION /* bNumConfigurations */
};

/* Private function prototypes */
static uint8_t USBD_VIDEO_Init(USBD_HandleTypeDef *pdev, uint8_t cfgidx);
static uint8_t USBD_VIDEO_DeInit(USBD_HandleTypeDef *pdev, uint8_t cfgidx);
static uint8_t USBD_VIDEO_Setup(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req);
static uint8_t USBD_VIDEO_DataIn(USBD_HandleTypeDef *pdev, uint8_t epnum);
static uint8_t *USBD_VIDEO_GetCfgDesc(uint16_t *length);

/* External functions */
extern uint8_t* USB_GetFrameBuffer(void);
extern void USB_FrameSent(void);

/* USB Video Class callbacks */
USBD_ClassTypeDef USBD_VIDEO = {
    USBD_VIDEO_Init,
    USBD_VIDEO_DeInit,
    USBD_VIDEO_Setup,
    NULL,                   /* EP0_TxSent */
    NULL,                   /* EP0_RxReady */
    USBD_VIDEO_DataIn,
    NULL,                   /* DataOut */
    NULL,                   /* SOF */
    NULL,                   /* IsoINIncomplete */
    NULL,                   /* IsoOUTIncomplete */
    USBD_VIDEO_GetCfgDesc,
    USBD_VIDEO_GetCfgDesc,
    USBD_VIDEO_GetCfgDesc,
    NULL,
};

static uint8_t US