/*
 * Copyright (c) 2024 Qualcomm Innovation Center, Inc. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include "codec_status.h"
#include "mercury_plugin.h"
#include <stdio.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <stdint.h>
#include <inttypes.h>

int32_t init(uint32_t *handle)
{
    int rc;

    lib_handle = dlopen(LIB_MER ,RTLD_NOW);
    if(!lib_handle) {
        printf("%s: %s\n", __func__, dlerror());
        rc = CDC_EABORTED;
        goto end;
    }

    mer_init = dlsym(lib_handle, "audio_mer_init");
    mer_deinit = dlsym(lib_handle, "audio_mer_deinit");
    mer_audio_pchime = dlsym(lib_handle, "audio_mer_pchime");
    mer_enable_tdm_path = dlsym(lib_handle, "audio_mer_enable_tdm_path");

    rc = (*mer_init)();

    if(rc != CDC_EOK) {
        printf("%s: audio mercury init failed\n", __func__);
        goto end;
    }

end:
    return rc;
}

int32_t deinit(uint32_t *handle)
{
    int32_t rc;

    rc = (*mer_enable_tdm_path)(false);
    rc = (*mer_deinit)();
    dlclose(lib_handle);

    if(rc != CDC_EOK)
        printf("%s: audio mercury deinit failed\n", __func__);

    return rc;
}

int32_t cmd(uint32_t *handle, int cmd_id, void *param, size_t length)
{
    int32_t rc = CDC_EOK;
    mer_param_t *config_param = (mer_param_t *)param;

    if (!config_param && config_param->type != CDC_DEVICE_TYPE_TX)
    {
        rc = CDC_EBADPARAM;
        goto end;
    }

    switch(cmd_id)
    {
        case CDC_CMD_SET_CONFIG:
            rc = (*mer_enable_tdm_path)(true);
            break;
        default:
            printf("Invalid command\n");
            rc = CDC_EUNSUPPORTED;
            break;
    }

end:
    return rc;

}

int32_t pchime(uint32_t *handle) {
    int32_t rc;

    rc = (*mer_audio_pchime)();

    return rc;
}

