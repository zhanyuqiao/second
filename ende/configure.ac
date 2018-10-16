dnl Process this file with autoconf to produce a configure script

AC_PREREQ([2.69])
AC_INIT(RSA, 1.0)

AC_CONFIG_SRCDIR([src/json_interface.h])
AC_CONFIG_HEADERS([config.h])

AC_CANONICAL_SYSTEM
AM_INIT_AUTOMAKE()

AC_PROG_CC
dnl Initialize Libtool
LT_INIT

# Checks for libraries.
# FIXME: Replace `main' with a function in `-lcrypto':
AC_CHECK_LIB([crypto], [main])
# FIXME: Replace `main' with a function in `-lm':
AC_CHECK_LIB([m], [main])

# Checks for header files.
AC_CHECK_HEADERS([stdlib.h string.h syslog.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.
AC_FUNC_MALLOC
AC_CHECK_FUNCS([gethostname memset strstr])

AC_CONFIG_FILES([Makefile
                 src/Makefile])
AC_OUTPUT
