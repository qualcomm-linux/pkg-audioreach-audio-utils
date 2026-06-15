pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = audioroute.pc
EXTRA_DIST = $(pkgconfig_DATA)

AM_CFLAGS = -Wundef \
        -Wstrict-prototypes \
        -Wno-trigraphs \
        -g -O0 \
        -fno-inline \
        -fno-short-enums \
        -fpic \
        -fstack-protector-all

AM_CFLAGS += -I $(top_srcdir)/include

h_sources = ${top_srcdir}/include/audio_route/audio_route.h

system_sources=${top_srcdir}/include/system/audio.h

c_sources = ${top_srcdir}/audio_route.c

library_includedir = $(includedir)/audio_route/audio_route/
library_include_HEADERS = $(h_sources)

system_includedir = $(includedir)/audio_route/system
system_include_HEADERS = $(system_sources)


lib_LTLIBRARIES = libaudioroute.la
libaudioroute_la_SOURCES := $(c_sources) $(system_source)
libaudioroute_la_CFLAGS := $(AM_CFLAGS)
libaudioroute_la_CFLAGS += -D__unused=__attribute__\(\(__unused__\)\)
libaudioroute_la_CFLAGS += -DNDEBUG
libaudioroute_la_LIBADD := -ltinyalsa
libaudioroute_la_LDFLAGS := -shared -avoid-version
