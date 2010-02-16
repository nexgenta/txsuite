dnl Transmission suite

m4_define([_TX_SHELL],[m4_translit([$1],[-.],[__])])

dnl TX_ENABLE_SUBDIR(subdir, help, default=yes)
dnl - Optionally enable building a subproject
dnl
AC_DEFUN([TX_ENABLE_SUBDIR],[
def=]m4_if[($3,,yes,$3)
_TX_SHELL(build_$1)=no
AC_MSG_CHECKING([whether to build $1])
if test -d $srcdir/$1 ; then
	AC_ARG_ENABLE([$1],[$2],
		_TX_SHELL([build_$1])=$enableval,
		_TX_SHELL([build_$1])=$def)
	AC_MSG_RESULT($_TX_SHELL([build_$1]))
else
	AC_MSG_RESULT([no (subproject not present)])
fi
])

dnl TX_CHECK_SUBPROJ_ENABLED(project)
dnl - Check that $need_$1 and $build_$1 don't disagree
AC_DEFUN([TX_CHECK_SUBPROJ_ENABLED],[
	if test x"$_TX_SHELL([need_$1])" = x"yes" ; then
		if test x"$_TX_SHELL([build_$1])" = x"yes" ; then
			true
		elif test x"$_TX_SHELL([have_$1])" = x"yes" ; then
			true
		else
			AC_MSG_ERROR([$1 is required by other sub-projects but is not present and cannot be found on your system])
		fi
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
	AC_ARG_WITH(included-[$1],[$2],[build_$1=$withval],[build_$1=$def])
	if test x"$build_$1" = x"yes" ; then
		true
	else
		if test x"$need_$1" = x"yes" ; then
			build_$1=yes
		elif test x"$have_$1" = x"yes" ; then
			build_$1=no
		else
			build_$1=no
		fi
	fi
    AC_MSG_RESULT([$build_$1])	
else
	build_$1=no
	if test x"$build_$1" = x"no" && test x"$have_$1" = x"no" && test x"$need_$1" = x"yes" ; then
			AC_MSG_ERROR([$1 is required by other sub-projects but is not present and cannot be found on your system])
	fi
	AC_MSG_RESULT([no (subproject not present)])
fi
])

dnl TX_BUILD_SUBPROJ(subdir)
dnl - Build a subproject
dnl
AC_DEFUN([TX_BUILD_SUBPROJ],[
if test x"$_TX_SHELL([build_$1])" = x"yes" ; then
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
