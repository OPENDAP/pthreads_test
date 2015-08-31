
dnl -*- autoconf -*-
dnl Process this file with autoconf to produce a configure script.

AC_PREREQ(2.63)
dnl Update version here and below at LIB_CURRENT, ..., if needed.
AC_INIT(p_test, 0.0, opendap-tech@opendap.org)

AC_CONFIG_SRCDIR([p_test.cc])
AC_CONFIG_AUX_DIR(conf)
AC_CONFIG_MACRO_DIR([conf])
AC_CONFIG_HEADERS([config.h])

AM_INIT_AUTOMAKE

AC_CANONICAL_HOST
AC_SUBST(host)

AC_PROG_CXX
AC_PROG_CC

AS_IF([test x$CC = xgcc],
      [AM_CONDITIONAL([COMPILER_IS_GCC],[true])],
      [AM_CONDITIONAL([COMPILER_IS_GCC],[false])])

AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_LIBTOOL

AC_CHECK_HEADERS([pthread.h])

AC_CHECK_LIB([pthread], [pthread_kill], 
	[PTHREAD_LIBS="-lpthread"],
	[AC_MSG_ERROR([I could not find pthreads])])
AC_SUBST([PTHREAD_LIBS])

dnl autoheader macros; tack some text at the top and bottom of config_dap.h.in

AC_CONFIG_FILES([Makefile])

AC_OUTPUT
