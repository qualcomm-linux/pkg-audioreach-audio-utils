AM_CFLAGS = -DLINUX_ENABLED
AM_CFLAGS = -Werror
AM_CFLAGS += -D__unused=__attribute__\(\(__unused__\)\) $(GLIB_CFLAGS)

AM_CFLAGS += -I ./ \
             -I ./inc

test_sources = dac_mer_testapp.c

bin_PROGRAMS = dac_mer_testapp
codec_plugin_testapp_CC = @CC@
codec_plugin_testapp_SOURCES = $(test_sources)
codec_plugin_testapp_CFLAGS = $(AM_CFLAGS)
