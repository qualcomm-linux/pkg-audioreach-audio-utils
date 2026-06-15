#ifndef KPI_QUEUE_H
#define KPI_QUEUE_H
/**
* \file kpi_queue.h
* \brief
*      Defines memory structure for the KPI memory queue.
*
*  Copyright (c) Qualcomm Innovation Center, Inc. All rights reserved.
*  SPDX-License-Identifier: BSD-3-Clause-Clear
*/

#define MEM_LOGGER_FUNC_NAME_MAX 61 // 61 to efficiently pack bytes into struct

#include <stdbool.h>
#include <stdint.h>

struct kpi_queue
{
    uint64_t timestamp;
    uint16_t pid;     // array to store queue element
    char func_name[MEM_LOGGER_FUNC_NAME_MAX];
    bool type;
}; // total size 72 bytes

#endif