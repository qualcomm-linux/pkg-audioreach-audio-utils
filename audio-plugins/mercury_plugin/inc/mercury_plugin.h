/*
 * Copyright (c) 2024 Qualcomm Innovation Center, Inc. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */
#include "codec_plugin.h"
#include <stdint.h>
#ifndef _MER_PLUGIN_H_
#define _MER_PLUGIN_H_

#define LIB_MER "/usr/lib/libaudio_mercury.so"

/* Payload for device config.*/
typedef struct mer_param {
    cdc_device_type_t type;
}mer_param_t;

void *lib_handle;
int32_t (*mer_init)(void);
int32_t (*mer_deinit)(void);
int32_t (*mer_audio_pchime)(void);
int32_t (*mer_enable_tdm_path)(bool);

#endif //_MER_PLUGIN_H_
