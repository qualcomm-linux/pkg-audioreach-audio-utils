/*
 * Copyright (c) 2024 Qualcomm Innovation Center, Inc. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */
#include <stdio.h>
#include <dlfcn.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <stdbool.h>
#include "dac_mer_testapp.h"
#define MERCURY_PLUGIN "/usr/lib/libmercury_plugin.so"
#define DAC_PLUGIN "/usr/lib/libdac_plugin.so"

state_t mer_state = CDC_STATE_NONE;
state_t dac_state = CDC_STATE_NONE;

void *lib_mer_handle;
void *lib_dac_handle;
int (*mer_plugin_init)(int*);
int (*mer_plugin_deinit)(int*);
int (*mer_plugin_cmd)(int*, int, void*, int);
int (*mer_plugin_pchime)(int*);
int (*dac_plugin_init)(int*);
int (*dac_plugin_deinit)(int*);
int (*dac_plugin_cmd)(int*, int, void*, int);

int setup_dac(int *dac_plugin_handle, int source_id)
{
    cdc_config_t cfg;
    int rc = 0;
    void *data = NULL;

    if (dac_state != CDC_STATE_INIT && dac_state != CDC_STATE_START) {
        rc = (*dac_plugin_init)(dac_plugin_handle);
        if (rc) {
            printf("DAC init failed: %d\n", rc);
            return -1;
        }
        dac_state = CDC_STATE_INIT;
    }

    if (dac_state == CDC_STATE_INIT || dac_state == CDC_STATE_START) {
        printf("Setting up DAC\n");
        cfg.type = CDC_DEVICE_TYPE_RX;
        cfg.source_id = source_id;

        data = &cfg;
        rc = (*dac_plugin_cmd)(dac_plugin_handle, CMD_SET_CONFIG,
                data, sizeof(cdc_config_t));
        if (rc) {
            printf("Speaker setup failed: %d\n", rc);
            (*dac_plugin_deinit)(dac_plugin_handle);
            dac_state = CDC_STATE_STOP;
            return -1;
        }

        dac_state = CDC_STATE_START;
    }
    return 0;
}

int setup_mercury(int *mercury_plugin_handle)
{
    cdc_config_t cfg;
    int rc = 0;
    void *data = NULL;

    if (mer_state == CDC_STATE_START)
        return 0;

    if (mer_state != CDC_STATE_INIT) {
        rc = (*mer_plugin_init)(mercury_plugin_handle);
        if (rc) {
            printf("Mercury init failed: %d\n", rc);
            return -1;
        }
        mer_state = CDC_STATE_INIT;
    }

    cfg.type = CDC_DEVICE_TYPE_TX;
    data = &cfg;
    rc = (*mer_plugin_cmd)(mercury_plugin_handle, CMD_SET_CONFIG,
            data, sizeof(cdc_config_t));
    if (rc) {
        printf("Mic setup failed: %d\n", rc);
        (*mer_plugin_deinit)(mercury_plugin_handle);
        mer_state = CDC_STATE_STOP;
        return -1;
    }
    mer_state = CDC_STATE_START;
    return 0;
}

int main(int argc, char *argv[])
{
    char buf[16], *p;
    int rc = 0;
    void *data = NULL;
    int mercury_plugin_handle;
    int dac_plugin_handle;
    int input = CDC_PLUGIN_TEST_NONE;
    int enable_playback = 0;

    lib_mer_handle = NULL;
    lib_dac_handle = NULL;
    printf("*************Testapp to configure external codecs*************\n");

    lib_mer_handle = dlopen(MERCURY_PLUGIN ,RTLD_NOW);
    if (!lib_mer_handle) {
        printf("%s\n", dlerror());
        return 0;
    }
    mer_plugin_init = dlsym(lib_mer_handle,"init");
    mer_plugin_cmd = dlsym(lib_mer_handle, "cmd");
    mer_plugin_deinit = dlsym(lib_mer_handle, "deinit");
    mer_plugin_pchime =  dlsym(lib_mer_handle, "pchime");

    lib_dac_handle = dlopen(DAC_PLUGIN ,RTLD_NOW);
    if (!lib_dac_handle) {
        printf("%s\n", dlerror());
        return 0;
    }
    dac_plugin_init = dlsym(lib_dac_handle,"init");
    dac_plugin_cmd = dlsym(lib_dac_handle, "cmd");
    dac_plugin_deinit = dlsym(lib_dac_handle, "deinit");

    while(input != CDC_PLUGIN_TEST_STOP_APP) {

        printf("\n\n\nPlease input the number corresponding to the operation you’d like to"
                " perform.\nPlease note that SOC to DAC path support is currently unavailable.\n");
        printf("1: Flash mercury\t\t2: SOC mercury\n3: SOC DAC\t\t\t4: Mercury Pchime\n5: Stop "
                "SOC mercury\t\t6: Stop SOC DAC\n7: Stop test app\n");

        fflush (stdout);
        p = fgets (buf, 16, stdin);
        if (p == NULL) {
            printf("Error: Invalid input !!");
            continue;
        }
        input = atoi(p);

        switch(input) {
            case CDC_PLUGIN_TEST_FLASH_MERCURY:
                if (mer_state != CDC_STATE_INIT && mer_state != CDC_STATE_START) {
                    rc = (*mer_plugin_init)(&mercury_plugin_handle);
                    if (rc) {
                        printf("Mercury init failed\n");
                        continue;
                    }
                    mer_state = CDC_STATE_INIT;
                }
                break;

            case CDC_PLUGIN_TEST_MERCURY_PCHIME:
                // DAC should be up for Pchime to work.
                if ((dac_state != CDC_STATE_START))
                    setup_dac(&dac_plugin_handle, CDC_NON_SOC_AUDIO);

                if ((mer_state != CDC_STATE_INIT))
                    setup_mercury(&mercury_plugin_handle);

                rc = (*mer_plugin_pchime)(&mercury_plugin_handle);
                if (rc)
                    printf("Mercury pchime failed\n");
                break;

            case CDC_PLUGIN_TEST_SOC_MERCURY_SETUP:
                setup_mercury(&mercury_plugin_handle);

                printf("Press 1 to enable playback setup, or 0 to skip.\n");
                fflush (stdout);
                p = fgets (buf, 16, stdin);
                if (p == NULL) {
                    printf("Error: Invalid input !!");
                    continue;
                }
                enable_playback = atoi(p);

                if (enable_playback) {
                    rc = setup_dac(&dac_plugin_handle, CDC_NON_SOC_AUDIO);
                    if (rc != 0)
                        printf("Failed to setup playback path\n");
                }
                break;

            case CDC_PLUGIN_TEST_SOC_DAC_SETUP:
                //TODO: Integrate SOC to DAC path setup logic.
                /*
                rc = setup_dac(&dac_plugin_handle, CDC_SOC_AUDIO);
                if (rc != 0)
                    printf("Failed to setup playback path\n");
                */
                printf("This option is currently unsupported.\n");
                break;

            case CDC_PLUGIN_TEST_SOC_STOP_DAC_SETUP:
                //TODO: Integrate SOC to DAC path tear down logic.
                /*
                if ((dac_state == CDC_STATE_START) || (dac_state == CDC_STATE_INIT)) {
                    rc = (*dac_plugin_deinit)(&dac_plugin_handle);
                    dac_state = CDC_STATE_STOP;
                }
                */
                printf("This option is currently unsupported.\n");
                break;

            case CDC_PLUGIN_TEST_SOC_STOP_MERCURY_SETUP:
                if ((mer_state == CDC_STATE_START) || (mer_state == CDC_STATE_INIT)) {
                    rc = (*mer_plugin_deinit)(&mercury_plugin_handle);
                    mer_state = CDC_STATE_STOP;
                }

                if (dac_state == CDC_STATE_INIT || dac_state == CDC_STATE_START) {
                    rc = (*dac_plugin_deinit)(&dac_plugin_handle);
                    dac_state = CDC_STATE_STOP;
                }
                break;

            case CDC_PLUGIN_TEST_STOP_APP:
                if (mer_state == CDC_STATE_INIT || mer_state == CDC_STATE_START) {
                    rc = (*mer_plugin_deinit)(&mercury_plugin_handle);
                    mer_state = CDC_STATE_STOP;
                }

                if (dac_state == CDC_STATE_INIT || dac_state == CDC_STATE_START) {
                    rc = (*dac_plugin_deinit)(&dac_plugin_handle);
                    dac_state = CDC_STATE_STOP;
                }
                dlclose(lib_mer_handle);
                dlclose(lib_dac_handle);
                break;

            default:
                printf("Invalid input\n");
                break;
        }
    }
    printf("Exiting the test app\n");
    return 0;
}

