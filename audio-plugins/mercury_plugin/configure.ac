#                                               -*- Autoconf -*-
# configure.ac -- Autoconf script for Mercury plugin

# Process this file with autoconf to produce a configure script.

# Requires autoconf tool later than 2.69
AC_PREREQ([2.69])
# Initialize the mercury plugin package version 1.0.0
AC_INIT([mercury_plugin], 1.0.0)
# Does not strictly follow GNU Coding standards
AM_INIT_AUTOMAKE([foreign])
# Disables auto rebuilding of configure, Makefile.ins
AM_MAINTAINER_MODE
# Verifies the --srcdir is correct by checking for the path
AC_CONFIG_MACRO_DIR([m4])

# Checks for programs.
AC_PROG_CC
AM_PROG_CC_C_O
AC_PROG_CXX
AC_PROG_LIBTOOL
AC_PROG_AWK
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
PKG_PROG_PKG_CONFIG

# Checks for header files.
AC_CHECK_HEADERS([stdint.h stdlib.h string.h])

AC_SUBST([CFLAGS])
AC_SUBST([CC])

PKG_CHECK_MODULES([AUDIOPLUGINHEADERS], [audio-plugin-headers])
AC_SUBST([AUDIOPLUGINHEADERS_CFLAGS])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
