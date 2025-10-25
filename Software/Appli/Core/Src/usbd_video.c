/**
  ******************************************************************************
  * @file    usbd_video.c
  * @brief   This file provides the USB Video Class core functions
  ******************************************************************************
  */

#include "usbd_video.h"
#include "usbd_ctlreq.h"

static uint8_t USBD_VIDEO_Init(USBD_HandleTypeDef *pdev, uint8_t cfgidx);
static uint8_t USBD_VIDEO_DeInit(USBD_HandleTypeDef *pdev, uint8_t cfgidx);
static uint8_t USBD_VIDEO_Setup(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req);
static uint8_t USBD_VIDEO_DataIn(USBD_HandleTypeDef *pdev, uint8_t epnum);
static uint8_t USBD_VIDEO_DataOut(USBD_HandleTypeDef *pdev, uint8_t epnum);
static uint8_t USBD_VIDEO_EP0_RxReady(USBD_HandleTypeDef *pdev);
static uint8_t *USBD_VIDEO_GetFSCfgDesc(uint16_t *length);
static uint8_t *USBD_VIDEO_GetHSCfgDesc(uint16_t *length);
static uint8_t *USBD_VIDEO_GetDeviceQualifierDesc(uint16_t *length);

/* USB Video Class Callbacks */
USBD_ClassTypeDef USBD_VIDEO =
{
  USBD_VIDEO_Init,
  USBD_VIDEO_DeInit,
  USBD_VIDEO_Setup,
  NULL,                 /* EP0_TxSent */
  USBD_VIDEO_EP0_RxReady,
  USBD_VIDEO_DataIn,
  USBD_VIDEO_DataOut,
  NULL,                 /* SOF */
  NULL,                 /* IsoINIncomplete */
  NULL,                 /* IsoOUTIncomplete */
  USBD_VIDEO_GetHSCfgDesc,
  USBD_VIDEO_GetFSCfgDesc,
  USBD_VIDEO_GetFSCfgDesc,
  USBD_VIDEO_GetDeviceQualifierDesc,
};

/* Video Probe and Commit control structures */
static USBD_VideoControlTypeDef videoCommitControl;
static USBD_VideoControlTypeDef videoProbeControl =
{
  .bmHint = 0x0000,
  .bFormatIndex = 0x01,
  .bFrameIndex = 0x01,
  .dwFrameInterval = 333333,  /* 30 fps = 333333 * 100ns */
  .wKeyFrameRate = 0x0000,
  .wPFrameRate = 0x0000,
  .wCompQuality = 0x0000,
  .wCompWindowSize = 0x0000,
  .wDelay = 0x0000,
  .dwMaxVideoFrameSize = VIDEO_FRAME_SIZE,
  .dwMaxPayloadTransferSize = VIDEO_PACKET_SIZE,
  .dwClockFrequency = 0x00000000,
  .bmFramingInfo = 0x00,
  .bPreferedVersion = 0x00,
  .bMinVersion = 0x00,
  .bMaxVersion = 0x00
};

/* USB Video Device Configuration Descriptor */
__ALIGN_BEGIN static uint8_t USBD_VIDEO_CfgDesc[USB_VIDEO_CONFIG_DESC_SIZ] __ALIGN_END =
{
  /* Configuration Descriptor */
  0x09,                                 /* bLength */
  USB_DESC_TYPE_CONFIGURATION,          /* bDescriptorType */
  LOBYTE(USB_VIDEO_CONFIG_DESC_SIZ),    /* wTotalLength */
  HIBYTE(USB_VIDEO_CONFIG_DESC_SIZ),
  0x02,                                 /* bNumInterfaces (Control + Streaming) */
  0x01,                                 /* bConfigurationValue */
  0x00,                                 /* iConfiguration */
  0xC0,                                 /* bmAttributes (Self powered) */
  0xFA,                                 /* bMaxPower (500mA) */

  /* Interface Association Descriptor */
  0x08,                                 /* bLength */
  0x0B,                                 /* bDescriptorType (IAD) */
  0x00,                                 /* bFirstInterface */
  0x02,                                 /* bInterfaceCount */
  0x0E,                                 /* bFunctionClass (Video) */
  0x03,                                 /* bFunctionSubClass (Video Interface Collection) */
  0x00,                                 /* bFunctionProtocol */
  0x00,                                 /* iFunction */

  /* VideoControl Interface Descriptor */
  0x09,                                 /* bLength */
  USB_DESC_TYPE_INTERFACE,              /* bDescriptorType */
  0x00,                                 /* bInterfaceNumber */
  0x00,                                 /* bAlternateSetting */
  0x01,                                 /* bNumEndpoints (Interrupt endpoint) */
  0x0E,                                 /* bInterfaceClass (Video) */
  0x01,                                 /* bInterfaceSubClass (Video Control) */
  0x00,                                 /* bInterfaceProtocol */
  0x00,                                 /* iInterface */

  /* Class-specific VideoControl Interface Descriptor */
  0x0D,                                 /* bLength */
  CS_INTERFACE,                         /* bDescriptorType */
  VC_HEADER,                            /* bDescriptorSubtype */
  LOBYTE(UVC_VERSION),                  /* bcdUVC */
  HIBYTE(UVC_VERSION),
  0x28, 0x00,                           /* wTotalLength of class-specific descriptors */
  0x00, 0x6C, 0xDC, 0x02,              /* dwClockFrequency (48MHz) */
  0x01,                                 /* bInCollection (1 streaming interface) */
  0x01,                                 /* baInterfaceNr(1) */

  /* Input Terminal Descriptor (Camera) */
  0x12,                                 /* bLength */
  CS_INTERFACE,                         /* bDescriptorType */
  VC_INPUT_TERMINAL,                    /* bDescriptorSubtype */
  0x01,                                 /* bTerminalID */
  LOBYTE(ITT_CAMERA),                   /* wTerminalType (Camera) */
  HIBYTE(ITT_CAMERA),
  0x00,                                 /* bAssocTerminal */
  0x00,                                 /* iTerminal */
  0x00, 0x00,                           /* wObjectiveFocalLengthMin */
  0x00, 0x00,                           /* wObjectiveFocalLengthMax */
  0x00, 0x00,                           /* wOcularFocalLength */
  0x03,                                 /* bControlSize */
  0x00, 0x00, 0x00,                     /* bmControls */

  /* Output Terminal Descriptor */
  0x09,                                 /* bLength */
  CS_INTERFACE,                         /* bDescriptorType */
  VC_OUTPUT_TERMINAL,                   /* bDescriptorSubtype */
  0x02,                                 /* bTerminalID */
  LOBYTE(TT_STREAMING),                 /* wTerminalType */
  HIBYTE(TT_STREAMING),
  0x00,                                 /* bAssocTerminal */
  0x01,                                 /* bSourceID (Input Terminal) */
  0x00,                                 /* iTerminal */

  /* Video Control Interrupt Endpoint Descriptor */
  0x07,                                 /* bLength */
  USB_DESC_TYPE_ENDPOINT,               /* bDescriptorType */
  VIDEO_CMD_EP,                         /* bEndpointAddress */
  0x03,                                 /* bmAttributes (Interrupt) */
  0x08, 0x00,                           /* wMaxPacketSize */
  0x10,                                 /* bInterval */

  /* VideoStreaming Interface Descriptor (Alternate Setting 0) */
  0x09,                                 /* bLength */
  USB_DESC_TYPE_INTERFACE,              /* bDescriptorType */
  0x01,                                 /* bInterfaceNumber */
  0x00,                                 /* bAlternateSetting */
  0x00,                                 /* bNumEndpoints */
  0x0E,                                 /* bInterfaceClass (Video) */
  0x02,                                 /* bInterfaceSubClass (Video Streaming) */
  0x00,                                 /* bInterfaceProtocol */
  0x00,                                 /* iInterface */

  /* Class-specific VideoStreaming Header Descriptor */
  0x0E,                                 /* bLength */
  CS_INTERFACE,                         /* bDescriptorType */
  VS_INPUT_HEADER,                      /* bDescriptorSubtype */
  0x01,                                 /* bNumFormats */
  0x47, 0x00,                           /* wTotalLength */
  VIDEO_IN_EP,                          /* bEndpointAddress */
  0x00,                                 /* bmInfo */
  0x02,                                 /* bTerminalLink (Output Terminal) */
  0x00,                                 /* bStillCaptureMethod */
  0x00,                                 /* bTriggerSupport */
  0x00,                                 /* bTriggerUsage */
  0x01,                                 /* bControlSize */
  0x00,                                 /* bmaControls */

  /* Uncompressed Video Format Descriptor */
  0x1B,                                 /* bLength */
  CS_INTERFACE,                         /* bDescriptorType */
  VS_FORMAT_UNCOMPRESSED,               /* bDescriptorSubtype */
  0x01,                                 /* bFormatIndex */
  0x01,                                 /* bNumFrameDescriptors */
  'Y', 'U', 'Y', '2',                   /* guidFormat (YUY2) */
  0x00, 0x00, 0x10, 0x00,
  0x80, 0x00, 0x00, 0xAA,
  0x00, 0x38, 0x9B, 0x71,
  0x10,                                 /* bBitsPerPixel */
  0x01,                                 /* bDefaultFrameIndex */
  0x00,                                 /* bAspectRatioX */
  0x00,                                 /* bAspectRatioY */
  0x00,                                 /* bmInterlaceFlags */
  0x00,                                 /* bCopyProtect */

  /* Uncompressed Video Frame Descriptor (400x400 @ 30fps) */
  0x1E,                                 /* bLength */
  CS_INTERFACE,                         /* bDescriptorType */
  VS_FRAME_UNCOMPRESSED,                /* bDescriptorSubtype */
  0x01,                                 /* bFrameIndex */
  0x00,                                 /* bmCapabilities */
  LOBYTE(VIDEO_WIDTH),                  /* wWidth */
  HIBYTE(VIDEO_WIDTH),
  LOBYTE(VIDEO_HEIGHT),                 /* wHeight */
  HIBYTE(VIDEO_HEIGHT),
  0x00, 0x00, 0x62, 0x0C,              /* dwMinBitRate (400x400x2x30x8) */
  0x00, 0x00, 0x62, 0x0C,              /* dwMaxBitRate */
  LOBYTE(VIDEO_FRAME_SIZE),             /* dwMaxVideoFrameBufferSize */
  HIBYTE(VIDEO_FRAME_SIZE),
  LOBYTE(VIDEO_FRAME_SIZE >> 16),
  HIBYTE(VIDEO_FRAME_SIZE >> 16),
  0x15, 0x16, 0x05, 0x00,              /* dwDefaultFrameInterval (333333 = 30fps) */
  0x01,                                 /* bFrameIntervalType (discrete) */
  0x15, 0x16, 0x05, 0x00,              /* dwFrameInterval (333333) */

  /* VideoStreaming Interface Descriptor (Alternate Setting 1 - Streaming) */
  0x09,                                 /* bLength */
  USB_DESC_TYPE_INTERFACE,              /* bDescriptorType */
  0x01,                                 /* bInterfaceNumber */
  0x01,                                 /* bAlternateSetting */
  0x01,                                 /* bNumEndpoints */
  0x0E,                                 /* bInterfaceClass (Video) */
  0x02,                                 /* bInterfaceSubClass (Video Streaming) */
  0x00,                                 /* bInterfaceProtocol */
  0x00,                                 /* iInterface */

  /* Bulk Endpoint Descriptor */
  0x07,                                 /* bLength */
  USB_DESC_TYPE_ENDPOINT,               /* bDescriptorType */
  VIDEO_IN_EP,                          /* bEndpointAddress */
  0x02,                                 /* bmAttributes (Bulk) */
  LOBYTE(VIDEO_PACKET_SIZE),            /* wMaxPacketSize */
  HIBYTE(VIDEO_PACKET_SIZE),
  0x00,                                 /* bInterval */
};

/* Device Qualifier Descriptor */
__ALIGN_BEGIN static uint8_t USBD_VIDEO_DeviceQualifierDesc[USB_LEN_DEV_QUALIFIER_DESC] __ALIGN_END =
{
  USB_LEN_DEV_QUALIFIER_DESC,
  USB_DESC_TYPE_DEVICE_QUALIFIER,
  0x00, 0x02,
  0x00, 0x00, 0x00,
  0x40,
  0x01,
  0x00,
};

/**
  * @brief  USBD_VIDEO_Init
  * @param  pdev: device instance
  * @param  cfgidx: Configuration index
  * @retval status
  */
static uint8_t USBD_VIDEO_Init(USBD_HandleTypeDef *pdev, uint8_t cfgidx)
{
  USBD_VIDEO_HandleTypeDef *hVideo;

  /* Allocate Video structure */
  hVideo = (USBD_VIDEO_HandleTypeDef *)USBD_malloc(sizeof(USBD_VIDEO_HandleTypeDef));

  if (hVideo == NULL)
  {
    pdev->pClassDataCmsit[pdev->classId] = NULL;
    return (uint8_t)USBD_EMEM;
  }

  pdev->pClassDataCmsit[pdev->classId] = (void *)hVideo;
  pdev->pClassData = pdev->pClassDataCmsit[pdev->classId];

  /* Initialize structure */
  hVideo->streaming_state = 0;
  hVideo->frame_id = 0;
  hVideo->data_length = 0;
  hVideo->bytes_sent = 0;

  /* Open EP IN */
  (void)USBD_LL_OpenEP(pdev, VIDEO_IN_EP, USBD_EP_TYPE_BULK, VIDEO_PACKET_SIZE);
  pdev->ep_in[VIDEO_IN_EP & 0xFU].is_used = 1U;

  /* Open Interrupt EP */
  (void)USBD_LL_OpenEP(pdev, VIDEO_CMD_EP, USBD_EP_TYPE_INTR, 0x08);
  pdev->ep_in[VIDEO_CMD_EP & 0xFU].is_used = 1U;

  /* Initialize the video Interface physical components */
  ((USBD_VIDEO_ItfTypeDef *)pdev->pUserData[pdev->classId])->Init();

  return (uint8_t)USBD_OK;
}

/**
  * @brief  USBD_VIDEO_DeInit
  * @param  pdev: device instance
  * @param  cfgidx: Configuration index
  * @retval status
  */
static uint8_t USBD_VIDEO_DeInit(USBD_HandleTypeDef *pdev, uint8_t cfgidx)
{
  /* Close endpoints */
  (void)USBD_LL_CloseEP(pdev, VIDEO_IN_EP);
  pdev->ep_in[VIDEO_IN_EP & 0xFU].is_used = 0U;

  (void)USBD_LL_CloseEP(pdev, VIDEO_CMD_EP);
  pdev->ep_in[VIDEO_CMD_EP & 0xFU].is_used = 0U;

  /* DeInit physical Interface components */
  if (pdev->pClassDataCmsit[pdev->classId] != NULL)
  {
    ((USBD_VIDEO_ItfTypeDef *)pdev->pUserData[pdev->classId])->DeInit();
    (void)USBD_free(pdev->pClassDataCmsit[pdev->classId]);
    pdev->pClassDataCmsit[pdev->classId] = NULL;
    pdev->pClassData = NULL;
  }

  return (uint8_t)USBD_OK;
}

/**
  * @brief  USBD_VIDEO_Setup
  * @param  pdev: device instance
  * @param  req: usb requests
  * @retval status
  */
static uint8_t USBD_VIDEO_Setup(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{
  USBD_VIDEO_HandleTypeDef *hVideo = (USBD_VIDEO_HandleTypeDef *)pdev->pClassDataCmsit[pdev->classId];
  uint16_t len = 0;
  uint8_t *pbuf = NULL;
  uint16_t status_info = 0U;
  USBD_StatusTypeDef ret = USBD_OK;

  switch (req->bmRequest & USB_REQ_TYPE_MASK)
  {
    case USB_REQ_TYPE_CLASS:
      switch (req->bRequest)
      {
        case SET_CUR:
          if (req->wIndex == 0x0001)  /* VideoStreaming Interface */
          {
            if (req->wValue == (VS_PROBE_CONTROL << 8))
            {
              (void)USBD_CtlPrepareRx(pdev, (uint8_t *)&videoProbeControl, req->wLength);
            }
            else if (req->wValue == (VS_COMMIT_CONTROL << 8))
            {
              (void)USBD_CtlPrepareRx(pdev, (uint8_t *)&videoCommitControl, req->wLength);
            }
          }
          break;

        case GET_CUR:
          if (req->wIndex == 0x0001)  /* VideoStreaming Interface */
          {
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
          }

          if (pbuf != NULL)
          {
            len = MIN(len, req->wLength);
            (void)USBD_CtlSendData(pdev, pbuf, len);
          }
          break;

        case GET_MIN:
        case GET_MAX:
        case GET_DEF:
          if (req->wIndex == 0x0001)  /* VideoStreaming Interface */
          {
            if (req->wValue == (VS_PROBE_CONTROL << 8))
            {
              pbuf = (uint8_t *)&videoProbeControl;
              len = sizeof(videoProbeControl);
            }
          }

          if (pbuf != NULL)
          {
            len = MIN(len, req->wLength);
            (void)USBD_CtlSendData(pdev, pbuf, len);
          }
          break;

        case GET_LEN:
          len = sizeof(videoProbeControl);
          pbuf = (uint8_t *)&len;
          (void)USBD_CtlSendData(pdev, pbuf, 2);
          break;

        case GET_INFO:
          pbuf = (uint8_t *)&status_info;
          status_info = 0x0003;  /* GET/SET supported */
          (void)USBD_CtlSendData(pdev, pbuf, 2);
          break;

        default:
          USBD_CtlError(pdev, req);
          ret = USBD_FAIL;
          break;
      }
      break;

    case USB_REQ_TYPE_STANDARD:
      switch (req->bRequest)
      {
        case USB_REQ_GET_STATUS:
          if (pdev->dev_state == USBD_STATE_CONFIGURED)
          {
            (void)USBD_CtlSendData(pdev, (uint8_t *)&status_info, 2U);
          }
          else
          {
            USBD_CtlError(pdev, req);
            ret = USBD_FAIL;
          }
          break;

        case USB_REQ_GET_INTERFACE:
          if (pdev->dev_state == USBD_STATE_CONFIGURED)
          {
            (void)USBD_CtlSendData(pdev, (uint8_t *)&hVideo->streaming_state, 1U);
          }
          else
          {
            USBD_CtlError(pdev, req);
            ret = USBD_FAIL;
          }
          break;

        case USB_REQ_SET_INTERFACE:
          if (pdev->dev_state == USBD_STATE_CONFIGURED)
          {
            hVideo->streaming_state = (uint8_t)(req->wValue);
          }
          else
          {
            USBD_CtlError(pdev, req);
            ret = USBD_FAIL;
          }
          break;

        case USB_REQ_CLEAR_FEATURE:
          break;

        default:
          USBD_CtlError(pdev, req);
          ret = USBD_FAIL;
          break;
      }
      break;

    default:
      USBD_CtlError(pdev, req);
      ret = USBD_FAIL;
      break;
  }

  return (uint8_t)ret;
}

/**
  * @brief  USBD_VIDEO_DataIn
  * @param  pdev: device instance
  * @param  epnum: endpoint number
  * @retval status
  */
static uint8_t USBD_VIDEO_DataIn(USBD_HandleTypeDef *pdev, uint8_t epnum)
{
  USBD_VIDEO_HandleTypeDef *hVideo = (USBD_VIDEO_HandleTypeDef *)pdev->pClassDataCmsit[pdev->classId];

  if (epnum == (VIDEO_IN_EP & 0x7F))
  {
    /* Continue sending frame data if there's more to send */
    if (hVideo->bytes_sent < hVideo->data_length)
    {
      uint32_t remaining = hVideo->data_length - hVideo->bytes_sent;
      uint32_t packet_size = (remaining > (VIDEO_PACKET_SIZE - UVC_PAYLOAD_HEADER_SIZE)) ?
                             (VIDEO_PACKET_SIZE - UVC_PAYLOAD_HEADER_SIZE) : remaining;

      /* Prepare packet with header */
      uint8_t packet[VIDEO_PACKET_SIZE];

      /* Copy header */
      packet[0] = UVC_PAYLOAD_HEADER_SIZE;
      packet[1] = hVideo->header[1];

      /* Check if this is the last packet */
      if (hVideo->bytes_sent + packet_size >= hVideo->data_length)
      {
        packet[1] |= UVC_HEADER_EOF;  /* Mark end of frame */
      }

      /* Copy data */
      memcpy(&packet[UVC_PAYLOAD_HEADER_SIZE],
             &hVideo->data_buffer[hVideo->bytes_sent],
             packet_size);

      /* Send packet */
      USBD_LL_Transmit(pdev, VIDEO_IN_EP, packet, packet_size + UVC_PAYLOAD_HEADER_SIZE);

      hVideo->bytes_sent += packet_size;
    }
    else
    {
      /* Frame transmission complete, toggle frame ID */
      hVideo->frame_id ^= 1;
      hVideo->bytes_sent = 0;
      hVideo->data_length = 0;
    }
  }

  return (uint8_t)USBD_OK;
}

/**
  * @brief  USBD_VIDEO_DataOut
  * @param  pdev: device instance
  * @param  epnum: endpoint number
  * @retval status
  */
static uint8_t USBD_VIDEO_DataOut(USBD_HandleTypeDef *pdev, uint8_t epnum)
{
  return (uint8_t)USBD_OK;
}

/**
  * @brief  USBD_VIDEO_EP0_RxReady
  * @param  pdev: device instance
  * @retval status
  */
static uint8_t USBD_VIDEO_EP0_RxReady(USBD_HandleTypeDef *pdev)
{
  USBD_VIDEO_HandleTypeDef *hVideo = (USBD_VIDEO_HandleTypeDef *)pdev->pClassDataCmsit[pdev->classId];

  if (hVideo->streaming_state == 1U)
  {
    /* Streaming interface is active */
    ((USBD_VIDEO_ItfTypeDef *)pdev->pUserData[pdev->classId])->Control(0, NULL, 0);
  }

  return (uint8_t)USBD_OK;
}

/**
  * @brief  USBD_VIDEO_GetFSCfgDesc
  * @param  length: pointer to data length
  * @retval pointer to descriptor buffer
  */
static uint8_t *USBD_VIDEO_GetFSCfgDesc(uint16_t *length)
{
  *length = (uint16_t)sizeof(USBD_VIDEO_CfgDesc);
  return USBD_VIDEO_CfgDesc;
}

/**
  * @brief  USBD_VIDEO_GetHSCfgDesc
  * @param  length: pointer to data length
  * @retval pointer to descriptor buffer
  */
static uint8_t *USBD_VIDEO_GetHSCfgDesc(uint16_t *length)
{
  *length = (uint16_t)sizeof(USBD_VIDEO_CfgDesc);
  return USBD_VIDEO_CfgDesc;
}

/**
  * @brief  USBD_VIDEO_GetDeviceQualifierDesc
  * @param  length: pointer to data length
  * @retval pointer to descriptor buffer
  */
static uint8_t *USBD_VIDEO_GetDeviceQualifierDesc(uint16_t *length)
{
  *length = (uint16_t)sizeof(USBD_VIDEO_DeviceQualifierDesc);
  return USBD_VIDEO_DeviceQualifierDesc;
}

/**
  * @brief  USBD_VIDEO_RegisterInterface
  * @param  pdev: device instance
  * @param  fops: Video Interface callback
  * @retval status
  */
uint8_t USBD_VIDEO_RegisterInterface(USBD_HandleTypeDef *pdev, USBD_VIDEO_ItfTypeDef *fops)
{
  if (fops == NULL)
  {
    return (uint8_t)USBD_FAIL;
  }

  pdev->pUserData[pdev->classId] = fops;

  return (uint8_t)USBD_OK;
}

/**
  * @brief  USBD_VIDEO_SendFrame
  * @param  pdev: device instance
  * @param  pbuf: pointer to frame buffer
  * @param  size: frame size
  * @retval status
  */
uint8_t USBD_VIDEO_SendFrame(USBD_HandleTypeDef *pdev, uint8_t *pbuf, uint32_t size)
{
  USBD_VIDEO_HandleTypeDef *hVideo = (USBD_VIDEO_HandleTypeDef *)pdev->pClassDataCmsit[pdev->classId];

  if (hVideo == NULL)
  {
    return (uint8_t)USBD_FAIL;
  }

  /* Check if streaming is active */
  if (hVideo->streaming_state != 1U)
  {
    return (uint8_t)USBD_BUSY;
  }

  /* Check if previous frame is still being transmitted */
  if (hVideo->data_length != 0)
  {
    return (uint8_t)USBD_BUSY;
  }

  /* Store frame information */
  hVideo->data_buffer = pbuf;
  hVideo->data_length = size;
  hVideo->bytes_sent = 0;

  /* Prepare header */
  hVideo->header[0] = UVC_PAYLOAD_HEADER_SIZE;
  hVideo->header[1] = UVC_HEADER_FID;

  if (hVideo->frame_id)
  {
    hVideo->header[1] |= UVC_HEADER_FID;
  }

  /* Send first packet */
  uint32_t first_packet_size = (size > (VIDEO_PACKET_SIZE - UVC_PAYLOAD_HEADER_SIZE)) ?
                                (VIDEO_PACKET_SIZE - UVC_PAYLOAD_HEADER_SIZE) : size;

  uint8_t packet[VIDEO_PACKET_SIZE];

  /* Copy header */
  packet[0] = hVideo->header[0];
  packet[1] = hVideo->header[1];

  /* Check if entire frame fits in one packet */
  if (first_packet_size >= size)
  {
    packet[1] |= UVC_HEADER_EOF;  /* Mark end of frame */
  }

  /* Copy data */
  memcpy(&packet[UVC_PAYLOAD_HEADER_SIZE], pbuf, first_packet_size);

  /* Transmit */
  USBD_LL_Transmit(pdev, VIDEO_IN_EP, packet, first_packet_size + UVC_PAYLOAD_HEADER_SIZE);

  hVideo->bytes_sent = first_packet_size;

  return (uint8_t)USBD_OK;
}
