dnl Transmission suite

dnl TX_ENABLE_SUBDIR(subdir, help, default=yes)
dnl - Optionally enable building a subproject
dnl
AC_DEFUN([TX_ENABLE_SUBDIR],[
def=]m4_if[($3,,yes,$3)
build_$1=no
AC_MSG_CHECKING([whether to build $1])
if test -d $srcdir/$1 ; then
	AC_ARG_ENABLE([$1],[$2],[build_$1=$enableval],[build_$1=$def])
	AC_MSG_RESULT([$build_$1])
else
	AC_MSG_RESULT([no (subproject not present)])
fi
])

dnl TX_WITH_INCLUDED_SUBDIR(subdir, help, default=auto)
dnl - Optionally enable building a dependent subproject
dnl
AC_DEFUN([TX_WITH_INCLUDED_SUBDIR],[
def=]m4_if[($3,,auto,$3)
build_$1=no
AC_MSG_CHECKING([whether to build included $1])
if test -d $srcdir/$1 ; then
	AC_ARG_WITH(with_[$1],[$2],[build_$1=$enableval],[build_$1=$def])
	if test x"$build_$1" = x"auto" ; then
		if test x"$need_$1" = x"yes" ; then
			build_$1=yes
		elif test x"$have_$1" = x"yes" ; then
			build_$1=no
		else
			build_$1=no
		fi
		AC_MSG_RESULT([$build_$1])
	fi
else
	build_$1=no
	AC_MSG_RESULT([no (subproject not present)])
fi
])

dnl TX_BUILD_SUBPROJ(subdir)
dnl - Build a subproject
dnl
AC_DEFUN([TX_BUILD_SUBPROJ],[
if test x"$build_[$1]" = x"yes" ; then
	AC_CONFIG_SUBDIRS([$1])
	PKG_CONFIG_PATH="$PKG_CONFIG_PATH:$top_builddir/$1"
fi
])

dnl TX_BUILD_SUBDIR(subdir)
dnl - Build a subproject which doesn't use GNU configure
dnl
AC_DEFUN([TX_BUILD_SUBDIR],[
if test x"$build_[$1]" = x"yes" ; then
	subdirs="$subdirs [$1]"
	PKG_CONFIG_PATH="$PKG_CONFIG_PATH:$top_builddir/$1"
fi
])
