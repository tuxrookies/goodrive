#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([GooDrive], [0.1b], [pradeep.tux@gmail.com])
AM_INIT_AUTOMAKE([subdir-objects])
AC_CONFIG_SRCDIR([src/linux-api.c])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CC

# Checks for libraries.

# Check for Packages.
PKG_CHECK_MODULES([OPENSSL], [openssl])

AC_SUBST([OPENSSL_CFLAGS])
AC_SUBST([OPENSSL_LIBS])

# Checks for header files.
AC_CHECK_HEADERS([limits.h malloc.h stddef.h string.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_UID_T
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_REALLOC

AC_CONFIG_FILES([Makefile src/Makefile tests/Makefile])
AC_OUTPUT
