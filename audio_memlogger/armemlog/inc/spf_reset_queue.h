#ifndef SPF_RESET_QUEUE_H
#define SPF_RESET_QUEUE_H
/**
* \file spf_reset_queue.h
* \brief
*      Defines memory structures & logging states
*      for the SPF reset memory queue.
*
*  Copyright (c) Qualcomm Innovation Center, Inc. All rights reserved.
*  SPDX-License-Identifier: BSD-3-Clause-Clear
*/

#include <stdint.h>

typedef enum
{
    SPF_RESET_DOWN,
    SPF_RESET_UP,
    SPF_RESET_STATE_MAX
} spf_reset_state;

struct ssr_pdr_queue
{
    uint64_t timestamp;
    spf_reset_state state;
};

#endif