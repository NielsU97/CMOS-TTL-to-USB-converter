/* usbd_conf.h - USB Device Configuration */

#ifndef __USBD_CONF_H
#define __USBD_CONF_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "stm32h7xx.h"
#include "stm32h7xx_hal.h"

/* Common Config */
#define USBD_MAX_NUM_INTERFACES         2
#define USBD_MAX_NUM_CONFIGURATION      1
#define USBD_MAX_STR_DESC_SIZ           512
#define USBD_SUPPORT_USER_STRING        0
#define USBD_DEBUG_LEVEL                0
#define USBD_LPM_ENABLED                0
#define USBD_SELF_POWERED               1

/* Video Class Config */
#define USBD_VIDEO_FREQ                 48000

/* Memory management macros */
#define USBD_malloc         malloc
#define USBD_free           free
#define USBD_memset         memset
#define USBD_memcpy         memcpy
#define USBD_Delay          HAL_Delay

/* DEBUG macros */
#if (USBD_DEBUG_LEVEL > 0)
#define USBD_UsrLog(...)    printf(__VA_ARGS__);\
                            printf("\n");
#else
#define USBD_UsrLog(...)
#endif

#if (USBD_DEBUG_LEVEL > 1)
#define USBD_ErrLog(...)    printf("ERROR: ");\
                            printf(__VA_ARGS__);\
                            printf("\n");
#else
#define USBD_ErrLog(...)
#endif

#if (USBD_DEBUG_LEVEL > 2)
#define USBD_DbgLog(...)    printf("DEBUG : ");\
                            printf(__VA_ARGS__);\
                            printf("\n");
#else
#define USBD_DbgLog(...)
#endif

#ifdef __cplusplus
}
#endif

#endif /* __USBD_CONF_H */