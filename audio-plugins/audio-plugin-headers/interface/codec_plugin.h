/*
 * Copyright (c) 2024 Qualcomm Innovation Center, Inc. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */
#include <stdint.h>
#include <stddef.h>
#ifndef _CDC_PLUGIN_H_
#define _CDC_PLUGIN_H_

typedef enum {
    CDC_DEVICE_TYPE_NONE = 0,
    CDC_DEVICE_TYPE_RX = 1,
    CDC_DEVICE_TYPE_TX = 2,
} cdc_device_type_t;

typedef enum {
    CDC_CMD_NONE = 0,
    CDC_CMD_SET_CONFIG,
    CDC_CMD_GET_CONFIG,
    CDC_CMD_MAX
}cdc_cmd_id_t;

typedef enum {
    CDC_NON_SOC_AUDIO = 0,
    CDC_SOC_AUDIO = 1,
}cdc_source_id;
/**
 * init codec driver module
 *
 * @return  0 for success, otherwise error code
 */
int32_t init(uint32_t *handle);

/**
 * deinit codec driver module
 *
 * @return  0 for success, otherwise error code
 */
int32_t deinit(uint32_t *handle);

/**
 * send custom commands
 *
 * @return  0 for success, otherwise error code
 */
int32_t cmd(uint32_t *handle, int cmd_id, void *param, size_t length);

#endif //_CDC_PLUGIN_H_
