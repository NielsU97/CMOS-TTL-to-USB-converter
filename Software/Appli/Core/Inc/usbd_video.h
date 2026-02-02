/**
  ******************************************************************************
  * @file    usbd_video.h
  * @brief   Header file for the usbd_video.c
  ******************************************************************************
  */

#ifndef __USBD_VIDEO_H
#define __USBD_VIDEO_H

#ifdef __cplusplus
extern "C" {
#endif

#include "usbd_ioreq.h"

/* UVC Version */
#define UVC_VERSION                                  0x0150  /* UVC 1.5 */

/* Video Interface Subclass Codes */
#define SC_UNDEFINED                                 0x00
#define SC_VIDEOCONTROL                              0x01
#define SC_VIDEOSTREAMING                            0x02
#define SC_VIDEO_INTERFACE_COLLECTION                0x03

/* Video Interface Protocol Codes */
#define PC_PROTOCOL_UNDEFINED                        0x00
#define PC_PROTOCOL_15                               0x01

/* Video Class-Specific Descriptor Types */
#define CS_UNDEFINED                                 0x20
#define CS_DEVICE                                    0x21
#define CS_CONFIGURATION                             0x22
#define CS_STRING                                    0x23
#define CS_INTERFACE                                 0x24
#define CS_ENDPOINT                                  0x25

/* Video Control Interface Descriptor Subtypes */
#define VC_DESCRIPTOR_UNDEFINED                      0x00
#define VC_HEADER                                    0x01
#define VC_INPUT_TERMINAL                            0x02
#define VC_OUTPUT_TERMINAL                           0x03
#define VC_SELECTOR_UNIT                             0x04
#define VC_PROCESSING_UNIT                           0x05
#define VC_EXTENSION_UNIT                            0x06
#define VC_ENCODING_UNIT                             0x07

/* Video Streaming Interface Descriptor Subtypes */
#define VS_UNDEFINED                                 0x00
#define VS_INPUT_HEADER                              0x01
#define VS_OUTPUT_HEADER                             0x02
#define VS_STILL_IMAGE_FRAME                         0x03
#define VS_FORMAT_UNCOMPRESSED                       0x04
#define VS_FRAME_UNCOMPRESSED                        0x05
#define VS_FORMAT_MJPEG                              0x06
#define VS_FRAME_MJPEG                               0x07
#define VS_FORMAT_MPEG2TS                            0x0A
#define VS_FORMAT_DV                                 0x0C
#define VS_COLORFORMAT                               0x0D
#define VS_FORMAT_FRAME_BASED                        0x10
#define VS_FRAME_FRAME_BASED                         0x11
#define VS_FORMAT_STREAM_BASED                       0x12

/* Terminal Types */
#define TT_VENDOR_SPECIFIC                           0x0100
#define TT_STREAMING                                 0x0101
#define ITT_CAMERA                                   0x0201
#define OTT_DISPLAY                                  0x0301

/* Video Class-Specific Request Codes */
#define RC_UNDEFINED                                 0x00
#define SET_CUR                                      0x01
#define GET_CUR                                      0x81
#define GET_MIN                                      0x82
#define GET_MAX                                      0x83
#define GET_RES                                      0x84
#define GET_LEN                                      0x85
#define GET_INFO                                     0x86
#define GET_DEF                                      0x87

/* VideoControl Interface Control Selectors */
#define VC_CONTROL_UNDEFINED                         0x00
#define VC_VIDEO_POWER_MODE_CONTROL                  0x01
#define VC_REQUEST_ERROR_CODE_CONTROL                0x02

/* VideoStreaming Interface Control Selectors */
#define VS_CONTROL_UNDEFINED                         0x00
#define VS_PROBE_CONTROL                             0x01
#define VS_COMMIT_CONTROL                            0x02
#define VS_STILL_PROBE_CONTROL                       0x03
#define VS_STILL_COMMIT_CONTROL                      0x04
#define VS_STILL_IMAGE_TRIGGER_CONTROL               0x05
#define VS_STREAM_ERROR_CODE_CONTROL                 0x06
#define VS_GENERATE_KEY_FRAME_CONTROL                0x07
#define VS_UPDATE_FRAME_SEGMENT_CONTROL              0x08
#define VS_SYNC_DELAY_CONTROL                        0x09

/* USB Video Payload Headers */
#define UVC_PAYLOAD_HEADER_SIZE                      2
#define UVC_HEADER_EOH                               0x80  /* End of header */
#define UVC_HEADER_ERR                               0x40  /* Error */
#define UVC_HEADER_STI                               0x20  /* Still image */
#define UVC_HEADER_RES                               0x10  /* Reserved */
#define UVC_HEADER_SCR                               0x08  /* Source clock reference */
#define UVC_HEADER_PTS                               0x04  /* Presentation time stamp */
#define UVC_HEADER_EOF                               0x02  /* End of frame */
#define UVC_HEADER_FID                               0x01  /* Frame ID */

/* Camera resolution and format */
#define VIDEO_WIDTH                                  320 //400
#define VIDEO_HEIGHT                                 240 //400
#define VIDEO_FRAME_SIZE                             (VIDEO_WIDTH * VIDEO_HEIGHT * 2)
#define VIDEO_PACKET_SIZE                            512  /* Bulk transfer packet size */
#define VIDEO_TOTAL_BUF_SIZE                         (VIDEO_FRAME_SIZE + UVC_PAYLOAD_HEADER_SIZE)

/* Endpoint addresses */
#define VIDEO_IN_EP                                  0x81  /* Streaming endpoint */
#define VIDEO_CMD_EP                                 0x82  /* Interrupt endpoint for events */

/* Configuration Descriptor Size */
#define USB_VIDEO_CONFIG_DESC_SIZ                    (0x09 + 0x08 + 0x09 + 0x0D + 0x12 + 0x09 + 0x07 + 0x09 + 0x0E + 0x1B + 0x1E + 0x09 + 0x07)

/* Video Probe and Commit Controls */
typedef struct
{
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
  uint32_t dwClockFrequency;
  uint8_t  bmFramingInfo;
  uint8_t  bPreferedVersion;
  uint8_t  bMinVersion;
  uint8_t  bMaxVersion;
} __PACKED USBD_VideoControlTypeDef;

typedef struct
{
  uint32_t data_length;
  uint32_t bytes_sent;
  uint8_t  *data_buffer;
  uint8_t  header[UVC_PAYLOAD_HEADER_SIZE];
  uint8_t  frame_id;
  uint8_t  streaming_state;
} USBD_VIDEO_HandleTypeDef;

typedef struct
{
  int8_t (*Init)(void);
  int8_t (*DeInit)(void);
  int8_t (*Control)(uint8_t req, uint8_t *pbuf, uint16_t length);
  int8_t (*PrepareFrame)(uint8_t **pbuf, uint32_t *psize);
} USBD_VIDEO_ItfTypeDef;

/* External variables */
extern USBD_ClassTypeDef USBD_VIDEO;
#define USBD_VIDEO_CLASS &USBD_VIDEO

/* Function prototypes */
uint8_t USBD_VIDEO_RegisterInterface(USBD_HandleTypeDef *pdev, USBD_VIDEO_ItfTypeDef *fops);
uint8_t USBD_VIDEO_SendFrame(USBD_HandleTypeDef *pdev, uint8_t *pbuf, uint32_t size);

#ifdef __cplusplus
}
#endif

#endif /* __USBD_VIDEO_H */
