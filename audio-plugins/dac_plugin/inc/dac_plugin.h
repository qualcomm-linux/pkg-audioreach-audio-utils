/*
 * Copyright (c) 2024 Qualcomm Innovation Center, Inc. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */
#include "codec_plugin.h"
#include <stdint.h>
#include <stdbool.h>
#ifndef _DAC_PLUGIN_H_
#define _DAC_PLUGIN_H_

#define LIB_DAC "/usr/lib/libaudio_dac.so"
#define I2C_DEVICE_ADDR  0x4C
#define I2C_DEVICE       11

typedef struct dac_config {
    uint16_t i2c_device; /*/dev/i2c* */
    uint16_t i2c_addr; /*i2c slave address*/
} dac_config_t;

/* Payload for device config.*/
typedef struct dac_param {
    cdc_device_type_t type;
    cdc_source_id source_id;
}dac_param_t;

void *lib_handle;
int32_t (*dac_init)(dac_config_t*);
int32_t (*dac_deinit)(void);
int32_t (*dac_expander_setup)(bool, int);
int32_t (*dac_setup)(void);

#endif //_DAC_PLUGIN_H_

