#ifndef GRAPH_QUEUE_H
#define GRAPH_QUEUE_H
/**
* \file graph_queue.h
* \brief
*      Defines memory structures & logging states
*      for the graph memory queue.
*
*  Copyright (c) Qualcomm Innovation Center, Inc. All rights reserved.
*  SPDX-License-Identifier: BSD-3-Clause-Clear
*/

#include <stdint.h>

typedef enum
{
    GRAPH_START,
    GRAPH_STOP,
    GRAPH_OPEN,
    GRAPH_CLOSE,
    GRAPH_PAUSE,
    GRAPH_RESUME,
    GRAPH_STATE_MAX
} graph_queue_state;

struct graph_queue
{
    uint64_t timestamp;
    graph_queue_state state;
    int result;
    void *stream_handle;
}; // total size 24 bytes

#endif