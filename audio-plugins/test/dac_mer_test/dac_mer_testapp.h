/*
 * Copyright (c) 2024 Qualcomm Innovation Center, Inc. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef _DAC_MER_TEST_H_
#define _DAC_MER_TEST_H_

typedef enum {
    CDC_DEVICE_TYPE_NONE = 0,
    CDC_DEVICE_TYPE_RX = 1,
    CDC_DEVICE_TYPE_TX = 2,
} device_type_t;

typedef enum {
    CMD_NONE = 0,
    CMD_SET_CONFIG,
    CMD_GET_CONFIG,
    CMD_MAX
}cmd_id_t;

typedef enum {
    CDC_PLUGIN_TEST_NONE = 0,
    CDC_PLUGIN_TEST_FLASH_MERCURY = 1,
    CDC_PLUGIN_TEST_SOC_MERCURY_SETUP = 2,
    CDC_PLUGIN_TEST_SOC_DAC_SETUP = 3,
    CDC_PLUGIN_TEST_MERCURY_PCHIME = 4,
    CDC_PLUGIN_TEST_SOC_STOP_MERCURY_SETUP = 5,
    CDC_PLUGIN_TEST_SOC_STOP_DAC_SETUP = 6,
    CDC_PLUGIN_TEST_STOP_APP = 7,
    CDC_PLUGIN_TEST_MAX = 8,
}input_id_t;

typedef enum {
    CDC_STATE_NONE = 0,
    CDC_STATE_INIT = 1,
    CDC_STATE_START = 2,
    CDC_STATE_STOP = 3,
}state_t;

typedef enum {
    CDC_NON_SOC_AUDIO = 0,
    CDC_SOC_AUDIO = 1,
}source_id;

typedef enum {
    MERCURY_AUDIO,
    SOC_AUDIO,
}source_t;
/* Payload for device config.*/
typedef struct cdc_config {
    device_type_t type;
    source_t source_id;
}cdc_config_t;

#endif //_DAC_MER_TEST_H_
