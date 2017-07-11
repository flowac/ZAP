#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.68])
AC_INIT([torrent], [0.0.1])
AM_INIT_AUTOMAKE(torrent, 0.0.1)

# conditional statement to check for OS
AM_CONDITIONAL([CHECK_OS], [test $(OS) -eq "Windows_NT"])

# set default prefix to local dir (this is temporary)
AC_PREFIX_DEFAULT(.)

AC_CONFIG_SRCDIR([include/ssl_fn.h])
AC_CONFIG_HEADERS([config.h])

# flag for static libraries
AC_PROG_RANLIB

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC

# Checks for libraries.
# FIXME: Replace `main' with a function in `-lcrypto':
AC_CHECK_LIB([crypto], [SHA1_Final], echo "Error: Requires library crypto 1.0.0 or later" && exit 1)
# FIXME: Replace `main' with a function in `-lssl':
AC_CHECK_LIB([ssl], [SHA1_Final], echo "Error: Requires library ssl 1.0.0 or later && exit 1)
# FIXME: Replace `main' with a function in `-lpthread':
AC_CHECK_LIB([pthread], [pthread_create], echo "Error: Requires library pthread 2.15 or later" && exit 1)
# FIXME: Replace `main' with a function in `-lrt':
AC_CHECK_LIB([rt], [pthread_create], echo "Error: Requires library rt 2.15 or later" && exit 1)

# Checks for header files.
AC_CHECK_HEADERS([limits.h stddef.h stdint.h stdlib.h string.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_SIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_TYPE_UINT8_T
AC_CHECK_TYPES([ptrdiff_t])

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_CHECK_FUNCS([clock_gettime memmove memset mkdir strerror strstr])

AC_CONFIG_FILES([
Makefile
src/Makefile
extern/7z/Makefile
extern/Makefile
])
#AC_CONFIG_SUBDIRS([extern/transmission])
AC_OUTPUT
