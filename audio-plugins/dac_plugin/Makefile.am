pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = dac_plugin.pc

EXTRA_DIST = $(pkgconfig_DATA)

AM_CFLAGS = -Wall \
        -Wundef \
        -Wstrict-prototypes \
        -Wno-trigraphs \
        -I .

AM_CFLAGS += -I $(top_srcdir)/inc/


h_sources = inc/dac_plugin.h

c_sources = src/dac_plugin.c

library_includedir = $(pkgincludedir)

lib_includedir = $(includedir)
lib_include_HEADERS = $(h_sources)

lib_LTLIBRARIES = libdac_plugin.la
libdac_plugin_la_CC = @CC@
libdac_plugin_la_SOURCES = $(c_sources)
libdac_plugin_la_CFLAGS = $(AM_CFLAGS) -DUSE_GLIB -D_LINUX_LOG_ @AUDIOPLUGINHEADERS_CFLAGS@
libdac_plugin_la_LDFLAGS = $(AUDCAL_LIBS) $(AUDIOALSA_LIBS) @GLIB_LIBS@ -shared -avoid-version
