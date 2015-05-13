#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.68])
AC_INIT([ccometd], [0.9], [bugs@ccomed.com])
AM_INIT_AUTOMAKE([-Wall -Werror -Wno-extra-portability foreign])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_SRCDIR([src/comet_buffer.c])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CC
LT_INIT

# Checks for libraries.
PKG_CHECK_MODULES([libcurl],[libcurl])
PKG_CHECK_MODULES(libjansson,jansson)

# Checks for header files.
AC_CHECK_HEADERS([stddef.h stdlib.h string.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_TYPE_SIZE_T

# Checks for library functions.
AC_CHECK_FUNCS([memset strstr])

AC_CONFIG_FILES([Makefile
                 src/Makefile])
AC_OUTPUT
