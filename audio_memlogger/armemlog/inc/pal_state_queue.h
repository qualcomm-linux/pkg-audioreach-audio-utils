#ifndef PAL_STATE_QUEUE_H
#define PAL_STATE_QUEUE_H
/**
* \file pal_state_queue.h
* \brief
*      Defines memory structures & logging states
*      for the PAL state memory queue.
*
*  Copyright (c) Qualcomm Innovation Center, Inc. All rights reserved.
*  SPDX-License-Identifier: BSD-3-Clause-Clear
*/

#define STATE_DEVICE_MAX_SIZE 3
#define CAP_PROF_MAX 55

#include <stdint.h>

typedef enum
{
    MEMLOG_ACD_STATE_NONE,
    MEMLOG_ACD_STATE_SSR,
    MEMLOG_ACD_STATE_IDLE,
    MEMLOG_ACD_STATE_LOADED,
    MEMLOG_ACD_STATE_ACTIVE,
    MEMLOG_ACD_STATE_DETECTED,
} pal_mlog_acd_state;

typedef enum
{
    MEMLOG_ENG_IDLE,
    MEMLOG_ENG_LOADED,
    MEMLOG_ENG_ACTIVE,
    MEMLOG_ENG_DETECTED,
} pal_mlog_acdeng_state;

typedef enum
{
    PAL_STATE_CLOSED,
    PAL_STATE_INIT,
    PAL_STATE_OPENED,
    PAL_STATE_STARTED,
    PAL_STATE_PAUSED,
    PAL_STATE_SUSPENDED,
    PAL_STATE_STOPPED,
} pal_state_queue_state;

struct pal_state_que_media_type
{
    uint32_t sample_rate;                /**< sample rate */
    uint16_t device;
    uint8_t bit_width;                  /**< bit width */
    uint8_t channels;
}; // total size 8 bytes

struct pal_mlog_acdstr_info
{
    pal_mlog_acd_state state_id;
    pal_mlog_acdeng_state eng_state_id;
    uint32_t event_id;
    uint32_t context_id;
    char CaptureProfile_name[CAP_PROF_MAX];
    uint8_t model_id;
}; /*size 72 bytes*/

union pal_mlog_str_info
{
    struct pal_mlog_acdstr_info acd_info;
};

struct pal_state_queue
{
    uint64_t timestamp;
    uint64_t stream_handle;     // array to store queue element
    struct pal_state_que_media_type device_attr[STATE_DEVICE_MAX_SIZE]; //nonexistent for Opened
    pal_state_queue_state state;
    int32_t error;
    uint16_t stream_type;
    int8_t direction;
    uint64_t session_handle;
    union pal_mlog_str_info str_info;
};

#endif