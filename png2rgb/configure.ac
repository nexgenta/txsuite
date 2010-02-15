AC_INIT([png2rgb],[master])
AM_INIT_AUTOMAKE([foreign])
AM_MAINTAINER_MODE

AC_PROG_CC
AC_PROG_LIBTOOL

AC_SYS_LARGEFILE

AC_ARG_WITH([libpng],[AS_HELP_STRING([--with-libpng=DIR],[specify libpng installation prefix])],[libpng_config="$withval/bin/libpng-config"],[libpng_config="libpng-config"])

PKG_CHECK_MODULES([libpng],[libpng >= 1.2.27],[
	AC_MSG_RESULT([located via pkg-config])
	LIBPNG_CPPFLAGS="$libpng_CFLAGS"
	LIBPNG_LIBS="$libpng_LIBS"
],[
	AC_MSG_CHECKING([for libpng])
	libpng_prefix=`$libpng_config --prefix`
	if test x"$libpng_prefix" = x"" ; then
		AC_MSG_ERROR([cannot find libpng: use --with-libpng=DIR to specify the libpng installation prefix])
	fi
	libpng_version=`$libpng_config --version`

	AC_MSG_RESULT([$libpng_version installed in $libpng_prefix])

	LIBPNG_CFLAGS="`$libpng_config --ccopts`"
	LIBPNG_CPPFLAGS="`$libpng_config --cppflags` `$libpng_config --I_opts`"
	LIBPNG_LDFLAGS="`$libpng_config --ldopts` `$libpng_config --L_opts` `$libpng_config --R_opts`"
	LIBPNG_LIBS="`$libpng_config --libs`"
])

AC_SUBST([libpng_prefix])
AC_SUBST([libpng_version])
AC_SUBST([LIBPNG_CFLAGS])
AC_SUBST([LIBPNG_CPPFLAGS])
AC_SUBST([LIBPNG_LDFLAGS])
AC_SUBST([LIBPNG_LIBS])

AC_CONFIG_HEADERS([config.h])

AC_CONFIG_FILES([
Makefile
])

AC_OUTPUT
