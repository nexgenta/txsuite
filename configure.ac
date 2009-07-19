dnl Transmission Suite

AC_INIT([TransmissionSuite],[master],,[txsuite])
AM_INIT_AUTOMAKE([foreign])
AM_MAINTAINER_MODE

AC_PROG_CC
AM_PROG_LEX
AC_PROG_YACC

examplesdir='${datarootdir}/examples/${PACKAGE_TARNAME}'
AC_SUBST([examplesdir])

dnl TX_SUBPROJ(subdir, var, opt, help)
AC_DEFUN([TX_SUBPROJ],[
$2=no
if test -d $srcdir/$1 && test -f $srcdir/$1/configure ; then
	AC_ARG_ENABLE([$3],[$4],[$2=$enableval],[$2=yes])
fi
if test x"$[$2]" = x"yes" ; then
	AC_CONFIG_SUBDIRS([$1])
fi
])

dnl TX_SUBDIR(subdir, var, opt, help)
AC_DEFUN([TX_SUBDIR],[
$2=no
if test -d $srcdir/$1 ; then
	AC_ARG_ENABLE([$3],[$4],[$2=$enableval],[$2=yes])
fi
])

TX_SUBPROJ(dss, build_dss, dss, AS_HELP_STRING([--disable-dss],[don't build Darwin Streaming Server]))
AM_CONDITIONAL(BUILD_DSS,[test x"$build_dss" = x"yes"])

TX_SUBPROJ(tsdemux, build_tsdemux, tsdemux, AS_HELP_STRING([--disable-tsdemux],[don't build the tsdemux utility]))
AM_CONDITIONAL(BUILD_TSDEMUX,[test x"$build_tsdemux" = x"yes"])

TX_SUBPROJ(png2rgb, build_png2rgb, png2rgb, AS_HELP_STRING([--disable-png2rgb],[don't build the png2rgb utility]))
AM_CONDITIONAL(BUILD_PNG2RGB,[test x"$build_png2rgb" = x"yes"])

TX_SUBPROJ(tstools, build_tstools, tstools, AS_HELP_STRING([--disable-tstools],[don't build the Transport Stream Tools]))
AM_CONDITIONAL(BUILD_TSTOOLS,[test x"$build_tstools" = x"yes"])

TX_SUBPROJ(mpeg4ip, build_mpeg4ip, mpeg4ip, AS_HELP_STRING([--disable-mpeg4ip],[don't build the MPEG4IP suite]))
AM_CONDITIONAL(BUILD_MPEG4IP,[test x"$build_mpeg4ip" = x"yes"])

TX_SUBDIR(mhegc, build_mhegc, redbutton, AS_HELP_STRING([--disable-redbutton],[don't build the RedButton MHEG compiler/decompiler]))
AM_CONDITIONAL(BUILD_MHEGC,[test x"$build_mhegc" = x"yes"])

TX_SUBPROJ(shroedinger, build_shroedinger, shroedinger, AS_HELP_STRING([--disable-shroedinger],[don't build the Shroedinger Dirac encoder]))
AM_CONDITIONAL(BUILD_SHROEDINGER,[test x"$build_shroedinger" = x"yes"])

AC_CONFIG_FILES([
Makefile
buildtools/Makefile
mhegc/Makefile
mhegc/helloworld/Makefile
])

AC_OUTPUT

AC_MSG_RESULT([

Configuration summary:

Darwin Streaming Server:     $build_dss
RedButton mhegc/mhegd:       $build_mhegc
tsdemux:                     $build_tsdemux
png2rgb:                     $build_png2rgb
Transport Stream Tools:      $build_tstools
MPEG4IP:                     $build_mpeg4ip
])
