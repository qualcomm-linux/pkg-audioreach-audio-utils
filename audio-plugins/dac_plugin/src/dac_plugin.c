/*
 * Copyright (c) 2024 Qualcomm Innovation Center, Inc. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include "codec_status.h"
#include "dac_plugin.h"
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <dlfcn.h>
#include <inttypes.h>

int32_t init(uint32_t *handle)
{
    int32_t rc = CDC_EOK;
    dac_config_t *config;

    lib_handle = dlopen(LIB_DAC ,RTLD_NOW);
    if(!lib_handle) {
        printf("%s: %s\n", __func__, dlerror());
        rc = CDC_EABORTED;
        goto end;
    }

    dac_init = dlsym(lib_handle, "audio_dac_init");
    dac_deinit = dlsym(lib_handle, "audio_dac_deinit");
    dac_expander_setup = dlsym(lib_handle, "audio_dac_expander_setup");
    dac_setup = dlsym(lib_handle, "audio_dac_config_setup");

    config = (dac_config_t *)malloc(sizeof(dac_config_t));
    if (!config)
    {
        rc = CDC_ENOMEMORY;
        goto end;
    }

    config->i2c_device = I2C_DEVICE;
    config->i2c_addr = I2C_DEVICE_ADDR;
    rc = (*dac_init)(config);
    if (rc != CDC_EOK)
        printf("Failed to initilize DAC, err: %d\n", rc);

    free(config);

end:
    return rc;
}

int32_t deinit(uint32_t *handle)
{
    int32_t rc = CDC_EOK;
    int source = CDC_NON_SOC_AUDIO;

    rc = (*dac_expander_setup)(false, source);
    rc = (*dac_deinit)();
    dlclose(lib_handle);
    return rc;
}

int32_t cmd(uint32_t *handle, int cmd_id, void *param, size_t length)
{
    int32_t rc = CDC_EOK;
    dac_param_t *config_param = (dac_param_t *)param;
    int source = CDC_NON_SOC_AUDIO;

    if (!config_param && config_param->type != CDC_DEVICE_TYPE_RX)
    {
        rc = CDC_EBADPARAM;
        goto end;
    }

    switch(cmd_id)
    {
        case CDC_CMD_SET_CONFIG:
            source = config_param->source_id;
            rc = (*dac_expander_setup)(true, source);
            usleep(300*1000);
            rc = (*dac_setup)();
            break;
        default:
            printf("Invalid command\n");
            rc = CDC_EUNSUPPORTED;
            break;
    }

end:
    return rc;
}

