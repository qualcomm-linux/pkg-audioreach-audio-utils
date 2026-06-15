
AM_CFLAGS = -Wall \
        -Wundef \
        -Wstrict-prototypes \
        -Wno-trigraphs \
        -I .

AM_CFLAGS += -I $(top_srcdir)/inc/

h_sources = inc/mercury_plugin.h

c_sources = src/mercury_plugin.c

lib_includedir = $(includedir)
lib_include_HEADERS = $(h_sources)

library_includedir = $(pkgincludedir)
lib_LTLIBRARIES = libmercury_plugin.la
libmercury_plugin_la_CC = @CC@
libmercury_plugin_la_SOURCES = $(c_sources)
libmercury_plugin_la_CFLAGS = $(AM_CFLAGS)  -D_LINUX_LOG_ @AUDIOPLUGINHEADERS_CFLAGS@
libmercury_plugin_la_LDFLAGS = $(AUDCAL_LIBS) $(AUDIOALSA_LIBS) -shared -avoid-version
