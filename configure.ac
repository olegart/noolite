#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.63])
AC_INIT(noolite, 0.6.0, oleg@olegart.ru)
AM_INIT_AUTOMAKE(noolite, 0.6.0)
AC_OUTPUT(Makefile src/Makefile doc/Makefile)
AC_CONFIG_SRCDIR([src/nooliterxcfg.c])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CC

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([fcntl.h stdlib.h string.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_CHECK_FUNCS([memmove strstr])

AC_OUTPUT
