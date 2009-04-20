dnl Transmission Suite

AC_INIT([TransmissionSuite],[master],,[txsuite])
AM_INIT_AUTOMAKE([foreign])
AM_MAINTAINER_MODE

AC_PROG_CC
AM_PROG_LEX
AC_PROG_YACC

examplesdir='${datarootdir}/examples/${PACKAGE_TARNAME}'
AC_SUBST([examplesdir])

if test -d $srcdir/dss ; then
	AC_CONFIG_SUBDIRS([dss])
	build_dss="enabled"
fi

build_mhegc="enabled"

AC_CONFIG_FILES([
Makefile
buildtools/Makefile
mhegc/Makefile
mhegc/helloworld/Makefile
])

AM_CONDITIONAL([BUILD_MHEGC],[test x"$build_mhegc" = x"enabled"])
AM_CONDITIONAL([BUILD_DSS],[test x"$build_dss" = x"enabled"])

AC_OUTPUT

AC_MSG_RESULT([

Configuration summary:

Darwin Streaming Server:     $build_dss
RedButton mhegc/mhegd:       $build_mhegc
])
